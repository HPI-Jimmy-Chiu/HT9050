# Main Control Commands JSON

## Purpose

`page/Main.html` maps BCB6 `TfMain` main-window actions to `JSON/Main-command-request.json`. HTML sends a request in Debug mode only. It must not directly change machine, temperature, tester, run mode, recipe, FT/RT/EQC, light, or fan state.

Debug command mailbox writes are strict: `Main-command-request.json` and `Site-toggle-request.json` require a user-granted `D:\HT9045\JSON` directory. When it is not authorized, the command is not sent and must never fall back to a browser download. Grant it from Main Debug menu: `JSON 寫入資料夾...`.

C++ owns all validation and performs the original event. It writes `JSON/Main-command-ack.json` with the result, then publishes final UI state through `Production-update.json`. A rejected command must leave the HTML display unchanged.

## Envelope

```json
{
  "schemaVersion": "1.0.0",
  "source": {"toolchain": "HTML simulation", "page": "Main.html", "event": "TfMain::spbSetClick"},
  "state": "requested",
  "requestId": "main-command-20260903-1",
  "requestedAt": "2026-09-03T14:40:00+08:00",
  "expected": {"recipeName": "HT9046LS-HIDRA-8-FT2_85C", "productionSeq": 0},
  "action": "set-temperature",
  "payload": {"temperature": 85, "soakTime": 5}
}
```

## Event Mapping and Examples

| BCB6 event | HTML control | `action` | `payload` example |
|---|---|---|---|
| `spbSetClick` | `#spbSet` | `set-temperature` | `{"temperature":85,"soakTime":5}` |
| `edWorkTemperBaseMouseDown` | `#edWorkTemperBase` | local numeric keyboard | No machine command; `spbSetClick` submits the entered value. |
| `Panel42Click` | `#Panel42` | `toggle-temperature-mode` | `{}` |
| runtime image | `#imgTempOnOff` | none | Status image only; C++ publishes its temperature mode, enabled state, and visual status. |
| `imgTesterClick` | `#imgTester` | `toggle-tester-connection` | `{"requestedConnection":"OFF_LINE"}` |
| runtime panel | `#palTesterMode` | none | Status caption only; C++ publishes `2D SORT`, DIO, GPIB, TCP/IP, or RS232 type. |
| `imgRunModeClick` | `#imgRunMode` | `cycle-run-mode` | `{}` |
| `cbRunStartModeChange` | `#cbRunStartMode` | `set-run-start-mode` | `{"index":1,"caption":"Initial Start"}` |
| `cbSetupFileNameChange` | `#cbSetupFileName` | `load-setup-file` | `{"recipeName":"HT9046LS-HIDRA-8-FT2_85C"}` |
| `palFTClick` | `#palFT` | `set-ft-mode` | `{"manual":true}` |
| `palRTClick` | `#palRT` | `set-rt-mode` | `{"manual":true}` |
| `palEQCClick` | `#palEQC` | `set-eqc-mode` | `{"manual":true}` |
| `spbChamberFanClick` | `#spbChamberFan` | `toggle-chamber-fan` | `{"requestedOn":false}` |
| `spbLightClick` | `#spbLight` | `toggle-light` | `{"requestedOn":true}` |
| `spbFanClick` | `#spbFan` | `toggle-big-fan` | `{"requestedOn":false}` |

`requestedOn` is a request intent only. The button caption/state must be updated from a C++-published runtime snapshot, not from that intent.

在 Debug 模擬中，`imgTester` 額外廣播暫時性的 `HT_BIN_MODE_OVERRIDE.testerOffline` 與當下 `startMode`。切至 `OFF_LINE` 時，`Data.SortCT.html`、`Status.ShowBinSelect.html` 與 `Setup.BinSel.html` 立即預覽 Off-Line Binasgn；再次點擊回 On-Line 時依該 Start Mode 預覽，而不使用舊的 runtime Bin index。`palFT`／`palRT` 在 Debug 依目前的 Continuous/Initial 型態，選擇相對應的 FT 或 Re-Test Start Mode，並同步相同三頁。內嵌於 `background.html` 的頁面用既有 `postMessage`，直接開啟的 Debug 頁面則以同源 `BroadcastChannel('ht9045-bin-mode')` 同步。這些只在 Debug 模擬中視為成功；實際 C++ 串接時，任何含 `context.mainControls.imgTester` 的下一筆 `HT_SETTINGS` 快照都會清除跨頁 preview，再以 C++ 的 `context.activeBinSelectIndex` 為準，即使該命令被拒絕也不會保留 HTML 的預覽狀態。

