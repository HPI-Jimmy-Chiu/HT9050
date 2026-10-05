---
name: ht9045-alarm-dismissal
description: HT9045 告警「解除」機制知識庫 —— 四種組合（stop / nonstop × note / message）各自用什麼鍵解除、實體 IO 面板鍵與畫面按鈕的分工、解除前的權限閘（DoUnlockPassword / DoPassword / DoPassword_MBox）。當使用者詢問 alarm 怎麼解除、怎麼關掉、Note 關不掉、MessageBox 關不掉、按了 Retry 沒反應、Alarm Reset 為什麼不關窗、K_Pause、K_Retry、ScanKey、ScanPannelKey、SnFKStart、SnFKPause、UpdateButtonStatus、KeyComp、ReturnCode、解除密碼、bAlarmUnlockPassWord、GetJemUnlockPassWord、NonStop 告警怎麼關 等問題時，應先載入此技能。關鍵字：alarm 解除, 解除告警, 關閉 alarm, Note 關不掉, dismiss, ScanKey, ScanPannelKey, SnFKPause, SnFKStart, SnFKRetry, K_PAUSE, KeyComp, ReturnCode, UpdateButtonStatus, DoPassword, DoUnlockPassword, DoPassword_MBox, SpecialPanel, PanSpecialNoteClick, bErrPan_err, SpecialErrNote.ini, special-note, mbox-password, W906_DoPasswordMBox, W906_SpecialPanelLocked, [I37_1] FIFO（D-034）, bAlarmUnlockPassWord, GetJemUnlockPassWord, bDisableKeypad, AlarmReset, NonStop, Alert.Note, MyMessageBox。另涵蓋告警「位置」的顯示：ShowErrorUnit, FlushPanel, iPosition, palSys, arguments.position, display.flushPanel, dbFlush, palInArm, palSafeDoor9, Panel5, tsHandler, 紅框閃爍, 告警位置指錯, Motion View 內嵌, ErrShowToForm, reDescription, reBigDescription, AlarmDescription, Error 資料夾 dat, Alarm-description.json, AlarmCodeList, 單元編號, Edit3, codeUnits, 交叉驗證, Alert.MotionView, Alert.MotionView9050, almDoors, createTrayStatusUnit, sim_alarm, HTDialogBridge.build。另涵蓋 C++ 單槽、網頁排隊時「被丟下的框」怎麼關（S-17，§7.y）：WaitTagScope, WaitNotifyAckReply, WaitOtherReply, no query pending:superseded-by, no-pending-notice:superseded-by, Dialog-close-request recent, 契約 1.3.1, 框關不掉, 排在後面的框出不來。
---

# HT9045 告警解除機制

> **//Steven 20260922** — 本檔的每一條都逐行核對過 golden：
> `HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2`。
> 寫成 skill 的理由：web 端（`Alert.Note.html` / `Alert.MyMessageBox.html` /
> 兩支 `*.NonStop.html`）必須忠實複製這套解除規則，
> 而這套規則的**非對稱之處全部違反直覺**，靠記憶一定會翻錯。
>
> 當日完整變更紀錄：`<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260922_Steven.md`
> 相關技能：`ht9045-html-version`（web 端翻譯規矩）、`ht9045-io-control`（Sen/SW）

## References

| 主題 | 檔案 | 什麼時候看 |
|------|------|-----------|
| **`ShowErrorUnit()` 79 分支對照**（Pos → FlushPanel → Motion View 模組） | [references/showerrorunit-panel-map.md](references/showerrorunit-panel-map.md) | 問「紅框為什麼亮在這裡／為什麼不亮」「position 要怎麼變成 panel」 |
| **落到 `else → palSys` 的 230 個 unit**（A/A?/B/C 分級待填表） | [references/unmapped-units.md](references/unmapped-units.md) | 要補漏掉的軸、判斷某個 `M*` 常數該歸哪個 panel |

兩份都由腳本自 golden 機械產生（來源 `note.cpp:4551-4780` ＋ `cmydef.cpp`），**不要手改產出，要改改產生器**。
`unmapped-units.md` 的「確認 / 備註」兩欄例外 —— 那是人工判定，重產時會讀回保留。

**執行期產物**（由 `scripts/gen_alarm_unit_map.py` 產生，吃上面兩份 ＋ golden ＋ `Alert.Note.html`）：

| 檔案 | 用途 |
|------|------|
| `web/JSON/Alarm-unit-map.json` | 前端拿 `arguments.position` 查 panel／Motion View 模組 |
| `web/JSON/js/Alarm-unit-map.js` | `file://` 傳輸墊片（Edge 封鎖 XHR），內容與 JSON 相同 |
| `web/JSON/Alarm-description.json` | `reDescription` 的內容（`scripts/gen_alarm_description.py` 從 `Error\<語系>\*.dat` 抽全文）|
| `web/page/Alert.MotionView.html` | HT9045 內嵌整機圖（`scripts/build_alert_motionview.py`）|
| `web/page/Alert.MotionView9050.html` | HT9050 內嵌整機圖（`scripts/build_alert_motionview9050.py`）|
| `web/page/alert-motionview-alarm.js` | 兩頁共用的標紅層 |
| `web/page/ht9045_alarm_motionview.js` | 注入層：查表、補 `dbFlush`、疊整機圖、填 `reDescription` |
| `scripts/sim_alarm.py` | 模擬告警（走與 C++ 同一條信箱路徑）|

改完 `unmapped-units.md` 的判定後**要重跑產生器**，否則執行期還是舊的對照。

---

## 0. 一分鐘結論

| | **note**（`fNote`，會停機） | **message**（`MyMessageBox`，通常不停機） |
|---|---|---|
| 解除鍵（實體） | **兩段式**：先按功能鍵選動作（Skip/Retry/TrayFeed/TrayEnd/CleanOut/Reset/Home/OneCycle），再按 **START** 或 **PAUSE** 確認。現場最常見的組合是 **K_SKIP 或 K_RETRY ＋ K_PAUSE** | **一段式**：**PAUSE** 或 **RETRY** 或 **SKIP** 任一鍵，直接關 |
| 解除鍵（畫面滑鼠） | 同上（點畫面上的 BtnXxx + BtnStart / BtnPause） | 點 `pnlPause` |
| Alarm Reset 鍵 | **不解除**，只熄蜂鳴器與塔燈 | **不解除**，只熄蜂鳴器與塔燈 |
| 權限閘 | `DoUnlockPassword()` → `DoPassword()`，**實體鍵與滑鼠兩條路都會過** | `DoPassword_MBox()`，**只有滑鼠路徑會過；實體鍵路徑繞過** |
| 停機 | `ShowErrorMessage()` 內無條件 `StopAllMotor()` | `if(!iUnLoaderCount){ … StopAllMotor(); }` → 多半跳過 |

**NonStop 兩頁（`Alert.Note.NonStop.html` / `Alert.MyMessageBox.NonStop.html`）**：
實體鍵一樣可以解除（走同一條 C++ 路），另外**允許滑鼠直接點畫面按鈕解除**，
且**不要求權限** —— 因為機台根本沒停，攔著操作員沒有意義。

> 口語說的「**K_Retry / K_Skip ＋ K_Pause**」就是 note 這條兩段式：
> 前半是第一段的功能鍵，後半是第二段的確認鍵。
>
> ⚠ 順帶一提：**面板上沒有 Stop 鍵**（不存在 `SnFKStop` / `K_STOP`）。
> 要讓機台停，用的是 **PAUSE**（`SnFKPause` → `BtnPauseClick()` → `SoftStop=true`）。
> 見 §2 的完整鍵表。

---

## 1. Golden 依據（檔案：行）

### note（會停機）

