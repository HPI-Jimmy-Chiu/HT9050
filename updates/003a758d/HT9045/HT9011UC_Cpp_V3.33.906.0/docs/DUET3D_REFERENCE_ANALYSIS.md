# Duet3D 借鑑分析 —— 別人怎麼用瀏覧器控制機台，我們哪裡可以截長補短

AI(W906-DUET-REF) 20260920。使用者 20260920 提問：「Duet3D 透過網頁控制 CNC／機台，
此專案能不能也這樣做？他們有開源碼嗎？抓下來後續解析。」同日下班前追加：
「核心目標是借鑑別人開發經驗來強化自身專案，徹底分析並製作有價值的文件，未來可以借鏡；
可以 loop 執行、自行分派 model／effort；能理解就做到底。」

這份是**長期文件**，不是一次性報告：之後每一輪對 Duet（或其他開源機台控制軟體）的解析都
**追加**到這裡，§12 變更紀錄逐筆記。檔名刻意不帶時間戳，理由與 `START_CAMPAIGN_PLAN.md`
等計畫書相同——它會一直被改。

---

## 0. 範圍、來源、怎麼讀

### 0.1 來源副本

六個 repo 的 shallow clone（只有 HEAD 一顆 commit）在
`D:\HT9045\backup\duet3d_20260920\`，**被 `.gitignore` 排除、不進 ht9045.git**。
清單、HEAD、行數、解析入口見該目錄 `README_PROVENANCE.md`。
代理的逐檔筆記（含每一筆 `CITE 檔:行 | 原文`）在同目錄 `notes\`：

| 筆記 | 內容 | 引用驗證 |
|---|---|---|
| `A_connectors.md` | 瀏覧器↔韌體通訊層（TypeScript） | 549／549 PASS |
| `B_rrf_objectmodel_http.md` | 韌體 Object Model 宣告、seqs、HTTP 入口 | 526／526 PASS |
| `C_dsf_webserver.md` | SBC 中介層的 REST／WebSocket／patch／code 管線 | 542／542 PASS |
| `D_dwc_frontend_and_objectmodel_ts.md` | 前端 SPA 架構、型別層、外掛、多語 | 345／345 PASS |
| `E_rrf_command_layer_and_safety.md` | 多通道排程、資源鎖、急停、事件→巨集、M291 | 420／420 PASS |
| `F_peer_projects_comparison.md` | Klipper／Moonraker、LinuxCNC、grblHAL／FluidNC、OctoPrint 橫向比較 | 207 個 URL，自標 A/B/C 三級可信度；主迴圈抽驗 2 條 |
| `G_dwc_cnc_panels.md` | **20260922 第二輪**：DWC 的四個 CNC（工具機）面板、它們組合到的子面板、`src/types` | 286／286 PASS |
| `H_dsf_sourcegen.md` | **20260922 第二輪**：DSF 的 Roslyn 產生器 14 檔 —— patch 套用的第二份獨立實作 | 161／161 PASS |
| `I_objectmodel_remainder.md` | **20260922 第二輪**：`ObjectModel` 前一輪未讀的部分（唯一 LGPL-2.1、唯一有重用討論空間） | 400／400 PASS |
| `J_rrf_hal_storage.md` | **20260922 第二輪**：RRF 的 `Hardware`／`Storage`／`Tools`／`GPIO` —— 餵 P18 的 HAL 題 | 593／593 PASS |
| `K_dsf_remainder.md` | **20260922 第二輪**：`DuetHttpClient`、六個示範程式、**＋「沒有對應物」清冊** | 312／312 PASS |
| `V_v906_webbridge_inventory.md` | **對照組**：我們自己的 WebBridge 今天長什麼樣 | 主迴圈自量 |

**第二輪合計**：270 個原始碼檔、20,829 行、**1,752 筆引用全數 PASS／0 FAIL**。
兩輪相加 **4,134 筆**。第二輪的標的是第一輪**量出來的缺口**，不是重讀。

驗證方法：`tools/cite_check_notes.py` 逐筆解析 `CITE 路徑:行號 | 片段`，
讀該行、去空白比對子字串；FAIL 逐條修正或刪除。記憶
`agent-justifications-fail-more-than-code` 說過代理的引用比程式碼更常錯，
所以**本文件只收錄通過驗證的引用**。五份筆記合計 **2,382 筆，全數通過**。

⚠ 驗證器自己也有過兩次盲點，而盲點的表現形式是**通過**不是報錯，值得記住：
(1) `README.md` 這種多 repo 同名檔會解析到錯的那個，把正確引用誤判成造假
（已加 per-notes-file 的偏好 repo 表）；(2) E 份有 73 筆用 `→` 而非 `|` 的行內引用
**被靜默跳過**，而那份正是最可行動的一份（已讓樣式同時認三種分隔符）。

行號都是 20260920 HEAD 的；日後 `git fetch --unshallow` 或更新副本後行號會漂，
引用時以 `notes\` 裡的原文片段重新定位。

### 0.2 授權邊界（先講，免得誤用）

| 目錄 | 授權 | 能做什麼 |
|---|---|---|
| RepRapFirmware、DuetWebControl、DuetSoftwareFramework、DWC-CNC | **GPL-3.0** | 只讀架構、學做法。**一行都不能抄進閉源的 HT9045.exe 或 wb_serve** |
| Connectors、ObjectModel（TypeScript 小套件） | **LGPL-2.1** | 在「獨立模組、動態載入、可替換」前提下有重用討論空間，**要使用者裁決** |

### 0.3 怎麼讀這份文件

- **只有五分鐘：§9.0「如果只做三件事」。** 那一節是整份文件的行動輸出。
- 只想知道「能不能、值不值」：§1 ＋ §9.0 ＋ §9.5。
- 要改 WebBridge／wb_serve：§3、§4、§7 ＋ §9.1 的提案表。
- 要改網頁端（Steven 的 HMI）：§3、§6 ＋ §9.1 提案 **P22／P23／P25／P24**（＋資料面收斂看 P3）。
- 要規劃「網頁 START／緊急停止／警報處置」：§7 ＋ §9.1 提案 **P12（統一管線）／P11＋P19（急停）／P30（警報處置變資料）／P27（modal）**。
  ⚠ 20260922 更正：以上兩行原本寫「P3／P4」與「P6／P7」，那是提案表只有 8 條時寫的；
  P4 是重啟偵測、P6 是背壓、P7 是 PONG 逾時，都不是這兩個題目。

---

## 1. 第一輪結論（20260920 上午，30 分鐘的表層調查）

**Duet3D 是什麼。** 英國公司，賣 3D 印表機／CNC 控制板（Duet 2、Duet 3）。板上跑開源韌體
RepRapFirmware（C++，ARM Cortex），操作介面 Duet Web Control（DWC）是純瀏覧器頁面。
論壇 forum.duet3d.com 有專門的「CNC & Laser Cutters」分區；CNC 專用儀表板 2021 年併進 DWC 3.4
（`DWC-CNC/README.md` 首段）。

**HT9045 能不能這樣做。** 可行，而且 V906 樹自 20260805 起走的就是同一個結構
（`docs/web-fw-legacy/docs/ARCHITECTURE.md`）。差別不在能不能，在 Duet 有幾件事做得比我們成熟。

**第一輪的對照表**（後面各節會逐項深化或修正）。
⚠ 下表有兩格在深讀後被更正，已就地標示——**這一節保留的是當時的判斷，不是最終事實**。

| 層 | Duet | V906 現況（20260920） |
|---|---|---|
| 狀態出去 | Object Model：每個類別一張 `constexpr` 表宣告要曝露的欄位，`live` 旗標分高低頻，`seqs` 計數器讓瀏覧器只抓有變的區段 | `TagSnapshot` 扁平 tag 表（約 5,349 個 tag／代），UI thread timer tick 發布，server 逐連線算 delta 送 `patch` |
| 指令進來 | 單一入口 `rr_gcode`，字串進 HTTP 專用 `GCodeBuffer`，安全檢查全在直譯器內 | `{type:"cmd",cmd,tag,value}` 進 `CommandQueue`，dispatch 到 golden handler，互鎖在 handler 內。原則相同，沒有通用文字直譋器 |
| 瀏覧器接法 | 只透過 `BaseConnector` 抽象，同一套 UI 可接「HTTP 輪詢」或「WebSocket 推送」兩種後端 | WebSocket `/ht9045` 一條（3 個檔在用）＋ Steven 頁面的 file:// JSON 全量輪詢（16 個檔在用，由 C# 模擬器餵） |
| 操作權 | ~~session key，一個 IP 一個 session（3.5-b4 起可多）~~ → **更正（§4.6）：session 上限 8 個、8 秒 idle 即掉、IP + `X-Session-Key` 雙比對；無密碼時自動放行。Duet 允許 8 個 session 同時送 G-code，沒有操作權互斥** | `control.acquire` 單一操作 token 已實作（FW-W3）；ST 戰役的 S1 指的是把它接到 START。**這一項我們比 Duet 嚴，是對的（§7.5.4）** |
| 執行緒 | 網路 responder 與運動控制在同一 MCU 但不同 RTOS task；網路層永不碰運動 | `WebBridgeServer` 單一 socket thread 永不呼叫機台邏輯，契約寫在標頭 |

**根本差異（不是可以抄的）。** Duet 的運動在專用 ARM MCU 上做即時控制，PC 只是瀏覧器。
HT9045 的運動控制在 Windows 行程裡透過 PCI 卡（1203／Galil／MN200）下命令，
web server 與機台邏輯**同行程**。V906 的執行緒契約處理了這件事，但這是我們比 Duet 多出來的風險面。

---

## 2. Duet 架構總覽

```
                 獨立模式（Duet 2／Duet 3 standalone）
 ┌──────────┐  HTTP GET rr_connect / rr_model / rr_gcode / rr_reply   ┌───────────────────────────┐
 │ 瀏覧器    │ ◀──────────────────────────────────────────────────────▶ │ RepRapFirmware (C++, MCU) │
 │ DWC SPA  │      PollConnector：每 250 ms 一發 rr_model flags=d99fn  │  HttpResponder            │
 │ (Vue 2)  │      seqs 變了才用 d99vno 重抓那一區                       │  ObjectModel (表驅動)      │
 └──────────┘                                                         │  GCodes (多通道直譋器)      │
                                                                      │  Movement / Heat / …       │
                 SBC 模式（Duet 3 + Raspberry Pi）                      └───────────────────────────┘
 ┌──────────┐  REST /machine/*  +  WebSocket /machine（OK 背壓）  ┌──────────────────┐  SPI  ┌─────┐
 │ 瀏覧器    │ ◀────────────────────────────────────────────────▶ │ DSF (C#, Linux)  │ ◀───▶ │ RRF │
 │ DWC SPA  │      RestConnector：第一包 full model，之後 patch   │  DuetWebServer   │       └─────┘
 └──────────┘                                                    │  DuetControlServer│
                                                                 │  plugins (IPC)    │
                                                                 └──────────────────┘
```

- **同一套 DWC** 兩個後端都能接：`Connectors/src/index.ts:12`
  `export const connectorTypes = [PollConnector, RestConnector];`——照順序試，
  第一個成功的就用（§3.1）。
- **Object Model** 是所有人的單一真相：UI 讀它、G-code 巨集能讀它（`{state.status}` 之類的表達式）、
  DSF 用它算 patch、PanelDue 觸控螢幕也讀它（`notPanelDue` 旗標就是為此存在）。
- **G-code 字串**是唯一的指令入口：網頁按鈕、觸控螢幕、USB、SD 卡檔案、巨集，
  全部進同一個直譋器的不同通道。安全檢查因此只需要寫一次。
  ⚠ **更正（§4.5）：有一個刻意的例外。** `M112`（緊急停止）在 network task 內
  **直接呼叫 `EmergencyStop()`**（`GCodeInput.cpp:179`），繞過佇列。
  DSF 側也對應：`M112` 等帶 `IsPrioritized` 且用全程式 token
  「so they survive channel resets」，並**刻意讓預設攔截器看不到它**（§5.5、§5.6）。
  ⇒ 「唯一入口」這條有且僅有一個例外，而那個例外是急停。這正是提案 P11／P19 的形狀。

---

## 3. 深讀 I：通訊層 Connectors（`Connectors/src/`，8 檔 2,711 行，LGPL-2.1）

> 完整筆記 `notes/A_connectors.md`（1,089 行，549 筆引用全數通過驗證）。
> 這一層是「瀏覧器怎麼接機台」的全部；DWC 本體 25,577 行不知道後端是誰。

### 3.1 連線建立與後端判別

- **不偵測後端，照固定順序試**：`index.ts:12` 先 `PollConnector`（RRF `rr_connect`）再
  `RestConnector`（DSF `machine/connect` + WebSocket）。只有「非 `LoginError`」的失敗才 fallback
  （`index.ts:33-35`）；拿到 `InvalidPasswordError`／`NoFreeSessionError`／`BadVersionError`
  代表已經連到端點但被拒，立刻停。
- DSF 有一層 `rr_*` 相容模擬；Poll 端看到回應的 `isEmulated` 就主動讓路給 REST
  （`PollConnector.ts:69-71` 丟 `OperationFailedError`，屬非 LoginError ⇒ fallback）。
- `connect()` 只建立實例；**輪詢／WebSocket 迴圈要等 `setCallbacks()` 才啟動**
  （`index.ts:17` 的註解明講）。UI 因此可以先把 handler 掛好再收第一包。
- 密碼以 URL query 明文帶（`rr_connect?password=`），成功後每個請求改帶 `X-Session-Key` header
  （`PollConnector.ts:146-147`、`RestConnector.ts:154-155`），WebSocket 放 `?sessionKey=`
  （`RestConnector.ts:46`）。韌體端 session idle timeout 8,000 ms（`HttpResponder.h:43`）。
- 版本協商：`rr_connect` 回 `apiLevel`，Poll 端 `apiLevel` 缺或 0 就 `BadVersionError`
  （`PollConnector.ts:72-75`）；韌體現值 `ApiLevel = 2`（`RepRapFirmware.h:71`）。

### 3.2 PollConnector 更新迴圈（獨立模式）

- 是 `do … while (isConnected)` 加尾隨 `setTimeout(updateInterval)`（`PollConnector.ts:497`、`:658-663`），
  **不是固定頻率**：週期 = 該輪所有 await 耗時 + 250 ms（`Settings.ts:146`）。
- **全量輪**（剛連上／偵測到重啟）：先抓 `rr_model key=seqs`（`:502`），清積壓回覆，
  再逐頂層 key 用 `d99vno` 抓完整子樹（`:514-538`），每抓一個 key 回報進度百分比 = 已抓／總數（`:532`）。
  頂層 key 清單**由 ObjectModel 套件的類別欄位決定**，不寫死（`:25`）。
- **增量輪**：一發 `rr_model flags=d99fn` 拿全部 live 欄位（`:544`），然後逐 key 比 `seqs`，
  **哪個 key 的 seq 變了就用 `d99vno` 重抓那一個 key**（`:573-575`）。`seqs` 只給 connector 用，
  推給 UI 前 `delete`（`:547-548`）。
- **韌體重啟偵測**：`state.upTime` 倒退（`:607`）→ 下一輪走全量、待決指令全部
  `OperationCancelledError`、重送 `rr_connect` 校時（`:608-620`）。`reconnect()` 刻意**不**清 `lastUptime`
  （`:363` 註解 `don't reset lastUpTime in order to be able to detect resets`）。
- **reply seq → rr_reply**：`seqs.reply` 變了才去拉 `rr_reply`（`:623-626`）。

### 3.3 `flags` 字母表（Connectors 內零註解，定義在韌體 `ObjectModel.cpp:518-577`）

| 字母 | 意思 | 韌體行 |
|---|---|---|
| `a<n>` | 陣列從第 n 個元素開始回（分頁） | `:532-539` |
| `d<n>` | 最大深度；`d99` 實務上不限 | `:540-547` |
| `f` | **只回 live（高頻變動）欄位** | `:548-550` |
| `n` | 含 null 值（讓 UI 能把欄位清空，而非「沒提到＝不變」） | `:554-556` |
| `o` | 含 obsolete 欄位 | `:557-559` |
| `v` | 含 verbose（平時省略）欄位 | `:566-568` |

⇒ `d99fn` = 每輪 live 快照；`d99vno` = 完整轉儲。這就是「高頻小包／低頻全量」的分流。

### 3.4 RestConnector（SBC 模式：WebSocket 推送＋REST）

- 第一則 WebSocket 訊息就是**完整 Object Model**（`RestConnector.ts:48-52`），之後每則是 patch。
- **OK 背壓**：收到 patch → `onUpdate` → 回 `"OK\n"` 才會收下一包（`:247`、`:297`）；
  DSF 端 `WebSocketController.cs:341` `if (line == "OK")`。`updateDelay` > 0 時延後回 OK ＝ 限制 patch 頻率
  （`:294-298`）。
- 應用層 PING/PONG（閒置 2,000 ms 才送，`Settings.ts:155`；`RestConnector.ts:258-266`），
  理由是瀏覧器 WebSocket API 拿不到 ping frame（`:259-261` 註解）。
  **沒有「PONG 沒回就斷線」的計時器**（grep `pong|pongTimeout|pingTimeout` 只命中註解與比對字串）；
  斷線偵測靠 `send()` 失敗觸發 `onerror`/`onclose`。
- connector **不自己套 model**，交給 UI：`Callbacks.ts:30` 說用 `omInstance.update(data)`。
- `reconnect()` 成功後推 full model 再呼叫 `onReconnected`（`:366-369`）；
  **`onReconnected` 全 src 只有這一處**，PollConnector 從不呼叫它，重連成功以 `onConnectProgress(0→…→-1)` 表達。
- REST 端點 18 個（含 WS）全表在筆記 §3.6；機台控制只有一條 `POST machine/code`，
  其餘全是檔案（7）與外掛／套件（8）。

### 3.5 指令送出與回覆對應

| | PollConnector（`rr_gcode`） | RestConnector（`POST machine/code`） |
|---|---|---|
| 回應 | 只回 `{"buff":N}` 緩衝空間（韌體 `HttpResponder.cpp:566`）；空指令可拿來探測緩衝（`:559` 註解） | **同步回覆文字**（DSF `MachineController.cs:321`） |
| 結果怎麼來 | 送出前記 `seqs.reply`（`PollConnector.ts:827`），之後 `seqs.reply` 變化 → `rr_reply`；所有 `code.seq < seq` 的待決指令**共用同一段回覆文字**（`:857-861`）。**不是 per-command** | 就是回應本體 |
| `noWait` | 不登記待決（`:841`） | `?async=true`，DSF 只入列 |
| 排除 | M997（韌體更新）／M999（重置）不等回覆（`:841`） | — |
| 錯誤 | `buff===0` → `CodeBufferError`（`:832-834`） | **永不 reject**，錯誤變成 `"Error: …"` 字串（`:423-425`）；預設**無逾時**（`:165`） |
| 503 特例 | 韌體回覆積壓沒緩衝 → 先拉 `rr_reply` 再重送原請求（`:180-183`） | — |

**沒有指令佇列、沒有節流**（grep `queue|throttl|debounce` 只命中 doc 註解）。
`pendingCodes` 是回覆對應表不是送出佇列（`:413`）。並行控制推給 UI 與韌體緩衝。

### 3.6 介面、設定、錯誤族

- `BaseConnector` 22 個方法／abstract 13 個：機台控制**只有 `sendCode` 一個字串通道**，
  連線 6、檔案 7、外掛 8（筆記 §5）。
- `DefaultSettings` 13 個值集中一張表、每個註解寫單位與影響（`Settings.ts:90-161`）；
  `maxRetries=2` ⇒ 單次請求逾時 = `sessionTimeout/(maxRetries+1)` ≈ 2,667 ms（`PollConnector.ts:157`）；
  `retryDelay` 2,000 ms（`Settings.ts:140`）。
- `errors.ts` 17 個類別分四族：網路／檔案／登入／指令。**`LoginError` 族就是停止 fallback 的訊號**。
- 五個 callback（`onConnectProgress`／`onConnectionError`／`onReconnected`／`onUpdate`／`onVolumeChanged`）
  全部觸發點在筆記 §6；Rest 從不呼叫 `onConnectProgress`。

### 3.7 讀碼時發現的疑點（借鑑要避開）

1. `PollConnector.ts:556` `if (status === MachineStatus.simulating)`：函式內沒宣告 `status`，
   瀏覧器會解析到 `window.status`（字串）⇒ `wasSimulating` 永遠不會由此變 true。
2. `:820` 換行判定少了 `[i]`；`:238` 401 自動重登後重送沒帶 `retry+1`（理論上可迴圈）；
   `BaseConnector.ts:148` 錯誤訊息名字貼錯；`errors.ts:97` `FileError` 訊息前綴是 Network error。
3. `RestConnector.ts:526` `filename` 放錯參數位；`:220-222` `ontimeout` 沒清 `requests`。

### 3.8 這一層對我們的啟示（意見）

1. **per-section 序號分流**（高頻 live 小包 ＋ 低頻分區全量）比每輪整包重送省，也比 diff 整棵樹簡單。
   V906 今天是逐連線 diff 整張 5,349 tag 的 map（`WebBridgeServer.cpp:1440-1466`）。
2. **指令「收下／沒收下」與「執行結果」分兩條流**——V906 的 ack 語意已經是這樣（UI thread 處理完才 ack），
   但要注意 Duet 的回覆**不是 per-command**；我們的 `id` 對應是比它好的地方，要守住。
3. **OK 背壓**讓慢瀏覧器自己降頻，伺服端不必猜 UI 吃得多快。V906 用「backlog > 256 KiB 就斷線」
   （`WebBridgeServer.h:130-132`）——那是保護伺服器，不是讓慢客戶端活下去。
4. 應用層 PING/PONG **要補 Duet 沒做的 PONG 逾時**；V906 已有 `idleTimeoutMs` 45,000 ms，這點我們比它完整。
5. 用單調遞增的 `upTime`（或 boot id）**倒退**偵測後端重啟，比等連線斷更早，且能區分「網路閃斷」與「程式重開」。
   V906 沒有這個：wb_serve 重啟後瀏覧器只看到重連＋新 snapshot。
6. 錯誤用**型別**分四族，UI 才能可靠決定「重連／重試／要密碼／只是指令錯」。V906 的 ack 只有 `error` 字串。
7. Duet 通訊層完全沒有指令佇列與互斥——handler 這種「一次點擊機台會動」的場景，
   **佇列與互斥必須在 C++ 端做**（V906 的 `CommandQueue` 有界 64、`control.acquire` 已做，是正確方向）。
8. 所有可調參數集中一張表、註解寫單位與影響。V906 的 `WebBridgeConfig` 已接近，但散在 wb_serve 的旗標更多。

---

## 4. 深讀 II：韌體 Object Model 與 HTTP 入口（`RepRapFirmware/src/`，GPL-3.0）

> 完整筆記 `notes/B_rrf_objectmodel_http.md`（1,055 行，526 筆引用全數通過驗證）。
> 副本 HEAD `50dee19`，`src/Version.h:15` `MAIN_VERSION "3.7.0-rc.1+3"`。

### 4.1 Object Model 的宣告機制：60 張 constexpr 表 ＋ 3 個編譯期斷言

`ObjectModelTableEntry` 只有三欄（`ObjectModel/ObjectModel.h:420-422`）：
`name`（字串）、`func`（取值函式指標）、`flags`。整張表是 `constexpr` POD 陣列，
因為設計限制寫死在 `ObjectModel.h:410-411`：必須能放在 flash 裡當初始化資料，
所以**不能用類別階層**。`func` 是無捕捉 lambda。

`DECLARE_OBJECT_MODEL`（`:471-475`）給類別加一個虛函式 override ＋三個 static 成員；
`DEFINE_GET_OBJECT_MODEL_TABLE`（`:489-497`）在 .cpp 端定義類別描述子，並在函式體內放**三個
`static_assert`**：描述子長度對不對（`:493`）、表長與描述子加總相不相符（`:494`）、
**表有沒有照字母序**（`:495`）。

⇒ 「哪些欄位曝露、哪些高頻」的錯誤在**編譯期**就被抓住，不是執行期才發現畫面空白。

規模：`grep -rln "^\s*DECLARE_OBJECT_MODEL" --include=*.h src` → **62 個 .h**；
`grep -rn "objectModelTable\[\] =" --include=*.cpp src` → **60 張表**。

`objectModelTableDescriptor` 的語意：第 0 個元素 = 子表數，其後每個元素 = 該子表的筆數，
所有子表**接在同一個陣列裡**靠描述子切段。例 `Fans/Fan.cpp:41` `{ 2, 10, 3 }` = 2 張子表、
第 0 張 10 筆、第 1 張 3 筆。`RepRap` 有 7 張（`Platform/RepRap.cpp:423`）。

### 4.2 `ObjectModelEntryFlags` 六個值（`ObjectModel.h:184-196`）

| 值 | 註解原文 | 效果 |
|---|---|---|
| `none = 0` | nothing special | 只在非 `f` 模式輸出 |
| `live = 1` | fast changing data, included in common status response | `flags=f` 時只留這些 |
| `important = 2` | important when it is present … Used for message boxes | `flags=i` 時即使值為 null 也輸出 |
| `verbose = 4` | omit reporting this value by default | 預設排除，`v` 解除 |
| `obsolete = 8` | entry is deprecated | 預設排除，`o` 解除；表達式查到會標記 `SetObsoleteFieldQueried()` |
| `notPanelDue = 16` | not of interest to PanelDue | `flags=p` 時排除 |

### 4.3 查值二分搜、輸出線性掃——字母序是為了前者

`flags` 字母（`a d f i n o p s v`）**只在 `ObjectExplorationContext` 建構子一處解析**
（`ObjectModel.cpp:518-577`）。key 為空時預設深度 1。字母表見 §3.3。

緩衝耗盡時：整鏈釋放、回 **503**（碼內註解寫 501 是舊的）。root 陣列用掉一半緩衝池就切頁、
回 `"next"`——對應 Connectors 的 `a<n>` 分頁（§3.3）。

### 4.4 seqs：把「哪個子樹髒了」壓成 17 個整數

- `RepRap.h:245-246` 15 個 `uint16_t *Seq` 成員，**16-bit、無鎖、單純 `++`**。
- `seqs` 子表 17 個 key（12 無條件 + 5 條件式，`RepRap.cpp:381-408`）。
  `reply` 不是 RepRap 成員，來自 `HttpResponder::GetReplySeq()`（`HttpResponder.h:136`
  `static std::atomic<uint16_t> seq;`）。`volChanges` 是每個儲存裝置一個的陣列。
  `:394` 註解寫 `// no need for 'limits' because it never changes`——**不變的東西不給 seq**。
- 16 個 `*Updated()` 方法（`RepRap.h:155-170`），各為 `{ ++xxxSeq; }`。
- **呼叫點合計 203 處／16 個方法**（MoveUpdated 53、StateUpdated 28、InputsUpdated 21…）。
  模式是：**寫入端在改完狀態的那一行手動呼叫**，沒有集中式 dirty tracking，OM 表本身不知道誰改了它。
  例：`GCodes/GCodes3.cpp:133` `reprap.MoveUpdated(); // because we may have updated axesHomed…`。
- ⚠ 這是 Duet 這套設計**最脆的一環**：漏呼叫一處，畫面就不更新，而且沒有任何編譯期檢查抓得到。

### 4.5 鎖與執行緒邊界——這是 PC 上不能直接照搬的一節

- **OM 讀取幾乎無鎖**：`GetObjectLock()` 預設回 `nullptr`（`ObjectModel.h:346`），
  全樹只有 **1 個 override**（`RepRap.cpp:416-419`，只鎖 `state` 子表的訊息框）；
  `RepRap` 的 10 個陣列只有 3 個帶 `ReadWriteLock`（fans、tools、ledStrips），其餘 7 個是 nullptr。
- 之所以成立，是因為在 Cortex-M 上讀一個對齊的 32-bit 值是原子的。
  **在 x64 上讀 `double`／`AnsiString` 不是**——這條在 §9.1 的 P9 會回來。
- HTTP responder 跑在**獨立的 NETWORK task**（`Network.cpp:519`，
  `TaskPriority::SpinPriority = 1`），與主迴圈 MAIN task **同優先權**。
  碼內自己就標了邊界，例如 `HttpResponder.cpp:1476` 註解說某個釋放動作不能在 Main task 做，
  因為緩衝可能正被 Network task 使用。
- **network 層對運動／加熱 0 處直接呼叫**——唯一例外是 `M112` 在 network task 內直接
  `EmergencyStop()`（`GCodeInput.cpp:179`），**刻意繞過佇列**。這是「急停要走專用路徑」的教科書寫法。

### 4.6 HTTP session 與 rr_gcode 入口

- session 上限 **8** 個、**8 秒** idle 即掉（`HttpResponder.h:43` `HttpSessionTimeout = 8000`）、
  IP + `X-Session-Key` 雙比對、密碼定長常數時間比對、**無密碼時自動放行**。
- ⚠ `Authenticate()` 的 key 唯一性檢查迴圈索引用錯變數（`sessions[numSessions]`），**實質無效**。
- `rr_gcode` = `strlen+1` 整段原子塞進 **255 byte ring buffer**，回傳的 `buff` 就是剩餘空間。
- 回覆是**廣播式** `gcodeReply` 堆疊，第 N 個 session 取走才清——**作者自己留了 FIXME**。
- `M409`（G-code 查 OM）、`rr_model`（HTTP 查 OM）、週期性 `d99fi` 自動回報（給 PanelDue）
  **三者共用 `RepRap::GetModelResponse()`**。一個查詢實作，三個消費者。
- 14 個活的 rr_ 端點，其中 13 個需要 session，**只有 `rr_connect` 不需要**。
  `rr_status`／`rr_config` 已 `#if 0`。
- 靜態網頁與 API **同一個埠、同一個 responder**：非 `rr_` 前綴就當檔案送，找不到又無副檔名就回
  index 讓 SPA router 接手（`HttpResponder.cpp:925-928`）。一個 exe、一個埠、打開瀏覽器就是 HMI。

### 4.7 這一層對我們的啟示（意見，摘自筆記 §9）

1. **「一張表描述一個類別的可見狀態」比「到處手寫 JSON」耐用。** V906 的 tag 目錄若也用
   「名稱／取值函式／旗標」三欄靜態表，就能把散落的 emit 碼收回一處。PC 上不受 flash 限制，
   可以用 `std::string_view` + `std::function`，但**表的形狀值得照抄**，尤其是三個 `static_assert`。
2. **`live` 旗標是宣告式的，差異比對是計算式的，兩者可並存**：宣告 live 決定「比對哪些」，
   差異比對決定「送哪些」。
3. **seqs 的 203 個手動 `++` 是反面教材**。對 handler 這種狀態機密集的軟體，應該在**狀態機的
   transition 函式**內集中呼叫（例如 `SetTask()`／`SetLotState()` 一處），而不是散到 200 個地方。
4. **命令入口只做「塞進佇列」，語意判斷全在直譯器**——與 V906 方向一致；要守住的是
   **不要在 WebBridge 加第二套判斷**。但 `M112` 那條例外也要學：急停繞過佇列，且只此一條、明確標記。
5. **不要學它的廣播式回覆**。V906 的 `id` 對應比 Duet 進步一截，要守住。
6. **緩衝耗盡回 503、絕不送半截 JSON**；深度限制與陣列分頁是現成解法。
7. **Duet 沒有操作權互斥**——它允許 8 個 session 同時送 G-code。**不要期待從 Duet 抄到這個**。
8. network 與主迴圈是同優先權兩個 task，靠 mutex 與 ring buffer 交握，三個邊界物件都有明確 owner。
9. **OM 幾乎無鎖這件事不能搬到 x64**：V906 的 `TagSnapshot` 每個非 POD 欄位都要有鎖或 copy-on-write。
   我們現在的雙緩衝＋一把 mutex 已經是對的做法。
10. 靜態網頁與 API 同埠同 responder，對部署很方便——V906 的 wb_serve 已經是這樣。

---

## 5. 深讀 III：DSF 中介層——REST、WebSocket patch、code 管線與攔截（`DuetSoftwareFramework/`，GPL-3.0）

> 完整筆記 `notes/C_dsf_webserver.md`（542 筆引用全數通過驗證）。
> 這一層是 Duet 架構裡**最像我們處境**的部分：一個在通用 OS 上跑的中介行程，
> 上接瀏覧器、下接即時控制器。

### 5.1 patch 不是 RFC 6902，是「只含變更欄位的子樹」

- 查證：`grep -rlE "6902|JsonPatch|FindDifferences|MakePatch|\"op\"" src/DuetAPI src/DuetControlServer` → **0 檔**。
- patch 是巢狀字典（`ModelSubscription.cs:95`），序列化時遞迴寫物件。

### 5.2 變更偵測在 setter，不是比快照 ★

這是整套機制的根，也是與 V906 差最多的一點：

```
ModelObject.cs:27-35
protected void SetPropertyValue<T>(ref T storage, T value, [CallerMemberName] string name = "")
{ if (!Equals(storage, value)) { PropertyChanging?.Invoke(...); storage = value; PropertyChanged?.Invoke(...); } }
```

**值相同就不發事件——這就是 diff**，不需要保存上一版快照去比對。
DCS 啟動時遞迴掛上整棵 OM 的 `PropertyChanged`（`Observer.cs:24`、`ModelObjectObserver.cs:72-121`），
事件轉成「路徑 + 型別 + 值」（`Observer.cs:14`）。三種變更型別：`Property`、`Collection`、
`MessageCollection`（後者只 append 不 clear，因為訊息本來就是易逝的）。

訂閱 processor **不保存上次送出的整份快照**；只把送出之間發生的每個變更**就地合併**進 `_patch`，
送出後清空（`ModelSubscription.cs:224-233`）。有變更就不等，沒變更才 await
（`:192-199`）；`Provider` 在**寫鎖釋放時**廣播更新（`Provider.cs:87-93`）。

⚠ 代價：**所有**寫入都得經過那個 setter。直接寫成員的舊碼會漏推。
這正是 golden 翻譯樹最容易踩的坑——我們有 655 個 `//AI(ht9045-v899)` 標記的既有碼在直接寫全域。

### 5.3 filter：讓每個頁面只訂它顯示的東西

- 語法「類似 XPath」（`SubscribeInitMessage.cs:40-43`）：`heat/heaters[*]/current`；
  `**` 代表整個命名空間，**只能放在結尾**；`*` 可用於名稱或索引。
- 解析在 `Filter.cs`：`[*]` → `-1` 表任意；路徑分隔可用 `.` 或 `/`；
  **沒有 `**` 時必須整條路徑精確匹配**。
- 初始快照過濾時，集合中未選中的項目放**佔位物件**，維持索引對齊。
- DWS 自己用的 filter 只有 4 條；**給瀏覧器的 WS 是 `[]`，即整棵**（`WebSocketController.cs:204`）。

⇒ 我們的 `tools/pagewire/` 三元組 `[文件, 區段, 鍵]` 可以直接映成這種路徑字串。

### 5.4 WebSocket 背壓：兩層 Acknowledge

- 瀏覧器回 `"OK"` 才發下一包（`WebSocketController.cs:341`）；只支援兩個字：`OK` 與 `PING`
  （`:40` 的錯誤訊息把這件事寫死）。
- 慢的分頁不會被灌爆，而是收到一包**合併後的較大 patch**——因為 `_patch` 一直在就地合併。
- DWS 對 DCS 也要 Acknowledge，是**兩層鏈**。
- 實作成本極低（一個 auto-reset event）。

### 5.5 code 管線六個 stage ★

`PipelineStage.cs`：`Start` → `Pre` → `ProcessInternally` → `Post` → `Firmware` → `Executed`。
每個**通道**各建一套，每個 stage 一個堆疊（巨集／檔案各推一層），每層是**有界** `Channel<Code>`
（`MaxCodesPerInput = 32`，滿了 `FullMode = Wait`），Executed 層無界。

| Stage | 做什麼 |
|---|---|
| Start | 非 prioritized 的 code 先等 `Unbuffered` 計數歸零 |
| **Pre** | 交給 Pre 攔截器；被 Resolve 就直接跳 Executed，**不送韌體** |
| ProcessInternally | DCS 自己能處理的 G/M/T/keyword |
| **Post** | 交給 Post 攔截器；這是「要送韌體前」的最後一道 |
| Firmware | **假 stage**：no-op，由 SPI 通道處理器直接從這層拉 |
| Executed | 更新檔案位置、錯誤前綴、交 Executed 攔截器（**還能改結果**）、`SetFinished`/`SetCancelled` |

優先通道：M108/M112/M122/M292/M999 B0 標 `IsPrioritized`（`SimpleCode.cs:108-113`），
註解寫「always go to an idle channel so we (hopefully) get a low-latency response」，
且用**全程式 token**而非通道 token，「so they survive channel resets (e.g. emergency stop)」。

### 5.6 CodeInterception：外掛能否決一個指令 ★

攔截器可回四種命令（`CodeInterception.cs:29-35`）：

| 命令 | 效果 |
|---|---|
| `Cancel` | 拋 `OperationCanceledException` ⇒ code 以 `Result = null` 取消。**可以取消** |
| `Resolve` | 設定結果、跳到 Executed，**不送韌體** |
| `Rewrite` | `code.CopyFrom(...)` 後繼續原流程。**可以改寫** |
| `Ignore` | 什麼都不做 |

- 攔截時外掛還能**同時送一般命令**（插入別的 code、查 OM），直到送回四種終結命令之一。
- 誰會被攔：不是自己送出的 code、優先旗標相符、通道在清單內、filter 比對 `ToShortString()`（如 `G1`）。
- 同一 mode 多個攔截器**依註冊順序**逐一問，第一個 Resolve 就結束。
- **兩個結構性漏洞**（筆記明列）：(a) 韌體發起的 code 與 `M112` 等緊急碼帶 `IsPrioritized`，
  預設攔截器看不到——**這是刻意讓急停不被外掛卡住**；(b) 攔截器自己插入的 code 不會被自己攔，
  且跳過 Start/Pre。
- ⚠ 筆記的關鍵判斷：**真正的運動安全（限位、熱失控、急停）不在 DSF 這層，在 RRF 韌體。
  攔截器是「政策層」，不是「保護層」。**

### 5.7 session、REST 與文件漂移

- session key 在標頭、WS 用 query、**WS 掛著就不過期**、8 秒閒置即失效、無密碼時自動開 session。
- 兩級權限（ReadOnly／ReadWrite）在型別上存在，但**只發過 ReadWrite**。
- HTTP 層無狀態：每個 REST 呼叫都新開一條 UNIX socket 到 DCS。
- ⚠ **文件會漂**：OpenAPI 寫 GET 實作 POST、`systemPackage` 漏列、README 提到不存在的設定、
  PLUGINS.md 路徑錯。像 DSF 這種維護良好的專案也擋不住。

### 5.8 這一層對我們的啟示（意見，摘自筆記 §9）

1. **單一 model 是所有推送的源頭，UI 永遠不輪詢。** 我們的 tag 串流是「每個 tag 一條事件」，
   缺的是「一棵有結構的樹 ＋ 子樹 patch」。建議在 C++ 側定義一棵 HMI model
   （狀態機、各站、警報、配方摘要），用「變更子樹 JSON」推送，而不是散裝 tag。
2. **變更偵測放在 setter，不要比快照**——但代價是所有寫入都得經過包裝器，
   直接寫成員的舊碼會漏推。這是我們翻譯樹最容易踩的坑，要先評估再決定。
3. **背壓要明確**，包含「兩層鏈」。實作成本極低，建議照抄。
4. **filter 讓每個頁面只訂它顯示的東西**；pagewire 三元組可直接映成路徑字串。
5. ★ **命令走「一條有 stage 的管線」，而不是每個按鈕各自呼叫底層。**
   START／PAUSE／jog 全部變成同一種命令物件進同一條管線，Pre stage 放互鎖與操作權檢查，
   Executed stage 統一把結果推回 model。這樣 **SECS/GEM、GPIB、網頁三種來源共用同一道閘門**——
   **正是 `fMain->Start()` 32 個呼叫點只閘 3 個的問題的結構解**（工具實測，見 §9.1.1；
   ⚠ 20260922 更正：此處原寫「19 個只閘 2 個」，那是已作廢的人工盤點數字）。
6. **但 Pre stage 不能是唯一防線**：我們的 C++ 核心同時扮演 DCS 與 RRF 兩個角色，
   運動層自己的安全門、真空、Home 狀態檢查仍要在最底層存在。
7. **同步／非同步兩種命令語意都要有，且同步要有逾時。** DSF 的同步路徑**沒有逾時**，
   對機台軟體不可接受：START 這種可能等數十秒的動作應一律非同步 ＋ 用狀態欄位回報。
8. session 模型可以直接借；但兩級權限要真的發 ReadOnly key，
   且 S1 的操作權 token 應是「同時只有一個 ReadWrite 生效」，這是 DSF 沒有的。
9. HTTP 層無狀態、狀態全放 C++ 側，讓 web 伺服器可獨立重啟。
10. **我們的 REST/WS 表面要有「從程式碼產生表面清單」的 gate，而不是手寫文件**——
    DSF 這種維護良好的專案都擋不住文件漂移。

---

## 6. 深讀 IV：DWC 前端架構與 ObjectModel TS 型別層（`DuetWebControl/` + `ObjectModel/`）

> 完整筆記 `notes/D_dwc_frontend_and_objectmodel_ts.md`（345 筆引用全數通過驗證）。
> 這一份對照的是我們最弱的一塊：69 頁手工 HTML、無 framework、無型別層、無 router、無 store。

### 6.1 規模（每個數字都附量法，見筆記 §10）

| 項目 | DWC |
|---|---|
| 程式碼行數（`.ts`/`.vue`/`.js`） | 25,577 |
| 內建路由 | 13（＋外掛 4） |
| panel 檔 | 39（全域註冊 34） |
| 元件檔總數 | 81（全域註冊 74） |
| Vuex module | 根 3 靜態 + 1 動態；每台機器 3 個子 module |
| i18n 語系 | 13 |
| 內建外掛 | 5（預設啟用 2） |
| ObjectModel `.ts` 檔 / 行數 | 73 / 2,675 |
| `extends ModelObject` 的類別 | 86 |
| `export enum` | 37 |

### 6.2 兩份 model 的分工 ★（最值得抄的結構）

DWC 同時保有兩份：**`typedState`**（真正的 `ObjectModel` 實例，負責解析與型別檢查）
與 **`state`**（`JSON.parse(JSON.stringify(...))` 的深拷貝，負責被 Vue 觀察）。
patch 進來時先套進 `typedState.update(data)`，再同步到被觀察的那份。

為什麼這件事重要：**它把「資料怎麼來」跟「UI 怎麼看」解耦了**。

### 6.3 型別層怎麼跟韌體同步：**沒有 codegen，靠版號釘死**

- `grep -rniE "codegen|generator|swagger|openapi" ObjectModel/` → **0 命中**。
  `package.json` 的 scripts 只有 tsc/jest；CI 只有 CLA 與 release。
  ⇒ **韌體改欄位 → 人工改 TS。**
- **`ObjectModel` 與 DWC 本體裡 `apiLevel` 0 命中**——那個握手在 `Connectors` 那一層（§3.1），
  型別層本身沒有協商。
- 實際做法：三件套版號全部釘在同一個數字（3.6.3／`~3.6.x`），
  連上 2 秒後用 semver 比對主板韌體版號，patch 級只記 warning、major/minor 才彈對話框，
  而且**這個檢查可以被關掉**。DSF 版號變了就直接 `location.reload()`。

⇒ 我們有約 5,349 個 tag，手寫 TS 不可能。
tag 表本來就在 C++ 側（提案 P9 的靜態表），**前端型別應該從那裡產生**。

> ⚠⚠ **20260922 更正：不要把這一節讀成「Duet 沒有 codegen」。** 本節的 grep
> **只掃 `ObjectModel/`（TS 套件）**，在那個範圍內 0 命中是對的。但 DSF 的 **C# object model
> 是產生出來的**：`DuetSoftwareFramework/src/DuetAPI.SourceGenerators/`（**14 檔、1,969 行**
> 的 Roslyn source generator），為每個 ModelObject 產生 `Assign.cs`／`Clone.cs`／
> `UpdateFromJson.cs`（461 行）／`UpdateFromJsonReader.cs`（463 行）。
> ⇒ 正確敘述是：**跨語言那一段（C++ 韌體 ↔ TS 型別）沒有 codegen；同語言那一段（C# 模型）有。**
> ⇒ 所以 P9 的理由要改：**不是「我們該做得比它好」，是「Duet 在 C# 側已經這樣做了，抄那一側」。**
> ⚠ **更正（同日，我自己的第一版寫錯）**：這個目錄**不是 0 引用**。C 份筆記
> **早就記到了** —— `C_dsf_webserver.md:37` 把 `UpdateFromJson.cs`（461 行）列進檔案清冊，
> `:188` 明寫「`UpdateFromJson` 是**產生器產出**的 partial 方法」。
> ⇒ 所以這不是「沒讀到」，而是**筆記知道、主文件的結論沒跟著改**。
> 本文件 §6.3／Q2 把它摘要成全稱句「Duet 沒有 codegen」，與自己筆記裡的事實相反。
> **這比單純漏讀更值得記**：逐檔筆記正確，收斂到主文件時被壓成了錯的句子。

### 6.4 一個全域凍結閘勝過逐鈕判斷 ★

DWC 所有送 code 的按鈕都經過同一個 `CodeBtn`，而 `CodeBtn` 把 `uiFrozen` 硬編在
`disabled` 上（`CodeBtn.vue:2` 是 `$props.disabled || uiFrozen`，OR 進去所以頁面
只能加嚴不能放鬆）。⇒ 「斷線時**送 code 的按鈕**不可能還能送」是結構保證。

> ⚠⚠ **20260922 第二輪更正：這一節原本的結論要反過來寫。**（證據 `notes/G_dwc_cnc_panels.md`）
>
> 實測全樹（`DuetWebControl/src`）：`<code-btn` 使用點 **25** 個分布 10 檔；
> 但手寫 `disabled="uiFrozen…` 的綁定 **60 處**（30 檔提到 `uiFrozen`、共 103 次）。
> **也就是說結構保證只覆蓋了少數，多數還是靠「記得寫」。**
>
> 最刺眼的反例：`SpindleSpeedPanel.vue` 的 5 顆 `v-btn` ＋ 1 個 `v-combobox`
> ——**主軸啟停與 RPM 設定，一個 `uiFrozen` 都沒有**。
> 同一份筆記逐顆盤點 `CNCMovementPanel.vue` 的 14 個互動控制項：5 個走 code-btn、
> 3 處手寫、**3 處完全沒有守衛**（設工件零點、回工件零點、WCS 下拉）。
> 五個面板用了**四種不同做法**（純結構／混合／全手寫／命令式 early-return／完全沒有）。
>
> ⇒ **正確的教訓**：不是「有了單一命令元件就安全」，而是
> **「只要還留著裸的 `sendCode`，就一定會有人繞過它」**。
> 單一命令元件必須搭配「**它是唯一路徑**」的強制（lint／型別／code review 閘），
> 否則只是多一個沒人用的好習慣。
>
> ⇒ **而且 `uiFrozen` 不是安全閘。** 它**只看連線狀態**（`store/index.ts:116`），
> 不看忙不忙、不看有沒有 home、不看警報。P23 要的是三件事
> （`connected,idle,level>=2`），Duet 只給了第一件。
>
> 這同時更正 `notes/D_dwc_frontend_and_objectmodel_ts.md:584` 的
> 「任何 code 按鈕都逃不掉」—— 那句對「code 按鈕」成立，
> **不能推廣成「DWC 的按鈕都被凍結」**。

### 6.5 路由表是資料，不是程式碼

`Menu` 是一個可觀察的資料結構（5 個分類），router 從它生成，每項的 `condition`
是 getter，所以選單會隨機台狀態自動增減。整套在 371 行內做完，
**不依賴 Vue 的單檔元件**——只依賴「路由表是資料」這個想法。

### 6.6 `messageBox` 帶 `seq` ★

Duet 的對話框狀態有 `seq` 欄位，所以「使用者按的是哪一個對話框」不會搞錯——
換頁、重連、多個瀏覧器同時開，回覆都對得上。
另外兩個低成本的設計：持久性 modal 半秒後浮出急停鈕；dialog 內嵌 jog
（用 bitmask 指定哪幾軸），對「請手動把手臂移開再按確定」這類排除對話框直接可用。

### 6.7 這一層對我們的啟示（意見，摘自筆記 §11）

1. ★ **最該抄的是「兩份 model 的分工」，不是 Vue。**
   我們的問題正好對稱：tag 串流是扁平 dotted key、JSON 快照是巢狀物件，
   **兩者沒有共同的中介表示**，所以 69 頁各自解字串。
   先做一層「HMI 端 model」——不管資料從哪條面進來都先套進同一棵物件——
   那 69 頁綁的就是同一棵樹，兩條資料面誰先誰後、誰被淘汰，前端可以完全不知道。
   **這一步不需要任何 framework，純 JS 就能做。**（⇒ 這是提案 P3 的最小可行形）
2. ★ **`uiFrozen` 這種單一全域凍結閘。** 機台 HMI 的「不該按」比「不該看」嚴重得多。
   現在每頁自己判，等於 69 份互相不一致的安全規則。
3. **列舉順序不要變成對外介面。** DWC 的 `MessageBoxMode` 是數字列舉、UI 寫 `mode >= okOnly`，
   插一個新模式就會動到所有比較式。我們要做型別層的話，狀態類一律用字串列舉。
4. ★★ **陣列 patch 語意要先講清楚，這是最容易踩的坑。**
   `ModelObject.update()` 對陣列是**長度對齊 + 逐格覆寫**，不是差異合併——
   這對 RRF 成立是因為韌體一定送整條陣列。
   **我們的 patch 是逐連線 diff 出來的**，如果出現 `Arm.Site[3].Status` 這種單格更新，
   而型別層照抄 Duet 的語意，**陣列會被截成 4 格**。
   抄 `update()` 之前必須先確定自家 patch 的陣列語意是「整條」還是「單格」。
5. **集中存檔 + debounce 的概念值得抄，但不要用字串匹配分流。**
   DWC 用 `mutation.type.indexOf("/cache/")` 分流，那種程式碼改個 module 名就靜默失效。
6. **先做 router，不要先做 framework。** 可行的小步驟：把 69 頁的清單變成一份 JSON
   （路徑、標題、顯示條件、需要哪些 tag），先用它產生側邊選單與權限灰化。
   **這一步不改任何一頁的內容**，但立刻拿到「哪一頁在什麼機種／狀態下該出現」的單一出處。
7. ★ **`messageBox` 的 `seq` 正是我們缺的。** `ShowMyMessage` 目前是機台端阻塞式彈窗；
   未來要讓 web HMI 代答（FW-W5 modal 往返），**`seq` 是最低成本的正確性保證，值得一開始就加**。
8. **不要抄的**：Vue 2 + Vuex 3（DWC 自己留了至少 3 處 FIXME 說要升 Vue 3）；
   `patch.js`（作者自標 obsolete，存在的唯一理由是 Vue 2 的響應式限制）；
   手寫 schema（見 §6.3）；**把 JSON 全量快照當長期方案**——
   DWC 的 Poll connector 之所以還在是為了沒有 SBC 的老機型，
   我們兩條資料面並存若是過渡，就應該有**明確的淘汰時間表**，
   否則會複製 Duet「兩個 connector 各自維護一套 model 維持邏輯」的長期成本。

---

## 7. 深讀 V：韌體指令層與安全分層（`RepRapFirmware/src/`，GPL-3.0）

> 完整筆記 `notes/E_rrf_command_layer_and_safety.md`（1,255 行，420 筆引用全數通過驗證）。
> **這一份是整份文件裡對我們最直接可用的一份**：它回答的正是我們還沒設計的那幾件事——
> 多來源指令怎麼仲裁、急停走哪條路、對話框怎麼往返、警報處置怎麼從硬編碼變資料。

### 7.1 規模

| 項目 | 數 |
|---|---|
| `GCodeChannel` 通道 | 15（其中 8 個有外部輸入） |
| `GCodeResult` 值 | 14 |
| `EventType` 值 | 13 |
| `MessageType` 旗標 | 41 個列舉值（25 個單一位元、3 個遮罩、13 個預組合） |
| `GCodeState` 值 | 80 個相異值／82 個宣告 |
| 資源鎖 | `NumMovementSystems + 1`（單運動系統板 = 2） |
| `MaxMessageBoxes` | 8 |

### 7.2 緊急停止：在字元寫進緩衝**之前**就攔截 ★★

這是筆記認定最值得抄的一招。HTTP 收到的每一個字元先過一個 3 狀態機掃描器
（`GCodeInput.cpp:118-210`），命中 `M112` 就直接 `reprap.EmergencyStop()`
（`:179`）。呼叫點在 `NetworkGCodeInput::Put(char c)` 裡，**在字元被寫進環形緩衝之前**。

⇒ 不排隊、不等輪到 HTTP 通道、不等任何鎖。正在跑的檔案、巨集、正在等鎖的指令全不相干。

三個配套細節：

- **正常路徑留作後備**：`GCodes2.cpp:1946` 的 `case 112` 註解自己寫
  「acted upon in Webserver, but also here in case it comes from USB etc.」。
- **急停豁免「已停機就拒絕指令」的閘門**：`GCodes2.cpp:621` 明寫 `code != 112 && code != 999`。
- ⚠ **誠實標出有缺口的路徑**：`GCodes.cpp:125` 註解
  「this path does not support out-of-band emergency command detection (M112)」。

`RepRap::EmergencyStop()` 的順序（`RepRap.cpp:1043-1098`）：煞車先咬合 → 主軸／雷射 →
關 heat/move 任務 → 通知遠端節點 → 最後才動軟體狀態與日誌。
**刻意不關總電源**，怕熱端熔壞周邊。

### 7.3 多來源仲裁：沒有優先權，只有 round-robin ＋ 資源鎖 ★

這一節的答案出乎意料地簡單，而且筆記判斷它適合我們：

- **沒有優先權排程**。優先權只保留給一個通道（AutoPause）與一個 ISR 旗標（急停）。
- **鎖沒有等待佇列、沒有鎖計數、沒有 priority inheritance**：`LockResource` 拿不到就回 false，
  呼叫端下一輪再試。單執行緒汲取模型下完全夠用，而且**沒有優先權反轉可以發生**。
- **防死鎖只靠兩條規則**：資源編號遞增取得；取不到就把已取得的較低號釋放掉。
- **鎖的釋放收斂成一個函式**（`UnlockAll`），掛在三處：指令完成、丟例外、狀態機回到 normal。

### 7.4 合作式非阻塞：`notFinished` 是整個架構的關鍵字

handler 回 `notFinished` 時 dispatcher **不從佇列拿掉**，下一個 tick 再呼叫同一個 handler。
★ 關鍵細節：**`HandleResult` 在 `notFinished` 路徑上提早 return、沒有走到 `UnlockAll`**——
重試期間鎖必須保留，否則等鎖的指令會被別人插隊，永遠等不到。

### 7.5 M291/M292 對話框往返：三個原語 ★★

- **對話框是獨立物件，不是呼叫堆疊上的一個框。** `MessageBox` 有 `seq`、`mode`、`timeout`、
  `limits`，掛在一條上限 8 的串列上，**生命週期與任何函式呼叫無關**。
- **等待端只設一個 per-state 的 bit 然後讓出。** `waitingForAcknowledgement` 只擋自己這條流，
  其他通道照跑；而且 M291 明確 `UnlockMovement(gb)` 把運動鎖放掉，
  **讓使用者能在對話框開著時 jog 軸**。
- **答案是廣播的，不是回給發問者的。** `MessageBoxClosed()` 對全部 15 個通道呼叫
  `MessageAcknowledged(seq, ...)`，因為 M292 可能從 HTTP 來而等的人是 File 通道。
- **逾時 ＝ 自動取消 ＋ 回傳預設值**。對話框永遠不能無限期擋住流程。

### 7.6 事件→巨集：把警報處置從硬編碼變成可編輯檔 ★

- **事件佇列 ＋ 同類去重**，判準是「same type, device number, CAN address and parameter」四元組。
- **預設行為與客製行為分離**：`if (SysFileExists(macroName)) 跑巨集; else 跑內建預設;`。
- **處置在專屬通道（AutoPause）上跑**，不佔用正在跑的流程。
- `isBeingProcessed` **留在佇列頭不移除**，所以處置期間同類事件仍被去重擋掉。
- ⚠ Duet 自己的分界：`Platform/Event.h:18` 說**觸發器與斷電不走事件機制**。

`daemon.g` 每 10 秒跑一次、檔案不存在不報錯、**只在通道真的閒著時才跑**、
擴充板模式下停用（會拉長迴圈時間）。

### 7.7 訊息路由：目的地做成位元圖

一個 `uint32_t`，低位是目的地位元圖（與通道同序），高位是語意旗標與 2-bit log 級別。
`GetResponseMessageType()` 讓「指令從哪來，回覆就回哪去」變成一行，
而且 `responseMessageType` 是 **const**，通道建立時就釘死，不可能被改錯。
⚠ `MessageType.h:17` 自己警告：**位元順序與通道列舉順序是耦合的**。

### 7.8 這一層對我們的啟示（意見，摘自筆記 §12）

**我們已經對的地方（不要動）**：socket thread 不碰機台邏輯、指令走佇列由 UI tick 汲取，
就是 RRF 的 `GCodeInput` → `GCodeBuffer` → `Spin()` 分層；
互鎖留在 handler 內不在 web 層疊第二層，RRF 也是如此——
**web 層疊第二層互鎖只會產生兩份會漂移的真相**。

1. ★★ **M291/M292 的形狀直接可抄，不需要改架構。**
   我們的 modal 往返卡在「handler 端是同步阻塞的 `ShowMyMessageBox_YES_NO`」。
   解法是建一個 `TModalRequest` 物件池（含 seq、逾時、預設答案），
   讓呼叫點改成「建立請求 + 回傳 notFinished」，而不是「進 modal loop」。
   **對 HT9045 的意義**：對話框開著的時候，SECS host 的查詢必須照樣回（30 秒預算），
   面板的急停必須照樣按得下去。**現在的同步 modal loop 兩者都做不到。**
   `modal.answer` 也應該帶 seq 並廣播，這樣網頁、機台面板、SECS host 三邊都能回答同一個對話框。
   逾時加預設答案對機台是**必要**不是加分。
2. ★ **急停抄字元級攔截**：socket thread 在解析 frame 之前先掃一次 `estop`，
   命中就直接呼叫既有急停入口，**不進那 64 格佇列**。
   理由和 Duet 一樣：佇列滿了、UI thread 卡住、handler 正在等鎖——
   **這些正是最需要急停的時刻**。同理 SECS remote stop 與 GPIB abort 也該有各自的 out-of-band 路徑。
   要像 RRF 一樣**明確列出「哪些入口沒有 out-of-band 急停」**，而不是假設全部都有。
   ⚠ 我們的對應是「不要在急停時關氣源」，因為夾持中的 IC 會掉。
3. ★ **多來源仲裁先做資源鎖，別做優先權。** 鎖的粒度對應到物理子系統：
   `InArm`／`OutArm`／`Index`／`InShuttle`／`OutShuttle`／`TrayArm`／`HotPlate`／`AutoClean`／`Recipe`。
   具體三步：把每個手動／遠端動作改成「拿鎖 → 做 → 放鎖」；拿不到鎖回明確的 `busy`；
   web/SECS 端看到 `busy` 就重試或報告。
   **光這三步就能消掉大部分「網頁按鈕跟自動流程打架」的類別。**
   前置：我們現在**沒有統一的「指令完成」收斂點**，那是要先補的。
4. **`notFinished` 引進 handler 簽章**，比「開一條執行緒等」安全得多。
   ⚠ 重試期間鎖必須保留，這個細節不抄會踩坑。
5. ★ **警報處置的低風險演進路徑**：把現有硬編碼原封不動留著當 fallback，
   另外加一層「`ALARM\JAM0610.txt` 存在就跑它」。**第一版連腳本語言都不需要**，
   只要一個動作清單格式（關真空／停軸／提示／暫停／繼續）。
   ⚠ **安全鏈（急停、安全門、氣壓低）不要走警報佇列**，那是另一條更短更硬的路。
6. **`daemon.g` 的對應**：客戶自訂週期檢查。價值在**把客戶碼從 C++ 裡趕出去**——
   我們現在的客戶碼 gating（超豐／KYEC／ASE 各自的 `if`）正是這個問題的病徵。
7. **不要抄的三件**：`GCodeState` 那種 80 個值的巨型扁平列舉；
   「G-code 當內部 IPC」（對他們合理，對我們是把型別安全丟掉）；
   `GrabResource`（無視現任擁有者直接搶鎖——在有真空吸嘴與氣缸的機台上太危險，
   要留後門也只能給急停路徑用）。

---

## 7.5 橫向：同類開源專案怎麼做（Klipper／LinuxCNC／grblHAL／OctoPrint）

> 完整筆記 `notes/F_peer_projects_comparison.md`（714 行，207 個來源 URL）。
> **這一份的證據等級與其他幾份不同**：它靠公開文件，不是讀本機原始碼。
> 筆記自己把每個事實標了三級（A 引述／B 摘述／C 只在搜尋摘要出現），
> 並列出 6 個明確 fetch 失敗的來源（FluidNC 官方 wiki 從這台機器連不上）。
> 主迴圈抽驗了兩條最承重的引述（Grbl 的 realtime 字元、OctoPrint 的 throttle），
> 與原文一致。

### 7.5.1 五家對照（摘要，完整表在筆記 §6）

| | Duet RRF | Duet DSF | Klipper + Moonraker | LinuxCNC | grblHAL / FluidNC | OctoPrint |
|---|---|---|---|---|---|---|
| web server 在哪 | 韌體內 | 旁邊的 Linux | 旁邊的 Python | **沒有官方的** | 韌體內（插件） | 旁邊的 Python |
| 狀態模型 | 單一 Object Model | 同左 + patch | printer objects 命名空間 | NML + HAL pin | Grbl 狀態文字行 | 固定形狀快照 |
| 變更偵測 | 15 個 `*Seq` | setter 側 | 訂閱欄位 + `eventtime` | 無 | 無 | 無（定時全量 2 Hz） |
| 背壓 | 無 | **兩層 ACK 串鏈** | UNKNOWN | 無 | **128 字元 RX buffer** | **client 送 `throttle`** |
| 緊急停止 | M112（同通道） | 同左 | 同通道 | E-STOP 走 HAL 鏈 | **`0x18` 旁路，不進 buffer** | 同通道 |
| 單一操作權 | 無 | 無 | 無 | 無 | 近似（ESP3D `activeID` 廣播） | 無 |
| 授權 | GPLv3 | GPLv3 | GPL-3.0 | **GPL-2.0** | GPL-3.0 | **AGPL-3.0** |

### 7.5.2 七個共同模式（至少三家這樣做 ⇒ 那大概是對的）

1. **指令通道就是機器原本的語言，web 只是載體。沒有任何一家為了 web 發明第二套語意指令集。**
   理由顯而易見：兩套語意就會有兩套 bug、兩套互鎖漏洞。
2. **web 層一律在控制層外面，從不進入控制迴圈。** 沒有一家讓 HTTP 處理跑在會影響運動的路徑上。
3. **前端是純靜態資產 + 一條長連線。** 不需要模板引擎、不需要 server-side rendering。
   ESP3D-WebUI 甚至壓成單一 `index.html.gz` 放進 flash。
4. **認證放在 web 層，控制層不管誰在下命令。**
5. **緊急停止一定有，而且是最先被實作的少數幾個端點之一。** 四家全部有。
6. **狀態推送一定包含「連上線先給一份完整的」。** 沒有一家假設 client 從第一則增量就能重建狀態。
7. ★ **全部是 copyleft。GPL-2.0／GPL-3.0／AGPL-3.0，零家 MIT/BSD/Apache。**
   這對我們是硬事實：**這個領域沒有可以直接抄進閉源產品的參考實作。**
   包含前端——Mainsail、Fluidd、DWC、ESP3D-WebUI 全是 GPL，**我們的網頁不能以它們為基礎改**。

### 7.5.3 六個分歧點（沒有共識 ⇒ 是取捨，要看我們的條件選）

1. web server 放韌體內 vs 放旁邊的 host。
2. 狀態用「有版本號的樹」vs「欄位級訂閱 + 時戳」vs「定時全量」vs「文字行」。
   Bambu 同一份物件在 X1 送全量、P1P 送差異，**直白證明這是效能取捨**。
3. 背壓四種都有人用。但注意：**唯一為多 client 網頁設計的那一家（DSF）選了 ACK 串鏈**。
4. **緊急指令是否旁路佇列**。Grbl 系是，其餘否。對高危機台是實質差異。
5. 認證的強制程度。
6. 是否有單一操作權——**幾乎無人做**。

### 7.5.4 這一份對我們的啟示（意見，摘自筆記 §9）

**先說哪幾家不適用，免得白花時間**：

- **Klipper 的核心創新對我們是零價值。** 它的全部價值在「把步進脈衝規劃從弱 MCU 搬到強 host」。
  我們的 PCI 運動卡**就是那顆負責準時的 MCU**，我們的 Windows 行程**就是那個 host**——
  這件事硬體廠商已經幫我們做完了。⇒ 只借 Moonraker 那一層的協定形狀。
- **OctoPrint 整家不能用（AGPL）**，且它的前提是「韌體不能改」，我們的韌體就是我們自己。
  ⇒ 只借它的權限模型與 `throttle` 概念。
- **LinuxCNC 的 RT 架構不適用**（我們沒有 RT 核心也不該引進），但 **HAL 概念高度適用**。
- **Machinekit 是警示案例**：通用化的 HAL-over-網路 做到一半就停了。
  ⇒ 我們的 tag 快照**不要**朝「暴露全部內部變數」的方向長。

**我們現在的做法在這張地圖上不是異類，是主流之一** ★：
行程內嵌 HTTP+WebSocket、單一 socket thread、雙緩衝快照、有界指令佇列——
這就是 **Duet RRF + grblHAL 那一派**。兩家成熟專案都這樣做。
⇒ **不需要因為「別人都用 Node/Python 伺服器」而懷疑這個決定。**
我們的條件（沒有 Linux SBC、要與運動核心共享行程內狀態、要閉源）恰好是這一派的典型條件。

**最值得抄的一件事：把 IO/馬達表提升成「可觀測的 HAL」。**
LinuxCNC 的 pin/signal 模型跟我們的 `IO_Table.csv`／`TMySensor`／`TMySwitch`／`TMyCylinder`
**是同一個東西**，差別在 LinuxCNC 把它做成執行期可查詢、可監看的介面（`halmeter`、`halscope`）。
⇒ 把 tag 命名空間明確定義成「pin 名稱空間」，讓網頁的 IO 監看頁自然等於 `halmeter`。
**這比一頁一頁刻 formview 更省力，對維修現場的價值更高。**

**緊急停止應該學 Grbl，不要學 Moonraker** ★：Grbl 把 `0x18`/`!` 設計成
在接收當下攔截、**永遠不進 buffer**；Moonraker 的 `emergency_stop` 跟一般指令共用佇列。
我們的機台會夾人。⇒ WebBridge 的指令佇列應該有一條旁路。
⚠ 但同時要說清楚：**這仍然只是軟體停止，不能取代硬體安全鏈**。半導體廠稽核看的是後者。

**單一操作權：我們比所有人都嚴，這是對的，不要因為「別人沒做」而拿掉。**
五家沒有一家有強制的單一操作權。但它們控制的是 3D 印表機與桌上型 CNC，
**壞掉的後果是一個報廢件**；我們控制的是會夾 IC、130°C、多軸同動的 handler，
且現場可能有人站在旁邊。⇒ 應保留並強制，且應擴充成「機台端實體鑰匙優先於網頁」。

**SECS/GEM：這五家給不了任何幫助，別去找。**
沒有一家有「上位 host 會問問題、我方有 30 秒回覆預算」的對應物。
⇒ web 層與 SECS 層必須是兩條獨立的路，共用的只有底下的狀態來源。
特別是：**絕不可讓 SECS 回覆路徑上的任何東西等待 web socket thread**——
一個瀏覧器的 TCP 塞住不該變成 SECS T3 逾時。筆記建議把這條寫進架構硬規則。

---

## 8. 對照組：V906 WebBridge 今天長什麼樣（20260920 主迴圈自量）

> 完整盤點 `notes/V_v906_webbridge_inventory.md`。這裡只放對照要用的骨架。

### 8.1 三層與契約

| 元件 | 角色 | 關鍵契約 |
|---|---|---|
| `WebBridge/WebBridgeServer` | 內嵌 HTTP+WS 伺服器，**一條** `select()` 線程 | 永不呼叫機台邏輯；只讀 `TagSnapshot`、只推 `CommandQueue`；預設 loopback＋readOnly；Origin gate；backlog 256 KiB 斷線 |
| `WebBridge/TagSnapshot` | 狀態半邊：雙緩衝 `map<tag,TagValue>` | 發布路徵 UI thread 專用；`commitPublish()` 只做一次 swap＋generation++；每 tick 必須 stage 全集，沒 stage 的就是 removed |
| `WebBridge/CommandQueue` | 指令半邊：有界（64）佇列 | `tryPush()` 永不等 UI thread；滿了立刻 ok:false；`capacity==0` = 唯讀模式 |
| `WebBridgeTags.cpp` ＋ `SetExtraTagPublisher` | 機台與瀏覧器**唯一**相遇處 | 來源沒載入就發 null 不發 0；額外發布者（SECS 741 筆）在 `commitPublish()` 前掛進來 |
| `tools/wb_serve.cpp` | 目前唯一的宿主行程 | HTTP `/api/recipe`、`/api/system`（40 檔可讀寫）、`/api/text`（6 源唯讀）；WS 指令 14 個 |

### 8.2 wire 協定

`snapshot`（連上全量）／`patch`（逐連線 delta，消失→null）／`ack`／`alarm`／`modal`／`query`
（S→B）；`cmd`／`ping`（B→S）。`control.acquire/release` 在 socket thread 直接答，其他指令要持有
操作權（`WebBridgeServer.cpp:1339-1380`）。瀏覧器側契約 `web/js/transport/ws.js:8-19`，重連退避
`[500, 1000, 2000, 4000, 8000, 15000]`。

### 8.2.1 ⚠ 快照的組成：97% 是一個子系統（20260921 查證）

文件前面反覆引用「約 5,349 個 tag／代」。那個數字的**出處與組成**很重要，
因為它直接改變提案的優先序。

| 來源 | 筆數 | 出處 |
|---|---|---|
| `pci1203.*` | **4,481** | `WebBridgeTags.cpp:2162` 註解，MEASURED 20260917（`stagedTagCount()`）；該行自己寫 `Do not hand-edit this number` |
| `build.*` | 10 | 同上 |
| 其他全部 | **117** | 同上 |
| 小計 | **4,608** | 同上；`WebBridgeTags.h:351` 重複同一數字 |
| `secs.sv.*` | 741 | 20260920 `90a1946`，commit 標題寫「線上快照實測」 |
| **合計** | **約 5,349** | ⚠ **兩次獨立量測的加總，不是一次量到的**。要精確數字請重跑一次 wb_serve |

`pci1203.*` 之所以有 4,481 筆，是因為它用**固定上界的迴圈**逐軸、逐 IO 點發：

```
EtherCAT/Pci1203Monitor.h:1538-1540
    kPci1203TagAxes    = 32
    kPci1203TagDiPorts = 128
    kPci1203TagDoPorts = 96
```

每軸 27 個直接樣板（`pci1203.ax%d.opened/state/cmdPos/actPos/driveErr/...`）
再加 `limit.` / `gear.` / `enc.` / `home.` 四個巢狀家族；DI 每埠 6 個、DO 每埠 3 個。
**這些是上界不是實際安裝數**，所以在任何一台真機上大多數應該是 null。

**三個由此得出的推論：**

1. **「5,349 個 tag 很多」這個問題，其實是「一個子系統發了 4,481 個」。**
   機台流程本身（lot、arm、sort、temp、status…）只有 117 個。
   ⇒ 談 P1／P2 的成本時要用這個分母，不要用總數。
2. ★ **這強力支持 P18（tag 命名空間 ＝ HAL pin 名稱空間）。**
   `pci1203.*` 實質上**已經是**一個 HAL pin 名稱空間了——逐軸、逐點、
   欄位固定、名稱可預測。把 `IO_Table.csv`／`TMySensor`／`TMySwitch`／`TMyCylinder`
   也做成同樣形狀，是**延續一個已存在的模式，不是發明新的**。
   LinuxCNC 的 `halmeter` 之於 HAL，正是我們缺的那個通用監看頁之於這批 tag。
3. ★ **這讓 P15（欄位級訂閱）比 P1（分區 seqs）更值錢。**
   一頁 lot status 需要的大約是 10 個 tag，不是 4,481 個運動診斷。
   分區 seqs 能讓瀏覧器「少抓」，但訂閱能讓伺服器「少算、少送」。
   ⇒ 若只做一個，先做 P15。這也呼應 F 份的裁決一傾向（Moonraker 路線）。

### 8.3 今天最大的結構事實：兩條資料面並存

- **WebSocket tag 串流**（wb_serve）：dotted tag、逐連線 patch、約 5,349 tag／代。消費者 3 個檔
  （`background.html`、`page/ht9045_recipe_client.js`、`page/simulator-bridge.js`）。
- **file:// JSON 全量快照輪詢**（Steven 的 `Runtime-bridge-contract`，schemaVersion 1.0.0）：
  巢狀物件、`seq` 單調遞增、atomic replace、250／1,000／2,000 ms 三種檔；消費者 16 個檔／137。
  **目前由同事的 C# `HT9045.JsonSimulator.exe` 餵**，不是 wb_serve（`D:\HT9045_web\docs\STATUS.md` §0，20260914）。
- 69 頁 HTML 手工頁面，每頁自己綁資料。

這一點在 §9 會反覆出現：Duet 的「一個 Object Model、一個 connector 抽象、UI 不知道後端是誰」
正好對著這個裂縫。

---

## 9. 截長補短

### 9.0 如果只做三件事（讀到這裡就夠）

提案表有 **32 條（P1–P32）**。以下是它們的優先序，依據是「風險 × 是否擋住別的事」，不是「多有趣」。
⚠ 本節只排了其中 **20 條**；**P2／P5／P6／P7／P8／P10／P14／P17／P20／P21／P24／P25 這 12 條沒有出現在任何一批**，
其中 **P21（SECS 回覆路徑不得等待 web）是全文唯一「只有我們需要」的硬規則**，優先序未定是本節的缺口。

**第一批：先做，不必問任何人（三條都小、都不可逆風險為零、而且會擋住後面的事）**

> **20260922 第二輪更新**：P26 已定案（§9.1.2），**P22 解除封鎖**。
> 第一批現在是 **P22 → P23 → P35**，其中 P35 是 P26 定案時浮出來的
> （「值未知」與「tag 不存在」在我們這裡撞成同一個 `null`，而 `TagPatch` 的結構本來就是對的，
> 只差 wire 層不要壓扁 ⇒ 小）。
> P23 的**理由要照 §6.4 的更正框改寫** —— 不是「Duet 有結構保證所以抄它」，
> 而是「Duet 只覆蓋了 25 處、手寫了 60 處，連主軸啟停都漏掉，所以**單一元件必須搭配唯一路徑的強制**」。

| | 做什麼 | 為什麼是第一批 |
|---|---|---|
| ~~**P26**~~ **✅ 已定案 20260922** | **定義 patch 的陣列語意** | **答案：一律送整條陣列。** 兩份獨立實作都是長度對齊＋逐格覆寫＋從尾端截斷，送單格會把陣列**截到 1 格**且無錯誤訊息；Duet 安全的前提只寫在一段註解裡。完整推導見 **§9.1.2**。⇒ **P22 的前置已解除，可以開始。** 但同時浮出一條新的必做項 **P35**（「值未知」與「tag 不存在」要分開） |
| **P22** | **網頁端一層純 JS 的中介 model**：兩條資料面都先套進同一棵物件 | 不需要 framework、不需要先決定淘汰哪一條資料面，但 69 頁從此綁同一棵樹。**這是 P3 裡風險最低、不必裁決的那一半。** |
| **P23** | **單一全域凍結閘**：`data-requires="connected,idle,level>=2"` + 一支掃描器 | 現在 69 頁各自判斷斷線／狀態／權限，等於 **69 份互相不一致的安全規則**。機台 HMI 的「不該按」比「不該看」嚴重得多。 |

**第二批：C++ 側。第一條是唯一「解掉一個卡住的戰役」的提案**

> **20260922 第二輪新增進第二批**（都不必裁決）：
> **P34**（重連後的狀態重同步）—— 這是第二輪發現「我們整塊沒做」的一項，
> 而且它跨 `ws.js` 與 Steven 的 wire 引擎兩個客戶端 ⇒ 要一份共用文件＋共用測試。
> **P33**（命令值 `.cmd` 與回讀 `.fb` 分開）與 **P36**（`.part`＋rename）都小；
> P33 **必須與 P9／P18 同一次做**，否則事後要動 4,481 個 tag 名。
> **P37**（jog transaction）與 P12 是同一個結構問題，跟著 P12 走。

- ★★ **P27 `TModalRequest` 物件池**。FW-W5（modal 往返）卡住的原因是 handler 端的
  `ShowMyMessageBox_YES_NO` 是同步阻塞的。RRF 的解法不需要改架構：
  **對話框是獨立物件，不是呼叫堆疊上的一個框**，等待端只設一個 bit 然後讓出。
  **對機台的實質意義**：對話框開著的時候，SECS host 的查詢必須照樣回（30 秒預算），
  面板的急停必須照樣按得下去。**現在的同步 modal loop 兩者都做不到。**
  答案要帶 seq 且廣播，網頁／面板／SECS 三邊都能回答同一個對話框；每個 modal 都要有逾時與預設答案。
- ★ **P28 + P29 資源鎖 ＋ `notFinished`**。仲裁不要做優先權，做鎖就夠。
  鎖的粒度對應物理子系統（`InArm`／`Index`／`TrayArm`／…）。三步：手動與遠端動作改成
  「拿鎖 → 做 → 放鎖」；拿不到回明確的 `busy`；web 與 SECS 看到 `busy` 就重試或報告。
  **光這三步就能消掉大部分「網頁按鈕跟自動流程打架」的類別。**
  前置：我們**沒有統一的「指令完成」收斂點**，那要先補。
- **P9 + P18** 合併做：tag 目錄改成靜態表（`{name, fetch, flags}` ＋ `static_assert`），
  同時把命名空間定義成 HAL pin 形狀。`pci1203.*` 那 4,481 筆**實質上已經是**這個形狀了（§8.2.1-2），
  所以這是延續既有模式。副產品：網頁的 IO 監看頁自然等於 `halmeter`，不必一頁一頁刻。
- **P15**（欄位級訂閱）：優先於 P1（分區 seqs）。一頁 lot status 要 10 個 tag，不是 4,481 個運動診斷。
- **P30** 警報處置變資料：**現有硬編碼原封不動留著當 fallback**，另加一層
  「`ALARM\JAM0610.txt` 存在就跑它」。第一版連腳本語言都不需要。低風險正是因為 fallback 不動。
- **P16**（從程式碼產生 REST/WS 表面清單並與文件比對）、**P4**（`bootId` 重啟偵測）、
  **P31**（訊息目的地位元圖）：都小。
- **P32**（週期性使用者腳本，把客戶碼趕出 C++）：方向性改變、影響面大，值得先討論再排。

**第三批：等使用者裁決才動**（§9.5 的四件事）

- **P11 + P19** 急停旁路。🔴 與 ST 戰役的 S3 武裝同級，**這一步機台會動**，不可先做。
  ⚠ 但形狀已經很清楚了（§7.2）：Duet 與 Grbl **都**是在字元／frame 解析之前就攔截，
  不進佇列。要抄的三個配套是：正常路徑留後備、急停豁免「已停機就拒絕」的閘門、
  **明確列出哪些入口沒有 out-of-band 急停**。停的順序抄 RRF，但我們**不要在急停時關氣源**。
- **P12** 統一命令管線。架構決定。
- **P3** 的 write 側（兩條資料面收斂）。牽涉同事的地盤。
- **裁決一**：狀態走 Duet 路線（狀態樹 + 版本號）還是 Moonraker 路線（欄位級訂閱 + 時戳）。
  ⚠ 本文件有**兩條獨立的證據線**都指向 Moonraker 路線：F 份的橫向比較（§7.5.4），
  以及我們自己快照組成的量測（§8.2.1-3）。但這仍是你的決定。

**明確建議不做的**

- **P13**（變更偵測放 setter）。代價是所有寫入都得經過包裝器，而我們有 655 個
  `//AI(ht9045-v899)` 標記的既有碼直接寫全域；漏推的失敗模式是「畫面靜止」且無從偵測。
  現行「重 stage 全集 + 比對」雖然貴，但**不可能漏**。
- **抄任何一家的程式碼**。查到的五家同類專案 ＋ Duet 全部是 GPL/AGPL，連前端也是（§7.5.2-7）。

---

### 9.1 Duet 的長、我們的短 → 提案表

> 每一條都寫「現況／差距／提案／落點／規模／風險／要不要使用者裁決」。
> 規模用「檔數＋預估行數」，不用百分比。這張表會隨 §4–§7 補入而增修。

| # | 主題 | V906 現況 | Duet 做法 | 提案 | 落點 | 規模／風險 | 裁決 |
|---|---|---|---|---|---|---|---|
| P1 | **分區序號（seqs）** | 逐連線 diff 整張 map，每代 5,349 tag；快照無分區概念 | 頂層 key 各一個 seq，UI 只重抓變了的區（§3.2） | 在 `TagSnapshot` 加「命名空間 generation」：以 tag 前綴（`secs.`、`pump.`、`status.`…）為區，`commitPublish()` 時只對變了的區 ++；wire 加 `{"type":"seqs","data":{ns:gen}}` 或把 seqs 塞進 patch。瀏覧器可以只訂閱幾個區 | `WebBridge/TagSnapshot.{h,cpp}`、`WebBridgeServer.cpp` patch 迴圈、`ws.js` | 2 檔 ~150 行；純新增，讀者不訂閱就沒差。風險低 | 否（純效能／結構） |
| P2 | **live／verbose 分級** | 所有 tag 同一頻率 | 欄位表帶 `live` 旗標，`f` 只回 live（§3.3） | tag 登錄時帶「頻率等級」（live／slow／static），`PublishHandlerTags` 依等級決定每 tick 是否重讀；static（如 741 筆 SECS metadata）只在 snapshot 出現 | `WebBridgeTags.cpp`、`SecsTagPublish.cpp` | 需要一次盤點 110＋741 個 tag 的等級；風險低 | 否 |
| P3 | **一個 model、一個 connector** | 兩條資料面、兩套 schema（§8.3）；16 檔輪詢 C# 模擬器餵的 JSON，3 檔用 WS | DWC 只透過 `BaseConnector`，PollConnector／RestConnector 可換（§3.1） | 在 Steven 的頁面側寫一個 **`HT9045Connector` 抽象**：`FileConnector`（現況，讀 JSON shim）與 `WsConnector`（接 wb_serve tag 串流），頁面只認 connector 的 `onUpdate(model)`。這是把 C# 模擬器退場、卻**不用改 69 頁**的唯一路 | `D:\HT9045_web\client\`（Steven 主導）；C++ 側要補一個把 tag 串流投影成 `Runtime-bridge-contract` 形狀的 adapter（`tools/pagewire/` 已有三元組對照可用） | 前端 ~300 行＋C++ 投影器 ~400 行；風險在兩套 schema 的語意對不齊（null／""／0） | **是**——牽涉同事的地盤與 web repo 分工（`WEB_COLLAB_CYCLE.md`） |
| P4 | **重啟偵測** | wb_serve 重啟 ⇒ 瀏覧器只看到重連 | `state.upTime` 倒退 ⇒ 全量重抓＋取消待決（§3.2） | 快照固定帶 `bridge.bootId`（啟動時隨機）＋ `bridge.upTime`；`ws.js` 看到 bootId 變就丟掉本地 model、清待決 cmd | `WebBridgeTags.cpp`（2 tag）、`ws.js`（~20 行） | 極小；風險無 | 否 |
| P5 | **錯誤分族** | ack 只有 `error` 字串（`control-held`／`not-operator`／WAR 碼混在一起） | 17 個錯誤類別分網路／檔案／登入／指令四族（§3.6） | ack 加 `kind` 欄位：`transport`／`auth`／`control`／`reject`／`handler`（WAR/JAM 碼放 `code`）。瀏覧器據 `kind` 決定重連／要密碼／顯示警報 | `WebBridgeServer.cpp` `AckJson`、`ws.js`、Steven 的 `ht9045_wire_engine.js` | ~60 行；要一次列完現有 error 字串（grep `SendAck(`） | 否 |
| P6 | **背壓** | backlog 超過 256 KiB 直接斷該連線 | 客戶端回 OK 才推下一包（§3.4） | 保留斷線保護，但**在斷之前**先降頻：連線 backlog 超過門檻時該連線改成「只送 seqs，不送 patch」，等它回 `{"type":"ok"}` 再恢復 | `WebBridgeServer.cpp` patch 迴圈 | ~80 行；風險低 | 否 |
| P7 | **PONG 逾時** | 已有 `idleTimeoutMs` 45,000 | Duet **沒有**（§3.4） | 無需動作；記錄「我們這點比 Duet 完整」 | — | — | — |
| P8 | **可調參數表** | `WebBridgeConfig` 已集中；wb_serve 另有十幾個旗標 | 一張 `DefaultSettings` 表、註解含單位與影響（§3.6） | 把 wb_serve 的旗標收進 `WebBridgeConfig` 或一個 `WbServeOptions` struct，每欄註解寫預設、單位、影響誰 | `tools/wb_serve.cpp` | 整理性工作；風險無 | 否 |
| **P9** | **表驅動的 tag 目錄** ★ | `WebBridgeTags.cpp` 2,188 行手寫 emit；110 個 dotted tag 散在程式裡；漏一個沒人知道 | 60 張 `constexpr` 三欄表 ＋ **3 個 `static_assert`** 在編譯期抓表長／描述子／字母序（§4.1） | 把 tag 目錄改成靜態表 `{ name, fetch, flags }`，用 `static_assert` 抓重名與排序；PC 上可用 `std::string_view` + 函式指標。**flags 就是 P2 的頻率等級**，兩案合併做 | `WebBridgeTags.{h,cpp}`、`SecsTagPublish.cpp` | 2,188 行的重構，分波做；每波可用「tag 集合不變」當回歸斷言。風險中 | 否（結構重構，但要先量現有 tag 集合當基準） |
| **P10** | **seqs 的 `++` 放哪裡** | 無 seqs 概念 | 203 個手動 `++`／16 個方法，**漏一處就畫面不更新且無編譯期檢查**（§4.4） | 做 P1 時**不要**學它散到 200 處。V906 的優勢是狀態機集中：在 `SetTask()`／`SetLotState()`／警報發出點等少數 transition 函式內集中 `++`。另 `limits` 那種永不變的東西不給 seq | 同 P1 | 設計選擇，無額外成本 | 否 |
| **P11** | **急停走專用路徑** ★ | 無。web 沒有急停；`fMain->Start()` **32 個呼叫點只閘 3 個**（`tools/start_sites_census.py`，§9.1.1） | `M112` 在 **network task 內直接** `EmergencyStop()`，刻意繞過佇列（§4.5）；DSF 讓 M112 帶 `IsPrioritized` 且用全程式 token「so they survive channel resets」，並**刻意讓預設攔截器看不到它**（§5.5、§5.6） | 若日後 web 要有急停按鈕：**專用 frame（不是 `cmd`）、不進 `CommandQueue`、socket thread 直接呼叫**，且全樹只此一條、明確標記。**這條要跟 S3 武裝一起裁決，不可先做** | `WebBridgeServer.cpp`、機台側 EMO 路徑 | 小，但**不可逆且機台會動** | **是**（🔴，與 S3 同級） |
| **P12** | **統一命令管線** ★★ | 每個指令各自 dispatch 到 golden handler；SECS RCMD／GPIB／網頁／面板按鈕各走各的路 | 六個 stage 的管線，**所有通道共用**；Pre 可否決、Post 可改寫、Executed 可改結果（§5.5、§5.6） | 把 START／PAUSE／jog 等變成同一種命令物件進同一條管線：Pre 放操作權與互鎖、Executed 統一推回狀態。**這是 `fMain->Start()` 32 個呼叫點只閘 3 個的結構解**——不是逐一加 gate，而是讓它們全部經過同一個入口 | `WebStart.cpp`、dispatch 表、未來的 SECS/GPIB 入口 | 大；呼叫點清單**跑 `tools/start_sites_census.py`，不要人工盤**（§9.1.1 三次量錯）。**與 ST 戰役 S1/S3 直接相關** | **是**（架構決定） |
| **P13** | **變更偵測放 setter** | 每 tick 重 stage 全集，再逐連線比對 | `SetPropertyValue` 的 `Equals` 檢查就是 diff，不保存上一版快照（§5.2） | **建議先不做**。代價是所有寫入都得經過包裝器，而我們有 655 個 `//AI(ht9045-v899)` 標記的既有碼在直接寫全域，漏推的失敗模式是「畫面靜止」且無從偵測。現行「重 stage 全集 + 比對」雖然貴，但**不可能漏**。等 P9 表驅動落地後再評估 | — | — | 否（結論是不做，但要記錄理由） |
| **P14** | **同步／非同步指令語意** | ack 一律等 UI thread 處理完才送；START 這種長動作會讓 ack 等很久 | DSF 有 `async=true`；但其同步路徑**沒有逾時**，筆記判定「對機台軟體不可接受」（§5.8-7） | 指令分兩類：短指令維持現行同步 ack；**長指令（START／HOME／CleanOut）ack 只回「已接受」，結果走狀態 tag 與 `alarm`／`modal` frame**。同步路徑加固定上限 | `CommandQueue`／dispatch、`ws.js` | 中；要逐指令分類 | 否 |
| **P15** ★ | **filter／訂閱**（優先於 P1，見 §8.2.1-3） | 每個連線收全部約 5,349 個 tag，其中 4,481 是 `pci1203.*` | XPath 式 filter，`**` 只能放結尾；未選中的集合項目放佔位物件維持索引（§5.3） | 連線可送 `subscribe` 指定前綴清單；伺服器只對該連線算那些區的 delta。**與 P1 同一個資料結構**，一起做 | 同 P1 | 小（P1 做完後） | 否 |
| **P16** | **表面清單從程式碼產生** | `D:\HT9045_web\docs\API.md` 手寫 160 行 | DSF 有 OpenAPI.yaml，**但實測會漂**（GET 寫成 POST、漏列端點、README 提到不存在的設定）（§5.7） | 加一支 gate：從 `ApiRoute()` 與 dispatch 表**產生**端點／指令清單，與 `API.md` 比對，不一致就紅燈 | `tools/`（新腳本）、CI gate | 小；風險無 | 否 |
| **P17** | **權限兩級要真的發** | `auth.level` 對映 golden `AccessLevel`，`control.acquire` 管操作權 | DSF 型別上有 ReadOnly／ReadWrite，**實際只發過 ReadWrite**（§5.7） | 我們的方向比它完整（viewer／operator 分離已在設計裡）。要確認**實作真的有 viewer 級**，而不是跟 DSF 一樣型別存在但從不發 | 驗證工作 | 小 | 否 |
| **P18** ★ | **tag 命名空間 ＝ HAL pin 名稱空間**（§8.2.1-2：`pci1203.*` 實質已是） | tag 名是臨時取的（`pump.*`、`status.*`、`secs.sv.*` 混編），但 `pci1203.*` 4,481 筆已經是逐軸逐點的 pin 形狀 | LinuxCNC 的 pin/signal 模型**就是**我們的 `IO_Table.csv`／`TMySensor`／`TMySwitch`／`TMyCylinder`，差別在它是執行期可查詢、可監看的介面（`halmeter`、`halscope`）（§7.5.4） | 把 tag 命名空間定義成 pin 名稱空間：`io.sensor.<name>`、`io.switch.<name>`、`io.cylinder.<name>.<state>`、`motor.<name>.<field>`，直接由 `IO_Table.csv`／`Mot_Table.csv` 產生。**網頁的 IO 監看頁就自然等於 halmeter，不必一頁一頁刻** | `WebBridgeTags.cpp`（與 P9 同一次做）、網頁一個通用監看頁 | 中；tag 集合會大幅擴張，要搭 P15 訂閱一起做，否則快照爆掉。⚠ **20260922 修訂：這條的依據要改，見 §9.1.3 —— Duet 沒有可執行期查詢的 pin 目錄，抄不到** | 否 |
| **P19** | **急停旁路（精修 P11）** | 無 | **Duet 其實也是旁路的**（§7.2 更正了 §7.5.3-4 的表層印象）：`M112` 在**字元寫進環形緩衝之前**就被 3 狀態機掃出來，直接 `EmergencyStop()`，不排隊不等鎖。Grbl 的 `0x18` 同理 | socket thread 在解析 frame 之前先掃一次 `estop`，命中就直接呼叫既有急停入口，**不進那 64 格佇列**。理由：佇列滿了、UI thread 卡住、handler 正在等鎖，**這些正是最需要急停的時刻**。三個配套一起抄：正常路徑留作後備；急停豁免「已停機就拒絕指令」的閘門；**明確列出哪些入口沒有 out-of-band 急停**。⚠ 停的順序抄 RRF，但我們的對應是**不要在急停時關氣源**，夾持中的 IC 會掉 | 同 P11 | 同 P11 | **是**（併入 P11 裁決） |
| **P20** | **client 自報 throttle（精修 P6）** | backlog 超過 256 KiB 直接斷線 | OctoPrint 讓 client 送 `throttle` 訊息指定倍率，慢的 client 自己降頻（原文已抽驗）；DSF 用兩層 ACK。**OctoPrint 這招對單 socket thread 架構更合適**（§7.5.4） | 加一個 `throttle` 指令讓 client 說「我每 N 個 tick 收一次」。比 ACK 串鏈簡單，且不需要伺服器等待 | `WebBridgeServer.cpp`、`ws.js` | 小（比 P6 的 ACK 版更小） | 否 |
| **P22** | **HMI 端中介 model** ★（P3 的最小可行形） | tag 串流是扁平 dotted key、JSON 快照是巢狀物件，**沒有共同中介表示**，69 頁各自解字串 | DWC 保有 `typedState`（解析與型別）與 `state`（被觀察）兩份，把「資料怎麼來」跟「UI 怎麼看」解耦（§6.2、§6.7-1） | 在網頁端做一層純 JS 的 HMI model：兩條資料面都先套進同一棵物件，69 頁綁那棵樹。**不需要任何 framework**，也不需要先決定要不要淘汰哪一條資料面 | `D:\HT9045_web\client\` | 前端 ~200 行；**這是 P3 裡風險最低、可以先做的那一半** | 否（P3 的 write 側才要裁決） |
| **P23** | **單一全域凍結閘** ★ | 每頁自己判斷斷線／狀態／權限，等於 69 份互相不一致的安全規則 | DWC 所有送 code 的按鈕都經過同一個 `CodeBtn`，`uiFrozen` 硬編在 `disabled` 上 ⇒ **結構保證**（§6.4） | 一個 `data-requires="connected,idle,level>=2"` 屬性 + 一支掃描器，把三件事收斂成一處 | `D:\HT9045_web\client\` | 小；**機台 HMI 的「不該按」比「不該看」嚴重得多** | 否 |
| **P24** | **對話框帶 `seq`** ★ | `modal`／`query` frame 有 `qid`，但 `modal` 沒有；`ShowMyMessage` 是機台端阻塞式 | Duet 的 `state.messageBox` 有 `seq`，換頁／重連／多瀏覧器同時開都對得上（§6.6） | FW-W5 modal 往返落地時，**一開始就加 `seq`**，不要事後補。另兩個低成本設計值得抄：持久 modal 半秒後浮出急停鈕；dialog 內嵌 jog（bitmask 指定軸）對「請手動把手臂移開再按確定」直接可用 | `WebBridgeServer.cpp`、`ws.js`、FW-W5 | 小（若一開始就做） | 否 |
| **P25** | **路由表是資料** | 69 頁 HTML 沒有共同外框 | DWC 的 `Menu` 是可觀察的資料結構，router 從它生成，`condition` 是 getter 所以選單隨機台狀態自動增減，整套 371 行（§6.5） | 把 69 頁清單變成一份 JSON（路徑、標題、顯示條件、需要哪些 tag），先用它產生側邊選單與權限灰化。**這一步不改任何一頁的內容** | `D:\HT9045_web\` | 小；**立刻拿到「哪一頁在什麼機種／狀態下該出現」的單一出處** | 否 |
| **P26** | ⚠ **陣列 patch 語意** ★★ **（20260922 已定案，見 §9.1.2）** | 未定義 | **兩份獨立實作都是長度對齊 ＋ 逐格覆寫 ＋ 從尾端截斷**（TS 手寫一份、C# source-generated 一份）。前提「韌體永遠送整條陣列」**只寫在一段註解裡**（`RepRapFirmware/src/ObjectModel/ObjectModel.cpp:1096-1099`），沒有型別或編譯期檢查守住 | **決定：我們的陣列 patch 一律送整條。** 逐連線 diff 若只算出單格變更，發布端必須把該陣列**整條補回**再送。理由與完整推導見 **§9.1.2** | wire 契約 ＋ `TagSnapshot` 的 diff 發布端 | 零成本（現在定義）／很貴（事後才發現） | 否（**已定案**，P22 可以開始） |
| **P27** | **`TModalRequest` 物件池** ★★（解掉 FW-W5 的卡點） | FW-W5 modal 往返做不完，因為 handler 端是同步阻塞的 `ShowMyMessageBox_YES_NO`（`WEBBRIDGE_WRITEPATH_DESIGN.md` §4） | RRF 的 `MessageBox` 是**獨立物件不是呼叫堆疊上的框**：有 seq、mode、timeout、預設答案，掛在上限 8 的串列上，生命週期與任何函式呼叫無關；等待端只設一個 bit 然後讓出（§7.5） | 建一個 `TModalRequest` 物件池；`ShowMyMessageBox_YES_NO` 的呼叫點改成「建立請求 + 回傳 `notFinished`」而不是進 modal loop。答案帶 seq 且**廣播**，讓網頁／機台面板／SECS host 三邊都能回答同一個對話框。**每個 modal 都要有逾時與預設答案** | `ShowMyMessage` 呼叫點、`WebBridgeServer.cpp`、`ws.js` | 中；但**這是 FW-W5 唯一的解**。現在的同步 modal loop 讓「對話框開著時 SECS 照樣回、面板急停照樣按得下去」兩件事都做不到 | 否（設計已明確） |
| **P28** | **資源鎖，不要優先權** ★（P12 的仲裁半邊） | 網頁／SECS／GPIB／面板按鈕沒有統一仲裁 | RRF **沒有優先權排程**，只有 round-robin ＋ 資源鎖；鎖無等待佇列、無鎖計數、拿不到就回 false 下輪再試；防死鎖只靠「編號遞增取得 + 取不到就釋放較低號」（§7.3） | 鎖的粒度對應物理子系統：`InArm`／`OutArm`／`Index`／`InShuttle`／`OutShuttle`／`TrayArm`／`HotPlate`／`AutoClean`／`Recipe`。三步：手動／遠端動作改成「拿鎖→做→放鎖」；拿不到回明確的 `busy`；web/SECS 看到 `busy` 就重試或報告。**光這三步就能消掉大部分「網頁按鈕跟自動流程打架」** | dispatch 表、各 handler | 中。**前置：我們沒有統一的「指令完成」收斂點，要先補** | 否（但與 P12 一起規劃） |
| **P29** | **`notFinished` 進 handler 簽章** | handler 多半是「做完才回」或「原地等」 | RRF 的 `notFinished` 是整個架構的關鍵字：dispatcher 不從佇列拿掉，下 tick 再呼叫同一個 handler（§7.4） | 加 `notFinished` 語意。⚠ **關鍵細節：`notFinished` 路徑必須提早 return、不走 `UnlockAll`**——重試期間鎖要保留，否則等鎖的指令會被插隊永遠等不到。比「開一條執行緒等」安全得多 | dispatch 表、handler 簽章 | 中；與 P28 綁在一起做 | 否 |
| **P30** | **警報處置變資料** ★ | 處置硬編碼在 C++，說明來自 `.dat` | RRF：事件佇列 ＋ 四元組去重、`if (巨集存在) 跑巨集 else 跑內建預設`、處置在專屬通道跑、`isBeingProcessed` 留在佇列頭所以處置期間仍去重（§7.6） | **現有硬編碼原封不動留著當 fallback**，另加一層「`ALARM\JAM0610.txt` 存在就跑它」。第一版連腳本語言都不需要，只要一個動作清單格式（關真空／停軸／提示／暫停／繼續） | 警報發出點、新的處置步序機 | 中；**低風險因為 fallback 不動** | 否 |
| **P31** | **訊息目的地做成位元圖** | 警報文字／操作提示／log 的路由散在各處 | 一個 `uint32_t`：低位是目的地位元圖（與通道同序），高位是語意旗標與 2-bit log 級別；`responseMessageType` 是 **const**，通道建立時釘死不可能改錯（§7.7） | web 橋接、SECS、GPIB、UI log 共用同一個 `TMsgDest` 位元圖，`SendMsg(dest, level, text)` 一個入口。⚠ 位元順序與通道列舉順序耦合，要加 `static_assert` 鎖死 | 訊息發送路徑 | 中 | 否 |
| **P32** | **週期性使用者腳本**（`daemon.g` 的對應） | 客戶碼 gating 散在 C++ 裡（超豐／KYEC／ASE 各自的 `if`） | `daemon.g` 每 10 秒跑、檔案不存在不報錯、**只在通道真的閒著時才跑**、擴充板模式下停用（§7.6） | 客戶自訂週期檢查走外部腳本。價值在**把客戶碼從 C++ 裡趕出去**——現在的 gating 正是這個問題的病徵。⚠ 兩個防護要一起抄：只在真的閒著時跑、檔案不存在不報錯 | 新的週期執行點 | 中；方向性改變，要先有 P30 的動作清單格式 | 否（但影響面大，值得先討論） |
| **P21** | **SECS 回覆路徑不得等待 web** ★ | 未明文規定 | 五家同類專案沒有一家有「上位 host 有時限」的對應物——**這件事只有我們有**（§7.5.4） | 寫進架構硬規則：**SECS 回覆路徑上的任何東西都不得等待 web socket thread**。一個瀏覧器的 TCP 塞住不該變成 SECS T3 逾時。現行契約（socket thread 永不呼叫機台邏輯、backlog 超限就斷線）已經滿足，但要**明文寫成不可退讓的規則並加測試** | `WEBBRIDGE_WRITEPATH_DESIGN.md`、一個回歸測試 | 小；主要是文件與測試 | 否 |

#### 20260922 第二輪新增的提案（P33–P37）

| # | 主題 | V906 現況 | Duet 做法 | 提案 | 落點 | 規模／風險 | 裁決 |
|---|---|---|---|---|---|---|---|
| **P33** | **命令值與回讀必須是兩個 tag** ★ | 未區分。`pci1203.*` 4,481 筆裡哪些是命令值、哪些是硬體回讀，沒有在命名上表達 | `state.gpOut[i].pwm` 其實是 `lastPwm`（命令值），`gpIn` 是輪詢 —— **它沒有區分，這是缺點不是優點**（§9.1.3） | tag 命名強制帶後綴：`…​.cmd` 與 `…​.fb`（feedback）。**只有其中一個的，缺的那個發 null 而不是把命令值當回讀**。網頁對「只有 cmd 沒有 fb」的點要視覺上區分 | `WebBridgeTags.cpp`（與 P9／P18 同一次做）、監看頁 | 小（若與 P9 同時做）／若事後補要動 4,481 個名字 | 否 |
| **P34** | **重連之後怎麼知道自己還是對的** ★★ | 只有重連退避 `[500,1000,2000,4000,8000,15000]` ＋ 45,000 ms idle timeout。**重連後的重同步、失效偵測、待決指令處置全部沒有** | 主動 PING/PONG（2 秒門檻）、重連清 `_seqs` 觸發全量重取、`upTime` 回頭偵測韌體重開、**重連時把所有未回覆指令全部取消**、逾時分三級（§9.1.4） | 四件一起做：(1) 主動心跳門檻，不要被動等 45 秒；(2) 重連後強制全量 snapshot（我們本來就是全量，成本低）；(3) `bridge.bootId` 變了就丟掉本地 model（**這就是 P4，優先序提高**）；(4) **重連時 reject 所有待決 cmd**，不重送 | `WebBridgeServer.cpp`、`ws.js`、Steven 的 wire 引擎 | 中；跨兩個客戶端 ⇒ 要**一份共用文件＋共用測試** | 否 |
| **P35** | **「值未知」與「tag 不存在」要分開** ★ | **撞在一起**：wire 把 removed 映成 `null`，而「來源沒載入」也發 `null` | Duet 分得開：欄位**不出現** ＝ 保留舊值；`present-and-null` ＝ 真的是 null（§9.1.2） | 三態要各有表示：`null` ＝ 值暫時未知（溫控斷線）／`patch` 裡**不出現** ＝ 沒變／**明確的 removed 清單** ＝ 這個 tag 不存在了（換機種少一組 site）。⚠ SECS host 尤其需要這個區分 | `TagSnapshot`（`TagPatch` 已有 `removed` 欄位，是 wire 層把它壓成 null）、`ws.js` | 小（`TagPatch` 的結構已經是對的，只差別在 wire 層不要壓扁） | 否 |
| **P36** | **落盤改成 `.part` ＋ rename** | 「備份→驗證→刪備份」（20260918 裁決） | **Duet 的設定儲存路徑完全沒有保護**：`OpenMode::write`＝`FA_CREATE_ALWAYS`（開檔就清空舊內容），寫失敗還 `DeleteSysFile` 把舊設定一起弄丟。上傳路徑才有保護（`.part` ＋ 長度 ＋ CRC32 ＋ rename）（§9.1.4） | **不要放鬆現行做法** —— 我們已經比它強。兩個加強：(1) 驗證時**也比對預期長度**（抓「寫的人算錯要寫多少」）；(2) 改成 `.part` ＋ rename ——**Windows 的 `MoveFileEx(MOVEFILE_REPLACE_EXISTING)` 是原子的，我們的原語比 Duet 的 FAT `f_rename` 好**（它甚至要先刪目標）。副作用：不留備份垃圾，正合 20260918 的「驗證通過就刪備份」 | `wb_serve.cpp` 的 `/api/system` 寫入路徑、`realfile_guard` | 小；風險降低 | 否 |
| **P37** | **jog 是兩端都認的 transaction** ★ | 網頁 jog（若做）與自動流程共用座標模式，沒有 push/pop | `M120\nG91\nG1 <軸><±步距> F<feedrate>\nM121`，而**韌體為它開了具名例外**：對話框等確認時所有指令被擋，只放行狀態查詢、`M292` 與 `M120/M121`，註解直接寫 `// DWC sends M120 G91 G1 ... M121 to jog axes`（§9.1.4） | jog 送出的是**一個不可分割的包**，C++ 側 dispatch 要認得它並整包處理。⚠ **最重要的一條**：閘門（對話框／警報／單一操作權）必須**整包擋或整包放**，**絕不可只擋中間那行** —— 那會留下沒有 pop 的 modal 狀態，**比整包擋掉更糟** | dispatch 表、未來的 web jog | 中；與 P12 同一個結構問題 | 否 |

（§6–§7 進來後預期再新增：DWC 的 store／router／panel 組織 vs 我們 69 頁手工 HTML；
M291/M292 對話框往返 vs 我們的 `modal`／`query`／`modal.answer`；
RRF 事件→巨集（實際路徑是 `0:/sys/heater-fault.g` 這種**平放**檔名，
這份 3.7.0-rc 的碼裡**沒有 `events/` 子目錄**——見 `notes/E_rrf_command_layer_and_safety.md:533`）
vs 我們硬編碼的警報處置；
多通道公平排程與資源鎖 vs 我們的 UI tick drain。）

### 9.1.1 P12 的輸入：全樹「能啟動機台」的呼叫點 census（20260921）

提案 P12 說「不是逐一補 gate，而是讓它們全部經過同一個入口」。要做那件事，
第一步是知道「它們」到底有幾個。**這張清單被量錯三次，第三次是這一輪的我。**　**最新（20261001）：34 個／活 32／閘 2**（St02 MR !33 接上 GPIB 遠端 START）——數字一律以 `tools/start_sites_census.py` 為準，下面的舊數字是當時的量測。

**三次錯誤**（寫出來是因為它們共同證明了 P12 的價值）：

1. 20260915 的盤點（`WebStart.h`）說「約 19 個呼叫點，其中只有 2 個在 `#if 0` 裡」，
   並把 `SECSGEM/uHGemHT9045.cpp:4722` 列為**活的第一個例子**。實際上它從
   `0b95c18`（20260809，PT-W5b）起就在 `#if 0 // GATE G01` 內，**比那份盤點早五週**。
   會看漏的原因很具體：**GATE G01 橫跨 212 行**（4521-4733），往回目視找「最近的 `#if`」
   只會看到 4182 那個已經 `#endif` 掉的。
2. 同一份盤點列出的**每一個行號都已漂移**。
3. **20260921 我自己重量時又漏了 11 個。** 我只 `git grep 'fMain->Start('`，
   結果只命中那些「碰巧在行尾帶了 `// golden fMain->Start("...")` 註解」的
   `W7C1_FMAIN_START(...)` 呼叫（`DoCleanOutFinishCheck` 那 6 個）。
   沒帶那行註解的 `DoOneCycleFinishCheck` 5 個與 `DoART_AfterCleanOut` 6 個
   **完全沒出現在結果裡**。我因此在文件與 commit 訊息裡寫了「21 個」，那也是錯的。

**⇒ 第三次就改機制**（記憶 `same-trap-thrice-fix-the-tool`）。
量法現在是一支工具，不是一條 grep：

```bash
python HT9011UC_Cpp_V3.33.906.0/tools/start_sites_census.py
python HT9011UC_Cpp_V3.33.906.0/tools/start_sites_census.py --check 34 30 4   # AI(W906-R80) 20260927: 原為 33 30 3（HandlerGpibMsg.cpp:717 被 #define W906_REMOTE_START_WIRED 0 閘住，工具原本只認 #if 0）；ctest START_SitesCensus。AI(W906-SENSORSCAN) 20260924: 原為 32 29 3；cSensorScan.cpp 照翻加入一個活的 fMain->Start("AMR")（golden main.cpp:14251）
```

它做兩件目視做不到的事：同時比對直接呼叫與包裝巨集且**去掉註解後才比對**；
對每個候選**整檔線性累積 `#if 0` 深度**，而不是往回找最近的 `#if`。
`--check` 形式把今天的數字釘住，任何人新增或移除啟動路徑就紅燈。
**這支工具第一次跑就抓到上面第 3 個錯誤**，所以這個機制是划算的。

**正確結果（工具產出）**：生產碼 **32 個**呼叫點（`tests/` 另有 1 個不計），
**3 個**被閘住、**29 個是活的**。

| 群組 | 位置 | 觸發原因 | 數量 |
|---|---|---|---|
| SECS/GEM host RCMD | `SECSGEM/uHGemHT9045.cpp:5810 :5815 :7882 :7900 :7913` | host 下 RCMD 就能啟動；裸的 `if(SystemStart==false)` 包著，沒有 gate | 活 5 |
| SECS/GEM host RCMD | `SECSGEM/uHGemHT9045.cpp:4722` | 同上 | **閘（GATE G01）** |
| Clean-out 自動重啟 | `csystem.cpp:5807 5821 5914 5967 6003 6073` | `DoCleanOutFinishCheck` 1-6 | 活 6 |
| One-cycle 自動重啟 | `csystem.cpp:7275 7368 7937 7966 7970` | `DoOneCycleFinishCheck` 1-5 ★ **兩次人工盤點都漏** | 活 5 |
| ART 清機後重啟 | `csystem.cpp:8601 8622 8641 8647 8664 8675` | `DoART_AfterCleanOut` 1-6 ★ **兩次人工盤點都漏** | 活 6 |
| InArm 取料錯誤重啟 | `ainarm9045.cpp:10717 10735 10796 10812` | `ProcessMES0101InArmPickLoaderError` 1-4 | 活 4 |
| AGV／自動化 | `Automation/uRENESAS_Server.cpp:1590 :1831`、`Automation/automation.cpp:1981` | FTCT_50、`ProcessBuffer` | 活 3 |
| 遠端指令通道 | `Command.cpp:16305 :17056` | RemoteControl Start、TCP `HTSET,333` | **閘 2（SAFETY-GATE W906-FW-CMD-C）** |

中間三群都經 `W7C1_FMAIN_START` 巨集。**沒有任何一個呼叫點使用回傳值**
（對全部 32 個搜 `if/while/return/=/&&/||/!` 帶 Start 呼叫的樣式，命中 0 筆）。

**另一條繞道**：`SoftStart = true` 全樹 10 處。
⚠ 但**那不是 10 條啟動路徑**——它是一個共用的加速斜坡旗標。逐一讀過的分類：

| 位置 | 是什麼 | 算不算啟動路徑 |
|---|---|---|
| `SECSGEM/uHGemHT9045.cpp:5504 :5538 :7837 :7919` | S2F42 host command 遠端啟動 | **是** |
| `WebStart.cpp:3656 :3667 :3690` | 我們自己翻譯的 `StartFromWeb` 落點 | 就是那個落點本身 |
| `csystem.cpp:16789 :16813` | `AccelateTask` 斜坡（慢速 10 秒 → 換生產速度） | 否，不是冷啟動 |
| `forms/fMain.cpp:964` | 馬達上電後的 HOME 序列（下一行就是 `iHome=1`） | 否 |

⇒ **工具只會數，語意要人讀。** 引用這個數字時務必連分類一起引。

**結論不變**：override `TfMain::Start()` 會一次武裝 29 條路徑，所以
`StartFromWeb()` 刻意不是 override。這個判斷完全存活，漂掉與漏掉的只是證據。
`WebStart.h`、`CLAUDE.md` 已就地更正並改指向工具。

⇒ **P12 的價值因此更清楚**：逐一裁決 29 次本身是個壞形狀，而且光是
「維護一張正確的呼叫點清單」這件事，我們已經連錯三次。統一管線把
「29 個入口」變成「1 個入口 + 29 個來源標記」，清單就不必用人工維護。

### 9.1.2 P26 定案：陣列 patch 語意（20260922）

> 證據：`notes/H_dsf_sourcegen.md`（161／161 PASS）、`notes/I_objectmodel_remainder.md`、
> `notes/K_dsf_remainder.md`。這一節取代 §6.7-4 的初步判讀。

**全 Duet 只有兩份 patch 套用演算法**（K 份更正了「三份」的前提：`DuetHttpClient`
不自己實作，它呼叫 `DuetAPI.ObjectModel`）：TypeScript 手寫一份、C# 由
`DuetAPI.SourceGenerators` 產生一份。**兩份的陣列語意相同**，產生出來的碼是固定三段：

| 段 | 產生的碼 | 效果 |
|---|---|---|
| 1 | `int newCount = jsonProperty.Value.GetArrayLength();`（`UpdateFromJson.cs:165`） | 新長度**就是 patch 裡那個陣列的長度** |
| 2 | `for (i = 0; i < Math.Min(X.Count, newCount); i++)`（`:166`） | 重疊區逐格覆寫（`:197` 比對不同才寫回） |
| 3 | `for (i = X.Count; i < newCount; i++) X.Add(...)`（`:216`） | 不夠長就補尾 |
| 4 | `while (X.Count > newCount) X.RemoveAt(X.Count - 1)`（`:260`／`:263`） | **太長就從尾端刪掉** |

**⇒ 送不完整的陣列，接收端會把它截短到 patch 的長度，而且沒有任何錯誤訊息。**

**具體災難（用我們的東西舉例）**：6 個測試站，`arm.site` 是 6 格。第 1 站狀態變了，
逐連線 diff 只算出這一格，於是送 `{"arm":{"site":[{"status":"TESTING"}]}}`
⇒ `newCount = 1` ⇒ `while (6 > 1) RemoveAt` ⇒ **第 2～6 站從畫面消失**，
操作員以為機台只有一站。

**Duet 為什麼不出事**：三條保證，**全部是約定，沒有一條由型別系統或編譯期檢查守住**：

1. 只有「**根陣列**」會分塊（`ObjectModel.cpp:1100` 的 `isRootArray`），
   巢狀陣列與所有純量陣列永遠完整送出。**這條最關鍵，而它只存在於一段給客戶端看的註解裡**（`:1096-1099`）。
2. 分塊時緩衝區用一半就 `SetNextElement` 並在回應帶 `next`，接收端 `last = (next == 0)`，
   **只在最後一塊才截短**（`StaticModelCollection.cs:151`）。
3. 唯一會被腰斬（限長，非分塊）的巢狀陣列是 `move.axes`，上限 `MaxReportedAxes = 5`，
   而**這個上限自己被發布在物件模型裡**（`limits.reportedAxes`），客戶端比對筆數後主動改走單獨查詢。
   ⇒ **值得抄的一條：任何對快照陣列的上限，本身必須是一個 tag**，否則客戶端會靜默顯示短清單。

**★ null 語意：我們跟 Duet 是相反的，這比截斷更容易被忽略。**

| 情況 | Duet | 我們今天 |
|---|---|---|
| 欄位**不出現** | **保留舊值** —— 這是它唯一的「不動」表達式 | 不適用（每 tick stage 全集） |
| `present-and-null` | 真的是 null（可為 null 的型別設 null；不可為 null 的型別**丟例外中止整個 patch**） | 「值未知／未安裝」 |
| tag 消失 | 陣列沒有「空洞」表示法，**移除一格只能靠送一條變短的整陣列** | **也映成 `null`** |

⇒ 我們把 removed 映成 `null`，於是「**值暫時未知**」（溫控斷線）與
「**這個 tag 不存在了**」（換機種少一組 site）**撞成同一個表示**，SECS host 只能靠猜。
**這是 P26 定案之外要一起處理的第二件事**（新提案 **P35**）。

**🔴 順帶抓到 Duet 自己的地雷（不要抄）**：兩份產生實作的**例外保護不對稱** ——
`UpdateFromJsonReader` 的集合分支**完全沒有 try/catch**（唯一的 try 在純量分支），
而 DOM 版有。⇒ 同一筆壞資料走 DOM 路徑被記 log 後繼續、走串流路徑讓**整個模型更新中止**。
而且 DOM 版對 `float[]`/`int[]` 元素用 `!=`（參考比較）而 Reader 用 `SequenceEqual`（值比較）
⇒ DOM 路徑每次更新都重寫該格、每次都發變更通知。

**⇒ 我們的決定：一律送整條陣列。** 三個理由：
1. 我們已經每 tick stage 全集，整條陣列本來就在手上，補回去是零額外成本；
2. 「部分更新」在 JSON 陣列裡**沒有正確的表示法**（沒有空洞，而 null 已被用掉）；
3. 兩份成熟實作走同一條路，而它們的差異點（例外保護、相等比較）正是分歧會帶來的代價。

**⇒ 分塊要在邊界處消化掉，不要讓模型層看到分塊。** K 份量到三套分塊重組，
其中 C# 的**逐塊套用進活模型**（`PollConnector.cs:368`），讀者會看到
「前半新、後半舊」的陣列 —— `lock (Model)` 只保護單次套用、不保護跨塊原子性。
TS 的先 `concat` 完才 `onUpdate`。**我們的雙緩衝快照本來就避開了這件事，
不要為了省記憶體改成逐塊就地更新。**

### 9.1.3 P18 修訂：Duet **沒有**可執行期查詢的 pin 目錄（20260922）

> 證據：`notes/J_rrf_hal_storage.md`（593／593 PASS）。

P18 原本的依據是「LinuxCNC 的 pin/signal 模型就是我們的 `IO_Table.csv`，
差別在它執行期可查詢」。第二輪去讀 RRF 的 `src/Hardware`／`GPIO` 之後，
**這條依據對 Duet 不成立**：

- Duet 的 pin 抽象是三層：`Pin`（MCU port+bit）→ `LogicalPin`（pin 表索引）→ `IoPort`（持有者物件）。
  `IoPort` **只有 2 bytes**（一個索引 ＋ 四個 1-bit 旗標），由 `static_assert` 釘死。
- 每個 pin 的執行期狀態在兩個長度 `NumNamedPins` 的靜態陣列：
  `PinUsedBy portUsedBy[]`（16 值列舉）與 `int8_t logicalPinModes[]`。
- ⛔ **但 `portUsedBy` 與 `PinTable[]` 都不進物件模型**（`git grep`：`portUsedBy`
  只出現在 `IoPorts.{h,cpp}`）。唯一的執行期查詢是 per-port 的、回一個人類可讀字串
  （`AppendPinName`／不帶參數的 `M950`）。**這不是 `halcmd show pin`。**

⇒ **Duet 的等價物是編譯期的 `constexpr PinTable[]` ＋ static_assert**
（`NumNamedPins == NumRealPins + NumVirtualPins` 之類）。
也就是說：**我們執行期可編輯的靜態表，在它那裡是編譯期表＋斷言。**
⇒ 「抄 Duet 做 halmeter」這條路**不存在**。P18 若要做，要嘛自己設計、要嘛照 LinuxCNC，
但**不能寫成「Duet 這樣做」**。這反而讓 P9（靜態表 ＋ `static_assert`）更有依據 ——
那一半 Duet 確實做了。

**可以直接拿的四件：**

1. ★ **`PinUsedBy::temporaryInput`** —— 「讀一個點的值但**不擁有它**、不掛中斷」。
   那正是監看頁該有的存取層級，`IoPort` **完整實作了，而全樹零呼叫者**。
   ⇒ 現成的設計，連「為什麼需要這一級」都幫我們想過了。
2. **`seqs.volChanges` 那種單調計數器**：近乎靜態的目錄（IO 清冊）只在變了才重抓，
   客戶端比一個整數。⇒ 這是 P1／P15 在「IO 目錄」這個子問題上的最小版本。
3. **分級**：IO 的**值**放高頻集合、IO 的**目錄**放低頻集合（Duet 用 `liveNotPanelDue` 旗標）。⇒ 即 P2。
4. **`OBJECT_MODEL_FUNC_IF(isMounted, ...)`**：未掛載的磁碟**沒有那個欄位**，
   而不是顯示一個誤導的 0。⇒ 與我們「來源沒載入就發 null 不發 0」同族，
   但它更進一步：**連欄位都不出現**。

**★ 負面教訓（新提案 P33 的來源）**：`state.gpOut[i].pwm` 是 **`lastPwm`，
也就是「我們命令它的值」，不是硬體回讀**；`gpIn` 也是輪詢的（作者自己註記中斷是未來目標）。
⇒ **「畫面顯示 ON 而線圈其實是死的」比沒有畫面更糟。**

### 9.1.4 第二輪的其餘結論（20260922）

> 證據：`notes/G_dwc_cnc_panels.md`、`notes/J_rrf_hal_storage.md`、`notes/K_dsf_remainder.md`。

**(a) ★ 急停旁路做在「接收端的最底層」，不是發起端。這句話要改。**
瀏覽器側**沒有**任何旁路：`EmergencyBtn` 本身就是一個 `code-btn`（送 `M112\nM999`），
連它都被 `uiFrozen` 凍結，走的是跟 jog 完全相同的 `sendCode` 路徑。
旁路在**韌體**：RRF 在字元寫進環形緩衝之前就掃出 `M112`（`GCodeInput.cpp:116`），
完全繞過 `GCodeBuffer` 與佇列，且 `M112`／`M999` 被排除在「已停機就拒絕指令」的閘門外。
⇒ 對 **P11／P19** 的含義比原本明確：
**既不要在網頁按鈕上開 `bypassQueue: true`**（發起端什麼都繞不過），
**也不要把守衛放在 `disabled` 上**（另外 31 個呼叫點不受管）。旁路要做在 socket thread 的解析之前。

**(b) 「軸數不寫死」是怎麼做到的 —— 這是 69 頁那個問題的機制答案。**
`CNCAxesPosition.vue` 整檔**沒有 X/Y/Z 字樣**，只有 `move.axes.filter(axis => axis.visible)`，
`:key` 用 `axis.letter`。`visible` 是韌體的**派生值**（`GetVisibleAxes()`）不是儲存旗標，
由 `M584` 維護。步距的資料布局是 `Record<軸字母, number[]>` **外加一個 `default` 鍵**，
取值找不到就退回 `default`，而「每行幾顆按鈕」＝ `default.length`
⇒ **多一根 U 軸不需要新增任何設定**。0 軸時整塊內容 `v-show` 消失並顯示專屬提示。
⚠ 唯一寫死 XYZ 的地方是**未連線時的佔位模型**。
⇒ 餵 **P22／P25**：中介 model 的形狀要讓「有幾個站／幾根軸」成為資料。

**(c) ⚠ 兩個要避開的實作坑（Duet 自己踩的）。**
- **隱藏耦合**：前端拿 `filter` 後的索引去查 `sensors.endstops[axisIndex]`，
  **只因為 `visible` 是「前 N 根」這個前綴性質才對齊**。若韌體改成任意隱藏集合，
  這一行會查到錯的 endstop **而不報錯**。⇒ 我們的站號／吸嘴號映射不要依賴「連續前綴」。
- **死的 watcher**：`CNCMovementPanel` 有 `watch: { isConnected() {...} }`，
  但 computed 裡**沒有** `isConnected`。Vue 2 watch 一個不存在的屬性
  **不報錯、永不觸發** ⇒ CNC 畫面斷線時對話框不會被關掉（FFF 版有，CNC 版漏了）。
  ⇒ 我們做 P22／P23 時，「斷線要收拾什麼」要有一個**會失敗的測試**，不能只靠寫了 watcher。

**(d) 客戶端失效偵測：Duet 兩個官方客戶端不一致，其中一個有安靜的 bug。**
`bootId` 這個概念在 Duet 客戶端**不存在**（grep 零命中），它用 `state.upTime` 的單調性。
★ C# 客戶端用 `newSeq > seq`，而板子重開後 seqs 從 0 重算 ⇒ `3 > 57` 不成立 ⇒ **不重取，
永遠停在舊值**；TS 客戶端用 `!==` 所以正確，而且只有 TS 有 `upTime` 兜底。
退避方面**我們比它好**（它是固定延遲：C# 250 ms、TS 2000 ms，長時間離線等於在打洪水）。
⇒ **P4（`bootId`）比 Duet 的 `upTime` 做法更明確，保留並提高優先序**（併入 P34）。
⇒ ★ 我們有 `ws.js` 與 Steven 的 wire 引擎**兩個客戶端角色**，
Duet 的教訓是「同一個協定的兩個客戶端會安靜地不一致」⇒
**「什麼情況要丟掉手上的快照」必須寫成一份文件、兩邊照著實作、而且要有共用測試。**

**(e) P17 的對照更新：DSF 的權限系統是宣告性的，不是強制的。**
建立自訂 HTTP endpoint 需要 `SbcPermissions.RegisterHttpEndpoints`（有真的檢查），
但**「誰能呼叫那個 endpoint」DWS 完全不檢查** —— `CustomEndpointMiddleware` 把
`sessionId` 初始化成 `-1`，解析失敗也照樣往下走，只把它當**資料**塞給外掛。
整個 348 行 `Authorize`／`Policies`／`Claim`／`User` 零命中。
另外命令連線那側，pid 對不上已註冊外掛的**一律授予除 SuperUser 外的所有權限**。
⇒ P17 的結論不變但理由更強：**我們的 viewer／operator 兩級必須驗證「實作真的會發 viewer」**，
因為 DSF 正是「型別上有 ReadOnly、實際只發過 ReadWrite」。
⇒ 並且**不要把授權責任下推給頁面**（DSF 下推給外掛作者，示範程式自己也沒檢查）。

**(f) 儲存與配方：我們已經比 Duet 強，但配方逐行改要改。**
J 份查到 `SecureDelete` 的註解證明 `OpenMode::write` 會**重配 cluster**
⇒ 任何「只改一行」透過 write 模式做，其實都是**整檔重寫**。
⇒ 我們的 `recipe.doc.put`「只改點名的那幾行」如果底層是 write 模式，
語意上並不是就地改 ⇒ 應改成 **copy → edit → verify → rename**（即 P36）。

### 9.2 我們的長、Duet 沒有的（不要在借鑑時丟掉）

| 主題 | 我們 | Duet |
|---|---|---|
| per-command ack | `{"type":"ack","id":n,"ok":…}` 逐指令對應，UI thread 處理完才送 | `rr_gcode` 回覆共用一段文字，不是 per-command（§3.5） |
| 指令有界佇列 | `CommandQueue` 64、滿了明確拒絕、`capacity==0` 唯讀模式 | 通訊層無佇列，靠韌體 `buff` |
| 單一操作權 | `control.acquire` token、斷線／閒置釋放、`control.owner` tag 讓所有瀏覧器看到誰在操作 | session key 只管「有沒有登入」，多個瀏覧器可同時送指令 |
| 應用層 idle timeout | 45,000 ms 沒收到就斷 | 無 PONG 逾時 |
| Origin gate | 預設拒絕非自己 origin 的 WS upgrade（含 `null`），擋同一台瀏覧器裡別的頁面 | 未見（待 §4 確認 HttpResponder 是否查 Origin） |
| 互鎖在 handler 內、不疊第二層 | 20260819 裁決；忠實翻譋的 handler 自帶 `Insufficient()`／狀態檢查 | 相同哲學（檢查在 G-code 直譋器內），這點兩邊一致 |
| SECS/GEM host、配方文件、備份→驗證→刪備份 | 半導體廠務必需 | 不適用 |

### 9.3 不要抄的

- **GPL 程式碼本體**。只讀架構（§0.2）。
- **G-code 文字直譋器**當成我們的指令層——HT9045 的操作不是連續運動軌跡，是狀態機事件；
  硬套 G-code 只會多一層翻譯。但「**唯一入口、檢查只寫一次**」的原則要抄（我們已經是 dispatch 表）。
- **檔案管理 API**（上傳／下載／目錄）——我們的配方走 `recipe.doc.put` 逐行改，比檔案級操作更安全，
  不要退回去。
- **外掛市集**——目前沒有第三方開發者。

### 9.4 開發流程的借鏡（不是程式碼）

- `Connectors`／`ObjectModel` 抽成獨立 npm 套件（LGPL），DWC 與第三方工具（Python API、PanelDue）共用 ⇒
  **協定層獨立成套件、獨立版號**。對照：我們的 `ws.js` 契約寫在註解裡、Steven 的契約是另一份 JSON。
  → 提案：把 wire 契約抽成 `web/contract/ht9045-wire.schema.json`（JSON Schema）兩邊都驗。
- 韌體 `rr_connect` 回 `apiLevel`，前端不合就 `BadVersionError` ⇒ **協定版本協商**。
  → 提案：`snapshot` 第一包帶 `bridge.apiLevel`，`ws.js` 不合就顯示「請更新頁面」而不是靜默錯位。
- DSF 有 `OpenAPI.yaml`＋`UnitTests/`（14 檔）；RRF 有 `Developer-documentation/`（運動規劃的 .odt／.wxmx 推導）。
  → 我們的 `D:\HT9045_web\docs\API.md` 已是同類，但沒有機器可讀的 schema。
- DWC 有 CLA bot（`cla.yml`）、`WHATS_NEW.md`；RRF 的 `BuildInstructions.md` 已改成「見 wiki」——
  **文件單一出處**的紀律，跟我們「權威寫入邊界在 policy.json 不在本文」同一種。

### 9.5 要使用者裁決的四件事（整份文件的收斂點）

其餘提案都是「可以做、風險低、不必問」。這四件不是。

**裁決一：狀態要走哪條路線？**（決定 P1／P15／P18 怎麼做，是本文最大的岔路）

| | Duet 路線：狀態樹 + 版本號 | Moonraker 路線：欄位級訂閱 + 時戳 |
|---|---|---|
| 做法 | 每個子樹一個 seq，client 比一個數字就知道要不要重抓 | client 宣告「我這一頁只要這 12 個 tag」，伺服器只推這些的變更 |
| 伺服器成本 | 低（一個整數） | 中（每連線一份訂閱表） |
| **失敗模式** | **漏敲一處 `++seq` ＝ 靜默過期的 UI，且無編譯期檢查**（Duet 有 203 個手動 `++`，§4.4） | 訂閱清單寫錯 ＝ 那個欄位不更新，但**清單是明示的、可稽核的** |
| 對我們的適配 | 我們狀態機集中，`++` 可以只放在少數 transition 函式（§9.1 P10） | **訂閱清單我們已經有了**——`tools/pagewire/` 的三元組 `[文件, 區段, 鍵]` 就是它 |

F 份筆記傾向 Moonraker 路線，理由是我們的 tag 數量級遠大於 3D 印表機（約 5,349／代），
且訂閱清單已經存在只差接上。兩者可並存（Bambu 就是高階全量、低階差異）。
**這需要你決定，文件不代決。**

**裁決二：要不要做統一命令管線（P12）？**
這是架構決定，不是最佳化。做了，`fMain->Start()` 那 29 條活的路徑、SECS RCMD、GPIB、
面板按鈕就共用同一個 Pre stage 的互鎖與操作權檢查；不做，就是 29 次獨立裁決，
而 §9.1.1 已經證明「維護一張正確的呼叫點清單」我們會做錯。

**裁決三：急停要不要做、怎麼做（P11 + P19）？**

> ✅ **已裁決（使用者 20260923 下班前，INBOX P11-ESTOP）：「不用，已經有實體控制」。**
> 網頁**不做**急停：不寫程式、不加畫面標示（使用者沒要求）。本題從待辦移除，P11／P19 兩列一併視為結案。
> 下面的更正與形狀建議保留作史料（若日後重開本題，理由與形狀從這裡接）。

> ⚠⚠ **20260922 更正：本題原本的風險錨點是假的。** 原文寫「與 ST 戰役的 S3 武裝同級，
> 不可先做」，但 **S3 早在 20260918 就已經武裝**（`c742cc1`「START 的四段鏈路全通——
> 瀏覽器按一下真的會走到 `StartFromWeb()`」；`tools/wb_serve.cpp:2122/:3220`、
> `WebStart.cpp:15-19` 的 `AI(W906-S3-CLAIM-VOID)`），比本文件建檔早兩天，
> 20260921 23:5x 才被發現。**所以「與一件還沒發生的事同級」這個理由已不存在。**
> 另外 20260922 使用者的常設指示（「全部動作都要執行，此專案就是要上線的」）
> 已把「機台會動」從開裁決單的理由中移除，新判準是「**翻譯決定 vs 操作決定**」。
> ⇒ 本題**仍然要裁決**，但理由改成：web 急停**不是 golden 的翻譯**，是新架構，
> 屬操作／架構決定。（此推論是主迴圈的，使用者未就 P11 表態。）
> ⚠ `CLAUDE.md` 第 40 行仍寫「S1／S3（武裝）絕不自動做」，同樣已過期，待使用者確認後再改。

形狀建議學 Grbl（socket thread 當下攔截、
不進佇列），但**不能取代硬體安全鏈**。不可先做。

**裁決四：兩條資料面要不要收斂（P3）？**

> ⚠ **20260922 部分被既成事實搶先。** 使用者就 Q30 第 8 題裁決「依據建議**甲**」
> （`docs/INBOX_QUEUE.md:1251`），甲案＝**C++ 去寫 Steven 的檔案信箱 JSON**
> （`Main-command-request.json` / `Main-command-ack.json`）。也就是說：
> **警報對話框這一片已經決定走檔案面，方向與「收斂到 WebSocket」相反。**
> 選甲的理由是工程性的（`web/` 是 gitignored，我們寫進他頁面的東西下一包就被覆蓋，
> Q27 已吃過這個虧），不是架構表態。
> ⇒ 正確敘述是：**通用收斂問題仍未裁決，但警報對話框那一片已落在檔案面。**
> （此歸類是主迴圈的推論；沒有任何文件寫「裁決四決定了」。）

牽涉同事的地盤與 `ht9045_web.git` 的分工。今天是 WebSocket tag 串流（3 個檔在用）與
file:// JSON 全量輪詢（16 個檔在用，由 C# 模擬器餵）並存。
Duet 那邊的對照是「一個 model、一個 connector 抽象、UI 不知道後端是誰」。

---

## 10. 後續解析待辦

| # | 標的 | 為什麼 | 狀態 |
|---|---|---|---|
| Q1 | 把 §9.1 提案表補齊 | 本輪 | **完成**：六份筆記全到齊，提案 8→32 條 |
| Q2 | `ObjectModel/src` 與 RRF C++ 表的同步方式 | 決定我們的 tag schema 方向 | **完成** → §6.3：**跨語言那一段**（C++ 韌體 ↔ TS 型別）沒有 codegen，人工改 TS ＋ 版號釘死。⚠ **20260922 更正：同語言那一段有** —— DSF 的 C# 模型由 `src/DuetAPI.SourceGenerators/`（14 檔 1,969 行）產生。⇒ P9 的理由改成「抄 DSF 的 C# 側」，不是「做得比它好」 |
| Q3 | `M291/M292` vs 我們的 `modal`／`query`／`modal.answer` | FW-W5 還沒做完 | **完成** → §7.5、提案 **P27**。答案是：對話框要變成獨立物件，等待端只設 bit 然後讓出 |
| Q4 | 事件→巨集能不能對應到我們的 WAR/JAM 處置 | 把警報處置從硬編碼變資料 | **完成** → §7.6、提案 **P30**。低風險路徑：現有硬編碼留作 fallback，另加一層外部動作清單 |
| Q5 | DSF `CodeInterception` vs 我們的 dispatch 表 | 統一攔截點 | **完成** → §5.6、提案 **P12**＋**P28**。⚠ 關鍵判斷：攔截器是「政策層」不是「保護層」，運動安全仍要在最底層 |
| Q6 | 其他同類開源專案的橫向比較 | 同一題多看幾家再定我們的 schema | **完成** → §7.5、`notes/F_peer_projects_comparison.md` |
| Q7 | 真機量測：每代約 5,349 tag 的 patch 頻寬與 UI thread 負載 | P1／P2／P15 的必要性要用數字證明 | 等機台時間。⚠ 分母見 §8.2.1：其中 4,481 是 `pci1203.*` |
| **Q8** | **mainline DWC 的四個 CNC 面板**：`CNCAxesPosition.vue`(87)／`CNCContainerPanel.vue`(205)／`CNCDashboardPanel.vue`(47)／`CNCMovementPanel.vue`(269)，合計 **608 行** | ★ **這是六個 repo 裡唯一「工具機而非 3D 印表機」的畫面**，而我們是機台不是印表機。§6 分析的是 DWC 的**結構**（兩份 model／凍結閘／路由即資料），**沒有讀 CNC 模式的畫面本身**。DRO（軸座標顯示）、jog 面板正是我們 69 頁裡最像的東西 **完成**（20260922 第二輪）→ `notes/G_dwc_cnc_panels.md`（63 檔／4,180 行／286 筆引用全 PASS）。成果進了 **§6.4 的結論反轉**、**§9.1.4 (b)(c)**、新提案 **P37**。⚠ `DWC-CNC/` 那個 repo 是 **3.4.0-b1 的死 fork**（README 首段自述已併入 DWC 3.4-b2、不再更新），**不要去讀它** |
| **Q9** | **DSF 的 `src/DuetAPI.SourceGenerators/`**（14 檔 **1,969 行** Roslyn 產生器） | 它產生 `UpdateFromJson.cs`(461)／`UpdateFromJsonReader.cs`(463)，**那就是 patch 套用的第二份獨立實作**。P26（陣列語意）目前只有 TS 側一份證據 | **已識別、未深讀**。⚠ 更正：不是 0 引用 —— `C_dsf_webserver.md:37` 已把 `UpdateFromJson.cs` 列進檔案清冊、`:188` 已寫明它是產生器產出，但**沒有任何 CITE 進到它的函式本體**，而且**主文件 §6.3／Q2 的結論與筆記這個事實相反**（見 §6.3 的更正框）。⚠ 已初步查證識別字含 `GetArrayLength`／`Count` 比對／`Add`／`RemoveAt` ⇒ **也是長度對齊＋逐格覆寫，與 TS 側一致**。**完成**（20260922 第二輪）→ `notes/H_dsf_sourcegen.md`（26 檔／2,538 行／161 筆引用全 PASS）。**P26 因此定案，見 §9.1.2** |
| **Q10** | **清冊裡標 MAYBE 的六項**（`notes/K_dsf_remainder.md` §18／§19） | 第二輪把「還沒讀的是什麼」列出來並分三級判定。⚠ **剩下的不是都無關** —— 六項被判 MAYBE，其中三項標 ★（比看起來相關） | **待排**。逐項：**(1) `src/GCodes`(40) —— 不是「沒有對應物」，那就是我們缺的統一命令管線本身（＝P12）。所有掛在管線上的好處，我們一個都拿不到，直到它存在。** **(2) ★ `FilamentMonitors`(12)**：介面是 `Check(isPrinting, fromIsr, isrMillis, filamentConsumed)` —— 它做的是「**把指令要求的移動量與感測器量到的移動量比對，不符就報警**」，＝我們的「命令位置 vs 編碼器實際位置」與「真空吸到了 vs IC 真的在吸嘴上」。還附 `enableMode`（只在生產中檢查／一直檢查）與 `noDataReceived` 狀態，兩者我們都需要。**(3) ★ `Accelerometers`(4)**：不是即時閉環補償，是「**按需求收一段固定長度的原始資料、落成 CSV、離線分析**」⇒ 對我們現在量不到的那一類問題（手臂高速移動的震動／掉料，見記憶 `ht9045-soft-simulte-cannot-validate-position-faults`）是**可行的工具形狀**。`numSamplesRequested` + 預先算好緩衝、避免動態配置也是對的。**(4) `Tools`(6)**：`Tool` 的語意是「一組具名的、可整體切換的硬體配置＋偏移量」＝我們的機種／Socket／Kit 配置（`setup.inf`、`CurrentSetupData.txt`），差別是 Duet **一個指令就能在執行中切換**，我們是重讀設定檔。對「換料換配方」流程有意義。**(5) `Networking/MQTT`**：`retain` 語意（broker 保留最後一筆給新訂閱者）正好解「**新連上的監看端要先看到現狀**」。**(6) `src/SBC`(5)／`DuetControlServer/SPI`(51)**：`SbcMessageFormats.h` 那種「用一個 header 定義雙方共用的線上格式、雙方各自實作」，正是我們與 PCIE-1203／PLC 之間該有的形狀（目前雙方各自寫 struct）。⛔ 判為 NO 的：`LedStrips`（可借的結構我們在 `mymotor.h`／`mysensor.h` 已經有）、`Display`、`bossa`、`LinuxApi`、`DuetPiManagementPlugin`（功能不需要，但它是「把 OS 管理實作成指令」的範本） |

---

## 11. 方法與可信度

- 20260920 主迴圈（Fable 5.1）＋ 5 個唯讀子代理起手；20260921 主迴圈（Opus 5）
  以一個 workflow 補跑被 session limit 打斷的三份。每批扇出 ≤ 5，符合常設上限。
- 每份筆記回來先跑 `tools/cite_check_notes.py`。**五份合計 2,382 筆，全數通過。**
  實際抓到並修掉的：A 份 1 筆行號差 2 行、C 份 1 筆差 1 行。
- **驗證器自己的兩次盲點也修了**，見 §0.1 的警告。
- 本文件只收錄通過驗證的引用；代理的「啟示」一律標為意見，且與事實分節。
- V906 側的行號是主迴圈自己量的，不是代理給的。其中 START 呼叫點那一張
  **量錯三次後改成工具**（§9.1.1），工具第一次跑就抓到主迴圈自己的錯。
- 沒量到的寫 UNKNOWN 或「等機台時間」，不寫推測值。

**★ 覆蓋率（20260922 實測，回答「是不是徹底解析完了」）：不是，也不該是。**

**20260922 第二輪之後重量**（同一支量法，寬鬆比對；`DWC-CNC` 強制歸零）：

| repo | 原始碼檔數 | 第一輪 | 第二輪後 | 第一輪% | 第二輪後% |
|---|---|---|---|---|---|
| `Connectors` | 8 | 8 | 8 | **100%** | **100%** |
| `ObjectModel` | 75 | 33 | **48** | 44% | **64%** |
| `DuetWebControl` | 177 | 66 | **92** | 37% | **52%** |
| `DuetSoftwareFramework` | 505 | 106 | **145** | 21% | **29%** |
| `RepRapFirmware` | 679 | 68 | **126** | 10% | **19%** |
| `DWC-CNC` | 170 | 0 | 0 | 0% | 0%（死 fork，見 Q8） |
| 合計 | 1,614 | 281 | **419** | 17.4% | **26.0%** |
| **（扣掉 DWC-CNC 死 fork 的分母）** | 1,444 | 281 | **419** | **19.5%** | **29.0%** |

⚠ 嚴格比對（只認 `檔名:行號`）的第一輪數字是 275／1,614 ＝ 17.0%；
兩種量法的差異與各自的誤差方向見下方兩段。**引用格式與 repo 對應**：
第二輪五份筆記合計 **1,752 筆全數 PASS／0 FAIL**，兩輪相加 **4,134 筆**。

**⇒ 「徹底了嗎」的誠實答案：29%，而剩下的 71% 已經從「未知」變成「列出來並分類過」。**
`notes/K_dsf_remainder.md` §18／§19 是那份清冊，判準是 NO／MAYBE／**有對應物**三級，
且明文禁止把「其實有對應物」的東西誤判成沒有。見 **Q10**。

量法：`git ls-files` 取每個 repo 的原始碼檔（`.ts/.cpp/.h/.cs/.vue/.js`，排除 `node_modules`），
與五份筆記裡的相異檔名以 basename 比對。上表的 `DWC-CNC` 手動歸零（見下）。

**兩個方向的誤差，都要講：**

| | 嚴格（只認 `檔名:行號`） | 寬鬆（也認只提檔名） |
|---|---|---|
| 相異檔名 | 180 | 214 |
| 命中／1,614 | 275（17.0%） | 327（20.3%） |

- ⚠ **會高估**：basename 比對讓跨 repo 同名檔互相命中。`DWC-CNC` 是 `DuetWebControl` 的
  fork，115 個檔同名，所以它「命中 35 個」全是假的——七份筆記對 `DWC-CNC` 字串
  **0 次命中**，真值是 0。上表已手動改成 0（若分母也剔掉它的 170 檔，是 240／1,444 ＝ **16.6%**）。
- ⚠ **會低估，而且我當場被它咬到**：第一版抽取器**只認 `檔名:行號`**，
  於是把「筆記只提了路徑、沒帶行號」的檔判成未讀。我因此一度宣稱
  `DuetAPI.SourceGenerators` 是 0 引用——**錯的**，C 份筆記 `:37`／`:188` 都提到它。
  ⇒ 又一次印證記憶 `call-site-census-needs-a-tool-not-a-grep` 與
  `verify-a-reduction-by-what-disappeared`：**「查無」要逐筆看消失清單，不能直接當結論。**

**行數加權（另一個角度）**：六個 repo 共 **348,899 行**原始碼。
「被引用過的檔」合計 94,433 行 ＝ **27.1%**（上界，引到一行也算整檔）；
已驗證引用 2,382 筆 ＝ **0.7%**（下界，只算明確引到的行）。⇒ 真實閱讀量在這兩者之間。

**低覆蓋率大部分是刻意的，不是缺口。** RRF 零引用的 16 個子目錄裡，
`src/Display`(40)、`src/bossa`(17，bootloader)、`src/FilamentMonitors`(12)、
`src/LedStrips`(12)、`src/Accelerometers`(4)、`src/libc`(4)、`src/DuetNG`(5)
對 IC handler 沒有對應物。**真正的缺口只有 Q8、Q9 兩條**（合計 2,577 行），
它們之所以是缺口，是因為它們**恰好是最貼近我們的那兩塊**卻落在原本的分析入口之外。
- F 份是唯一靠公開文件而非原始碼的一份，它自標 A/B/C 三級可信度並列出 6 個
  fetch 失敗的來源；主迴圈另抽驗了兩條最承重的引述。

---

## 12. 變更紀錄

| 日期 | 誰 | 內容 |
|---|---|---|
| 20260920 | W906-DUET-REF | 建檔。§0–§3、§8、§9 初稿、§10、§11。A 份筆記 549／549 引用通過。 |
| 20260921 | W906-DUET-REF | 收 B（526／526）、C（542／542）、F（207 URL，抽驗 2 條）。新增 §4、§5、§7.5。提案表 8→21 條。新增 §9.5（四件要裁決的事）。Q6 結案。 |
| 20260921 | W906-DUET-REF | §9.1.1 START 呼叫點 census。**先寫 21 個，那是錯的**；做成工具後正確數字是 **32 個、29 活、3 閘**（`tools/start_sites_census.py`，含 `--check` gate）。`WebStart.h`、`CLAUDE.md` 同步更正。 |
| 20260921 | W906-DUET-REF | 收 D（345／345）。新增 §6，提案 21→26。§1／§2 兩處被深讀推翻的判斷就地標示。§8.2.1 查證快照組成（4,481／4,608 是 `pci1203.*`），據此把 P15 提到 P1 之前。補 §9.0「如果只做三件事」。 |
| 20260921 | W906-DUET-REF | 收 E（**420／420**；20260922 更正：此處原寫 351，與 §0.1 的 420 及總數 2,382 不符——549+526+542+345+420=2,382，351 會少 69 筆。agent 被 session limit 打斷但檔已寫完）。新增 §7，提案 26→32。**P27（`TModalRequest`）解掉 FW-W5 的卡點**；P28/P29 給出多來源仲裁的具體形狀；P30 給出警報處置變資料的低風險路徑。Q1／Q3／Q4／Q5 一併結案。六份筆記全到齊，引用合計 **2,382 筆全數通過驗證**。 |
| 20260922 | W906-DUET-REF | **時效稽核（沒有新增分析，只更正已被推翻的數字）**。六處：§5.8 與 P11／P12 三處殘留的「19 個呼叫點只閘 2 個」→ **32／29／3**（跑 `tools/start_sites_census.py --check 32 29 3` 實測 PASS）；§9.1.1 的 `WebStart.cpp:3495/3506/3529` → **3656/3667/3690**（`74211ce` 加了 34 行註解）；§9.0 開頭「26 條」→ **32 條**，並標出**有 12 條未排進任何一批**（含 P21）；裁決二「18 次獨立裁決」→ **29 次**；§9.1 尾註的 `sys/events/*.g` → 實際是平放的 `0:/sys/heater-fault.g`。**兩件時效性大事就地加註**：(a) **§9.5 裁決三的風險錨點是假的——S3 早在 20260918 `c742cc1` 就武裝了**，比本檔建檔早兩天；(b) 20260922 使用者常設指示把「機台會動」從裁決理由移除，新判準是「翻譯決定 vs 操作決定」；(c) 裁決四的警報對話框那一片已由 Q30 第 8 題「走甲」落在檔案面。**P1–P32 至今零條開工**（`TModalRequest`／`apiLevel`／`ht9045-wire.schema.json` 全樹只出現在本檔）。 |
| 20260922 | W906-DUET-REF | **覆蓋率量測 ＋ 兩個新缺口**（回答使用者「是不是徹底解析完了」）。實測 `git ls-files` × 筆記引用：1,614 個原始碼檔被引用 240 個 = **15%**，`Connectors` 100%／`RepRapFirmware` 9%／**`DWC-CNC` 0%**。低覆蓋率大部分刻意（RRF 的 Display／bossa／FilamentMonitors 等對 IC handler 無對應物），數字寫進 §11。**兩個真缺口新增為 Q8／Q9，合計 2,577 行**：Q8 = mainline DWC 的四個 `CNC*.vue` 面板（608 行，七份筆記 0 引用，卻是六個 repo 裡唯一「工具機而非印表機」的畫面）；Q9 = DSF 的 `DuetAPI.SourceGenerators`（1,969 行，0 引用）。**Q9 順帶更正一個結論**：Q2／§6.3 的「Duet 沒有 codegen」只對 TS 那一段成立，DSF 的 C# 模型是產生的 ⇒ P9 的理由從「我們該做得比它好」改成「抄 DSF 的 C# 側」。初步查證其 `UpdateFromJson` 也是長度對齊＋逐格覆寫，**P26 因此被第二份獨立實作證實**。另更正：§0.3 閱讀指引指向 P3／P4 與 P6／P7（8 條提案時代的殘留，實際是 P22/23/25/24 與 P12/P11/P19/P30/P27）；§12 的「E 351／351」→ **420／420**（549+526+542+345+420=2,382，351 會少 69 筆）。⛔ 已確認 `DWC-CNC/` 是 3.4.0-b1 死 fork（README 自述已併入 DWC 3.4-b2、不再更新），**不要去讀它**。 |
| 20260922 | W906-DUET-REF | **自我更正（同日第三顆）**：前一顆宣稱 `DuetAPI.SourceGenerators` 在筆記裡 0 引用，**那是錯的** —— `C_dsf_webserver.md:37` 已列入檔案清冊、`:188` 已寫明 `UpdateFromJson` 是產生器產出。根因是我的覆蓋率抽取器**只認 `檔名:行號`**，把「只提路徑、沒帶行號」的檔判成未讀。⇒ 真正的問題不是漏讀，而是**逐檔筆記正確、收斂到主文件時被壓成了錯的全稱句**（§6.3／Q2 的「Duet 沒有 codegen」）。Q9 狀態改為「已識別、未深讀」。§11 補上嚴格／寬鬆兩種量法（17.0% vs 20.3%）、`DWC-CNC` 歸零的理由（fork 同名檔 115 個，命中全是假的）、以及行數加權的上下界（27.1% / 0.7%，總行數 348,899）。⚠ 再次印證 `call-site-census-needs-a-tool-not-a-grep`：**「查無」要逐筆看消失清單**。 |
| 20260922 | W906-DUET-REF | **第二輪：徹底解析（收斂進本文件）。** 標的是第一輪**量出來的缺口**，不是重讀。五個唯讀代理讀 **270 檔／20,829 行**，產出 `notes/G_`～`K_` 五份（5,298 行），**1,752 筆引用全數 PASS／0 FAIL**（兩輪相加 4,134 筆）。覆蓋率 **19.5% → 29.0%**（扣死 fork 分母）。★★ **P26 定案**（新增 §9.1.2）：一律送整條陣列 —— 兩份獨立實作都是長度對齊＋逐格覆寫＋從尾端截斷，送單格會截到 1 格且無訊息，而 Duet 安全的前提只寫在 `ObjectModel.cpp:1096-1099` 一段註解裡。**P22 解除封鎖。**★ **§6.4 結論反轉**：`code-btn` 只有 25 處、手寫 `uiFrozen` 有 60 處，`SpindleSpeedPanel` 的主軸啟停**一個守衛都沒有**⇒ 教訓從「有單一元件就安全」改成「只要留著裸的 sendCode 就會有人繞過」；且 `uiFrozen` **只看連線、不是安全閘**。同時更正 `notes/D:584`。★ **P18 修訂**（新增 §9.1.3）：**Duet 沒有可執行期查詢的 pin 目錄**（`portUsedBy`／`PinTable[]` 都不進物件模型），它的等價物是編譯期表＋static_assert ⇒ 「抄 Duet 做 halmeter」不存在；但 `PinUsedBy::temporaryInput`（讀值不擁有，全樹零呼叫者）可以直接拿。**新增 P33–P37**：命令值 vs 回讀分開／重連後重同步／「值未知」vs「tag 不存在」／`.part`+rename／jog transaction。**Q8／Q9 結案，新增 Q10**（清冊裡六項 MAYBE，其中 `src/GCodes` 根本就是我們缺的 P12，`FilamentMonitors` 是「命令移動量 vs 感測移動量」＝我們的真空／編碼器對帳，`Accelerometers` 是「按需收原始資料落 CSV 離線分析」的工具形狀）。另記 §9.1.4：急停旁路做在**接收端最底層不是發起端**；Duet 兩個官方客戶端失效判準不一致（C# 的 `>` 在板子重開後永遠停在舊值）；DSF 的自訂 endpoint **呼叫側零授權檢查**；`OpenMode::write` 會重配 cluster ⇒ 配方「只改一行」其實是整檔重寫。 |
