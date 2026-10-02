# ShowErrorMessage / ShowMyMessage JSON Bridge

## Scope

This bridge is separate from `Production-update.json`. It represents two blocking BCB6 UI calls as
request -> modal -> close response handshakes:

- `ShowErrorMessage(...)`: C++ publishes an Alarm request and returns the selected integer K code unchanged.
- `ShowMyMessage(...)`: C++ publishes a Message request and resumes only after close and cleanup are acknowledged.

The authoritative machine-readable specification is `JSON/Dialog-bridge-contract.json`.

## Mailboxes

| Channel | C++ -> HTML | HTML -> C++ |
|---|---|---|
| Alarm | `Alarm-dialog-request.json` | `Alarm-dialog-response.json` |
| Message | `Message-dialog-request.json` | `Message-dialog-response.json` |
| C++ 主動關閉 | `Dialog-close-request.json` | `Dialog-close-response.json` |
| 權限驗證 | `Dialog-auth-result.json`（C++ 寫結果） | `Dialog-auth-verify.json`（HTML 寫憑證） |

Every invocation uses a unique `requestId` and increasing `seq`. A response must copy both values into
`requestId` and `requestSeq`. C++ must ignore stale, mismatched, or incomplete responses.

## Alarm Return Codes

`arguments.kCode` is a bitmask. HTML presents only enabled actions and writes the selected bit value to
`selectedAction.code`. C++ assigns that exact value to `fNote->ReturnCode` and returns it.

| Action | Decimal | Hex |
|---|---:|---:|
| RETRY | 1 | 0x0001 |
| SKIP | 2 | 0x0002 |
| CLEAN_OUT | 4 | 0x0004 |
| TRAY_FEED | 8 | 0x0008 |
| TRAY_END | 16 | 0x0010 |
| RESET | 32 | 0x0020 |
| HOME | 64 | 0x0040 |
| TRAIN | 128 | 0x0080 |
| FIX | 256 | 0x0100 |
| ONECYCLE | 512 | 0x0200 |
| PAUSE | 1024 | 0x0400 |
| START | 2048 | 0x0800 |

For `kCode == 0`, HTML shows an acknowledgement action and returns code `0`. For nonzero `kCode`, Escape
and browser close must not synthesize a result.

## Message Close Response

The Message response records `selectedAction`, `closedBy`, `completedAt`, and `durationMs`. Its
`sideEffects` values are confirmations written by the C++ host after applying the original
`TMyMessageBox::FormClose` behavior. HTML initially emits `null` because it cannot observe motor, servo,
SECS/GEM, or handler flags directly.

## C++ 主動關閉畫面

實體按鍵與 IO 狀態完全由 C++ 負責。HTML 不讀取、不輪詢、不 debounce，也不判斷任何 IO。

BCB6 已確認的流程包含 `TMyMessageBox::Timer1Timer()` 呼叫 `ScanPannelKey()`，偵測
`SnFKPause`／`SnFKRetry`／`SnFKSkip` 後直接 `Close()`；其他 Start/Pause 流程也由 C++ 決定
`ReturnCode` 與機台副作用。`SwFKPause`／`SwFKStart`／`SwRKPause`／`SwRKStart` 是對應燈號輸出名稱，
不可由 HTML 當成輸入狀態解讀。

C++ 偵測並處理 IO 後寫入 `Dialog-close-request.json`：

1. `target` 指定目前 Alarm/Message 的 `channel + requestId + requestSeq`。
2. `trigger` 記錄 C++ 偵測來源，例如 `source=io`、`inputName=SnFKStart`。
3. `resolvedAction` 是 C++ 已決定的 action/code；HTML 不再套用畫面按鈕的 KCode 規則否決它。
4. HTML 精確比對 target，先寫入原 channel 的 Alarm/Message response，再關閉 modal。
5. 畫面實際關閉後，HTML 寫入 `Dialog-close-response.json`，回報 close request 關聯、target、
	`dialogWasOpen`、`closedAt`、`closedBy`、selected action 與 normal response 檔案/seq。

操作員直接點 HTML 按鈕關閉時也會產生 `Dialog-close-response.json`，其 `closeRequestId` 與
`closeRequestSeq` 為 `null`、`trigger.source=html-action`。target 不符或無畫面時不得關閉目前 modal，
而是回 `state=error`、`error.code=CLOSE_REQUEST_REJECTED`。