| 位置 | 內容 |
|---|---|
| `note.cpp:2889` | `void __fastcall TfNote::ScanKey()` —— 實體面板鍵的唯一入口 |
| `note.cpp:3404` | `Timer1Timer()` 內呼叫 `ScanKey()`（所以是輪詢，不是事件） |
| `note.cpp:2895` | `Key=ScanPannelKey();`，`-1` 表示沒鍵按下 |
| `note.cpp:2891` | `TBtnPanel *Ptr[]={BtnSkip, BtnRetry, BtnTrayFeed, BtnTrayEnd, BtnCleanOut, BtnReset, BtnHome, BtnTrain, BtnOneCycle};` |
| `note.cpp:2892` | `int Index2[]={SnFKSkip, SnFKRetry, SnFKTrayFeed, SnFKTrayEnd, SnFKCleanOut, SnFKReset, SnFKHome, K_TRAIN, SnFKOneCycle};` |
| `note.cpp:2962-2964` | `if(Ptr[i]->Visible && Key==Index2[i]) { UpdateButtonStatus(Ptr[i]); … }` —— **只選取，不關窗** |
| `note.cpp:3084` | `if(Key==SnFKStart){ … Start(); }` |
| `note.cpp:3087` | `else if(Key==SnFKPause){ … BtnPauseClick(this); }` |
| `note.cpp:3090` | `else if(Key==SnFKAlarmReset && bAlarmBuzzer){ … }` —— **沒有 Close()** |
| `note.cpp:3551` | `TfNote::Start()` |
| `note.cpp:3553` | `int KeyComp[]={K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END, K_CLEAN_OUT, K_RESET, K_HOME, K_TRAIN, K_ONECYCLE};` |
| `note.cpp:3574-3576` | `for(i=0;i<9;i++) if(Select[i])` —— 逐一檢查哪顆被選 |
| `note.cpp:3590` | `if(DoUnlockPassword()==false) return;` |
| `note.cpp:3595` | `if(DoPassword()==false) return;` |
| `note.cpp:3689` | `ReturnCode=KeyComp[i];` |
| `note.cpp:3702` | `Close();` ← **解除發生在這裡** |
| `note.cpp:3707` | `if(KeyCode==0){ … }` —— 沒有任何動作鍵的 note，直接按 Start 也能關 |
| `note.cpp:3715` | 該分支的 `DoPassword()` |
| `note.cpp:3850` | `TfNote::BtnPauseClick(TObject *Sender)` |
| `note.cpp:3905` / `3910` | `DoUnlockPassword()` / `DoPassword()` |
| `note.cpp:4004` | `ReturnCode=KeyComp[i]; SoftStop=true; SoftStart=false;` |
| `note.cpp:4063` | `Close();` |
| `note.cpp:4076` | `KeyCode==0` 分支的 `DoPassword()` |

`Start()` 與 `BtnPauseClick()` 的差別**只在系統要往哪走**，不在解不解除：

```
Start()          -> fMain->Start("fNote::Start 1")   note.cpp:3693
                    SendCommand_ESD(ESD_SYSTEM_START) note.cpp:3701
BtnPauseClick()  -> SoftStop=true; SoftStart=false;   note.cpp:4005
                    SendCommand_ESD(ESD_SYSTEM_STOP)  note.cpp:4062
```

兩者都設 `ReturnCode=KeyComp[i]` 再 `Close()`。
**`ReturnCode` 才是呼叫端（`ShowErrorMessage` 的回傳）拿到的東西**，
「按了哪顆功能鍵」與「用 Start 還是 Pause 確認」是兩個獨立的維度。

### message（通常不停機）

| 位置 | 內容 |
|---|---|
| `mymessbox.cpp:539` | `void __fastcall TMyMessageBox::Timer1Timer(TObject *Sender)` |
| `mymessbox.cpp:563` | `ret=ScanPannelKey();` |
| `mymessbox.cpp:564` | `if(ret==SnFKPause \|\| ret==SnFKRetry \|\| ret==SnFKSkip)` |
| `mymessbox.cpp:566` | `if(bDisableKeypad==false)` |
| `mymessbox.cpp:568` | `Close();` ← **解除發生在這裡，一步到位** |
| `mymessbox.cpp:571` | `else if(ret==SnFKAlarmReset && bAlarmBuzzer)` —— **沒有 Close()** |
| `mymessbox.cpp:445` | `TMyMessageBox::pnlPauseClick(TObject *Sender)` —— 滑鼠路徑 |
| `mymessbox.cpp:453` | `if(bMBoxNeedPassword){ … DoPassword_MBox() … }` |
| `mymessbox.cpp:530` | `Close();` |
| `mymessbox.cpp:1141` | `MyMessageBox->pnlPause->OnClick = MyMessageBox->pnlPauseClick;` |
| `mymessbox.cpp:555` | `if(bSECSGEMAlarm==false)` 包住整段鍵掃描 —— SECS/GEM S10F3 期間**實體鍵全鎖** |

---

## 2. 實體面板鍵的完整清單

`cmydef.h:694-726`，前面板 `SnFK*` 與後面板 `SnRK*` 成對：

```
PowerOff  PowerOn  Reset  Pause  Home  Start  OneCycle
Retry     Skip     CleanOut  TrayFeed  TrayEnd  AlarmReset  CoverOpen
```

後面板另有 `SnRKManualStep`、`SnRKManualTStart`、`SnRKSafeLock`。

> **面板上沒有 Stop 鍵。** `grep -rn "SnFKStop\|K_STOP" --include=*.cpp --include=*.h` 回空。
> `cmydef.h:310` 只有 `extern const int K_PAUSE;`。

`ScanPannelKey()`（`ckernel.cpp:1943`）把前後面板**合併成同一個回傳值**：

```cpp
if(Sen[SnFKPause].IsOn() || bAsePause) { bAsePause=false; ret=SnFKPause; }
if(Sen[SnRKPause].IsOn() || bAsePause) { bAsePause=false; ret=SnFKPause; }   // ckernel.cpp:1985-1993
```

⇒ **後面板 Pause 也回 `SnFKPause`**。web 端不需要區分前後面板。

`ScanPannelKey()` 開頭有三道全域閘，回 `-1` 代表「這一輪所有實體鍵都不算數」：

| 位置 | 條件 |
|---|---|
| `ckernel.cpp:1948` | `SystemInitialOK==false` |
| `ckernel.cpp:1953` | `IsSafeLockCheck()` |
| `ckernel.cpp:1956` | `bEnableEmployeeIDCheck==true`（工號查驗中） |

`bFrontPadActive=Sen[SnRearPadActive].IsOff();`（`ckernel.cpp:1951`）
決定這一輪讀前面板還是後面板。

---

## 3. note 是兩段式 —— 這是最容易翻錯的一點

```
實體 RETRY  ->  ScanKey()  ->  UpdateButtonStatus(BtnRetry)   [只是把按鈕壓下去]
                               視窗仍然開著、機台仍然停著
實體 START  ->  ScanKey()  ->  Start()  ->  ReturnCode=K_RETRY -> Close()
   或
實體 PAUSE  ->  ScanKey()  ->  BtnPauseClick() -> ReturnCode=K_RETRY -> Close()
```

三個必然的推論：

1. **只按功能鍵不會關窗。** 現場「按了 Retry 沒反應」幾乎都是漏了第二段。
2. **`Ptr[i]->Visible` 是前提**（`note.cpp:2962`）。按鈕的可見性由 `ShowErrorMessage()`
   傳進來的 `KCode` 位元遮罩決定 —— 這一則 alarm 沒開放 Retry，
   實體 RETRY 鍵按下去就是完全沒作用，連壓下去都不會。
3. **`KeyCode==0` 的 note 沒有第一段**（`note.cpp:3707` / `4071`）：
   畫面上一顆動作鍵都沒有，直接按 START 或 PAUSE 就關，但**仍然要過 `DoPassword()`**。

`bScanKeyNo==true`（`note.cpp:2965-2968`，TSMC 客製）會讓第一段**只壓按鈕就 return**，
連 `NewRecordProcess` 都不做 —— 要再按一次才算數。

### ScanKey() 前段的一整排 return（`note.cpp:2899-2960`）

這些條件成立時，**實體鍵直接被吃掉**。web 端如果沒複製這段，
會出現「C++ 不給解除但 web 給解除」的致命不一致：

| 條件 | 例外鍵 |
|---|---|
| `CUSTOMER_CODE==CC_ASE_SG && bTesterSendPause` | `SnFKStart` 被擋 |
| `bErrPan_err && Pwd!=""` | 只放行 `SnFKAlarmReset` / `SnRKAlarmReset` |
| `bAutoRetestJam && bOpenAllDoor==false` | 全擋（要先開門） |
| `CosFunction.bOpenDoorCheckLoaderAfterTrayEnd && bOpenLeftDoor==false` | 只放行 AlarmReset |
| `TrayForm.iManualRemoveLoader==2 && bOpenLeftDoor==false` | 只放行 AlarmReset |
| `CosFunction.bPickupErrorAtLoaderNeedOpenDoor && … && bOpenLeftDoor==false` | 放行 AlarmReset + Retry |
| `CosFunction.bEnableHandlerResultServer && bNeedTCPAlarm` | 只放行 AlarmReset（要 IT 下命令解鎖，或 F4：`note.cpp:6493`） |
| `IniConfig.bP59UnloaderICFloattingAlarmAfterExit && iICFloattingCheckStep!=0` | 全擋 |
| `iOldKey==Key` | 防連點（`note.cpp:2956`） |
| `bSECSGEMAlarm && bSECSGEM_NoteAlarm==false` | `Timer1Timer` 在 `note.cpp:3171` 就先 return 了 |

---

## 4. Alarm Reset 鍵不解除告警

兩邊都一樣（`note.cpp:3090`、`mymessbox.cpp:571`），做的事只有：

