> 保存來源：`.claude/skills/ht9045-html-json/references/class-json-projection.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# BCB6 Class / Structure JSON 資料投影

> ⛔ **狀態（Steven 團隊 20260926）**：下面決策表的「整合 JSON」欄（`Production-runtime.json`、`IO-config.json`、
> `Motor-runtime.json`…）是 **A 路（靜態 JSON，已棄用）** 的落點。現行的 C++ 結構 → JSON 不寫這些檔：
> 設定類走 **C 路** `editlist.get`／`editlist.save`（`route-c-golden-bridge.md`）、IO／馬達走
> `GET /api/struct/io/*`、`/api/struct/motor/*` 與 `io.*`／`motor.*` tag、生產／alarm 走 tag 通道（skill
> `ht9045-json-bridge`）。「可輸出／禁止輸出」的欄位原則仍可參考。「LastSet 相容性」一節**仍有效**：
> 20260926 補 V912 欄位時照它只加在結構最後（S45，`LastSet.h` 檔尾 `iBinBaseRT[256]`／`bO25_RTBaselined`，
> commit `0609a14f`，見 json-bridge `write-inventory.md` 〇 TowerLight 列）。

本文件定義 HTML 模擬如何使用 BCB6 class 與結構資料。原則是**依資料用途投影至既有 JSON**，
不做 class 對 class 的完整序列化，也不讓 HTML 直接讀取 `.h`、`.dat`、`.csv` 或 `.ini`。

## 決策表

| BCB6 類別／結構 | 資料分類 | 整合 JSON | 可輸出欄位 | 禁止輸出 |
|---|---|---|---|---|
| `LastSet.h` 結構 | binary 持久化 | `Production-runtime.json` | 經 C++ 驗證的 observer、機台記錄與開站資料 | 整包 binary、未確認版本的欄位、為 JSON 而改動結構順序或型別 |
| `TMyCylinder` | IO 設定＋狀態 | `IO-config.json`、`IO-runtime.json` | 位址、Enable、Alarm/Delay；Push/Pop sensor 的衍生狀態 | Task、Timer、VCL 指標、整個 class 記憶體 |
| `TMySensor` | IO 設定＋狀態 | `IO-config.json`、`IO-runtime.json` | Name/Using/Ring/IP/Port/Bit/Type/Enable、觸發狀態 | 方法與 class layout |
| `TMySwitch` | IO 設定＋狀態 | `IO-config.json`、`IO-runtime.json` | Name/位址/Enable、輸出狀態 | 方法與 class layout |
| `TMySucker` | 真空 IO／診斷 | `IO-config.json`、`IO-runtime.json` | On/Off/Sensor IO、必要的真空完成與警報狀態 | private Task/Timer、buffer、原始 class |
| `TMyKitSuck` | 生產流程 | `Production-runtime.json`、`Production-update.json` | 畫面需要的 Item/Bin/2DID/AOI/狀態矩陣投影 | `Suck[4][8]` 全物件、指標、Timer、未使用陣列 |
| `TMyTray` | Tray／生產資料 | `Production-runtime.json.machineRecord`、`Production-update.json` | Tray identity、routing 與稀疏 `[row,col,value]` 資料 | 30x70 全矩陣與 `TMyProductionRecord*` 指標直接序列化 |
| `TMyMotor`／`TTrayMotor` | 馬達設定＋狀態 | `Motor-config.json`、`Motor-runtime.json` | 軸別、driver、limits、params、position、servo/alarm/home、診斷 | `HTMotor*`、Lock map、私有 Task/Timer、直接輸出 `MOT[300]` 記憶體 |

## 既有雙檔合併

IO 與馬達頁面應先載入 config，再以穩定 ID 合併 runtime：

```text
IO-config.points[ioId] + IO-runtime.points[ioId]
Motor-config.motors[motorId] + Motor-runtime.motors[motorId]
```

runtime 找不到時必須保留 `unknown`／`stale`，不可用預設 `false`、`0` 或 DFM 範例值偽造即時狀態。

## Production 投影規則

- `TMyKitSuck` 與 `TMyTray` 是生產流程物件，依頁面需求寫入 `Production-update.json` 的完整 mutable snapshot。
- 大型座標矩陣使用稀疏列資料；未列出者代表預設值，不以完整二維陣列傳輸。
- 每個 runtime section 必須含 `available`、`updatedAt`、`seq`、`updateClass` 與 `trigger`。
- C++ 寫入須先寫 `.tmp`、flush/close 後 atomic replace，並同步更新 `JSON/js/*.js` shim。

## LastSet 相容性

`LastSet.h` 為 binary layout 合約：既有欄位不可改名稱、型別、大小與順序；若必要新增，只能加在結構最後並通知團隊。JSON 是 C++ 端的讀取後投影，絕不能回寫或取代 LastSet binary。

## 靜態索引

需要查詢欄位、型別與 JSON 分類時，使用 `JSON/Define-index.json` 與
`page/IDE.define-to-json.html`。此索引是 schema 文件，不代表每個 `extern` 或 class 成員都存在即時值。
<!-- preserved-content:end -->