## Display Environment

Dialog 僅支援固定機台 FullHD 1920×1080；不設計或驗證手機／平板版面。
權威規格見 `.github/specs/html-display-environment.md`。

## dfm 生成的对话框頁（2026-09-02）

通用 modal 已改為由 **BCB6 dfm 生成的真实頁面**，`dialog-bridge.js` 以 overlay iframe 開啟：

| kind | dfm | 頁面 | 尺寸（client） | 依據 |
|---|---|---|---|---|
| `alarm` | `note.dfm`（fNote） | `page/Alert.Note.html` | 972×761 | `TfNote::FormShow()` `AUTO_EMPTY_COLOR<3` 分支 Width=980；ClientHeight 沿用 dfm 761 |
| `message` | `mymessbox.dfm`（MyMessageBox） | `page/Alert.MyMessageBox.html` | 472×219 | `TMyMessageBox::FormShow()` 預設分支 Height=250/Width=480 |

產生器 `_gen_dfm_abs.py`：
- `JOBS` 新增兩項；`PAGE_EXTRA` 掛 `dialog-page.js` 並設 `<body data-dialog="alarm|message">`。
- `RUNTIME_PROPS[dfm]`：執行期擺位覆寫（render 前改節點屬性）。mymessbox：pnlMain/lblMainMsg/lblChineseMsg/pnlPause/pnlYes/pnlNo 坐標、Font 12；pnlAlarmReset/lblSubMsg/moSecsGem/labStopTime 預設隱藏。note：pnlBottom/pnlPicker/ShowMessageEdit1/reDescription/RichEdit1 寬度依 Width=980；palHead Left=229；palFix4~6/palAuto4~6(_Car)/palOutSh3/palOutArm2/palLoad2(_Car)/palIonFan01~12/palCCD/TMyTray1/pnlMovie/btnMoveTo*/TrayEdit/pnlStopTime/sbBinEdit/labSecsGemLock 等依 Reset()/ShowErrorUnit()/ShowErrSite() 預設隱藏；12 個 palRedNotice 隱藏且 Left=160/Top=290。
- `HIDE_TABS={('note.dfm','pgcNote')}`：建構子 `TabVisible=false` → 頁籤列 display:none、pcBody top:0。
- `SKIP_TYPES` 加 `TMediaPlayer`；`TRichEdit` 改輸出 textarea（帶 Font.Color/size/readonly）。
- 參數來源為 `General-config.json`（model HT-9046AT、AUTO_EMPTY_COLOR=1、USE_OUT_SORT_ARM=0）；若機型改為 HT-9011UC（AUTO_EMPTY_COLOR≥3）需改 RUNTIME_PROPS/CLIENT_OVERRIDE 為 Width=1200 分支。

`dialog-page.js`（頁內）：
- ready 後 postMessage `HT_DIALOG_READY`；bridge 回 `HT_DIALOG_REQUEST{kind,request}`。
- alarm：填 edErrorCode/Edit3(Code[4..5])/edUnitName/ShowMessageEdit1（`Message : errPart (Again!!)`）/reDescription；BtnHome caption 依 `WAR07352`；依 kCode bit 顯示 9 顆 BtnPanel（SKIP/RETRY/TRAY_FEED/TRAY_END/CLEAN_OUT/RESET/HOME/TRAIN/ONECYCLE；FIX 不在 KeyComp），`BtnStart` 僅 kCode≠0；選擇鍵互斥 Down（UpdateButtonStatus）；未選鍵時 Start/Pause 無動作（同 cpp）；`display.flushPanel` → 該 pal* 加 `.dbFlush` 閃紅（ShowErrorUnit）。
- message：lblMainMsg/lblChineseMsg；pnlPause caption OK/Pause（ok‖secsGemAlarm‖haltHandler）；secs 時 moSecsGem 顯示、lblMainMsg 空；pnlAlarmReset 依 showAlarmReset；employeeIdCheck 時 pnlPause 隱藏。pnlAlarmReset 只送 `HT_DIALOG_EVENT ALARM_RESET`，不關閉（同 pnlAlarmResetClick）。
- 送 `HT_DIALOG_ACTION{kind,requestId,action:{name,code},pressedButton}`；bridge 驗證 requestId/kCode 後寫 response → 關 overlay → 寫 close response。