```cpp
bAlarmBuzzer=false;  bLampAlarmReset=false;
SECS_GEM_PPMUSIC_CONTROL_flag=false;  SECS_GEM_PPSIGNALTOWER_CONTROL_flag=false;
bAlarmReset=true;                     // 記錄「有被按過」
SW[SwFKAlarmReset].Off();  SW[SwRKAlarmReset].Off();
EventReport(SECS_EVENT.DoAlarmReset); // 30
```

**沒有 `Close()`。** 它是「安靜一下」，不是「解除」。
而且要 `bAlarmBuzzer` 為真才會理它 —— 已經安靜的時候再按沒有任何事發生。

---

## 5. 權限閘 —— 有，而且是 per-alarm-code

使用者問「部分 alarm 需要有權限進行解除，這個我們有設計了嗎？」
**golden 有，而且是兩層。**

### 第一層：獨立解除密碼 `DoUnlockPassword()`

```cpp
// note.cpp:5415
bool __fastcall TfNote::DoUnlockPassword()
{
    bool bFlag=true;
    if(CosFunction.bUseAlarmUnlockPassWord==true && bAlarmUnlockPassWord==true)
    {
        fPassword->edPassword->Text="";
        fQwertyKey->ShowQwertyKey(fPassword->edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);
        if(asUnlockPassword==fPassword->edPassword->Text)
            bAlarmUnlockPassWord=false;
    }
    if(bAlarmUnlockPassWord==true) bFlag=false;
    return bFlag;
}
```

哪些 alarm 要密碼，是**逐 code 查表**，在 `FormShow()` 決定：

```cpp
// note.cpp:1496-1503  （TfNote::FormShow）
if(CosFunction.bUseAlarmUnlockPassWord==true)
{
    bAlarmPassWord=fSecurity->GetJemUnlockPassWord(sJamArea, sJamCode);
    bAlarmUnlockPassWord = bAlarmPassWord;
}
```

| 符號 | 位置 | 說明 |
|---|---|---|
| `CosFunction.bUseAlarmUnlockPassWord` | `CosFunction.cpp:847` = true / `:4211` = false | 客戶別總開關 |
| `fSecurity->GetJemUnlockPassWord(area, code)` | `cSecurity.cpp:1019` / `:1131` / `:1571` | 逐 code 的勾選，UI 是 `cbUnlockPassWord`（`cSecurity.cpp:288`） |
| `asUnlockPassword` | `cmydef.cpp:4401`；載入於 `main.cpp:11012-11030` | 從檔案讀，不存在時預設 `"123400"`（`main.cpp:11019`） |

> ⚠ 這組密碼**與登入權限等級（levelset）完全無關**，是另一條獨立的機制。
> 不要把 `GetJemUnlockPassWord` 當成 level 比較。

### 第二層：一般密碼 `DoPassword()` / `DoPassword_MBox()`

| 函式 | 位置 | 呼叫點 |
|---|---|---|
| `TfNote::DoPassword()` | `note.cpp:5261` | `note.cpp:3595`、`3715`、`3910`、`4076` |
| `TMyMessageBox::DoPassword_MBox()` | `mymessbox.cpp:458` | `mymessbox.cpp:457`（`bMBoxNeedPassword`）、`:478`、`:492` |

兩者都是 **`false` 就 `return`，視窗不關**。

### ⚠ message 的非對稱：實體鍵繞過密碼

```
滑鼠點 pnlPause  -> pnlPauseClick()   -> bMBoxNeedPassword -> DoPassword_MBox() -> Close()
實體 PAUSE/RETRY/SKIP -> Timer1Timer() -> if(bDisableKeypad==false) Close()
                                          ^^^^^^^^^^^^^^^^^^^^^^^^
                                          沒有任何密碼檢查
```

`mymessbox.cpp:564-569` 這條路**沒有經過 `pnlPauseClick()`**，
所以 `bMBoxNeedPassword` 與 SECS/GEM 的工號檢查流程全部不會跑。
擋住實體鍵的只有 `bDisableKeypad`（`mymessbox.cpp:52` 預設 false，
在 `Timer1Timer` 的幾個分支與 `pnlYesClick`（`:1185`）裡臨時拉起）。

**這是 golden 的既有行為，不是 bug —— 翻譯時要照抄，不要自作主張補上密碼檢查。**
note 那邊沒有這個洞（實體鍵走 `Start()`/`BtnPauseClick()`，與滑鼠同一個函式）。

### V906 現況（20261002，todo D-034，`AI(W906-D034)`；細節見 `ht9045-login` §10）

- `DoPassword_MBox()`（906 `mymessbox.cpp:1208-1266`（V912 :1228-1286）；權限表第 35 項，＝0 也照樣跳登入框）的四個 golden 呼叫點：
  - Configuration `[I37_1]` FIFO 由關改開（906 `cConfiguration.cpp:7237-7253`（V912 :7353-7369））：**接上**，editlist.save 的 `reauth` point `i37_1`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 檔尾 `W906_DoPasswordMBox`）。
  - `pnlPauseClick` 的 `bMBoxNeedPassword`：**到不了** —— 只有 `ShowMyMessagePWD` 會設它，而它沒移植（筆電的閘 WebStart.cpp W906-ST-W7-C-DLG、PowerSavingMode.cpp GATE (3)）。
  - `pnlPauseClick` SECS 分支、`TSecsAlarmForm::btnOKClick`（V912 才有，906 沒有這張表單）：KYEC_LEE／JSCC_OS／SCC 專屬（RULINGS #25），沒做。
- 上面「實體鍵繞過密碼」那條 message 非對稱照舊（移植樹 `W906MbIoDismiss` 也不問密碼）。

### 第三層：SpecialPanel 密碼（`PanSpecialNoteClick`，只有 note）

- 開框 `TfNote::FormShow` 906 `note.cpp:1564-1606`（V912 :1574-1616）：[I] Index 掉料（JAM0303～0306／0314／0315）或 Tester Time Up（WAR07352）＋ `D:\HT9045\system\SpecialErrNote.ini` `[SUCK] TestSuck==1` ⇒ `bErrPan_err=true`、`Pwd` 讀 `[PASSWORD]`。
- 鎖著時**每一個按鍵都直接 return**：選鍵 `BtnSkipClick` `:2845`、面板 `ScanKey` `:2886-2891`（**Alarm Reset 例外**）、`BtnStartClick` `:3818`、`BtnPauseClick` `:3830`、`BtnResetClick` `:5221`（906 行號；V912 :2867／:2908-2913／:3858／:3870／:5261）。要先點紅色面板 `PanSpecialNote`、打對 `Pwd`（`:5489-5507`）才解；跟登入等級無關。
- 全域、不是每一則各一份：沒解鎖就被關掉的框，下一則（只要 TestSuck 還是 1）照樣鎖著。
- V906：信箱 `auth.kind:"special-note"`、`dialog.auth` 先比特殊密碼、三個閘先擋（`WebLogin.cpp` D-026 段）；面板鍵 `tools/wb_serve.cpp:7324`／`:7326` `W906_SpecialPanelLocked()`。頁面的紅色面板還沒畫（prompt 帶 En／Ch）。

---

## 6. NonStop 兩頁的解除規則

`Alert.Note.NonStop.html`、`Alert.MyMessageBox.NonStop.html`（20260922 新增）。

**NonStop 的定義只有一件事：C++ 那邊不呼叫 `StopAllMotor()`。**

```
會停   note.cpp:805-808   ShowErrorMessage() 內
                          if(Code!="WAR1635") StopAllMotor(); else StopAllMotor(false);
                          兩條路都停機 -- StopAllMotor(false) 只跳過 Galil MTestY1
                          的 VS0;SP0（myGALILmotor.cpp:4713-4719），其餘馬達照停
不停   mymessbox.cpp:303  if(!iUnLoaderCount){ … StopAllMotor(); }  -> iUnLoaderCount!=0 就跳過
```

解除規則（使用者 20260922 裁定，與 golden 的停機路並存）：

| | NonStop |
|---|---|
| 實體 IO 鍵 | ✅ 可以，走與停機版**同一條** C++ 路 |
| 畫面滑鼠 | ✅ **可以**，單一「確認」鍵直接解除（停機版不給） |
| 權限 | ❌ **不要求**。機台沒停，攔操作員沒意義 |
| 動作鍵 | ❌ 沒有。`kCode` 若非 0 代表路由判錯，`ht9045_nonstop_page.js:73` 會 `console.error` |

路由表：`web\config\AlarmNonStop.json`（＋`file:` 協定用的 `AlarmNonStop.js` 墊片）。
四層判斷順序見該檔 `decideBy`：

1. `requestedSideEffects.stopAllMotor === false`
2. request 上的 `nonStop` 旗標
3. `codes` / `prefixes` 查表
4. 保守預設 —— **判不出來就走會停機的那一頁**

---