`palNormal` and `palPrime` are display-only. `TfMain::SetNormalOrPrime()` decides their visibility and selected colour from `IniConfig.bFTBin2RTBin`, `IniConfig.bA02BinModelPrime`, and `iBinModelPrime`; HTML must not infer them from Run Mode.

## Runtime Display Projection

Publish `Production-update.json.state.context.mainControls` after every accepted or rejected Main command and whenever BCB6 refreshes its Main display. Each control is optional so a bridge can add fields incrementally; when present HTML applies `caption`, `visible`, `enabled`, `state`, `status`, and `color` without manufacturing a state from click intent.

```json
{
  "mainControls": {
    "palTesterMode": {"caption":"TCP/IP", "visible":true},
    "imgTester": {"connection":"ON_LINE", "status":"green", "enabled":true},
    "imgTempOnOff": {"status":"red", "enabled":true},
    "imgRunMode": {"status":"green", "enabled":true},
    "runModeValue": {"caption":"Normal"},
    "spbLight": {"caption":"Light OFF", "state":"off", "enabled":true},
    "spbFan": {"caption":"FAN ON", "state":"on", "enabled":true},
    "spbChamberFan": {"caption":"Chamber FAN OFF", "state":"off", "enabled":true},
    "palFT": {"caption":"FT", "visible":true, "enabled":true, "status":"green"},
    "palRT": {"caption":"RT", "visible":true, "enabled":true, "status":"neutral"},
    "palEQC": {"caption":"EQC", "visible":false, "enabled":false, "status":"neutral"},
    "palNormal": {"caption":"Normal", "visible":true, "status":"green"},
    "palPrime": {"caption":"Prime", "visible":true, "status":"neutral"},
    "palFunc": [{"key":"PalFuncATC", "caption":"ATC On Line", "visible":true, "status":"green"}]
  }
}
```

`mainControls.imgTester.connection` 必須是 BCB6 `LastSet.iTester` 的字串化值：`ON_LINE`、`OFF_LINE` 或 `2D_SORT`；HTML 分別使用 BCB6 `ONLINE_ENABLE.bmp`、`OFFLINE_ENABLE.bmp`、`2D_SORT.bmp`。`palFunc` includes one entry for every instantiated `TfMain::palFunc[]` panel. Keys must use BCB6 enum names: `PalFuncArm1Torque`, `PalFuncArm0Torque`, `PalFuncPMAlarm`, `PalFuncEPenconder`, `PalFuncSafeDoor`, `PalFuncOCR`, `PalFuncATC`, `PalFuncScanAOI`, `PalFuncSocketSen`, `PalFuncRTC`, `PalFuncAutomation`, `PalFuncFTP`, `PalFuncAutoClean`, `PalFuncTrayMap`, `PalFuncMonitor`, `PalFuncAutoSkip`, `PalFuncTriTemp`, `PalFuncPowerSave`, `PalFuncSummary`, `PalFuncCleanCnt`, `PalFuncAutoFTP`, `PalFuncRnsFTCT`, and `PalEnableIdxChk`. Do not emit raw enum indexes as the HTML contract.

## C++ Requirements

The BCB6 bridge must match `requestId`, check `expected.recipeName` and reject stale requests, then execute the original handler and all existing guards. It writes:

```json
{
  "schemaVersion": "1.0.0",
  "source": {"toolchain": "BCB6", "owner": "C++ runtime bridge"},
  "state": "completed",
  "requestId": "main-command-20260903-1",
  "action": "set-temperature",
  "accepted": true,
  "completedAt": "2026-09-03T14:40:01+08:00",
  "message": null,
  "productionSeq": 42
}
```

For rejected actions, set `accepted:false` and preserve the BCB6 reason in `message`. In particular, C++ must retain SystemStart, security, critical-parameter lock, machine-IC, clean-out/one-cycle, customer-specific password, temperature limit, and Auto Site Mapping checks.
