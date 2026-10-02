# 原型：HW.IoSetView ＋ Main.MotorView 唯讀原生視窗（20260928 晚）

> 狀態：**DEMO，不是定案**。給 Steven＋Jimmy 20260929 早上看，決定要不要照這個方向做。
> 分支 `v906/steven-native-forms-proto`（獨立分支，從 `380a6a8e` 開）。工作樹 `D:\AI_TempFile\wt-native-proto\`。
> 使用說明（怎麼建、怎麼開、畫面、限制）在 `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\ui\native\README.md`，這裡只記給之後工程師的重點。

## 1. 做了什麼

- 編譯期開關 `W906_NATIVE_FORMS`（預設 OFF）：`ui\native\NativeForms.cmake`，由根 `CMakeLists.txt:67`（原本的空行）`include()`。St02 之後沿用同名。
- 共用主機 `ui\native\NativeHost.*`：訊息泵（可巢狀、每層自己的 keepalive）、拖曳保活計時器、快速鍵、字型。
- 兩個原生視窗（虛擬 ListView＋custom draw）：
  - HW.IoSetView：列 IO 表每一點＋燈號；輸出鈕停用（唯一的真按鈕 `WS_DISABLED`、沒有 `WM_COMMAND` 分支）。
  - Main.MotorView：golden `UpdateMotorScreen`（V912 `main.cpp:8703-8858`）的八欄＋補充欄（Motor Test 十顆燈、伺服、警報…）；**沒有任何按鈕**。
- 掛法照主文件 §4／§7.1：wb_serve **主迴圈同一條執行緒**建窗、泵訊息（`PeekMessage`，每圈最多 200 則）。
- 資料 = 網頁同源、不經 JSON：IO 用 `HSys.IOTable`＋`TPci1203Monitor` 樣本＋同一支 `ResolveIoPoint`；馬達用 `HSys.MotTable`＋`MOT[]` 快取欄位＋
  EastSun 覆蓋掛鉤（經 `JsonBridge\ChanMotorPoints.cpp:209` 同一行加的 `W906_NativeMotorOverlay()` 取得同一支）。
- ctest（只在 ON 時登記）：`NativeIoView_Headless`、`NativeMotorView_Headless`、`NativeHost_Keepalive`；手動 `--modal-move`；展示 `--show <IO_Table.csv> <Mot_Table.csv>`。

## 2. 沒走 A 案（.rc＋GA-4）的原因

一晚內 GA-4 引擎搬 MinGW 未驗證、`iosetview.dfm` 是最大的對話框樹；Steven 的原話是「先表列全部的IO與相關的狀態」「MotorView也是這樣」，
golden MotorView 本來就是 StringGrid。原型要驗的是架構（主迴圈泵＋拖曳保活、直接讀 C++、構造上唯讀），版面之後可換（只換各 View 的畫面半邊）。

## 3. 做的時候踩到、之後會再遇到的

| # | 事實 | 對策 |
|---|---|---|
| 1 | **CMake 一行只能有一個指令**（`endif()  option(...)` 會 `Parse error. Expected a newline`） | 「同一行插入」在 CMakeLists.txt 只能插一個指令 ⇒ 佔一個空行放 `include(...)`，其餘放 include 檔；要等目標存在的部分用 `cmake_language(DEFER CALL ...)` |
| 2 | MinGW.org 6.3 的 `commctrl.h` 把 `LVS_OWNERDATA`／`NM_CUSTOMDRAW`／`LVS_EX_*`／`InitCommonControlsEx`／`NMLVCUSTOMDRAW::iSubItem` 擋在 `_WIN32_IE` 後面；全域只定 `_WIN32_WINNT`／`WINVER`（`CMakeLists.txt:43`） | 原生檔在第一個 include 之前 `#define _WIN32_IE 0x0600` |
| 3 | MinGW.org 沒有 `std::swprintf` | 用 `snprintf` 組 UTF-8 再 `MultiByteToWideChar(CP_UTF8)` |
| 4 | 手動 g++ 不會自動連 `gdi32` | CMake 明寫 `comctl32 gdi32 user32` |
| 5 | ⚠ **拖曳／改大小／捲軸拇指／系統選單時，Windows 在 `DefWindowProc` 裡跑自己的迴圈 ⇒ wb_serve 主迴圈停住** | 每個原生視窗開 50 ms 計時器；自己的泵拿到就丟掉，只有被 Windows 的內部迴圈分派到才跑 keepalive（同 VCL TTimer 在拖曳時仍跑的道理）。site 0（主迴圈）＝`tools\wb_serve.cpp` 檔尾 `W906_NativeKeepaliveMain`：照抄主迴圈的週期工作、**共用 main() 的 nextPump／nextIo**（B13 不變快）；site 1（阻塞等待框）＝只跑 `DoAvoidIndexMotorFallDown`。一般網頁指令排隊到放開；輸出與停止由 `W906_ServiceOutputs` 照常執行（屏障規則不變） |
| 6 | 只在 `WM_ENTERSIZEMOVE` 開計時器不夠：捲軸拇指、系統選單的內部迴圈不送它 | 視窗活著就一直開（泵每秒多丟約 20 則 WM_TIMER） |
| 7 | 主迴圈等待上限 50 ms ⇒ 視窗反應最慢約 50 ms；阻塞框（告警／是否／ShowMyMessage）期間主迴圈停住 | 在 `W906_ModalWaitTick` 同一行也泵一次（`tools\wb_serve.cpp:7622`），否則 5 秒後 Windows 標「沒有回應」 |
| 8 | ZEROARG（`tools\wb_serve.cpp:3649`）：零參數＝全功能，不該加命令列旗標 | ON 建置開機就開窗；關掉後 Ctrl+Alt+I／Ctrl+Alt+M（`RegisterHotKey(NULL, …)`）重開 |
| 9 | 這條執行緒上原本沒有任何視窗（`grep PeekMessage\|GetMessage\|DispatchMessage\|CreateWindow\|RegisterClass` 全樹 0 筆，`vclcompat\component_src` 除外，20260928 18:3x） | 泵用 `hwnd=NULL` 不會搶到別人的訊息；之後若有別的模組在主執行緒建窗，要重看 |
| 10 | `ChanMotorPoints.cpp` 的覆蓋掛鉤指標 `g_overlay` 在匿名 namespace、`FindCardAxis` 也是 | 同一行加一支 getter（`:209`）；`FindCardAxis`／`MotorIndexOf` 照抄到膠水（只用來寫原因） |
| 11 | golden MotorView 的「速度」是 `MOT.GetSpeed()` | 查過是快取：`TMyMotor::GetSpeed` → `HTMotor::ReadSpeed` 回 `iSpeed`（`Motor\HTMotor.cpp:60`），不打卡 |
| 12 | MotorView 目標位置：網頁 P5 規則在出貨組態給 null，golden 一直顯示 `MOT[].TargetPosition` | **Steven 20260929 17:32 Q53 照 golden**：每一列、每個組態都顯示 `MOT[].TargetPosition`（`NativeFormsWbServe.cpp:390`，MR !8 `4fb3f665`） |

## 4. 之後接著做（照 Steven 順序）

1. 1b 輸出鈕：綁 `JsonBridge\IoBtnPanelClick.cpp` 同一本體＋頁面表擁有者（主文件 §7.3 第 9～13 條）。
2. 版面決定（.dfm／表格）後再動畫面半邊。
3. MotorTest：jog 反應要重新量（主迴圈等待上限 50 ms）；保活在 jog 按住時的行為要驗。
4. ~~執行期逐頁選 `system\NativeForms.ini`（主文件 §7.7 (ii)）~~ → **不做**（Steven 20260929 17:32 Q54）；wb_serve 資料端提速也先不做（Q55）。
