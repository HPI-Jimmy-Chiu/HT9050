> 保存來源：`.claude/skills/ht9045-html-version/references/motor-access.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Motor Access 指令通道（motor-access.json / teach-access.json）

HTML 端各動作按鈕（Motor Test Panel20、Teaching pnlMotion、Teaching Set/Go）
不直接控制馬達，而是**送出指令請求**給 C++，並等待**完成回報**才解鎖。

## 檔案

| 檔案 | 方向 | 內容 |
|---|---|---|
| `D:\HT9045\JSON\motor-access.json` | HTML → C++ | `protocol` 規則、`commands` 指令目錄、`request` 指令槽 |
| `D:\HT9045\JSON\motor-access-ack.json` | C++ → HTML | `ack`：完成/失敗回報 |
| `D:\HT9045\JSON\teach-access.json` | 資料 | Teaching `SetButton*`/`GoButton*` 對照表（由 uteach.cpp 抽出） |

模組：`D:\HT9045\page\motor-access.js`（`window.HTMotorAccess`），
由 `HW.MotorTest.html`／`HW.teach.html` 以 `<script src="motor-access.js">` 載入。

## protocol 規則

```json
{
  "requestFile": "JSON/motor-access.json",
  "ackFile": "JSON/motor-access-ack.json",
  "mutex": true,
  "busyPolicy": "kind=motion 執行中鎖定所有按鈕，只有 allowedWhileBusy=true（btnStop）可操作",
  "unlockOn": ["done", "error", "aborted"],
  "ackPollMs": 200,
  "offlineAutoAckMs": 700
}
```

- **互斥**：同時只允許一筆 pending 指令；busy 時再按其他鈕會被拒絕並提示
  `Busy: <button> (<action>) running`。
- **運動鎖定**：`kind=motion` 送出後鎖住頁面所有 `button/input/select`，
  只留 `allowedWhileBusy=true` 的 `btnStop`（與 `alwaysEnabled` 白名單）。
- **解鎖**：收到 ack `done`／`error`／`aborted`；離線模式（`cppoffline=1`）以
  `offlineAutoAckMs` 自動回報 `done` 讓畫面可操作。
- **逾時**：`timeoutMs`（預設 30s）未收到 ack → `error`。

## request 結構

```json
{
  "seq": 12, "id": "cmd-12",
  "source": "uMotorTest",           // 或 uteach
  "button": "sbMotorTest_MoveP",
  "action": "moveRelative",
  "kind": "motion",                  // motion | control | edit
  "motors": ["MInArmX"],
  "params": { "speed": 1, "currentPos": 0, "interval": 1000, "targetPos": 1000,
              "softLimitP": 0, "softLimitN": 0 },
  "issuedAt": "2026-09-02T13:21:00+08:00",
  "state": "requested"
}
```

HTML 無法寫檔 → request 同時存在
`window.__MOTOR_ACCESS_REQUEST__` 與 `localStorage['ht9045-motor-access-request']`，
可用 `HTMotorAccess.exportRequest()` 匯出 JSON。未來改 TCP/IP 時直接送此物件。

## ack 結構

```json
{ "ack": { "seq": 12, "id": "cmd-12", "state": "done",
           "result": "", "message": "", "motorId": "MInArmX",
           "position": { "cmdPos": 1000, "encPos": 998, "targetPos": 1000 },
           "completedAt": "..." } }