## 7. web 端現況與缺口（20260922）

| 檔案 | 角色 |
|---|---|
| `web\page\dialog-bridge.js` | 兩層：`#dialogBridge`（z-index 20000，模態、會停機）與 `#dialogNonStop`（21500，非模態、不擋滑鼠） |
| `web\page\Alert.Note.html` / `dialog-page.js` | 停機版 note，九顆動作鍵的互斥選擇 |
| `web\page\Alert.MyMessageBox.html` | 停機版 message |
| `web\page\Alert.*.NonStop.html` / `client\ht9045_nonstop_page.js` | 不停機版，單一確認鍵 |
| `client\ht9045_nonstop_alarm.js` | 路由表載入（協定感知）、`route()`、`raise()`、`requireLevel()` |
| `web\page\Alert.Password.html` | 密碼輸入。**驗證一律由 C++ 做，HTML 只負責收輸入** |

`dialog-bridge.js` 的 `inspectClose()` 先看 `activeNS` 再看 `active`，
所以實體 IO 鍵送進來的關閉訊號能打到 NonStop 層 —— 這是 20260922 修掉的缺陷
（原本 NonStop 層收不到實體鍵）。

### ⚠ 20260922 實機驗證抓到的缺陷（已修）

`dialog-bridge.js` 的 `sendRequest()` 原本寫成：

```js
if (!f || !active || active.displayKind !== kind) return;
```

**只看 `active`，不看 `activeNS`。** 而不停機那一層刻意不共用 `active`，
所以 `renderNonStop()` 末尾那次 `sendRequest()` 必然在這一行被擋掉。
一個根因、兩個症狀：

1. 不停機的視窗是**空白的** —— 頁面從來沒收到 `HT_DIALOG_REQUEST`
2. **滑鼠關不掉它** —— 頁內 `current` 永遠是 `null`，
   `ht9045_nonstop_page.js` 的 `acknowledge()` 開頭就 `return`，
   `HT_DIALOG_ACTION` 根本沒送出

而「允許用滑鼠點畫面按鈕解除」正是不停機版和停機版**唯一**的差別
⇒ 整個功能等於是死的。

修法：先比 `activeNS` 再比 `active`（兩組 displayKind 互斥，不會有歧義）。

**這個缺陷靜態讀碼看不出來，只有真瀏覽器跑得出來。**
回歸測試：`tools\web-client\run_nonstop_selftest.py`
（`file:` 與 `http:` 兩種協定各 41 項；修正前 6 FAIL、修正後 0 FAIL）。

> ⚠ 測試本身的坑：`postMessage` 是**非同步**的。
> 第一版在 `raise()` 之後**同步**檢查內容，對修正前／修正後都失敗，
> 看起來像抓到 bug 其實是測試寫壞。每一步之間都必須真的等。

**已知缺口（尚未實作）：**

- 沒有把 §3 那張「ScanKey() 前段 return 表」翻到 web 端。
  目前 web 端只要收到鍵就當有效，C++ 擋掉時兩邊狀態會短暫不一致。
- `DoUnlockPassword()` 的獨立解除密碼沒有對應的 request 欄位，
  只有一般的 `auth: {required, kind, level, title, prompt, userIdRequired, defaultUserId}`。
- 兩段式（選取 → 確認）在 web 端已有，但**沒有複製 `bScanKeyNo` 的「再確認一次」**。

---

### 7.x ⛔ 20260930：kCode==0 的通知型告警框怎麼關（INBOX 119，Jerry J-5；`AI(W906-J5-ACK)`）

- 以前：kCode==0（ForwardShowErrorMessage 的 kcode==0 那一支、golden ShowMotorErrorMessage 的每一則 note）背後沒有等待迴圈，沒有 WS query、`dialog.response` 被拒，`D:\HT9045\web\page\ht9045_dialog_host.js` 在沒有 pendingQuery 時一律 reject ⇒ 框關不掉、重新整理又跳，只能重開 wb_serve。
- 現在：C++（筆電）新 WS 指令 `dialog.notifyAck`（tag＝requestId；契約寫在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp`「THE NOTICE CONTRACT」、檔尾 W906_NoticeAckCommand）；St01 的 `ht9045_dialog_host.js` submitResponse：沒有 pendingQuery、而且按的是「確認」（ACKNOWLEDGE／code 0）⇒ 送 `dialog.notifyAck`。ok:true 關框；ok:false `no-pending-notice` 也關框（早就關了）；`not-operator` 續一次權杖再送；其他 ok:false 框留著（可再按）。
- 頁面分不出是不是通知（closePolicy／buttons 兩種都一樣），所以由 C++ 核對這個 id 是不是 kCode==0 的通知；不是就回 `not-a-notice`，什麼都不做。
- 有 pendingQuery 的停機告警照舊走 `modal.answer`；訊息通道（show-my-message）照舊走 `dialog.response`。
- 還沒做（筆電 INBOX 122）：golden 按面板 START 也會關通知框（`TfNote::Start` KeyCode==0 那一段）。

### 7.y ⛔ 20261003：C++ 單槽、網頁排隊 —— 「被丟下的框」一律要能關（筆電卡 S-17＝INBOX 137；`AI(W906-S17)`，St01）

- 病根：C++ 每個信箱只有**一格**（Alarm-／Message-dialog-request、Dialog-close-request），網頁 `dialog-bridge.js` 卻把每一則**排隊**（`queueStop` 永不丟）。C++ 已經丟下的那一則（被新的覆蓋、被取代的不停機訊息框、別處答掉、實體鍵關掉）還在網頁上；它的回答要是被「留框」，排在它後面、C++ 正在等的那一框就永遠出不來，等待迴圈永遠不結束。
- 判準（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_dialog_mailbox.h`）：**沒有任何等待迴圈握著的 tag ＝ 被丟下的**。三個等待迴圈（`tools\wb_serve.cpp` 的 ForwardShowErrorMessage、ForwardShowMyMessageBoxYesNo、MbWait）進迴圈時用 `w906dlg::WaitTagScope` 登記自己的 qid（巢狀等待各自登記，外層不會被當成過期）；目前開著的不停機訊息框由 `W906_MsgBoxModelessAnswer` 先收。
- 各條路現在回什麼（都在等待迴圈裡）：
  - **A** `modal.answer`／`dialog.response` 的 tag 沒人握著 → `no query pending:superseded-by=<自己的 qid>`（`WaitOtherReply`；以前是 `modal-pending`＝留框）。網頁兩個畫框器都已經把 `/no query pending/` 當已關（`ht9045_dialog_host.js` 訊息通道、`ht9045_modal.js`），所以 JS 不用改正則。主迴圈本來就回 `no query pending`。
  - **E** 阻塞告警等待中收到 `dialog.notifyAck` → `WaitNotifyAckReply`：被覆蓋／較舊、沒人握著的通知 → `no-pending-notice:superseded-by=<qid>`（網頁關框）；自己那一則、空 tag、沒發過的 id → `not-a-notice`（留框，同以前）。以前一律 `not-a-notice` ⇒「通知 26 → 阻塞告警 27」卡死（J5-ACK-2 的 superseded 只在主迴圈有效）。YES/NO、MbWait 兩個等待迴圈同樣處理。
  - **B** `NotifyAckDecide`：id 只會變大（`g_nextQid++`），比格子裡那一則**舊**、又沒人握著的 → superseded。J5-ACK-2 的清單只留 64 筆，無人值守的迴圈（ShowLoadingIC 4 秒 8 則）半分鐘就超過，最舊的又會回 `request-mismatch`。
- 網頁那一半：
  - **C**（`ht9045_dialog_host.js`）告警框用**它自己的 requestId** 回答（pendingQuery 過期／沒有／是別的告警時）；通知的「確認」只要 requestId 不是目前 pendingQuery 就走 `dialog.notifyAck`（以前會被當成別的告警的 `modal.answer`）；告警回答收到 `no query pending` ＝ C++ 已不在等 → 關框。
  - **D**（`dialog-bridge.js`，契約 1.3.1，Steven 1003 同意改；`web\JSON\Dialog-bridge-contract.json` schemaVersion 1.3.1＋`dialogClose.recentRule`，js 墊片用 `_gen_json_shim.py` 同一種寫法重產）`Dialog-close-request` 多一個 `recent`（最近 8 則關閉，舊的在前、含最新）；`inspectClose` 跑過每一則 seq 比上次新的；目標還在**佇列裡**（還沒畫）的也直接移出，不會晚點畫出一個答不掉的框。C++ 端：`W906_DialogCloseRequest` 用 `w906dlg::CloseRequestWithRecent`／`CloseEntryJson`。