Alarm response 新增 `pressedButton`（`BtnStart`/`BtnPause`/null）：cpp `fNote::Start()` 與 `BtnPauseClick()` 回同一 ReturnCode，但前者 `fMain->Start`，後者 `SoftStop=true`；C++ 需依此分流。外部 IO 關閉時為 null（由 C++ 自知）。

## 置頂與層級（2026-09-02）

Alert 顯示時必須覆蓋其他所有畫面（同 BCB6 `ShowModal` + Timer `BringToFront`）。全部 overlay 都在 **background 頂層文件**，不在子 iframe 內：

| 層 | z-index | 說明 |
|---|---:|---|
| 桌面視窗 `.win` | zTop（幾百） | 一般頁面 |
| `#layoutBar` / `#compPalette` | 9999 | 版面拖曳工具 |
| `#dialogBridge` | 20000 | Alert overlay（含 Alert.Note / Alert.MyMessageBox iframe）；開啟時 `body.dialogOpen` 讓 desktop/taskbar/工具列 `pointer-events:none` |
| `#dialogAuth` | 21000 | 登入層（內嵌 dfm 生成的 `Alert.Password.html` iframe；唯一可出現在 Alert 之上的面板） |
| `HTQwerty .qkOv` | 99999 | 小鍵盤（background 已載入 `qwerty.js`，在頂層文件彈出） |

陷阱：頁內（iframe）自己彈的 qwerty/password 只能在該 iframe 內，MyMessageBox 472×219 容不下鍵盤，因此 Alert 頁不自己彈鍵盤，一律由 bridge 在頂層處理。

## 權限／密碼流程（Dialog-auth）

### 登入頁：`Alert.Password.html`（Password.dfm / fPassword，609×305）

由 `_gen_dfm_abs.py` 生成（JOBS），`RUNTIME_PROPS['Password.dfm']`：`cbUserName`/`btnDownload`/`lblPWDownload` 預設隱藏（`bTechComUseComboBox`/`bFtpPasswordDownload` 預設 false）；`HIDE_TABS` 含 `PageControl1`（`bShowTab==0` 只留 tsPassword，tsEventlogLogin/tsEventLogLoginModify 為 ASE-CL 專用）。TEdit `PasswordChar` → `type="password"`；|Font.Height|>14 的 Edit 帶 font-size（edUserName/edPassword 30px）。

`login-page.js`（`<body data-dialog="auth">`）：
- `HT_DIALOG_AUTH_REQUEST{auth,message}` → 清空 edPassword，`kind=employee-id` 顯示 Label5/Label6（SECS 工號檢查），`userIdRequired=false` 隱藏 lblUserName/edUserName（bLoginASECL / JCET_FOR_EVAN）。
- edUserName/edPassword mousedown → `HT_DIALOG_INPUT{field,flags}` → bridge 在頂層彈 HTQwerty（登入頁 609×305 容不下鍵盤）→ `HT_DIALOG_INPUT_RESULT{field,value,commit}`；密碼 Enter 即送出（同 `edPasswordKeyDown` Enter→Close）。
- btnOK → `HT_DIALOG_AUTH_SUBMIT{userId,password}`；spbCancel → `HT_DIALOG_AUTH_CANCEL`（只關登入層，無 JSON）。
- `HT_DIALOG_AUTH_RESULT{accepted:false,message}` → Label3/Label4 顯示（可帶 C++ message）、清密碼。

登入頁不持有密碼本，不比對；帳密只經 bridge 寫 `Dialog-auth-verify.json`（或 `HTDialogHost.verifyAuth`）交 C++ 裁決。

BCB6 依據：`TfNote::DoPassword()`（`fSecurity->GetJamLevel` vs `AccessLevel`，在 `Start()`/`BtnPauseClick()` 內）、`DoUnlockPassword()`、`PanSpecialNoteClick`（SpecialErrNote.ini Pwd）、`CheckEmployeeID`（SECS）、`TMyMessageBox::DoPassword_MBox()`（`bMBoxNeedPassword`）。密碼比對都在 C++；HTML **不驗證**、只收集輸入。

