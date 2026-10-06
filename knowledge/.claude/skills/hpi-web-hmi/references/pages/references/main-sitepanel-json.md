> 保存來源：`.claude/skills/ht9045-html-version/references/main-sitepanel-json.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Main SitePanel and Setup JSON

## Scope

`page/Main.html` is the BCB6 `main.dfm` / `TfMain` main-window simulation. The page reads UTF-8 JSON through `page/settings.js`; it must never parse `.Data`, `.ini`, or `SetUp.inf` in the browser.

## SitePanel

BCB6 authority:

- `TfMain::DrawTestSitePanel()`
- `TfMain::ShowTestHeadComp1()`
- `TfMain::CheckSiteMapIsStander()`

The HTML element `#mtDutOnOff` is a `<table>`, not a `TTMyTray`. Its cell mouse-up
event is a request only; it never changes the displayed Site state before C++ confirms it.

| Part | HTML representation | Source |
|---|---|---|
| Column headings | `a` through `h` | Site position names `Aa` through `Dh` |
| Row headings | `A` through `D` | Site position names `Aa` through `Dh` |
| Cell label | Site number | `production.context.siteMap[]`, or `recipe.documents.handlerCondition.sections.Configuration["Site " + position].value` |
| Arm enable state | `enabledByArm[0..1]` | `recipe.documents.testMode.sections.DutOnOff["Dut  " + position + optional "2"].value` |
| Colour index | final runtime state when available | `production.sitePanel.cells[]`; otherwise derived baseline |

The standard-map rule is:

$$
siteNumber = row + 1 + column \times rowCount
$$

with zero/unmapped cells ignored. For the normal 32-site setup, the first row is `1, 5, 9, 13, 17, 21, 25, 29`.

Color map ordering follows BCB6: `0=Lime`, `1=Aqua`, `2=Silver`, `3=White`, `4=Red`. The JSON-only offline baseline can derive Lime, Aqua, and Silver. White and Red require a C++ runtime projection, so HTML must not invent those states.

### Offline snapshot

`JSON/offline/SitePanel-runtime.offline.json` is generated only from `JSON/Setup-current.json` by `_gen_sitepanel_offline.py`. It is selected when `cppoffline=1`/`offline=1`, or after an `HT_CPP_OFFLINE` message enables offline mode. Run:

```powershell
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_sitepanel_offline.py
& d:\HT9045\.venv\Scripts\python.exe D:\AI_TempFile\_gen_json_shim.py
```

The generated file must contain `layout.columns`, `layout.rows`, `colorMap`, `standardSiteMap`, and `cells[]`. The shim generator creates `JSON/js/SitePanel-runtime.offline.js` for `file://` transport.

### Setup Test Mode Debug preview

`Setup.SetUp.html #ScrollBar1` is the BCB6 `TfSetup::ScrollBar1Change()` Test Mode selector. In `mode=debug`, its isolated `Setup-offline-debug.json` fixture provides multiple layout cases and sends `HT_SETUP_SITE_MODE` through the desktop parent or `BroadcastChannel('ht9045-setup-site-mode')` for directly opened pages. Main redraws the preview immediately. A C++ `HT_SETTINGS` response takes precedence when it publishes `production.context.setupForms["Setup.SetUp"].sitePanel`; it must include the normal `layout`, `colorMap`, `standardSiteMap`, and `cells[]` panel schema.

### Mouse-up site toggle

`TfMain::mtDutOnOffMouseUp()` is represented by `Site-toggle-request.json`. In Debug mode,
a `mouseup` on a mapped table cell writes the request through `HTJsonWriter`; its target
contains `row`, `col`, `position`, `siteNumber`, and the original mouse button/shift state.

HTML must not change `DutOnOff`, `bTestSiteUse`, `LastSet`, or cell colours directly. The
BCB6 handler owns all guards: `SystemStart`, security level 10, critical-parameter lot lock,
FIFO customer rule, `CanChangeSite()`, arm/shuttle/NN mapping, third-engineer control, and
Auto Site Mapping side effects. After a permitted toggle, C++ publishes a `site-toggle` event
in `Production-update.json` with `state.sitePanel`; `settings.js` merges that mutable section
and Main redraws the final colours. A rejected request leaves the current SitePanel unchanged.

## Main Setup Binding Audit

| Main element | JSON path | Status |
|---|---|---|
| `edSetupFileName` | `setup.current` | Bound; tooltip also shows recipe count and integrity. |
| `edWorkTemperBase` | `recipe.quick.temperature` | Bound. |
| `edSoakTime` | `recipe.quick.soakTime` | Bound both after local `HTSettings.load()` and `HT_SETTINGS` broadcast. Do not use a demo default. |
| `runModeValue` | `recipe.quick.testMode` | Bound. |
| Main trays | `recipe.documents.tray.sections` plus `production.machineRecord.normal.trays[]` | Bound; recipe controls format and production controls tray/IC state. |

The following main-window values are intentionally not initialized from Setup JSON because their authoritative sources are absent or runtime-only: Start Mode selection, Normal/Prime state, fail alarm count, pause state, cycle time, UPH, function connection status, clean count, and footer counters. Add them only when `Production-runtime.json` / `Production-update.json` defines a C++-owned field and its update trigger.

<!-- preserved-content:end -->