- 沒做（同類，待決）：YES/NO、MbWait 等待中，若告警信箱裡還有一則**活的**通知（不是被覆蓋的），按它的確認仍是 `not-a-notice`（留框）——要在等待迴圈裡退役它得跑 golden 的關框（W906_NoticeAckCommand），交 ST01-M／Steven 決定。
- 測試：`tests\test_notice_ack.cpp` [Q]（Q1 B、Q2 E 照阻塞告警等待迴圈的呼叫順序、Q3 A、Q4 D、Q5 三個等待迴圈與關閉寫入的原始碼釘）＋ node `S17_DialogHostPage`（`tools\webprobe\s17_dialog_host_selftest.cjs`）、`S17_DialogClosePage`（`s17_dialog_close_selftest.cjs`），兩支都有 CONTROL（指到改之前的 js 必須紅）。
- 與 St02 的面板鍵關不停機框（ScanKey 合併 MR，R188 M2）：被面板鍵關掉的框 tag 也沒人握著 → 同樣回 `no query pending…`，網頁關框；本次沒有動 `MbCloseModeless`、`s_mbModeless.qid = qid;` 那一行。

## 7.5 ⛔ 一定要停機的四類 —— 公司鐵律

**ESD alarm／安全門沒關／溫度異常／EMG 被按下去。**

停不停機**完全由 C++ 主控**，HTML 端不介入也無力介入 ——
`StopAllMotor()` 在 request 送到瀏覽器之前就已成定局。
HTML 端唯一的職責、也是唯一會出事的地方：
**不可以把這四類畫成「機台未停機，仍在運轉」。**

強制機制：`web\config\AlarmNonStop.json` 的 `neverNonStop` 是一張否決清單，
在 `route()` 的**最前面**執行，壓過所有判準 —— 包含
`requestedSideEffects.stopAllMotor === false`。
命中且 request 宣稱不停機時，`console.error` 吼出契約衝突並強制走停機頁。

| 類別 | golden | 代表碼 |
|---|---|---|
| EMG | `csystem.cpp:4483-4488` | `WAR1630`-`WAR1634`、`WAR16140` |
| 安全門 | `cmydef.cpp:1790-1795` / `:1808-1810`，發於 `csystem.cpp:2658` / `:2732` | 30 個 `MES16xx` ＋ Hatchway 12 個 ＋ `WAR16125` |
| ESD | `csystem.cpp:3434/3500/3516/3532` | `WAR2090/2095/2096/2097`，**整個 `WAR20` 前綴** |
| 溫度 | `cTemperFrom.cpp:1313/:1327` 動態組碼 | **整個 `WAR15` 前綴** ＋ `WAR1637` 等固定碼 |

完整規範與維護方式：`.github\specs\machine-stop-policy.md`
回歸測試：`run_nonstop_selftest.py` 的 F1～F10（每一項都用「C++ 說不停機」去撞）

---

## 7.6 告警佇列 —— 停機與不停機分開兩條

**多個 alarm 會連續出現**，golden 為此專門寫了保護（`note.cpp:824-828`）：

```cpp
if(fNote->fShow)
{
    MyDBIProcess("Exception", "Alarm at same time: "+Code, errPart);
    return 0;                          // <- 第二則直接丟棄，不排隊
}
```

其他證據：`TfNote::ScanKey()` 在 fNote **顯示中**還會呼叫 `ShowErrorMessage`
（`note.cpp:3069` / `:3077`，真正的巢狀）；全樹呼叫點 **1950 個**。

⚠ golden 丟棄那一則是**可接受**的，因為 `return 0` 在 `StopAllMotor()`
（`:805-808`）**之後** —— 機台已經停了。
但 web 版是兩個行程、兩條通道，「沒顯示」可能是操作員唯一會察覺的訊號。

⇒ **使用者 20260922 裁定：停機告警的佇列永不丟棄**（刻意偏離 golden）。

| 佇列 | 規則 |
|---|---|
| `queueStop` | **永不丟棄**。深度 >= 3 時 `console.error` 出聲，但一則都不丟 |
| `queueNS` | 上限 8，滿了丟**最舊**的（保留最新，符合 golden 的覆蓋語意） |

實作在 `web\page\dialog-bridge.js`，可觀測介面 `HTDialogBridge.queues()`。

⚠ 舊做法（`if (active) return;` 且**不推進** `lastSeq`，下一輪再讀）**不是佇列**，
是重讀 —— 它依賴 C++ 把 request 檔維持在 pending。
單槽檔案只要被 C++ 覆寫，前一則就永久消失且沒有痕跡。
現在改成讀到就**立刻推進 `lastSeq` 並入列**。

---

## 8. 陷阱清單

1. **面板上沒有 Stop 鍵**（`SnFKStop` / `K_STOP` 在整棵樹不存在）。
   要停機用的是 **PAUSE**（`SnFKPause` → `BtnPauseClick()` → `SoftStop=true`、`ESD_SYSTEM_STOP`）。
2. **Alarm Reset ≠ 解除。** 只熄蜂鳴器，沒有 `Close()`。
3. **note 兩段、message 一段。** 不要把 note 的規則套到 message。
4. **message 的實體鍵不只 PAUSE** —— `SnFKPause || SnFKRetry || SnFKSkip` 三個都關。
5. **message 的實體鍵不過密碼**，滑鼠才過。非對稱是 golden 的既有行為。
6. **`ReturnCode` 與「用 Start 還是 Pause 確認」無關**，它只記錄按了哪顆功能鍵。
   影響機台走向的是 `SoftStart`/`SoftStop` 與 `SendCommand_ESD()`。
7. **`Ptr[i]->Visible` 是功能鍵生效的前提。** KCode 沒開放就是死鍵。
8. **SECS/GEM S10F3 期間整組鍵鎖住**（`note.cpp:3171`、`mymessbox.cpp:555`），
   `bSECSGEM_NoteAlarm` 才會放行。
9. **前後面板合併**在 `ScanPannelKey()` 裡就做完了，上層看不到差別。
10. **`iOldKey==Key` 防連點**（`note.cpp:2956`）：同一顆鍵按住不放只算一次。
11. **`kCode` 與 `position` 是兩回事。** 前者決定「有哪些解除鍵」，後者決定「哪個面板閃紅」。`kCode==0` 的通知一樣會有 `position`；
    `position` 對不到也一樣會有解除鍵。不要用其中一個推另一個。
12. **V906 上紅框不會亮**（`flushPanel` 恆 `null`，見 §9.2）。看到「按鍵正常但沒有紅框」，那是移植缺口，不是 IO 壞掉。

---

## 9. 告警「位置」的顯示 —— `ShowErrorUnit` / `FlushPanel`

> 前面八節講的是**怎麼關掉**；這一節是**告訴操作員哪裡出事**。
> golden 裡是兩個互不相干的函式，但在 web 端是同一則 request 的兩個欄位
> （`arguments.kCode` 決定有哪些解除鍵、`arguments.position` 決定哪個面板閃紅），
> 混淆的代價是「按鍵對了但紅框指錯地方」，所以收在同一支 skill。

### 9.1 完整的一條線

```
ShowErrorMessage(Code, KCode, Pos, ...)        golden 2000 個 call site
        │                                       Pos = cmydef.cpp 的 unit id（0..573）
        ▼
ShowErrorUnit(Pos)            note.cpp:4551     79 個 else-if
        │  iPosition = Pos    note.cpp:4554
        ▼
FlushPanel = fNote->palXxx    note.cpp:4556-4779
        │  對不到 → 最後一個 else → palSys（:4776-4779）
        ▼
Timer1Timer                   note.cpp:3418-3425
        FlushPanel->Color = FlushFlag ? clRed : cDark      ← 紅框閃爍就是這兩行
        bWriteErrRefFlag 時直接 clRed（:3425，不閃，恆紅）
```

web 端：

```
C++   tools/wb_serve.cpp  DialogMailboxPostAlarm()
        "arguments":{"code":"%s","kCode":%d,"position":%d, ...}   ← position 已經在寫真值
        "display"  :{... ,"flushPanel":null}                      ← 硬寫 null
HTML  page/dialog-page.js:75
        if (d.flushPanel && $(d.flushPanel)) $(d.flushPanel).classList.add('dbFlush');
```

### 9.2 V906 的缺口 —— 以及它**不需要**翻譯

`ShowErrorUnit` 在移植樹**全樹零命中**（`forms/fNote.h:468-470` 自己記了這件事），
所以 `display.flushPanel` 恆為 `null` ⇒ **906 上那張 Handler 圖從頭到尾沒亮過**。

但 `Pos` 一路都在：

| 環節 | 位置 |
|---|---|
| hook 簽章 | `canary_support.cpp:86`　`int (*W906_ShowErrorMessage_Hook)(const char* Code, int KCode, int Pos)` |
| 轉發 | `tools/wb_serve.cpp`　`ForwardShowErrorMessage(code, kcode, pos)` |
| 寫信箱 | 同檔 `DialogMailboxPostAlarm()`　`"position":%d` |
| 事件流 | `JsonBridge/ChanAlarm.cpp:204`　`w.Key("position").Number(e.pos)` |