1. C++ 發 request 時已知是否需密碼 → 填 `request.auth={required,kind,level,title,prompt,userIdRequired,defaultUserId}`（kind：`access-level`/`unlock-password`/`special-note`/`employee-id`/`mbox-password`）。
2. 操作員按關閉動作（Start/Pause/OK）→ bridge 見 `auth.required && !authVerified` → 開 `#dialogAuth`（內嵌 `Alert.Password.html` iframe，見上節）。`ALARM_RESET` 不需密碼（同 cpp）。
3. Login → HTML 寫 `Dialog-auth-verify.json`：`{authId,target,kind,level,pendingAction:{name,code,pressedButton},credentials:{userId,password}}`（或 `HTDialogHost.verifyAuth(verify)`）。
4. C++ 比對後寫 `Dialog-auth-result.json`：`{authId,state:'completed',accepted,accessLevel,userId,message}`；HTML 每 200ms 輪詢同 authId，15s timeout。
5. `accepted=true` → 才寫 normal Alarm/Message response（`auth.verified=true, authId, accessLevel`）→ 關 overlay → close response。`accepted=false` → 对话框保持開啟、清密碼、顯示 message（同 palWrongPW）。Cancel → 只關登入層，不送任何 JSON。
6. 外部 IO close request 在 auth 進行中到達 → 直接關閉（`auth.bypassedByCloseRequest=true`），因 C++ 已自行解析实體鍵。

JSON 流程調整結論：**需要**——新增一組 `Dialog-auth-verify`（HTML→C++）/`Dialog-auth-result`（C++→HTML）握手，request/response 各加 `auth` 區塊；normal response 的語意不變（仍代表「畫面已完成」），只是在 auth.required 時延後到驗證通過。憑證注意：明碼只走本機 bridge；建議 WebView2/`HTDialogHost.verifyAuth`；檔案模式 C++ 讀後立即覆寫 `state=consumed`，不得寫 log。

## 多個 Alert 同時出現：提案（尚未实作）

BCB6 本身就是單一 modal：`ShowErrorMessage` 遇 `fNote->fShow` 直接 `return 0`（記 "Alarm at same time"）；`TfNote::FormShow` 會先 `MyMessageBox->Close()`；`ShowMyMessage` 遇 `MyMessageBox->fShow`：`iUnLoaderCount==0` 直接 return，`!=0`（滿盤不停機訊息）則 Close 舊的再顯示；fNote modal 中 Timer 仍跑，`ShowMyMessage` 可在 Note 上方嵌套。提案依此形式化：

| 情境 | 行為 | HTML | C++ |
|---|---|---|---|
| Message 開啟中，Alarm 到達 | Alarm 取代 Message | 先寫 Message response `state=completed, closedBy=superseded, supersededBy={channel,requestId}`，再開 Alarm | `ShowMyMessage` ShowModal 返回（同 FormShow Close） |
| Alarm 開啟中，Alarm 到達 | 拒絕 | 寫 response `state=error, error.code=DIALOG_BUSY, selectedAction.code=0` | 同 cpp `return 0`；C++ 應在發佈前就擋（fShow 判斷） |
| Alarm 開啟中，Message 到達 | 嵌套（深度 2） | Message overlay 疊在 Note 上（z 20000+1），完成後回到 Note | 同 BCB6 嵌套 ShowModal |
| Message 開啟中，Message 到達 | 依 `request.supersedePrevious` | true → 舊的 closedBy=superseded 再開新；false → `DIALOG_BUSY` | 對應 `iUnLoaderCount!=0` / `==0` |
| 任一 Alert 在 auth 中被取代 | 先 cancelAuth | auth.verified=false，closedBy=superseded | — |

实作需求：從單一 `active` 改為 `stack[]`（最多 2：alarm 底、message 頂）；closedBy 新增 `superseded`（contract 已預留）；error.code 新增 `DIALOG_BUSY`；request 新增 `supersedePrevious`；可選 `Dialog-stack.json`（HTML→C++ 目前堆疊快照）供 C++ 除錯。**不建議** FIFO 排隊顯示多個 Alarm：`ShowErrorMessage` 是 blocking，排隊會讓第二個呼叫點卡在非預期的機台狀態，與 BCB6 `return 0` 語意不同。目前 bridge 行為：單一 active，後到 request 不推進 lastSeq，前一個關閉後才顯示（隱式 FIFO）——待依上表改為取代/拒絕/嵌套。

## Transport

Production C++ atomically replaces both JSON and `JSON/js/*.js` shim files. HTML polls requests every
100 ms. Responses are submitted through `HTDialogHost.submitResponse`, WebView2 `chrome.webview`, or the
debug-only `HTJsonWriter`. A modal remains open when no response transport is connected.