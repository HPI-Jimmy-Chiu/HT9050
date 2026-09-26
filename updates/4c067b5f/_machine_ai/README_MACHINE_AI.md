# 給機台端 Claude：更新包 10（GitLab main `4c067b5f`，相對更新包 9 `378fbb77`）

> 筆電端 Claude 20260926 17:0x 產生。**先套更新包 3～9，再套這一包。要不要套由 Jimmy 決定。**
> 16 檔，底稿 `base_378fbb77\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：Steven02 測試機通訊的第二批（P2b(b)＋P2e＋P2d）

| 類 | 檔 | 對機台的影響 |
|---|---|---|
| **P2b(b)** atester 四段換成活的翻譯 | `atester.cpp` | `GetTesterResult`／`ProcessTestResult`／`ProcessTesterTimeOut`／`DoIndexSocketCheck` 以前是空殼或閘著，現在照 golden 跑（906 為底補 912；St02 在各函式檔頭登記了 42 個還閘著的地方） |
| **P2d** On-Line／Off-Line 切換本體 | `forms/fMain.cpp`／`fMain.h`、`TesterComm/Handler/*` | `ChangeTesterConnect` 以前回 0 不做事，現在照 golden 912 切換。**Off-Line 時不論配方選哪種測試介面，都走 GPIB 引擎的模擬**（使用者裁決；偏離 golden 的只有 RS232／TTL 配方：golden 是開 RS232Standard 自己的模擬、COM 埠會開，這裡不開 COM 埠）。**SECS/GEM 遠端切換（`SECSGEM/uHGemHT9045.cpp:3431`）現在也會真的切** |
| **P2e** 主畫面 Tester 鈕的 C++ 動作 | `JsonBridge/actions/MainTesterConnect.*`、`JsonBridge/ChanAction.cpp`、`CMakeLists.txt` | 新動作 `act.main.testerConnect`（golden `imgTesterClick`）：運轉中（SystemStart）照 golden 不動作、權限不足不動作。**網頁目前沒有按鈕送它**，所以畫面上看不到變化 |
| 測試 | `tests/test_sjson_chan.cpp`、`tests/test_w7_f1_wall2_probe.cpp`、`tests/CMakeLists.txt` | `test_w7_f1_wall2_probe.cpp:226` 是筆電合併時補的 `#include "LastSet.h"`（St02 那台不能建，原本編不過） |
| 文件 | `docs/NIGHT_REPORT.md`、`docs/TESTERCOMM_PORT_LEDGER.md` | — |

### ⚠ 要知道的兩件事

* St02 照他們的裁決，**先不做** golden「機台裡還有 IC 就不准切換連線模式」（MES1646，`fMain.cpp` 裡 `TODO(W906-GB-P2d): D2`）。
  所以從 SECS 遠端切 On-Line／Off-Line 時，機台裡有 IC 也不會被擋。
* 按鈕那條路（`act.main.testerConnect`，`ChangeTesterConnect(10)`）從 Off-Line 切到 On-Line 時，若 `CosFunction.bLockRTC` 而且 `COM2->bCCDDummyRum`，golden 會把 `config\config.ini` 的 `[RTC] Enable` 寫成 false —— 這裡照翻（不跳訊息、不重開程式，St02 的 D5）。
  這台 `bLockRTC` 若是 0 就不會發生。

## 在機台上要看的（輕量，不要跑生產）

1. **全量重編**（`fMain.h`、`HandlerTesterSide.h` 改了），開機後其他功能照常、主迴圈不卡。
2. `web/page/testercomm.html`：Off-Line 時引擎應該是 GPIB（模擬）；配方若是 RS232／TTL，也不應該再開 Tester／TTL 的 COM 埠。
3. log 沒有新的錯誤。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證（兩組態全量 gate，含上面補的 include）：出貨 192 項＝基準 3＋5 Disabled、模擬 192 項＝基準 18＋5 Disabled；
基準內失敗測試的子檢查 0 差異；system\ config\ 586 檔 0 變動。