```

`state`：`idle` / `accepted` / `running` / `done` / `error` / `aborted`；
HTML 以 `ack.seq === request.seq` 配對。

## 指令目錄（commands）

| source | button | action | kind |
|---|---|---|---|
| uMotorTest | sbMotorTest_JogP / JogN | jogP / jogN | motion（mousedown 送、放開送 stop）|
| uMotorTest | sbMotorTest_MoveP / MoveN | moveRelative | motion（帶 `cbbInterval`）|
| uMotorTest | btnGo | moveAbsolute | motion（`edtHomeOffset`）|
| uMotorTest | btnGoSoftP / btnGoSoftN | moveSoftLimitP/N | motion |
| uMotorTest | btnHome | home | motion |
| uMotorTest | btnLoopMove | loopMove | motion（pos1/pos2/waitTime）|
| uMotorTest | btnStop | stop | control（`allowedWhileBusy`）|
| uMotorTest | btnServoOff / btnMotorPower | servoToggle / motorPowerToggle | control |
| uMotorTest | btnSetPosP / btnSetPosN | setPos1 / setPos2 | edit（不鎖定）|
| uMotorTest **Panel23** | btnHighSpeed / btnLowSpeed / btnHomeHigh / btnHomeLow | setJogHighSpeed / setJogLowSpeed / setHomeHighSpeed / setHomeLowSpeed | edit（`value`=`ReadSpeed()`：runtime `motion.speed`，無則 `edtSpeed`）|
| uMotorTest Panel23 | btnSoftPPos / btnSoftNPos | setSoftLimitP / setSoftLimitN | edit（`value`=`ReadPos()`：`edtCommandPos`）|
| uMotorTest Panel23 | btnRange / btnRate | refreshParameter | edit（BCB6 只做 `UpdateMotorParameter()`）|
| uMotorTest Panel23 | btnSetRange / btnSetRate | setRangeAndInit / setRateAndInit | control（`edtMotorRange` 1~1000、`edtMotorRate` 1~100；SetRange/SetRate→InitMotor→HomeFlag=0，取消 btnLoopMove Down）|
| uMotorTest Panel23 | btnReloadMotorData | reloadMotorData | control（另已綁 `dbReload()`）|
| uMotorTest Panel23 | btResetMNet | resetMNet | control（`confirm()` YES 且 runtime `systemStart!==true`；`requiresSystemStop`）|
| uteach | btnJogP / btnJogN | jogP / jogN | motion |
| uteach | btnMoveP / btnMoveN | moveRelative | motion（`ComboBox1`）|
| uteach | btnMoveTo | moveAbsolute | motion（`edtMoveTo`）|
| uteach | btnHome / btnStop / btnServo | home / stop / servoToggle | motion / control |
| uteach | btnSetTo / btnSetToOffset | setTeachFromCurrent / setTeachFromOffset | edit |
| uteach | `SetButton*` | teachSet | edit |
| uteach | `GoButton*` | teachGo | motion |

> jog 按鈕本身會被 `lockAll` 停用而收不到 `mouseup`，模組改在
> `document` 與 `window.blur` 監聽放開事件。

### Panel23 參數設定鈕（HW.MotorTest.html `setParam()` / `setAndInit()`）

對應 BCB6 `btnHighSpeedClick` 等：`if(ActiveIndex==-1) return; MOT[i].Motor->P* = ReadSpeed()/ReadPos(); UpdateMotorParameter();`

1. 無選取馬達 → 狀態列 `No motor selected` 並 return（同 `ActiveIndex==-1`）。
2. 寫入 `motor.params.<key>`（jogHighSpeed / jogLowSpeed / homeHighSpeed / homeLowSpeed / softLimitP / softLimitN / range / rate）。
3. `renderSettingsGrid()`（＝UpdateMotorParameter）、`renderMotorDb()` 刷新。
4. `HTMotorAccess.send(button,{value})` 通知 C++ 端（edit/control 不鎖定）。
5. **不自動存檔**：需 `sbUpdate` 匯出 `Motor-config.json`（狀態列提示）。

`edtMotorRate` / `edtMotorRange` Click → BCB6 `ShowQwertyKey(N_INTEGER, max, min)`；HTML 改 `type=number` 加 `min/max`，頁面有 `HTQwerty` 時另彈小鍵盤。

## teach-access.json（Set/Go 對照表）

由 `_gen_motor_access.py` 解析 **BCB6** `uteach.cpp` 產生：

- `motorAxle`：`TECH_MotorAxle(motor, button)` → 121 筆（軸控鈕 → motorId）
- `techPoints`：`TECH_PARA`（267）＋ `TECH_TWOPARA`（52）＝ 319 筆

```json
{ "setButton": "SetButton066", "goButton": "GoButton066",
  "motorIds": ["MTestZ1"], "edits": ["setEditIndex1ToSht1Z"],
  "keys": ["setEditIndex1ToSht1Z"], "params": ["Tech.iTestZ1ShutlePick"],
  "type": "TECH_PARA" }
```

- `GoButton*` → `teachGo`（motion）：`targetPos` 取自 `edits` 欄位值；
  `TECH_TWOPARA` 為兩顆馬達，`targetPos` 為陣列。
- `SetButton*` → `teachSet`（edit）：帶 `edits`/`keys`/`currentPos`，不鎖定畫面。
- 點 Set/Go 會先 `teachSelectMotor(motorIds[0])` 切換目前馬達（同 C++ 行為）。

## 產生器

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_motor_access.py
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```

`_gen_motor_access.py` 改動 `COMMANDS` 或 `uteach.cpp` 後重跑；
**接著必須重跑 `_gen_json_shim.py`**（file:// 下靠 `JSON\js\*.js` 墊片載入）。

## 離線 JSON 寫入（2026-09-02）

`cfg.offline` 且 background 已授權 JSON 資料夾（`HTJsonWriter.ready()`）時，`publish()` 會真的寫回
`motor-access.json`（catalog+request），並在 done/error/aborted 時寫 `motor-access-ack.json`（`source.toolchain="HTML-offline"`），
同時更新 `js/*.js` 墊片。未授權時行為不變（只存 `window.__MOTOR_ACCESS_REQUEST__` / localStorage）。詳 [naming-release-writer.md](naming-release-writer.md) §4。

## 已知陷阱

- 按鈕被 `lockAll` 停用後不會觸發事件 → 放開/中止類監聽要掛在 `document`。
- `HTMotorAccess.debug()` 在 iframe 尚未載完時回傳 `cfg=null`，屬正常競態，
  等 `Motor access ready` 訊息出現後再操作。
- 離線模式自動 ack 只是讓畫面可用；接上 C++ 後請確保 `motor-access-ack.json`
  的 `seq` 與 request 相同，否則會等到 `timeoutMs` 才解鎖。

<!-- preserved-content:end -->
