# 正式頁：串接 JSON 的資料驅動版（`page/Main.MotionView9050.html`）

> 狀态：**已実作（2026-09-09）**。Main 頁由 `scripts/build-main-motionview9050.py` 從概念頁產生：
> 畫面／動畫／欄位與 [concept-animation.md](concept-animation.md) 完全同一套，另加三層：
> ① `applyLayout(L)` / `applyForms(settings)` 把 `MODS / ST / FP / HPF / TRAY / 步序秒數` 從 JSON 覆寫；
> ② `startPage()` 機種閘門：`HTSettings.machine().id!=='HT9050'` → `body.gated`，只顯示提示，不繪圖；
> ③ `applyRuntimeState()` / `applyLive(mv)`：**每次收到 Runtime JSON 都先停**（`stopSim()`），
>   `state.motionView.machine==='HT9050'` 才投影快照，否則停在 SIM 起始畫面並標黃底。
> 目的：與 `Main.MotionView.html`（HT9045 版）同契約 ——
> **正式頁載入後不自己動**（概念頁才自己動），**LIVE 畫面不推算、只顯示 Runtime 發布的狀态**。
> 幾何從 `MotionView9050-layout.json` 讀，能力集從 `Machine-profile.json` 讀，
> 逐格在籍／軸位置／LED 從 `Production-update.json.state.motionView` 讀。

## 1. 資料來源與載入順序

| 順序 | 檔案 | 用途 | 未載到時 |
|------|------|------|----------|
| 0 | `settings.js` `HTSettings.load()` | 一次載 General/Config/Setup-current/Production-update/Machine-profile，算出 `machine` | `gate({id:'?'})` |
| 1 | `JSON/Machine-profile.json`（經 `HTSettings.machine()`） | 機種判定：`?machine=` → 啟動選項 → `Gerneral.ini Model` 前綴 → localStorage → default | 落回 HT9045 → **閘門** |
| 2 | `JSON/MotionView9050-layout.json` | `modules[].rect`、`stations`、`strokes`、`defaults`、`flow.steps[].sec`、`axes.bindings` | `srcLead` 紅字「載入失敗」，不畫 |
| 3 | `JSON/Setup-current.json` | 只在 `layout.defaults.useRecipeForms===true` 才讀 Tray.Data / HotPlate.Data 的 `X/Y Division` | 用 `layout.defaults.trayForm / hotPlateForm` |
| 4 | `Production-update.json` | `state.motionView`（需 `machine:"HT9050"`）逐格快照 | SIM 動畫，`liveStatus` 黃底註明原因 |

`Teach-config.json` 目前**不讀**（HT9050 教點鍵名未定）；站點一律來自 `layout.stations`。