⇒ **不必把 79 個 else-if 翻成 C++**。那整段的唯一產出是一個 panel 名字，
前端拿 `position` 查 [references/showerrorunit-panel-map.md](references/showerrorunit-panel-map.md)
就有。C++ 一行都不用改。

### 9.3 四個一定會踩的點

1. **`palSys` 是 golden 的預設，不是 fallback bug。**
   `note.cpp:4776-4779` 的 `else { FlushPanel=fNote->palSys; }`。
   339 個 unit 裡有 **230 個**走這條，其中 83 個（C 級）走這條是**正確的**
   —— 那些是本機不存在的選配機構。不要為它們新增面板。

2. **範圍分支只涵蓋 `0..10` 與 `19..29`。**
   `if(Pos<=MInArmZH)` 只到 10、`Pos>=MOutArmX && Pos<=MOutArmZH` 只到 29。
   所以 `MInArmPitchY`(31)、`MInRotateKit`(41)、`MOutSortAa`(57)、
   `MLoaderY`(44)/`MEmptyY`(45)/`MColorY`(46)/`MAuto1Y`(47) 這些
   **變距軸、Rotator、Tray Y 軸全部掉到 `palSys`** —— 紅框亮在「System」，
   操作員完全看不出是哪一支手臂。這是真缺陷，清單見
   [references/unmapped-units.md](references/unmapped-units.md) 的 A 級 81 筆。

3. **`else if(Pos==MMOCR)` 後面帶行尾註解**（`//ChungHung 20120830 add OCR Function add`）。
   機械抽取若把「右括號後面緊接換行或左大括號」當成條件結尾，會漏掉這一條，
   讓 `palOCR` 被錯配到它後面第一個 `if(FileExists(...))`。
   這段裡只有這一處，但產生器必須保留對行尾註解的處理。

4. **料盤軌的 Z 軸屬「料車」，Y 軸屬「軌道」—— 兩族不同 panel。**

   | 軸族 | panel | golden 依據 |
   |---|---|---|
   | `M{軌}Z`：`MLoaderZ`(35) `MEmptyZ`(36) `MColorZ`(37) `MAuto1-6Z` `MLoad2Z`(158) | `pal{軌}_Car` | `cmydef.cpp:2658` `iTrayZMotor[]`，`iosetview.cpp:3489` 走 `Prod.TrayZ_Up/TrayZ_Mid` —— **料車升降**。對齊 golden 既有的 `MMTrayZ(245) → palLoad_Car` |
   | `M{軌}Y` / `_CCW`：`MLoaderY`(44) `MEmptyY`(45) `MColorY`(46) `MAuto1-6Y` `MLoad2Y`(159) | `pal{軌}` | `asendic.cpp:1294` `iStepMotor[]`，`LOAD_Y_USE_MOTOR` 的**軌道進出盤**。對齊 golden 既有的 `MMTrayY(167) → palLoad` |

   ⚠ 兩族名字只差一個字母，很容易一起歸到軌道面板 —— 那樣料車的升降異常會亮在盤位上。
   `MLoad2Z` / `MLoad2Y` 對到 `palLoad2_Car` / `palLoad2`（`note.cpp:1455-1456`，
   `USE_2nd_LOADER != eartUninstall` 才顯示；`ShowErrorUnit` 沒有它們的分支）。

   （Steven 20260924 指出，回查 golden 確認。）

### 9.4 tsHandler 的版面（改畫面前必讀）

`Alert.Note.html` 的 12 個分頁 `TabVisible` 全為 false（`note.cpp:226-229`），
`FormShow` 與 `Reset()` 都把 `ActivePage` 釘在 `tsHandler`（`:230`、`:265`）
—— **操作員永遠只看得到 tsHandler**。

它的結構只有兩層：

| 層 | 內容 |
|---|---|
| 底圖 | `PanelMain6 > Panel5`（947×531），整張機構圖 |
| 訊息槽 | **11 個面板共用同一個矩形 `(160, 290, 680, 160)`**，互斥出現：`palCheckSht` `palCheckLoader` `pnlOutArmDrop` `PanSpecialNote` `pnlIndex1Error` `pnlIndex2Error` `pnlContactOver` `pnlCleanSocket` `pnlContact` `palSGCheckList` `pnlPan_TriMachineSpecialNote`；另有 `pnlLotInfo`(28,8,361,197)、`t2DCode`(20,12,687,257)、`palWrongPW`(160,130,680,160) |

`note.cpp` 共有 **27 處** `->Parent=tsHandler`（16 個不同控制項）。
**要改這張畫面，先確認這 27 處還有家。**

`Panel5` 的縱向分帶（相對座標，改版面用這組數字）：

| 帶 | y | 內容 |
|---|---|---|
| 機構帶 | 4 – 233 | Shuttle／Test Arm／Temperature／Scanner／CCD／System／Interface／ION FAN 1-8,11 ＋ `In Arm`・`Out Arm` 條（208–233）|
| 料盤欄 | 235 – 355 | Plate1、Loader、Empty、Color、Auto1-6、`palTrayArm`（247–329）|
| Tray Arm 軌 | 361 – 381 | `palTrayArm2` |
| **Safe Door 9** | **381 – 401** | `pnlTrayCar` 的第一個子元件，**就在 Tray Arm 軌正下方** |
| 料車 | 401 – 531 | `*_Car` ×9、ION FAN 9/10/12、TrayEdit |

安全門是一圈，而且**十道門有九道在 `Panel5` 之外**：

```
    ┌─ pnlSafeDoorRear（PanelMain6 y0..23，947 寬）───────────────┐
    │  SD3 0..175 │ SD4 175..350 │ SD10 350..597 │ SD6 597..772 │ SD5 772..947 │
┌───┼──────────────────────────────────────────────────────────┼───┐
│SD2│  y 0..220                                                │SD7│
│   │              Panel5   947×531                            │   │
│SD1│  y 220..424                                              │SD8│
├───┼──────────────────────────────────────────────────────────┼───┤
│P8 │  Tray Arm 軌（361）／ Safe Door 9（381）／ 料車（401–531）     │P9 │
└───┴──────────────────────────────────────────────────────────┴───┘
  pnlSafeDoorLeft x0..26                        pnlSafeDoorRight x973..999
```

⚠ 後排的**視覺順序不是編號順序**：左到右是 **3 → 4 → 10 → 6 → 5**。照抄，不要「修正」。

### 9.5 已實作：把 Motion View 內嵌到 `Panel5`

（20260924 完成。切線原訂 `y 0..355`，**後來改成吃滿整個 Panel5** ——
門環與料車都畫進斜投影之後，下半段沒有還需要露出的 dfm 元件，
而 viewBox 因為多了那兩樣從 500 長到 632，高度不給滿字會糊。）

內嵌區取 `947×355` 之後的遮蔽結果已經逐一算過，寫在
[references/showerrorunit-panel-map.md](references/showerrorunit-panel-map.md) 的 `遮蔽` 欄：

- **十道安全門全部不會被蓋**（九道在 `Panel5` 外、SD9 在 381 > 355）
- ATC 九格、`palOCR`、九個 `*_Car`、ION FAN 9/10/12 也都在切線外
- 真正需要退回 tsHandler 原圖的**只有 14 個 panel**：
  `palIonFan01`–`08`、`11`、`palTemp`、`palScan`、`palCCD`、`palSys`、`palIF`

⚠ **切線不可以取 267**。那個數字是從訊息槽 `(160,290,680,160)` 反推的，
會把料盤欄（235–355）從中間切開，Loader 與 Auto 盤會露出下半截。

### 9.6 機種差異 —— HT9050 上目前有兩處錯位

告警視窗**不分機種**：`dialog-bridge.js:71` 的 `PAGES.alarm` 永遠是
`page/Alert.Note.html`，而那頁是 HT9045 `note.dfm` 的產物。
所以在 HT9050 上，操作員看到的是 HT9045 的機構圖。已知兩處會誤導：

| # | 錯位 | 說明 |
|---|---|---|
| 1 | **料車** | HT9050 的 Tray Table 是固定檯面，**沒有料車**。但 `Mot_Table_9050.csv` 裡 `MLoaderZ` `MEmptyZ` `MAuto1-3Z` 都是 `Enable=1`，報警時會亮 `palLoad_Car` 等面板 —— 一個該機種不存在的機構。（`MColorZ` 是 `Enable=0`，9050 沒有 Color 軌。）|
| 2 | **安全門** | HT9045 是十道（左 2 / 後 5 / 右 2 / 前 1），HT9050 是八道且方位不同（前 2 / 後 3 / 左 1 / 右 1 ＋ Index 開檢）。見 §9.4 的門環圖與 `ht9050-hw` 的 `hardware-overview.md`。|

