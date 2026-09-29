# S118：主畫面 Tray 的 producer（給 Main.MotionView）——帳本

> AI(W906-S118) 20260928（St02-E）。低優先（筆電：「動作流程之後，不急」）。RULINGS `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` 第 311 行（S118）。
> 這份是 St02 自己的帳本：json-bridge／motionview 那兩個 skill 不是 St02 的，請它們的擁有者各加一行指過來（不在別人的檔尾加節）。
> golden 一律寫樹名：906_0625_Steven＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`。

## 1. golden 與 V906 現況

- golden 在初始化時把主畫面每一個 Tray 元件綁到一個「tray 馬達」：906_0625_Steven `cinitial.cpp` 第 6430～6464 行、第 6470～6471 行
  （例 `MOT[MMTrayY].SetHTrayPanel(fMain->mtLoader);`）；元件畫的是那個馬達的 `TMyTray`（`mytray.h`：`XItem`／`YItem`、`Data[col][row]`）。
  - 派工單寫的 `cinitial.cpp:14519-14545` 在這棵樹是 Shuttle 的 latch 設定，綁定實際在 **:6430-6489**（已更正）。
  - 不是主畫面的：`fAutoAlignment` 兩個（:6468-6469）、`fObserveMagazine` 十四個（:6476-6489）——這次不發布。
  - golden 綁定時**不看機種**（那一段沒有任何 `MachineTypeChoice` 判斷）。
- V906：元件不存在，`TTrayMotor::SetHTrayPanel` 是空的、`pHTray` 一直是 NULL（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Motor\mymotor.h` 第 325～345 行）；
  但狀態還在：`MOT[m].Tray`，由 tick 執行緒上的 MainProc 寫。

## 2. 做了什麼（`74080bb3`，分支 `v906/steven-ela-wip`，兩組態只編譯、未執行）

- **`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMvTrays.h`**（新，header-only、不依賴任何函式庫）：
  - `HT9045_MV_TRAY_LIST`：34 個主畫面綁定，X(名稱, 馬達常數, golden 元件)，照 golden 順序。
    名稱：頁面讀的 15 個照舊 contract（`loader`、`hotPlate1`／`hotPlate2`、`auto1`～`auto6`、`fix1`～`fix6`；`web\JSON\Runtime-bridge-contract.json` 的 `motionView.trays.keys`），
    其他用 golden 元件名的 camelCase（`loaderBuffer`、`ocr`、`auto1Car`…、`empty`、`emptyCar`、`color`、`colorCar`、`empty1`、`empty1Car`、`autoClean`、`inRotateKit`、`outRotateKit`、`inArmAoaTray`、`outArmAoaTray`）。
  - `TrayJson`：`{"name","xItem","yItem","ver","cells"}`，`cells[row][col]`＝**原始** `Data[col][row]`（row＜YItem、col＜XItem；大小夾在 30×70）。
    沒有顏色／文字（`TTMyTray` 的 CellColorIndex／CellText 在 V906 不存在；頁面把 0 與 65535 當空格，`Main.MotionView.html` 第 952 行 `liveHas`）。
  - `UpdateTray`：跟上一個 tick 比「大小＋用到的每一格」（精確比較，最多 30×70 個 int）；有變才 `ver+1`、重組 JSON。
- **`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMvTrays.cpp`**（新）：`W906_StageMotionViewTrays(snap)`——
  在 tick 執行緒讀 `MOT[m].Tray`（就是寫它的那條執行緒，不用鎖），發布：
  - `motionView.trays.<名稱>`＝那個 tray 的 JSON 字串（tag 只能放純量，所以一個 tray 一個字串 tag）；
  - `motionView.trays.ver`＝任何一個 tray 有變就 +1；`motionView.trays.keys`＝名稱清單（JSON 陣列）。
  - 每個 tick 都要重新 stage（沒 stage 的 tag 下一代就消失，`WebBridge\TagSnapshot.h` 第 157～158 行），但 stage 的是快取字串；patch 協定只在值變的時候送出。
  - 馬達位址從同一個 X-macro 取（`static_assert` 數量一致、30×70 等於 `mytray.h`）。
- `CMakeLists.txt` 第 3398 行：`ChanMvTrays.cpp` 接在 St02 自己那一行（`MainTesterConnect.cpp`，GB-P2e）的 wb_serve 來源後面，行數不變。
- ctest **`MotionView_Trays`**（`tests\test_mv_trays.cpp`，`tests\CMakeLists.txt` 第 3482～3490 行，St02 的區段、`-Wall -Wextra`、不連任何函式庫）：
  綁定表（34 個、名稱／馬達／元件都不重複、頁面要的 15 個、抽查 `loader`＝`MMTrayY`＝`mtLoader`…）、JSON 對應（格子方向、只取用到的、原始值、夾大小）、只在變的時候發布。只用記憶體。

## 3. 還沒接上的（不是 St02 的檔，請認領；沒改）

1. `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` 第 439 行（檔案層的宣告那一長行，匿名 namespace 外面）：同一行尾加
   `std::size_t W906_StageMotionViewTrays(webbridge::TagSnapshot& snap);  /* AI(W906-S118): JsonBridge/ChanMvTrays.cpp */`。
2. 同檔第 2894 行（`PublishExtraTags` 裡 `n += ht9045::sjson::StageMotor(snap);`）：同一行、在 `//` 註解**前面**加 `n += ::W906_StageMotionViewTrays(snap);`。
   （`PublishExtraTags` 在 wb_serve 的匿名 namespace 裡，所以要第 1 點的檔案層宣告＋`::`，techniques.md 第 5 節。）
3. 頁面 `D:\HT9045\web\page\Main.MotionView.html` 第 2970 行固定呼叫 `applyRuntimeState(null)`（第 2904～2914 行的註解說 wb_serve 沒有 producer）：
   頁面要改成讀 `HT9045Tags` 的 `motionView.trays.keys`／`motionView.trays.<名稱>`（`JSON.parse`），組成 `{state:{motionView:{trays:{…}}}}` 再交給 `applyRuntimeState`。
   頁面的擁有者改；`main.html`（St01）不在這次範圍。
- 第 1、2 點接上之前，這個 producer 在 wb_serve 裡只是被連進去、沒有人呼叫（nm 看得到 `W906_StageMotionViewTrays`）。

## 4. 要 Steven 決定的

- **HT9050 有哪些 tray**：golden 對所有機種都綁同一組（沒有 HT9050 的 golden）；現在 producer 在 HT9050 也發布全部 34 個，由頁面依機種 profile 決定畫哪些
  （`web\JSON\Machine-profile.json` 的 HT9050 `caps.tracks`＝Loader、Auto1～3、Empty）。若要 producer 端就過濾，要一份 HT9050 的 tray 清單。

## 5. 上機要看（第 3 節第 1、2 點接上之後）

- tag 快照裡有 `motionView.trays.loader`，`xItem`／`yItem` 等於配方的 X／Y 數；Loader 盤上取放一顆 IC 時那一格變、`ver` 加 1；沒動作時不再送。