`file://` 下一律透過 `JSON/js/*.js` 墊片載入（Edge 封鎖 XHR）。改完任何 JSON 必跑：

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```

## 2. `MotionView9050-layout.json` 契約（v1.1.0，已落地）

```jsonc
{
  "schemaVersion": "1.1.0",
  "source": { "toolchain": "HTML-only", "runtimeSupported": false },
  "footprintMm": { "w": 2650, "h": 1790 },
  "tracks": ["Loader","Auto1","Auto2","Auto3","Empty"],
  "defaults": { "useRecipeForms": false, "trayForm": {"cols":2,"rows":1}, "hotPlateForm": {"cols":2,"rows":2} },
  "modules": [
    { "id":"InShuttle", "no":2, "name":"In Shuttle", "kind":"shuttle", "option":false,
      "rect":[180,330,340,380] }
  ],
  "stations": {                       // 動畫／IC 停位（mm，機構座標，Y 後正前負）
    "loader":{"x":404,"y":-1360}, "hotPlate":{"x":350,"y":-920},
    "inShuttle":{"x":350,"y":-520}, "test":{"x":1310,"y":-520},
    "outShuttle":{"x":1990,"y":-520}, "auto":[{"x":874,"y":-1360},{"x":1344,"y":-1360},{"x":1814,"y":-1360}]
  },
  "strokes": {                        // 行程常數（mm，示意；実際待硬體提供）→ ST.SHT_Y/RAIL_Y_*/IN_RAIL/OUT_RAIL/Z_*
    "shuttleY": -520, "railYIn": -170, "railYOut": -880, "trayRailY": -1080,
    "inRail": {"x0":150,"x1":700}, "outRail": {"x0":800,"x1":2500},
    "outShuttleYPick": 90, "outShuttleYTopAOI": 150,
    "outArmYCylinder": { "back": -610, "front": -1360 },
    "z": { "gantry":70, "armSafe":28, "indexRail":112, "indexSafe":60, "kit":22, "dut":4 }
  },
  "flow": { "steps": [ /* 見 §3，含 sec */ ], "unloadTargets": ["Auto1","Auto2","Auto3"] },
  "axes": { "bindings": [ /* 見 §4 */ ] }
}
```

- `modules[].kind` → 底色見 SKILL.md §2。
- `stations` / `strokes` 是從概念頁 `ST` 搬過來的；**正式頁不得再寫死在 JS 內**。
- 移除的模組（Die Clean / Hot air gun / One Touch Docking / Real time check / Bottom AOI）
  若日後要加回，直接加 `modules[]` 項目並標 `option:true`，不用改 JS。

## 3. `flow.steps`（12 步，含 `sec`）

每步 `{id, sec, trk, from, to, actor, label}`；`sec` 餵進概念頁引擎的 `buildOps()`（`STEP_DEF` 預設秒數的覆寫層，URL hash 再覆寫它）。排程順序見 concept-animation.md §2。

```jsonc
"steps": [
  {"id":"IN_PICK",  "trk":"inPP",   "from":"HotPlate","to":"Loader",    "actor":"InPP",  "label":"In P&P → Loader 取料 ×1"},
  {"id":"IN_HP",    "trk":"inPP",   "from":"Loader",  "to":"HotPlate",  "actor":"InPP",  "label":"放至 Hot plate（補滿）"},
  {"id":"IN_KIT",   "trk":"inPP",   "from":"HotPlate","to":"InShuttle", "actor":"InPP",  "label":"取已達 soak 的 IC → In Shuttle KIT"},
  {"id":"SHT_IN",   "trk":"inSht",  "from":"InShuttle","to":"Test",     "actor":"InSht", "label":"In Shuttle 進測區"},
  {"id":"IDX_PICK", "trk":"index",  "from":"Test",    "to":"Test",      "actor":"Index", "label":"Index 取料"},
  {"id":"SHT_BACK", "trk":"inSht",  "from":"Test",    "to":"InShuttle", "actor":"InSht", "label":"In Shuttle 出測區＋Line sensor"},
  {"id":"TEST",     "trk":"dut",    "from":"Test",    "to":"Test",      "actor":"Index", "label":"Index 下壓 DUT 測試"},
  {"id":"OSHT_IN",  "trk":"outSht", "from":"OutShuttle","to":"Test",    "actor":"OutSht","label":"Out Shuttle 進測區（Z sensor）"},
  {"id":"IDX_PLACE","trk":"index",  "from":"Test",    "to":"Test",      "actor":"Index", "label":"Index 放料"},
  {"id":"OSHT_OUT", "trk":"outSht", "from":"Test",    "to":"OutShuttle","actor":"OutSht","label":"Out Shuttle 出測區→Y 微降取料位"},
  {"id":"OUT_PICK", "trk":"outPP",  "from":"OutShuttle","to":"OutShuttle","actor":"OutPP","label":"Out P&P 取料（Y 懸臂縮回）"},
  {"id":"OUT_PLACE","trk":"outPP",  "from":"OutShuttle","to":"Auto1",   "actor":"OutPP", "label":"Out P&P 放料（Y 懸臂伸出）"}
]
```

`from`/`to` 必須是 `modules[].id`（`Test` 為 DUT 停位，可用 `stations.test` 對應）。
`unloadTargets` 由工具列切換，切換後 `OUT_PLACE.to` 即時改寫。

## 4. `axes.bindings` 與 Runtime 對應

> **軸讀數表的唯一資料來源**：`applyLayout()` 把 `axes.bindings`（在籍軸）與 `axes.absent`
> （HT9045 有、HT9050 沒有的軸）整份覆寫到 JS 的 `AXES`，再由 `renderAxes()` 畫表；
> 在籍列正常顯示、`absent` 列加 `axis-disabled` 淡出。欄位：
> `motorNo`（HT9045 Mot_Table 編號，僅供對照）、`motorId`／`ioId`、`axis`、`type`、`label`、`note`。

```jsonc
"bindings": [
  {"module":"InPP", "motorNo":"M00", "motorId":"MInArmX", "axis":"x", "label":"In P&P X", "note":"In P&P 龍門（伺服）"},
  {"module":"InPP",      "motorId":"MInArmY",      "axis":"y"},
  {"module":"InPP",      "motorId":"MInArmZA",     "axis":"z"},
  {"module":"InPP",      "motorId":"MInRotateKit", "axis":"theta"},
  {"module":"InShuttle", "motorId":"MInShuttle1",  "axis":"x"},
  {"module":"Index",     "motorId":"MTestZ1",      "axis":"z"},
  {"module":"OutShuttle","motorId":"MOutShuttle1", "axis":"x"},
  {"module":"OutShuttle","motorId":"MOutShuttleY", "axis":"y"},
  {"module":"OutPP",     "motorId":"MOutArmX",     "axis":"x"},
  {"module":"OutPP",     "ioId":"C_OutArmY_Ext",   "axis":"y", "type":"cylinder", "states":["back","front"]},
  {"module":"OutPP",     "motorId":"MOutArmZA",    "axis":"z"},
  {"module":"OutPP",     "motorId":"MOutRotateKit","axis":"theta"},
  {"module":"TrayArm",   "motorId":"MTrayX",       "axis":"x"}
],
"absent": [                          // 軸讀數表的淡出列（HT9045 有、HT9050 沒有）
  {"motorNo":"M02 / M31",       "motorId":"MInArmPitch / MInArmPitchY",  "note":"HT9050 無變距軸"},
  {"motorNo":"M13 / M16 / M15", "motorId":"MTestY1 / MTestY2 / MTestZ2", "note":"HT9050 無此三軸"},
  {"motorNo":"M12 / M18",       "motorId":"MInShuttle2 / MOutShuttle2",  "note":"HT9050 無第二組 Shuttle"},
  {"motorNo":"M20",             "motorId":"MOutArmY",                    "note":"改由氣壓缸懸臂取代（C_OutArmY_Ext／Ret）"}
]
```

- `type:"cylinder"` 的綁定讀 IO 狀態（二值），**不做線性內插**。
- 位置換算需 `JSON/Sim-scale.9050.json`，`source.toolchain` **必須**寫 `"HTML-only"`
  （`_gen_simscale.py` 的來源 `cinitial.cpp SetSimuScreenPara()` 沒有 HT9050 分支）。

## 5. `Production-update.json.state.motionView` 契約（HT9050）

```jsonc
"motionView": {
  "machine": "HT9050",                 // 必填；非 HT9050（如 HT9045 runtime 的 trays/ledGroups 形狀）一律忽略→維持 SIM
  "available": true, "paused": false,
  "loader":    { "sensorOn": true,  "cells": [1,1] },
  "auto":      [ { "sensorOn": true, "cells": [0,0] }, {...}, {...} ],
  "empty":     { "sensorOn": false, "cells": [0,0] },
  "hotPlate":  { "cells": [1,1,1,0], "soakDone": [true,false,false,false] },
  "inPP":      { "hold": true,  "vacuum": true,  "at": "loader",    "z": "down" },   // at: loader|hotPlate|inShuttle|test|outShuttle|auto1..3
  "outPP":     { "hold": false, "vacuum": false, "at": "outShuttle","z": "safe", "cylinder": "back" },  // cylinder: back|front
  "inShuttle": { "kit": 1, "inTest": false },
  "outShuttle":{ "kit": 0, "inTest": false, "yAtPick": true },
  "index":     { "hold": false, "z": "safe" },   // z: safe|kit|down
  "dut":       { "hasIC": false }
}
```

`applyLive()` 對應：cells → Tray Status Unit 格點＋下方盤格＋chips；sensorOn → LED on/off/unknown；hotPlate → `D.dHP` 格點（soakDone 紅圈）；
hold/vacuum → 吸嘴 inset；at/z/cylinder → 臂直接擺到站點（**不內插**）；kit/inTest/yAtPick → Shuttle 位置與 KIT 卡；index.z → Index Z。
大 IC 圓（A/B/C）在 LIVE 不畫，在籍一律由格點／KIT 卡表示。`null` / 缺欄位 →「—／不明」。**不可**用 Sensor 或 Motor 座標反推另一個值。
`axes`（pulse）尚未投影：需 `Sim-scale.9050.json`（無 BCB6 來源，須標 `toolchain:"HTML-only"`）。

## 6. 元件重用（與 HT9045 版同一套 helper）

| Helper | 來源 | 注意 |
|--------|------|------|
| `iso()` / `localPlate()` / `plate()` / `box()` / `lab()` | 概念頁 | 斜投影常數 `IS` 兩頁必須一致 |
| `trayPresenceGrid()` / `createTrayStatusUnit()` | `Main.MotionView.html` | 格點是 **`polygon` 平行四邊形**，名稱／LED 放盤面**前緣外側** |
| `drawHead()` | 概念頁 | 1 Picker + Rotator 角度盤 + `headIC` / `headState` 持料指示 |
| `putLabel()` | `Main.MotionView9050.html`（扁平版） | 字級一定 inline `style` |

## 7. 從概念頁到正式頁的對照（build 腳本実際做的事）

| 概念頁（寫死） | 正式頁（讀 JSON） |
|----------------|-------------------|
| `MODS` | `layout.modules[].rect`（`applyLayout`；缺的 id 用 JS 預設） |
| `ST.*` 站點 | `layout.stations.*` |
| `ST.OSHT_Y_PICK` / `OUT_CYL` / `Z_*` / rails | `layout.strokes.*` |
| `TRAY` / `HPF` | `layout.defaults`；`useRecipeForms:true` 才讀 `Setup-current.json` |
| `STEP_DEF[].d` | `layout.flow.steps[].sec`，再由 URL hash 覆寫：`buildOps(Object.assign({},DUR_JSON,HP))` |
| `STEP_DEF[].lbl`（動作記錄文字） | `layout.flow.steps[].label` |
| 軸讀數表（`AXES` 寫死 13＋4 列） | `layout.axes.bindings` ＋ `layout.axes.absent`，`renderAxes()` 重畫 |
| Option 模組卡的固定敘述 | `Machine-profile.json layoutModules`，`renderModules()` 產生「標配／Option」清單；Option 沒列進 `layout.modules[]` 就淡出標「未配置」 |
| `Using Flag —` 固定字 | `#hdUsingFlag`：`useRecipeForms:true` 才讀 `Setup-current.json` HotPlate.Data 的 `Using Flag`，否則顯示 `—（useRecipeForms:false）` |
| 寫死色票 `--bg/--ink/--line…` | theme.css design token `var(--form-bg)/var(--text)/var(--border)…`；另補 `:root[data-theme="dark"]` 壓暗 `--plate*／--cell*／--mech` 等 theme.css 沒有的機構色 |
| 頂層立即執行 `drawIso()…requestAnimationFrame(loop)` | 包成 `boot()`，由 `startPage()` 在 JSON 就緒後呼叫一次 |
| `render()` 直接擺位 | 拆成 `placeAll(...)`（SIM／LIVE 共用）；`render()` 在 `LIVE` 時直接 return |
| `applyPanels()` 推算 | SIM 仍用；LIVE 改 `applyLive(mv)` 直接投影 |
| `--panel` | `--cpanel`（避免與 theme.css 衝突） |
| `<p class="lead concept">` 概念説明 | `#srcLead` 資料來源狀态列；`#paramNote` 盤形／秒數來源；`#srcTable` 契約表 |
| 載入即 `PLAY=true` 自己跑動畫 | `PLAY=false`：畫面靜止，動不動由 `Production-update.json` 決定；SIM 播放列包成 `<span class="simctl debugOnly">`，release 隱藏；`LIVE` 時播放／下一步／重置直接 `return` |
| （無） | `#gate` 機種閘門、`#mvMachineTag`、`#modeTag`（`等待 JSON` → `SIM（停止）` / `LIVE`）、`stopSim()` |
| （無） | Runtime 推播兩條路都聽：`HT_SETTINGS_REFRESHED`（獨立開頁）＋ `message` 的 `HT_SETTINGS`（嵌在 background.html 的 iframe），開機後補一次 `HTSettings.request()` |