兩處都是**既有缺陷，不是內嵌 Motion View 造成的**；但既然要做機種切換，
最小修補是在同一支前端 script 裡依 `MachineTypeChoice` 改寫顯隱與 caption，
不必動 `note.dfm`。

> ⚠ 前端目前**拿不到** `MachineTypeChoice`。
> `Type_HT9050 = 800` 只存在於 V906 樹（`MachineType.h:553`，golden V912 只有七個機種），
> 而且全樹 0 處比對它，`WebBridgeTags` 也還沒發布這個值
> （見 `ht9050-hw` 的 `references/machine-type-entry.md` §2）。
> 要做機種分岔，得先補一個 `machine.typeChoice` tag。

### 9.7 待新增的面板：`palMagazineTray`

Magazine 家族原本落在「維持 `palSys`」那一級。**Steven 20260924 裁定改為長出新面板** ——
理由是它不是「本機沒有的機構」，而是「這台機裝了、但畫面沒畫」。

| 項目 | 值 | 依據 |
|---|---|---|
| 名稱 | `palMagazineTray` | 新增 |
| Parent | `pnlTrayCar` | 與 `palAuto3_Car` 同層 |
| 位置 | `Left=624 Top=24 Width=70 Height=120` —— **與 `palAuto3_Car` 完全重疊** | `note.dfm` 的 `palAuto3_Car` |
| `Panel5` 相對 y | **405** | `pnlTrayCar`(381) ＋ 24 |
| 顯示條件 | `AUTO3_IS_MAGAZINE == 1` | `cmydef.h:3081`；`acatchtray.cpp` / `aoutarm9045.cpp` 全程用這個旗標分岔 |
| 互斥 | 顯示時 `palAuto3_Car` 隱藏 | 同一個 Auto3 位置，硬體二選一 |
| 涵蓋 unit | **33 個** | `MMagazine`(140) `MCatchMgzTray`(141) `MMagYTrayOut`(142)、`MMMagazineTary1-14`(215-228)、`MMMagazineTaryTop`(229)、`MMMagazineBuffer`(230)、`MMBackupMagazineTary1-14`(231-244) |

⚠ **必須跟著 `palAuto3_Car` 一起搬位置。**
`note.cpp:1415-1432`：`AUTO_EMPTY_COLOR>=3` 且 `USE_OUT_SORT_ARM != eartUninstall` 時
`palAuto3_Car->Left=856`。新面板漏掉這段，9 軌機種上會停在 624 疊到 Auto6 上。

✅ `Panel5` y=405 **在內嵌 Motion View 的切線（355）之外** ——
不會被蓋住，整機圖也不必新增模組。

> ⚠ `MMBackupMagazineTary1`–`14`（231–244）在 golden **只有 `cinitial.cpp:3351-3364`
> 的 `SetAlias`，全樹其餘零使用** —— 今天沒有任何路徑會把它們送進 `ShowErrorUnit`。
> 一併歸到 `palMagazineTray` 無害；要不要拆成獨立的 `palBackupMagazineTray`，
> 等它真的被用到再決定。

完整清單與其餘分級見
[references/unmapped-units.md](references/unmapped-units.md) 的 B1 級。

### 9.8 `reDescription` 的內容 —— `TfNote::ErrShowToForm()`

`note.cpp:4347-4520`。**`DEBUG_NEW_ALARM_DESCRIPTION` 在 `MachineType.h:17` 是註解掉的**，
所以 `AlarmDescription.ini` 那條（`#ifdef`）不會執行，跑的是 `#else`：

```
Language = Chinese / English / Korea / Singapore
           CUSTOMER_CODE==CC_KYEC_LEE            -> Chinese（KYEC 要中英並列）
           IniConfig.iUserLanguage==eulKorea     -> Korea
                                  ==eulEnglish   -> English
                                  ==eulSingapore -> Singapore
           其餘看 LastSet.iLanguageCountry：0 -> English，否則 Chinese

TextPath = D:\\HT9045\\Error\\<Language>\\<Code>.dat          （iMotorErr == -1）
         = D:\\HT9045\\Error\\<Language>\\MOT<iMotorErr>.dat  （馬達錯誤，note.cpp:1112）

if(FileExists(TextPath)){
    reDescription->Lines->LoadFromFile(TextPath);
    reBigDescription->Lines->LoadFromFile(TextPath);      // Ifor 20200331
}
```

同一支還填 `edErrorCode=Code`、`Edit3=Code.SubString(4,2)`、`edUnitName=UName`、
`ShowMessageEdit1=Mes`、`sAlarmMes=Mes`，並 `PlayMovie("D:\\Movie\\<Code>.avi")`。

⚠ **`FileExists` 不成立時 golden 什麼都不做** —— `reDescription` 停在 `Clear()` 後的空白，
  **不是**顯示「查無資料」。web 端照做：查不到就不覆寫、不編內容，只在 console 說一聲。

**web 端原本拿不到全文。** `AlarmDescriptionOverride-index.json` 自己的 note 就寫
「僅列碼別，不含全文」。所以加了 `scripts/gen_alarm_description.py`，
從 `Error\\<語系>\\*.dat`（cp950）抽出來：

| 語系 | .dat 數 |
|---|--:|
| Chinese | 600 |
| English | 938 |
| Korea | 360 |
| Singapore | 377 |
| **有全文的碼（聯集）** | **948** |
| 短描述（`AlarmCodeList-index.json`）| 3079 |

⚠ **`reDescription` 在 dfm 是 `TRichEdit`，產生器產成 `<textarea>`** ——
  `textContent` 對 textarea 只改「預設值」，畫面上不會變，**要設 `.value`**。
  `dialog-page.js:15` 的 `setText()` 早就處理過這件事。

⚠ 兩個還接不了的：
  - **`MOT<n>.dat`**：request 沒帶 `iMotorErr`，`ForwardShowErrorMessage(code,kcode,pos)` 簽章裡沒有。
  - **語系**：golden 由 C++ 決定，前端拿不到。目前 `?lang=` → `localStorage` → 預設 Chinese、英文備援。

`D:\\HT9045\\docs\\manual\\AlarmCode_Manual`（ZH/EN/JA/KO，20260804，3096 碼，第 4 章有原因對策）
是**文件**，比 `.dat` 完整，但**不是程式顯示的那份**。要接是另一件事，不可拿它冒充。

### 9.9 交叉驗證：code 的單元編號 vs position

Alarm Code = `前綴` + **單元編號 2 碼** + `代碼`（手冊 `AlarmCode_Manual` §1）。
golden 自己就把它拆出來顯示：`note.cpp:4408` `Edit3=Code.SubString(4,2)`，畫面上的 `Subsidiary` 欄。

於是同一則告警有**兩個獨立來源**在講「哪個單元」：

| 來源 | 路徑 |
|---|---|
| (a) | `arguments.position` → `Alarm-unit-map.units` → panel |
| (b) | code 裡的 2 碼單元編號 → `Alarm-unit-map.codeUnits` |

**golden 不可能不一致** —— 兩者都來自同一個 `ShowErrorMessage` 呼叫點。
例：`JAM0109` 的四個呼叫點全是 `ShowErrorMessage("JAM0109", ..., MInArmX, ...)`，
單元編號 01 = 入料臂，position = `MInArmX` → `palInArm`，對得起來。

⇒ **對不上就是產生端餵了不可能的組合**，兩處都會擋：

- `scripts/sim_alarm.py` 直接拒絕（exit 2）並建議正確的 `--position`
- 前端 `console.error` ＋ 橫幅變深紅加 ⛔ 說明

`check:false`（不參與比對）的是「單元號不指向單一機構」的那幾個：
`00 Event`、`16 System`、`20 ESD`、`21/22 Log`、`23 Cassette`，
以及 **`24 Motor`（1287 碼）與 `31 Cylinder`（275 碼）—— 位置由 position 決定**。

> 這道守門是 20260924 使用者發現「描述寫 Input Arm、紅框卻標 In Shuttle」之後補的。
> 那次是模擬工具造出的假資料（`--position 11 --code JAM0109`），不是接線錯 ——
> 但「有兩個獨立來源卻不互相驗證」本身就是缺陷。

### 9.10 逐 unit 的 mv 覆寫

`tsHandler` 沒有專屬面板、但整機圖**畫得出來**的機構。
`units[].mv9045/mv9050` 若存在就**蓋過** `panels[panel]` 的值；
panel 維持 `note.dfm` 的事實不動。

| unit | panel（不變） | mv9045 | mv9050 |
|---|---|---|---|
| `MInRotateKit`(41)、`MInRotateB`–`H`(65-71) | `palInArm` | `InRotator` | `InPP` |
| `MOutRotateKit`(42)、`MOutRotateB`–`H`(72-78) | `palOutArm` | `OutRotator` | `OutPP` |
| `MPreciser`(85) | `palSys` | `Precisor` | — |
| `MMAutoClean`(508)、`MMAutoCleanKit`(185) | `palSys` | `AutoClean` | `AutoClean` |

後兩列原本走 `else -> palSys`，紅框亮在 System 等於沒講；現在整機圖直接指到本體。

⚠ 有覆寫就代表「有模組可指」，所以**不套 `hideMotionView`** ——
  那條是給「panel 被內嵌區蓋住、整機圖又沒有對應物」用的。
⚠ HT9050 **沒有獨立 rotator**（吸嘴內建，`ht9050-motionview-layout` skill §4），
  所以 `mv9050` 指回 `InPP` / `OutPP`。

### 9.11 HT9050 的告警頁

`scripts/build_alert_motionview9050.py` 從 `Main.MotionView9050.html` 產
`Alert.MotionView9050.html`。注入層依 `parent.MACHINE.id` 選頁，9050 自動帶 `?machine=HT9050`。

**比 9045 那支簡單**：9050 的模組幾何全部來自 `MotionView9050-layout.json`
（`applyLayout()` → `MODS[id]` → `M(id)`），所以**掃一遍 `MODS` 就能替每個模組疊透明錨點**，
不必像 9045 那樣逐個 `plate()` 呼叫點包 `MVA()`。layout 加模組不用改程式。

門環照 `ht9050-hw` 的硬體表，**8 道、方位與 HT9045 的 10 道完全不同**：

```
前  D1 前方左拉   D2 前方右拉
後  D3 後方左拉   D4 後方右拉   D5 後方右掀     <- 虛線：位置未經圖面確認
左  D6 左方掀     右  D7 右方掀
    D8 Index 開檢 —— 不在外框上，錨點掛在 Index 模組
```

⚠ **9050 頁三處都用 `fetch()`**（`Machine-profile.json`、`MotionView9050-layout.json`、
  `HT9045Live` 的 `/api/system/gerneral`），`file:` 下 Chromium 的 fetch 不支援這個 scheme，
  所以**整頁必然 gate**（9045 撐得住是因為有內建預設參數）。build 補了兩件：
  - `almJson()`：fetch 失敗退回 `JSON/js/*.js` 墊片；http（wb_serve）走原路不受影響
  - `fail()` 在有 `?machine=` 時直接 `startPage` —— 原頁「讀不到就不猜機種」的用意沒被違反，
    因為父視窗已經決定好了，`?machine=` 是**指令不是猜測**

### 9.12 `submit()` 第一段不通時必須往下掉（20260924 修）

`dialog-bridge.js` 的 `submit()` 原本：

```js
return Promise.resolve(global.HTDialogHost.submitResponse(...));   // 直接 return
```

而 `ht9045_dialog_host.js:143` 在「沒有待答的 query」時**回 reject**
（`這個畫面的來源不是 wb_serve 的 modal`）—— **檔案信箱送來的告警每一則都踩到**。
那個 reject 直接變成 `submit()` 的結果，後面三段傳輸與 debug 本機關閉**一次都沒機會跑**。

現場症狀：**選了 `K_SKIP`、按了 `K_PAUSE`，跳出「沒有待答的 query」，然後框還在。**

改成接住再往下掉（同步 throw 也接 —— 例外炸穿 `submit()` 會讓 `active.submitting`
卡在 `true`，對話框永久鎖死，那個坑 `ht9045_dialog_host.js` 檔頭記過）。

順帶加了版本標記，`file:` 下 Edge 會快取 .js，改了沒生效時第一個要確認的就是它：

```js
HTDialogBridge.build   // "20260924b-dialoghost-falls-through"
```

### 9.13 `Alert.Note.html` 的寬度是 1184 不是 972（20260924 修）

`note.dfm` 的 `ClientWidth = 1184`，但頁面被產成 **972**，實測 `Panel5` 只剩 **737×549**
（dfm 是 947×531）—— 機構圖右側 210px 連同 `pnlSafeDoorRight`(Left=973)
被 `.pcPane` 的 `overflow:hidden` **整段切掉**，畫面上根本看不到右側門條。

972 的來源是 golden `note.cpp:1440` 的 `Width=980` 扣邊框。但那條路徑**同時會重排**
（`palAuto4-6`／`palFix4-6` 隱藏、`palOutArm` 縮到 255，`note.cpp:1436-1451`）——
HTML 產生器只排絕對座標、不重排，繼承了寬度卻沒繼承重排。

改三處：`dialog-bridge.js` 的 iframe、`Alert.Note.html` 的 `.form`、
`_gen_dfm_abs.py` 的 `FORM_SIZE['note.dfm']`。

### 9.14 ⛔ 更正：`Message` 與 `UnitName` 也是 **Code 查表**，不是 position

20260924 稍早本檔把 `display.unitName` 描述成「position 推出來的」。**那是錯的。**

`note.cpp:856`：

```cpp
iEventID = MyDBIEvent(Code, 0, &AlarmID, &UnitNo, &AxleNo, &fNote->AlarmType,
                      &Message, &UnitName, ...);
```

`Message` 與 `UnitName` 都是 **out 參數**，`MyDBIEvent()` 用 **Code** 查出來的，
呼叫端沒有傳。`cMyDB.cpp:603-706`：

```
UnitNo   = atoi(Code.SubString(4,2))
Message  = fMain->AlarmCodeMap[Code]              <- Error\\AlarmCodeList.txt
UnitName = fMain->UnitNameMap->Strings[UnitNo]    <- AlarmUnit[32]（cMyDB.cpp:32-64）
           Code 第 4-5 碼 == "24" -> 強制 "Motor"（cMyDB.cpp:692-696）
Type     = JAM->1 / WAR->2 / 其他->3
查不到 Code -> AlarmID=41、Message="Unknown Alarm Code"
```

⚠ **`cMyDB.cpp:677-678` 那個 `else` 是註解掉的。**
就算 `CosFunction.bUseMDB` 成立、前面已經從 SQLite 的 `AlarmList` 查到 `Message`，
下面那段還是會**無條件再跑一次**，用 `AlarmCodeMap` 覆寫。
⇒ **`Error\\AlarmCodeList.txt` 才是 `Message` 的實際來源**，MDB 那條被蓋過去了。

#### 完整的來源表

| 畫面欄位 | 來源 | 位置 |
|---|---|---|
| Err Code | Code 本身 | `note.cpp:4406` |
| Subsidiary | `Code.SubString(4,2)` | `note.cpp:4408` |
| Unit Name | `AlarmUnit[UnitNo]` | `cMyDB.cpp:32-64` |
| Message | `AlarmCodeMap[Code]` | `cMyDB.cpp:692` |
| Description | `Error\\<語系>\\<Code>.dat` | `note.cpp:4485-4487` |
| AlarmType | `JAM->1 / WAR->2 / 其他->3` | `cMyDB.cpp:684-689` |
| **紅框** | `ShowErrorUnit(Pos)` | `note.cpp:4551` |

**六個欄位裡五個來自 Code，只有紅框來自 `position`。**

#### 前端自己做，C++ 不用碰（使用者 20260924 裁定）

推導是**純 Code 查表、沒有任何 runtime 狀態**，跟 `flushPanel` 是同一類（§9.2）。
三張表前端都已經有了：

| 要的 | 表 |
|---|---|
| `Message` | `web/JSON/AlarmCodeList-index.json`（3079 筆，＝ `AlarmCodeMap`）|
| `UnitName` | `Alarm-unit-map.json` 的 `codeUnits[].name`（＝ `AlarmUnit[32]`）|
| `Description` | `web/JSON/Alarm-description.json`（948 碼全文）|

`ht9045_alarm_motionview.js` 的 `fillFromCode()` 負責。規則：

* 產生端有給值 -> **尊重它**，只在與查表結果**不一致**時 `console.warn`
* 沒給（wb_serve 目前就是寫空字串）-> 前端推

這樣日後 C++ 真的填了不會打架，填錯了也不會被蓋掉看不見。

⚠ `AlarmCodeList-index.json` 是**快照**（`generatedAt` 2026-09-09），
  而機台上的 `AlarmCodeList.txt` 會被 `MyDBQAlarmCodeList()` 重生。
  兩邊漂掉時以機台為準；要即時可以日後由 `wb_serve` 供應同一個端點。

#### 這條為什麼會錯

因為模擬工具一開始把 `display.unitName` 填成 position 的 unit 常數名
（`MPreciser`）、`message` 讓人自由輸入，造出
「Unit Name=MPreciser、Subsidiary=16、description 寫系統電源錯誤」這種
**golden 不可能出現的畫面**。`scripts/sim_alarm.py` 現在兩個欄位一律寫空字串
（＝ wb_serve 的實際行為），讓前端那條路真的被驗到。