## 8. 驗證

1. `python D:\AI_TempFile\_build_main_motionview9050.py` → `node --check` 內嵌 script（取 `<script>…</script>` 第一段）
2. `scripts/regen-and-check.ps1`（重跑 shim）
3. `file:///D:/HT9045/page/Main.MotionView9050.html?mode=debug&machine=HT9050` → `srcLead` 綠邊、
   `modeTag`＝`SIM（停止）`、**畫面静止不動**、`liveStatus` 黃底；按「▶ 播放」才跑 SIM，
   console 跑 `verify-concept.js` PASS
4. `?machine=HT9045` → `body.gated`；不帶參數 → 依 `Gerneral.ini Model` 前綴
5. `background.html?mode=debug&machine=HT9050` console：`MACHINE.id === 'HT9050'`、iframe src 指向 Main.MotionView9050.html
6. LIVE 假資料：console `applyRuntimeState({productionUpdate:{state:{motionView:{machine:'HT9050',hotPlate:{cells:[1,1,1,0]},...}}}})`
   → `modeTag`=LIVE、`PLAY===false`、按「▶ 播放」無反應（LIVE 下不許 SIM 插手），無 pageerror
6b. `?mode=release&machine=HT9050` → SIM 播放列（`.simctl`）不顯示
6c. 嵌在 `background.html` 下改 `Production-update.json` 的 `event.seq` → iframe 內畫面跟著更新
    （走 `message` 的 `HT_SETTINGS`，不是 `HT_SETTINGS_REFRESHED`）
7. 回歸：`Gerneral.ini` 改回 `HT-9046AT` 後不帶 `?machine=` 應回到 `Main.MotionView.html`
