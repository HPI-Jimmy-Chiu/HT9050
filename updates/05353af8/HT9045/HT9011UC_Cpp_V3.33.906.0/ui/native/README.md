# 原生表單原型：HW.IoSetView ＋ Main.MotorView 唯讀（20260928 晚，DEMO，不是定案）

> 讀者：Steven、Jimmy（20260929 早上一起看，決定最終要不要這樣做）。
> 分支：`v906/steven-native-forms-proto`（從 `380a6a8e` 開出來的獨立分支，沒有合進任何人的分支）。
> 樹：`D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\`（移植樹，C++）。
> 撰寫：ST01-E 派的工程師（AI）。**沒有在這台 PC 上跑過 wb_serve**（規定不能對真機檔跑）；跑過的只有無顯示 ctest 與展示程式（都不讀寫機台檔）。
> 建置與測試結果在 §3.1。
>
> **20260929 更新（ST01-E3，不閃版）**：Steven 看完第一版說「一直在閃爍是你故意的嗎?」「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」「這樣的閃爍是不被允許的」。
> 表格從 ListView 換成自己畫的 `NativeGrid`（只重畫變了的格子、沒有擦背景），摘要改成雙緩衝標籤，更新頻率 200／500 ms → 20 ms。
> 看法與結果在 **§3.2**；實機要真的 20 ms 還差 wb_serve 那一側三件事（§8 第 9 條、§10）。

## 1. Steven 的原話與這一版做到哪裡

| Steven 20260928 | 這一版 |
|---|---|
| 「原生表單的評估和方案 我希望的順序是： IO, MotorTest, MotorView, teach, home, shuttle move」 | 做了 **IO（HW.IoSetView）** 與 **MotorView（Main.MotorView）** 兩頁，都只讀。MotorTest（第二頁，有按鈕、會動機台）沒做 |
| 「如果是使用define的方式隔開看是使用 cpp form或是 html form有，可以嗎？」 | **可以，已做編譯期開關** `W906_NATIVE_FORMS`（CMake option，預設 OFF）。執行期逐頁選（`system\NativeForms.ini`）今晚沒做 |
| 「IoSetView 先做只看燈號、不能按輸出的版本」 | 做了：輸出的燈號照實顯示，**輸出鈕全部停用**，而且視窗的程式根本沒有連到任何 IO 寫入（§5） |
| 「先表列全部的IO與相關的狀態」「MotorView也是這樣」 | 做了：IO 一張表列出 IO 表每一點；MotorView 照 golden 主畫面馬達表的八欄，再加補充欄（§7） |
| 「有空先做一個簡單版的，明天早上我跟Jimmy一起看成果後，決定是不是最終要這樣做」 | 就是這一版：**簡單版**，版面不是照 .dfm（§6 講為什麼） |
| 「可以獨立分支」 | 獨立分支 `v906/steven-native-forms-proto` |

## 2. 兩種看法（先看 A，不需要機台）

### A. 不碰任何機台檔的展示（建議先看這個）

`test_native_forms.exe` 有一個展示模式：讀 IO 表與馬達表（**只讀**），開兩個看得到的視窗，燈號與位置用假資料持續變化。
不讀 `Gerneral.ini`、不讀 `config.ini`、不寫任何檔、不碰卡。

```
D:\AI_TempFile\st01e3-native-build\test_native_forms.exe --show D:\HT9045\machines\HT9050\IO_Table.csv D:\HT9045\machines\HT9050\Mot_Table.csv
```

（20260929 不閃版在 `D:\AI_TempFile\st01e3-native-build\`；`D:\AI_TempFile\st01e-native-build\` 裡的是 20260928 會閃的第一版。）

- 任一個路徑寫 `-` 就用合成資料（例 `--show - -`）。
- **按住標題列拖曳視窗：燈號照樣在變** ＝ 拖曳保活有效（§8 第 1 條）。摘要列的「拖曳保活 N 次」會增加。
- 關掉的視窗可用 **Ctrl+Alt+I**（IO）／**Ctrl+Alt+M**（MotorView）重開；兩個都關掉程式就結束。
- 摘要列最前面寫 `[DEMO 假資料]`，燈號與位置不是機台的值；MotorView 的「golden 會列」在展示模式下是整張表（展示程式不會算 golden 的顯示規則）。
- 先看圖也可以：20260929 04:3x 用 `--snapshot` 拍的兩張（HT9050 的 IO 表／馬達表＋假燈號）——
  `D:\AI_TempFile\st01e-native-snapshots\HW.IoSetView_demo.png`、`D:\AI_TempFile\st01e-native-snapshots\Main.MotorView_demo.png`。
  自己重拍：`test_native_forms.exe --snapshot D:\AI_TempFile\snap <IO_Table.csv> <Mot_Table.csv>`（PrintWindow，螢幕鎖著也拍得到，存成 `snap_io.bmp`／`snap_motor.bmp`）。

### B. 在機台上跑真的（wb_serve 內建）

用 **ON 的建置**產生的 `wb_serve.exe`，照平常一樣**不帶參數**啟動（ZEROARG：零參數＝全功能，行為由建置決定，所以沒有新增命令列旗標）：

```
D:\AI_TempFile\st01e-native-build\wb_serve.exe
```

- 開機完成、主迴圈開始前，會自己跳出兩個視窗：「HT9045 V906 — HW.IoSetView（原生唯讀原型）」與「HT9045 V906 — Main.MotorView（原生唯讀原型）」。
  主控台印一行 `[native] W906_NATIVE_FORMS prototype (read-only): HW.IoSetView window OPEN, Main.MotorView window OPEN; reopen hotkeys Ctrl+Alt+I registered, Ctrl+Alt+M registered`。
- 關掉視窗 **不會** 關掉 wb_serve；要再開按 **Ctrl+Alt+I**／**Ctrl+Alt+M**（全域快速鍵；被別的程式佔用時主控台寫 `NOT registered`，那就只能重啟 wb_serve 再看）。
- 拖曳或改大小視窗時主控台會印 `[native] window drag/resize started ...` 與 `... ended: N keepalive ticks ran during it`。
- 網頁 HMI（瀏覽器）照舊，兩邊同時開沒有衝突：原生視窗只讀，網頁的 HW.IoSetView／Motor Test 照原本的樣子。
- ⚠ 這顆 exe 與平常的 wb_serve 一樣會讀寫機台檔（`D:\HT9045\system\Gerneral.ini` 等）——只在實驗機上跑，照平常跑 wb_serve 的規矩。
- 沒有 1203 卡的 PC（例如 SIM 組態的開發機）：IO 每一點是灰色 **null**，摘要列寫「1203 監看器：未連線（每一點都是 null，不是 off）」；
  MotorView 在 SIM 組態會顯示 MOT[] 的模擬值（與網頁 Motor Test 同一條規則）。這與網頁在同一台 PC 上看到的一樣（同一個資料來源，§4）。

## 3. 怎麼建

ON（有原生視窗）——這次用的就是這個：

```
set PATH=C:\MinGW\bin;%PATH%
cmake -S D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0 -B D:\AI_TempFile\st01e-native-build -G "MinGW Makefiles" ^
      -DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe -DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe -DW906_NATIVE_FORMS=ON
cmake --build D:\AI_TempFile\st01e-native-build -j 18
ctest --test-dir D:\AI_TempFile\st01e-native-build -R "Native" --output-on-failure
```

或用 `build.bat`（第一次 configure 才吃 `V906_CMAKE_ARGS`，所以要用新的建置目錄）：

```
set V906_BUILD_DIR=D:\AI_TempFile\st01e-native-build
set V906_CMAKE_ARGS=-DW906_NATIVE_FORMS=ON
build.bat gate
```

- 出貨組態再加 `-DW906_NO_SOFT_SIMULTE=ON`（兩個開關互相獨立）。
- **OFF（預設）**：什麼都不加就是 OFF。`ui/native/` 的碼一行都不進 exe；`wb_serve.cpp` 的五個呼叫點各只多呼叫一次空函式（檔尾 `#else`），行為與沒有這個開關時相同。
  ctest 清單也不變（三支 `Native*` 只在 ON 時才登記）。
- ON 時多三支 ctest：`NativeIoView_Headless`、`NativeMotorView_Headless`、`NativeHost_Keepalive`（都用 `test_native_forms.exe`，無顯示）。20260929 起是四支：多一支 `NativeGrid_Efficiency`（§3.2）。

## 3.1 這次的建置與測試結果（20260929 04:06～06:16，MinGW 6.3，SIM 組態）

| 項目 | ON（`-DW906_NATIVE_FORMS=ON`，`D:\AI_TempFile\st01e-native-build`） | OFF（預設，`D:\AI_TempFile\st01e-native-build-off`） |
|---|---|---|
| 建置 | 綠（05:12 最後一次增量；`ui/native/*`、`test_native_forms.cpp` 0 個警告） | 綠（全新目錄 05:13～05:43） |
| 原生 ctest | `NativeIoView_Headless`、`NativeMotorView_Headless`、`NativeHost_Keepalive` 3/3 通過 | 不登記（OFF 沒有這三支、也沒有 `test_native_forms.exe`） |
| 全部 ctest | 228 支、21 支失敗 | 225 支、21 支失敗 |
| 與基準比 | 21 支失敗的名字與 `380a6a8e` SIM gate 基準（`D:\AI_TempFile\st01-380a6a8e-gate.log`）**完全相同** ⇒ 沒有回歸 | 同左 |
| OFF 沒有原生碼的證據 | — | 所有 `flags.make` 都沒有 `W906_NATIVE_FORMS`；`wb_serve.exe` 裡 `w906native::` 符號 0 個；五個 `W906_NativeForms*` 都是空函式（`objdump`：push／mov／nop／pop／ret） |
| 真的 Windows 移動迴圈 | `test_native_forms --modal-move` 3/3：`WM_SYSCOMMAND SC_MOVE` 的迴圈裡保活被叫 5 次 | — |
| 機台檔 | 8 個檔（Gerneral.ini、config.ini、ContactInfo.ini、levelset.dat、lastdata.dat、Error\*\JAM0000.dat ×3）SHA256 每一段前後都相同 | 同左 |

- 基準的 21 支：config_db、ini_helpers、config_loaders、SimIO、W6_Canary、W6_4_TesterAnchor、HanaART、BarCodeHelpers、BarCode8CCDGlue、AGV_E84、Automation、
  dfm2rc_fidelity、dfm2rc_idempotent、W7_L1_Auto2、W7_L1_Color、W7_L1_Loader、W7_L1_AutoRT、GA2_C1_cinitial、WB_SimPump、mainproc_guard、TeachButtonsGen。
- 支數：基準 log 寫 223；新目錄是 225（OFF）＝多了 `FShow_Audit`／`FShow_Audit_SelfTest`（`380a6a8e` 裡就有，兩支都通過），ON 再多三支原生。
- **沒有跑 wb_serve**（規定不對真機檔跑），所以「wb_serve 裡開出來的視窗」這一段要 Steven／Jimmy 在實驗機上看（§2 B、§8.5）。

## 3.2 20260929 不閃版（ST01-E3）

**為什麼會閃（第一版）**：exe 沒有 manifest，用的是 comctl32 v5 的 ListView，它沒有雙緩衝（`LVS_EX_DOUBLEBUFFER` 是 v6 才有）。
每次 `LVM_REDRAWITEMS` 都先把整列擦白再畫；摘要 STATIC 每 200 ms `SetWindowText` 也是先擦再畫。展示的假資料幾乎每一列都在變，所以閃得最明顯。

**現在的畫法**（`ui\native\NativeGrid.h`／`.cpp`，純 Win32，不含機台碼）：
1. 表格控件自己有一張常駐的背景點陣圖。螢幕上看到的都是從它 BitBlt 過去的，`WM_ERASEBKGND` 回 1，所以不會有「先白再畫」的那一瞬間。
2. 每一格上次畫了什麼（種類、顏色、文字）存在快取。視窗只把「值變了」的列交給表格；表格逐格跟快取比，**一樣的格子不畫**，不一樣的才畫進背景圖。
3. 畫完只把變了的矩形送上螢幕（`InvalidateRect(FALSE)`＋`UpdateWindow`，同步）。沒東西變＝0 格、0 次 BitBlt。
4. 一格用一次 `ExtTextOutW(ETO_OPAQUE)` 同時填底色和寫字。這台量到 `DrawTextW` 一次 60～80 µs、`ExtTextOutW` 約 15 µs；只有估計寫不下時才改走 `DrawTextW` 加「…」。格線只在整張重畫時畫一次，之後畫格子不碰那一條像素。
5. 摘要是三行的雙緩衝標籤，換字時只重畫變了的那一行。第三行「畫面效能」直接顯示上次／最大更新毫秒、實際更新間隔、累計重畫格數。
6. 每一拍的成本也降下來：點表／馬達表的靜態欄位（Alias、位址…）只在表換了時建一次。glue 每一拍只重算值，沒有 AnsiString→std::string，也不配置記憶體；視窗的寬字串、搜尋用的小寫字串也預先算好。
7. 沒有用 manifest／comctl32 v6：加 manifest 會改到整顆 wb_serve（其他視窗、MessageBox 的外觀），OFF 建置也得跟著動。OFF 建置仍然一行 ui/native 的碼都不進 exe。

**建置與測試**（20260929 11:1x～11:3x，MinGW 6.3，SIM 組態，新目錄 `D:\AI_TempFile\st01e3-native-build`，`-DW906_NATIVE_FORMS=ON`）
- 當時 ST01-M 的 two-config gate 正在同一台做 SHIP 全量建置，CPU 100%，所以下面的毫秒數都是「被搶 CPU」的上限，不是機器閒的時候的數字。
- `test_native_forms` 綠，`ui/native/*` 與 `tests/test_native_forms.cpp` 在 `-Wall -Wextra` 下 0 個警告。`NativeFormsWbServe.cpp` 單獨編 object 綠。
  gate 做完後（12:38～12:47）**wb_serve 整顆 ON 建置、連結綠**（`D:\AI_TempFile\st01e3-native-build\wb_serve.exe`，exe 裡有 `W906NativeGrid`／`W906NativeLabel` 視窗類別）；
  沒有在這台跑 wb_serve（規定），實機上看要照 §2 B。
- ctest `^Native` 4/4 通過：`NativeIoView_Headless` 38/38、`NativeMotorView_Headless` 42/42、`NativeHost_Keepalive` 12/12、**新的 `NativeGrid_Efficiency` 21/21**。
- `NativeGrid_Efficiency` 驗的事：
  - 值沒變的更新重畫 0 格、不整張重畫；一個點切換只重畫 2 格（燈號、Raw）；看不到的列變了重畫 0 格。
  - 背景圖上燈號的像素顏色就是資料的顏色（ON／OFF、SVON 亮／暗）。
  - 60 軸位置全部在變時，每個看得到的列剛好重畫 1 格。
  - 效率（視窗不顯示，所以不含 BitBlt 上螢幕那一段）：
    - IO 700 點、每拍約 14 點變化、1000 次更新：平均 0.75 ms。
    - MotorView 60 軸每拍全動、1000 次：平均 2.6 ms（其中表格重畫 2.2 ms）。第一個寫法（還是用 DrawTextW＋每格畫格線）是 14 ms，改掉之後才降到這裡。
- 展示（`--show`，HT9050 的真表 967 點／48 軸，20 ms 一拍）：兩個視窗截圖看過，不閃；Steven 看了回「讚」。
  - 截圖當下 IO 摘要寫「上次更新 40 ms、實際間隔約 57 ms」，MotorView 寫「5 ms／35 ms」。當時 gate 佔滿 CPU，展示程式被搶。
  - **gate 做完、CPU 約 14% 時再量（12:48）**：IO（967 點）上次更新 **1.43 ms、實際間隔 20 ms**；MotorView（48 軸每拍全動）**2.82 ms、實際間隔 21 ms**。
    摘要裡的「最大」（286／141 ms）是 gate 還在跑時留下的，從開窗起累計。
- 機台檔：展示前後 `D:\HT9045\system\Gerneral.ini`、`D:\HT9045\config\config.ini` SHA256 相同；`D:\HT9045_Log` 沒有新檔。

## 4. 資料從哪裡來（與網頁同一份，但不經 JSON）

**IO（HW.IoSetView）**

| 項目 | 來源 | 檔案 |
|---|---|---|
| 點表（名稱、型別、位址） | `HSys.IOTable`（C++ 開機載入的 IO 表） | `...\database.h:357`；網頁 `/api/struct/io/config` 同源 `JsonBridge\ChanIoPoints.cpp:145-252` |
| 每一點的值 | 1203 監看器（`TPci1203Monitor`）的 DI byte 與 DO 回授 byte | `EtherCAT\Pci1203Monitor.h:1107`（DI）、`:1148`（DO）、`:1543`（取得監看器） |
| on／off／null 的換算 | **同一支** `ResolveIoPoint`（ring=Lane、station=IP、byte=Port/8、bit=Port%8；InType=0 反相） | `JsonBridge\ChanIoPoints.cpp:107-143`；證據在 `JsonBridge\ChanIo.h` 檔尾 |
| 取樣的寫法 | 照抄網頁 `/api/struct/io/runtime` 的 `IoRuntimeJson` | `JsonBridge\ChanIoPoints.cpp:359-406` → `ui\native\NativeFormsWbServe.cpp` `IoSnapshot()` |

- 20260929 起每 20 ms 比對一次（原本 200 ms）。但**值**還是跟著監看器 Poll 的節拍 `kIoTickMs`＝200 ms 變（§8 第 9 條）。輸出的燈號是**卡片的 DO 回授**，不是按鈕狀態。沒有 Alias 的列略過（網頁 `BuildIoIds` 也略過）。

**MotorView（Main.MotorView）**

| 項目 | 來源 | 檔案 |
|---|---|---|
| 列、順序 | golden `UpdateMotorScreen` 只列 Motor Test 可見的馬達、照 MotorTestClass 順序 | golden V912 `main.cpp:8703-8858`；移植樹 `forms\fMotorTest.cpp:1041` `W906_MotorTestVisibility()` |
| 1203 軸的位置／伺服／警報／燈 | EastSun 的監看器覆蓋掛鉤（網頁 `/api/struct/motor/runtime` **同一支**） | `WebMotorAccessLive.cpp:1110`；經 `JsonBridge\ChanMotorPoints.cpp:209` 同一行加的 `W906_NativeMotorOverlay()` 取得 |
| 目標位置、速度、Can／L／M／R、HomeFlag | `MOT[]` 的快取欄位（`TargetPosition`、`GetSpeed()`＝`HTMotor::ReadSpeed` 回快取 `iSpeed` 不打卡、`fCanMove*`、`HomeFlag`） | `Motor\mymotor.h:133-172`、`Motor\HTMotor.cpp:60` |
| SIM／SHIP 誰能當「值」 | 照 `MotorRuntimeJson` 的 P5 規則：出貨組態不拿 `MOT[]` 位置當卡片值 | `JsonBridge\ChanMotorPoints.cpp:226-345` → `NativeFormsWbServe.cpp` `MotorSnapshot()` |

- 20260929 起每 20 ms 比對一次（原本 500 ms）。1203 軸的值仍然跟著覆蓋掛鉤的抄本，在主迴圈 500 ms 拍子才更新（§8 第 9 條）。
- 目標位置：Steven 20260929 17:32 裁決 Q53「照建議值」＝照 golden（V912 `main.cpp:8809-8813`）——每一列、每個組態都顯示 `MOT[].TargetPosition`（引擎的目標，是 `MOT[]` 自己的成員，不讀卡、不需要 Motor 物件；`NativeFormsWbServe.cpp:390`）。原本出貨組態照網頁 P5 規則顯示 `—`。

## 5. 唯讀怎麼保證（不是只把按鈕灰掉）

1. **視窗本體（`NativeHost.cpp`、`NativeIoView.cpp`、`NativeMotorView.cpp`）只 include `<windows.h>`、`<commctrl.h>` 與 STL**，不 include 任何機台標頭。
   ctest 的 `test_native_forms.exe` 只連這三個檔就連得起來——IO 寫入、`motor.access`、運動、1203 指令的符號在那顆 exe 裡根本不存在。
2. IO 視窗唯一一顆真的按鈕（「輸出操作（1b 階段才開放，現在停用）」）是 `WS_DISABLED`，視窗程序裡**沒有**它的 `WM_COMMAND` 分支；
   表格「輸出鈕」欄是**畫出來的**停用按鈕（`DrawFrameControl(DFCS_INACTIVE)`），不是控件。
3. MotorView 視窗**一顆按鈕都沒有**（沒有 jog／move／home／servo／power）。
4. wb_serve 膠水 `NativeFormsWbServe.cpp` 只呼叫純函式與讀取函式（監看器的 const 讀取、覆蓋掛鉤的鎖內讀、`MOT[]` 欄位）。
5. ctest 驗：IO 視窗「可按的按鈕」數目是 0；MotorView 視窗按鈕數目是 0。

## 6. 為什麼是表格，而不是照 .dfm 版面（今晚的取捨）

方案文件（`D:\HT9045\.claude\skills\ht9045-cpp-generated-pages\references\native-forms-plan.md`）的 A 案是「dfm2rc 已產好的 `iosetview.rc`＋復原 GA-4 對話框引擎（commit `7b86cfdf` 刪掉的 `ui/layout/*`）」。今晚沒走這條，原因：

- GA-4 引擎約 2,800 行，當年是 **MSVC** 編、有 CDialog 殼，搬到 MinGW 6.3 沒驗證過（方案 §4、§8）；
- `iosetview.dfm` 37,988 行，是六頁中最大的對話框樹（巢狀 DIALOGEX、Stack 分頁、HT9050 專用群組、`LEDSqSmall` 自訂控件）——一晚做不完，做一半反而看不出結論。
- Steven 要的是「先表列全部的 IO 與相關的狀態」「MotorView 也是這樣」，表格正好是這句話；而且它已經能驗真正要決定的事：
  **(1) 訊息泵掛在 wb_serve 主迴圈行不行（含拖曳時的保活）；(2) 資料不經 JSON、直接讀 C++；(3) 唯讀是構造上做到的。**
- golden 的 MotorView 本來就是一張 StringGrid（主畫面分頁，沒有 TForm），表格就是它原本的樣子。
- 之後若決定照 .dfm 版面，換掉的只是 `NativeIoView.cpp` 的「畫面」半邊；資料（`IoRow`／`MotorRow`）、泵與保活（`NativeHost.cpp`）不用動。

技術上：純 Win32。20260928 版用 comctl32 的 ListView（`LVS_OWNERDATA`＋`NM_CUSTOMDRAW`），20260929 起換成自己畫的 `NativeGrid`（§3.2）。
MinGW 6.3 內建標頭就夠，沒有第三方依賴、沒有 MFC。

## 7. 畫面說明

**兩頁共通（20260929）**：摘要三行——第一行點數／軸數，第二行監看器、拖曳保活、更新時間（沒有值的原因接在後面），第三行「畫面效能」。
第三行的「上次更新」是上一次整個更新花的時間（比較＋畫格子＋送上螢幕＋摘要），「實際更新間隔」是兩次更新之間實際隔多久。
在 wb_serve 裡這個間隔目前約 50 ms，原因見 §8 第 9 條。

**HW.IoSetView**
- 摘要：SIM／SHIP 組態、監看器連線、點數、ON／OFF／null、good／bad／nosource／disabled、監看 poll 次數、拖曳保活次數、更新時間。
- 篩選：全部／只看輸入／只看輸出／只看 ON／只看 OFF／只看 null；搜尋 Alias 或位址碼（不分大小寫）。
- 燈號顏色：輸入 ON＝綠、輸出 ON＝紅橘、OFF＝深灰、**null＝白底灰框**（沒有值，不是 off）、bad＝黃（讀卡失敗）、disabled＝淺灰（表上 Enable=0）。

**Main.MotorView**
- 前八欄＝golden 主畫面馬達表：Alias｜目前位置｜目標位置｜速度｜Can｜L｜M｜R。
- 緊接著十欄燈號，每顆燈一欄（CW／HOME／CCW／EMG／ALM／SCW／SCCW／SALM／INP／SVON＝golden Motor Test ALed1..10，位元解碼同 golden `ScanMotorStatus`）。
- 其餘補充欄：亮的燈（文字）、伺服、警報、到位、忙碌、HomeFlag、卡、站/軸、品質、來源、說明（沒有值的原因、驅動器錯誤字串）。
- 篩選：golden 會列的（預設）／馬達表全部／只看 PCI1203／只看警報或歸零失敗／只看沒有值的；搜尋 Alias 或 No。
- 沒有值一律寫 `—`（或燈號白底灰框），不寫 0。

## 8. 已知限制與殘餘風險（明天要一起看的）

1. **拖曳／改大小視窗：已處理，但有殘餘。** Windows 在拖曳、改大小、按住捲軸拇指、打開系統選單時會在 `DefWindowProc` 裡跑自己的訊息迴圈，放開滑鼠才回來；
   泵掛在主迴圈，所以那段時間主迴圈會停。現在的做法（`NativeHost.h` 檔頭）：每個原生視窗開一個 50 ms 計時器，Windows 的那些內部迴圈照樣會分派它（golden VCL 的 TTimer 在拖曳時繼續跑也是同一個道理），
   這時跑「一拍主迴圈的週期工作」（`tools\wb_serve.cpp` 檔尾 `W906_NativeKeepaliveMain`：PumpTick／MainProc、輸出服務、1203 Poll、State Record、Motor Access 的死人開關、Tester 通訊），
   **共用主迴圈的截止時間**，所以 500 ms 拍子不會變快、放開後也不會連發補拍。仍在主迴圈執行緒上，1203 單執行緒規則不變。
   - 殘餘 a：**拖曳期間，一般網頁指令排隊**（指令的 drain／分派是 main() 裡的內嵌程式，保活沒有照抄），放開滑鼠後的第一圈才執行；網頁畫面也暫停更新。
     但**輸出與停止照樣會執行**：保活每一圈都跑輸出優先服務 `W906_ServiceOutputs`（`tools\wb_serve.cpp:6303`），它處理
     `io.btnPanelClick`、`pci1203.do.setBit/setByte`、`pci1203.ax.stop/emgStop`、`motor.stop`（`:6193-6205`），並照它的屏障規則
     「停止不會超過排在它前面的 jog／move／home」——排在前面的 jog 本身也不會執行，所以不會「先停後動」。
     另外 jog 的死人開關（`MotorAccessTick`，`WebMotorAccess.cpp:3763`：操作員連線消失就停）照跑。
   - 殘餘 b：告警／是否框開著的時候拖曳，保活只跑 Index 煞車保護（`DoAvoidIndexMotorFallDown`，同那三個等待迴圈每一圈跑的），不跑 1203 Poll 與面板鍵掃描。
   - 殘餘 c：保活是主迴圈週期工作的**照抄**，主迴圈改了這裡要跟著改。
   - 殘餘 d：保活跑到的 MainProc 若跳出告警框，那個等待迴圈會搶走拖曳迴圈的滑鼠訊息（拖曳可能要多點一下才結束）——golden VCL 在同樣情況也是這樣。
   - 驗證：ctest `NativeHost_Keepalive` 用模擬的內部迴圈驗（有叫、沒多叫、巢狀不重入、不在我們泵裡的迴圈不插手）；
     `test_native_forms.exe --modal-move`（不在 ctest，會短暫顯示視窗、移動滑鼠游標）驗 Windows 真的移動迴圈；展示模式拖曳看燈號是否繼續變。
   - ⓘ ST01-E 原本建議只在 `WM_ENTERSIZEMOVE`～`WM_EXITSIZEMOVE` 開計時器；這裡改成視窗活著就一直開，因為捲軸拇指、系統選單的內部迴圈都不送 `WM_ENTERSIZEMOVE`。代價是泵每秒多丟掉約 20 則 WM_TIMER。
2. 視窗反應最慢約 50 ms（主迴圈的等待上限）；原型只讀，看不出來，但 MotorTest 的 jog 要重新量。
3. 版面不照 .dfm：IO 沒有 Stack 分頁、沒有 HT9050 專用群組的顯隱規則（golden `FormShow :310`）、沒有 golden `ScanLed :2867` 的 50 ms 掃燈（這裡是 200 ms，跟卡片資料的更新頻率一致）。
4. ~~外觀是 comctl32 v5 的傳統樣式~~ → 20260929 起表格是自己畫的（`NativeGrid`），不再靠 comctl32 的樣式。篩選下拉與搜尋框仍是傳統樣式（沒有 manifest）。
   欄寬固定、不能拖，也沒有點欄頭排序。太長的字會加「…」，但是否加是用平均字寬估的（寬字算兩格），不是精確量出來的。
5. ON 建置每次開機都會跳出兩個視窗（沒有執行期開關；`system\NativeForms.ini` 逐頁選是方案 §7.7 的下一步）。
6. 頁面表（`WebPageTable.cpp`）沒有登記原生視窗；網頁那頁與原生視窗互不知道對方開著（唯讀所以今晚沒關係，1b 輸出前一定要做）。
7. 無顯示 ctest 需要有桌面的登入工作階段（Windows service session 0 不能建視窗——推論，沒在服務帳號下驗過）。
8. （已照 golden 改，Q53）MotorView 的「目標位置」每個組態都顯示 `MOT[].TargetPosition`；原本出貨組態照網頁 P5 規則顯示 `—`（§4 最後一條）。
9. **（20260929）畫面做得到 20 ms，但在 wb_serve 裡值還沒有 20 ms。** 下面三處都在 `tools\wb_serve.cpp` 等處，這次沒有改：
   - (a) 主迴圈一圈的等待上限是 50 ms（`wb_serve.cpp:4566`）。它等的是命令佇列的事件，不是視窗訊息，而且整個程式沒有呼叫 `timeBeginPeriod`，所以 `W906_NativeFormsPump` 實際約每 50 ms 才輪到一次。
   - (b) 1203 監看器的 Poll 是每 200 ms 一次（`kIoTickMs`，`wb_serve.cpp:2940`），而且 Poll 會卡在廠商驅動裡（`EtherCAT\Pci1203Monitor.h` 的說明；主迴圈註解記同事量過完整 Poll 約 140 ms）。這段時間主迴圈（也就是畫面）完全不動。
   - (c) 1203 軸的位置／燈來自覆蓋掛鉤的抄本，500 ms 拍子才更新。
   要真的 20 ms，這三處要另外決定（§10）；畫面這一側不用再改。
10. （20260929）拖曳視窗時，畫面由保活計時器帶動，計時器是 50 ms（`NativeHost.cpp kHostTimerMs`），所以拖曳期間畫面約 50 ms 更新一次。放開後回到 20 ms。

## 8.5 明天要實測的風險（白話版，照著試就看得到）

在實驗機上用 ON 建置的 `wb_serve.exe`（§2 B）試。每一條都寫了「怎麼試」與「正常該看到什麼」。

**風險 1：拖曳視窗的時候，主迴圈的工作是「塞在視窗訊息處理的中間」跑的。**
白話：按住標題列拖曳時，Windows 把程式關在它自己的拖曳迴圈裡；我們趁它每 50 ms 分派一次計時器的時候，
把主迴圈該做的事（MainProc、1203 讀取、輸出、Motor Test 的迴圈步進…）拿進來做一拍。它還是同一條執行緒，但呼叫堆疊比平常深一層。
- 怎麼試：網頁 Motor Test 開一個 Loop Move（或 SIM 組態跑生產），然後按住任一個原生視窗的標題列拖來拖去 10 秒以上，再放開。
- 正常該看到：
  1. 原生視窗摘要列最後的「更新 hh:mm:ss.mmm」在拖曳時**一直在跳**，「拖曳保活 N 次」一直增加；
  2. 放開後主控台印 `[native] window drag/resize ended: N keepalive ticks ran during it`，N 大約是「拖曳秒數 × 15～20」；
  3. 拖超過 5 秒也**不會**出現 `[WATCHDOG] 主執行緒已 … 秒沒有前進`；
  4. 放開後網頁 Motor Test 的 Loop 次數是持續增加的，不是停在拖曳開始那一刻。
- 不正常的樣子：更新時間停住、放開後才一次跳很多；或出現 `[WATCHDOG]`。那表示保活沒跑，主迴圈在拖曳時停了。
- 另一個要注意的：拖曳時如果 MainProc 剛好跳出告警框，拖曳可能要多點一下滑鼠才會結束（告警框的等待迴圈會拿走拖曳的滑鼠訊息）。golden 同樣情況也是這樣。

**風險 2：拖曳期間，一般的網頁指令會排隊，放開才執行。**
白話：保活只做「主迴圈每一拍的例行工作」，沒有處理網頁送來的一般指令（那段程式在 main() 裡面，搬不出來）。
例外是輸出與停止：網頁的 IO 輸出鈕、`motor.stop`、`pci1203.ax.stop／emgStop` 由輸出優先服務處理，拖曳時照樣執行。
- 怎麼試：一隻手按住原生視窗標題列不放，另一隻手（或另一台 PC）在網頁上按一般按鈕（例：開 Recipe 頁、存檔），再按一個 IO 輸出鈕。
- 正常該看到：一般按鈕**在放開視窗之前沒有反應**，放開後馬上執行；IO 輸出鈕**按了就動**（燈號也在原生視窗上跟著變）。
- 要特別看的：拖曳時在網頁按住 jog 再放開 —— jog 與 stop 兩個指令都會排隊（停止不能插隊到 jog 前面，這是既有的安全規則），
  放開視窗後同一圈依序執行：**軸可能會動一下又停**。在 SIM 看位置欄；真機請先用慢速或不接負載的軸試。

**風險 3：告警／是否／訊息框開著的時候拖曳，只保護 Index 煞車，其他都停。**
白話：告警框等待時本來就已經在 MainProc 裡面，不能再叫一次 MainProc（會重入）。所以這時的保活只跑 `DoAvoidIndexMotorFallDown`
（EMG／斷電／Index Z 伺服關 ⇒ 鎖 Index 煞車並停機，三個等待迴圈每一圈本來就跑它）；1203 讀取與實體面板鍵的掃描在拖曳期間都暫停。
- 怎麼試：讓機台跳一個會等回答的告警（SIM 生產時任一個 JAM），框開著的時候拖曳原生視窗 5～10 秒。
- 正常該看到：拖曳期間 IO 燈號（原生與網頁）**不更新**，放開後才更新；拖曳時按實體面板鍵（例 Retry）沒有反應，放開後再按才有；
  告警框本身仍然可以用網頁回答（放開後）。
- 不正常的樣子：放開後燈號還是不動、或告警框再也關不掉。

## 9. 檔案

| 檔案 | 內容 |
|---|---|
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeForms.cmake` | 開關 `option(W906_NATIVE_FORMS ... OFF)`；ON 時把檔掛進 wb_serve、加 ctest（20260929 起四支）|
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeHost.h`／`.cpp` | 共用主機：訊息泵、拖曳保活、快速鍵、字型（純 Win32） |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeGrid.h`／`.cpp` | （20260929）不閃、只畫變了的格子的表格控件＋雙緩衝標籤（純 Win32，不含機台碼） |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeIoView.h`／`.cpp` | IO 視窗（純 Win32，不含機台碼） |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeMotorView.h`／`.cpp` | MotorView 視窗（純 Win32，不含機台碼，沒有按鈕） |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\ui\native\NativeFormsWbServe.cpp` | wb_serve 膠水（`W906_NativeFormsStart/Pump/Stop`；讀 HSys.IOTable／HSys.MotTable／MOT[]／1203 監看器／覆蓋掛鉤） |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\tests\test_native_forms.cpp` | 無顯示 ctest（20260929 起四支，含 `--perf`）＋ `--modal-move` ＋ `--show` 展示 ＋ `--snapshot` 拍圖 |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\CMakeLists.txt` 第 67 行 | 佔用原本的空行放 `include(ui/native/NativeForms.cmake)`（CMake 一行只能一個指令；其後行號不動） |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp` | 五處同一行插入（:4536 開窗＋登記 nextPump、:4538 登記 nextIo、:4575 主迴圈每圈泵、:5967 關閉、:7622 阻塞等待框每圈泵）＋檔尾（保活兩支、OFF 空函式）；沒有任何一行位移 |
| `D:\AI_TempFile\wt-native-proto\HT9011UC_Cpp_V3.33.906.0\JsonBridge\ChanMotorPoints.cpp` 第 209 行 | 同一行加 `W906_NativeMotorOverlay()`（回傳既有的覆蓋掛鉤指標，只讀）；其後行號不動 |

## 10. 要 Steven／Jimmy 決定的

- 這個方向（原生視窗掛在 wb_serve 主迴圈，拖曳時用計時器保活）要不要繼續？§8 第 1 條的殘餘 a（拖曳期間一般網頁指令排隊；輸出與停止照常）可不可以接受？
- 版面：照 .dfm（A 案，GA-4 引擎＋`iosetview.rc`，工作量大）還是像這樣的表格／自訂版面？
- MotorView 的目標位置 → **已裁決**（Steven 20260929 17:32 Q53「照建議值」）：照 golden，一直顯示 `MOT[].TargetPosition`（§4 最後一條）。MotorTest 沒有目標位置欄，不受影響。
- 執行期逐頁選（`system\NativeForms.ini`，方案 §7.7 (ii)）→ **不做**（Steven 20260929 17:32 Q54「不用. 只有馬達移動相關的六頁有需要」）：只有建置時的 ON／OFF；原生只做這六個跟馬達移動有關的頁面（IoSetView、MotorView、MotorTest、ShuttleMove、home、teach）。
- 下一頁：照順序是 MotorTest（第一頁有按鈕、會動機台的）。
- （20260929）實機要不要真的 20 ms（§8 第 9 條）→ **先不改**（Steven 20260929 17:32 Q55「html端可以不用那麼快, 後續有問題才調整」）：下面 (a)～(c) 都不動，wb_serve 主迴圈 50 ms、覆蓋掛鉤 500 ms、1203 Poll 200 ms 維持，之後有問題再調：
  - (a) 主迴圈等待上限從 50 降到 20 ms，並加 `timeBeginPeriod(1)`；
  - (b) 1203 Poll 更快，或把 DI 讀取和整份 Poll 分開，只有 DI 每 20 ms 讀一次；
  - (c) 覆蓋掛鉤的抄本改成 20 ms 拍子。
  
  三件都動到主迴圈和 1203 的節拍（B13 裁決的 500 ms PumpTick 不受影響，但 CPU 與卡的負擔會變），要 Steven／Jimmy 決定。

## 11. 給 Jimmy 審核的 MR（20260929，ST01-E3）

> 讀者：Jimmy（審核）、Steven。Steven 20260929 15:3x：「c++原生的畫面, 推上去給jimmy審核」。
> 分支 `v906/steven-native-forms-review` → `main`。本節就是 MR 的說明（push option 只能放一行，所以寫在這裡）。

### 11.1 內容

六個原生 Win32 視窗的第一版，**全部只顯示**（不送任何會動機台的指令），掛在 wb_serve 的主迴圈上：

| 視窗 | 檔案 | 誰做的 | 快速鍵（ON 的 wb_serve） | ctest |
|---|---|---|---|---|
| HW.IoSetView | `ui\native\NativeIoView.*` | St01（ST01-E3） | Ctrl+Alt+I | `NativeIoView_Headless` |
| Main.MotorView | `ui\native\NativeMotorView.*` | St01（ST01-E3） | Ctrl+Alt+M | `NativeMotorView_Headless` |
| HW.MotorTest | `ui\native\NativeMotorTest.*` | St02（St02-E） | Ctrl+Alt+T | `NativeMotorTest_Headless` |
| HW.ShuttleMove | `ui\native\NativeShuttleMove.*`、`NativeShuttleMoveGlue.cpp` | St02 | Ctrl+Alt+S | `NativeShuttleMove_Headless` |
| HW.home | `ui\native\NativeHome.*` | St02 | Ctrl+Alt+H | `NativeHome_Headless` |
| HW.teach（表格版，260 列） | `ui\native\NativeTeach.*` | St02 | Ctrl+Alt+K | `NativeTeach_Headless` |

共用的部分：`NativeHost.*`（訊息泵、拖曳保活、快速鍵）、`NativeGrid.*`（不閃、只重畫變了的格子，每 20 ms 比一次；本檔 §3.2）、
`NativeFormsWbServe.cpp`（wb_serve 膠水，只讀記憶體：HSys.IOTable／MotTable、MOT[]、1203 監看器的覆蓋掛鉤）。
St02 四頁的細節見 `ui\native\README_ST02.md`（golden 看起來是顯示、其實會動機台的地方 v1 一律不做；指令鈕照列但全部 `WS_DISABLED`）。

**只顯示是構造上做到的**：視窗檔只 include `<windows.h>`、STL 與 `NativeHost.h`／`NativeGrid.h`；ctest 的測試程式只連視窗檔、連得起來，
就表示視窗沒有任何一條路通到運動、IO、卡或檔案（本檔 §5）。

### 11.2 預設 OFF：產品不變

- 開關 `option(W906_NATIVE_FORMS ... OFF)`（`ui\native\NativeForms.cmake`）。不加 `-DW906_NATIVE_FORMS=ON` 就是今天的 wb_serve。
- 除了 `ui\native\`、五支 `tests\test_native_*.cpp` 和一份 skill 參考文件（`prototype-20260928.md`），產品程式只動三個檔，全部**同一行插入、其後行號不動**：
  - `CMakeLists.txt` 一行 `include(ui/native/NativeForms.cmake)`（佔原本的空行）；
  - `JsonBridge\ChanMotorPoints.cpp` 同一行加 `W906_NativeMotorOverlay()`（回傳既有的覆蓋掛鉤指標，只讀）；
  - `tools\wb_serve.cpp` 五處同一行插入（:4536、:4538、:4575、:5967、:7622）＋檔尾（保活兩支＋OFF 的五個空函式）。
- OFF 的證據（本 MR 的建置，見 11.6）：ctest 不登記任何 Native 測試；`wb_serve.exe` 裡 `w906native` 符號 0 個；五個接縫都是空函式。

### 11.3 怎麼看

1. **不碰機台檔的展示（先看這個）**：IoSetView＋MotorView 兩頁，假燈號持續變化，不讀 Gerneral.ini／config.ini、不寫檔、不碰卡：
   `test_native_forms.exe --show D:\HT9045\machines\HT9050\IO_Table.csv D:\HT9045\machines\HT9050\Mot_Table.csv`
   （ON 建置的 exe；本 MR 的在 `D:\AI_TempFile\st01e3-nreview-on\test_native_forms.exe`。St02 的四頁沒有展示模式，要用 2。）
2. **實驗機上的 ON wb_serve**：用 `-DW906_NATIVE_FORMS=ON` 建出來的 `wb_serve.exe`，照平常不帶參數啟動（本檔 §2 B）。
   開機後自己開出 IoSetView／MotorView，其他四頁用上表的快速鍵開；關窗不會關 wb_serve。
   ⚠ 這顆 exe 跟平常的 wb_serve 一樣讀寫機台檔，只在實驗機上跑。St01／St02 的規定不在開發機上對真檔跑 wb_serve，所以**視窗開在 wb_serve 裡的樣子還沒有人看過**。

### 11.4 三個拖曳風險（本檔 §8.5，實驗機上照著試）

1. **拖曳時主迴圈的工作是塞進 Windows 的拖曳迴圈裡跑的**（每 50 ms 一拍，同一條執行緒、堆疊深一層）。機制已驗（`--modal-move`），實際影響沒驗：
   拖曳 10 秒以上時，更新時間要一直跳、不能出現 `[WATCHDOG]`、Loop 次數要持續增加。
2. **拖曳期間一般網頁指令排隊，放開才執行**；輸出與停止（IO 輸出鈕、`motor.stop`、`pci1203.ax.stop／emgStop`）照跑。
   拖曳時在網頁按住 jog 再放開：jog 與 stop 都排隊，放開後同一圈依序執行，**軸可能會動一下又停**（真機先用慢速或不接負載的軸試）。
3. **告警／是否／訊息框開著時拖曳，只跑 Index 煞車保護**（`DoAvoidIndexMotorFallDown`），1203 讀取與面板鍵掃描暫停，放開才恢復。

### 11.5 Steven 的兩題（20260929 17:32 已裁決）

- **Q3（＝Q53）照 golden**：出貨版 MotorView 也一直顯示 `MOT[].TargetPosition`（Steven「照建議值」；改在 11.7 的 commit）。MotorTest 沒有目標位置欄，不受影響。
- **Q4（＝Q54）不做**：不加執行期逐頁切換、不用 `D:\HT9045\system\NativeForms.ini`；原生只做這六個跟馬達移動有關的頁面（Steven「不用. 只有馬達移動相關的六頁有需要」）。建置時的 ON／OFF 維持。另外 Q55：wb_serve 資料端的節拍先不改（§10 最後一條）。

### 11.6 這個 MR 的組成與驗證

**組成**（`v906/steven-native-forms-review`）：
- St02 的 `v906/steven-native-forms-st02` `449c5cd7`（四頁，底是 St01 的不閃版 `68a7a5f0`）；
- 合併 `6a264693`：St01 原型分支的 `70af7218`（本檔 §3.2，St02 分支裡沒有這一顆）；
- 合併 `bddf4fa8`：`origin/main` `a84d25cc`。唯一衝突是 `tools\wb_serve.cpp` 兩處：
  - :4575：main 在 `W906_TesterCommTick` 後面加了 `W906_StateRecordDrainLog`，本分支在同一行前面加 `W906_NativeFormsPump(0)`，兩邊都留。合併後那一行拿掉 main 加的那段，就跟本分支原本的那一行一字不差。
  - 檔尾：兩邊都接在檔尾。main 那一段放前面（行號跟 main 一樣，例如 `W906_OwnerHeldSince` 仍在 :8017），原生那一段接在後面。
  - 其他四個呼叫點（:4536、:4538、:5967、:7622）自動合併，main 原本的字一個不少（只有插入）。
- `debcbdc6`：`tests\test_native_teach.cpp` 第 3 項的預期值 17 → 16（同一行）。
  `449c5cd7` 把假資料列數 265 改成 260；測試用 `i / 20` 決定頁籤，Tab13 原本只有第 260～264 列，所以現在只剩 Tab00～Tab12。視窗程式是對的，只是測試沒跟著改。已經請 ST01-M 轉告 St02。

**驗證**（20260929 15:5x～16:53，MinGW 6.3，SIM 組態，ST01-E3 這台，`-j4`）：

| 項目 | ON（`-DW906_NATIVE_FORMS=ON`，`D:\AI_TempFile\st01e3-nreview-on`） | OFF（預設，`D:\AI_TempFile\st01e3-nreview-off`） |
|---|---|---|
| 建置 | 綠：`wb_serve`＋五支原生測試程式，33 分鐘；`ui\native\*`、`tests\test_native_*` 0 個警告 | 綠：`wb_serve`，15 分鐘 |
| `ctest -R Native` | **8/8 通過**，共 238 項檢查。IoView 38、MotorView 42、Keepalive 12、Grid_Efficiency 21、MotorTest 25、ShuttleMove 22、Home 28、Teach 50 | 登記 0 支 |
| 每次更新的平均時間（預算 20 ms） | IO 0.46 ms、MotorView 1.12 ms、MotorTest 1.17 ms、Home 1.61 ms、Teach 0.30 ms | — |
| OFF 沒有原生碼 | — | `nm wb_serve.exe` 的 `w906native` 符號 **0 個**（ON 4,586 個）；沒有任何 `flags.make` 帶 `W906_NATIVE_FORMS`（ON 254 個）；五個 `W906_NativeForms*` 都是 6 位元組的空函式 |
| 機台檔 | 8 個檔（Gerneral.ini、config.ini、ContactInfo.ini、levelset.dat、lastdata.dat、Error\Chinese／English／Singapore\JAM0000.dat）的 SHA256 在 16:00 和 16:53 完全相同；`D:\HT9045_Log` 仍是 152 個檔、沒有新檔 | 同左 |

- 第一次跑 ON 的時候，這台 CPU 100%（OFF 正在 `-j4` 編，還有別的工作）。當時 `NativeGrid_Efficiency`（3.76 ms）與 `NativeMotorTest_Headless`（4.29 ms，最差 273 ms）沒過 3 ms 這條檢查。OFF 編完之後重跑就過了（上表），檢查門檻沒有放寬。
- 只做了原生測試，**沒有跑全部 ctest**。這個 MR 在原生以外只改了上面三個檔的同一行插入：OFF 時 wb_serve 呼叫的五個接縫都是空函式，ChanMotorPoints 多的那個只讀取得函式沒有人呼叫。要全量 gate 的話，可以交給 ST01-M 代跑。
- 沒有在這台跑 `wb_serve`（規定）。視窗開在 wb_serve 裡的樣子，要照 11.3 的第 2 點在實驗機上看。

### 11.7 Q53 的修改（20260929 17:4x，ST01-E3）

- `ui\native\NativeFormsWbServe.cpp:390` 同一行：`if (motLive)` → `if (mi >= 0 && mi < MAX_TRAY_MOTOR)`。golden 在 MotorTestClass 會列的每一列都顯示 `MOT[iMot].TargetPosition`，沒有組態條件、也不看 Motor 物件，所以這裡只檢查索引合法。SIM 也跟著一樣：以前 SIM 只有 Motor 物件存在的列才有值。
- 這一行只在 ON 的 wb_serve 裡（膠水），無顯示 ctest 只連視窗檔，所以沒有 ctest 驗「—」，也不用改測試。驗證：膠水檔 ON 的語法檢查 SIM／SHIP 各 0 個錯誤，警告數跟改之前一樣（43／43）；ON 重編 wb_serve 綠、`ctest -R Native` 結果見下一行。
- `ctest -R Native`（17:45～17:5x，ON）：6 支第一次就過（IoView 38、MotorView 42、Keepalive 12、ShuttleMove 22、Home 28、Teach 50）；`NativeGrid_Efficiency` 與 `NativeMotorTest_Headless` 只有「平均更新 < 3 ms」這一條，在這台忙的時候沒過（CPU 93%：4.94／3.36 ms；重跑第 1 次 3.04／4.32 ms），重跑第 2 次通過（2.79／2.01 ms）。這兩支測試程式是 16:28 編的，跟 16:47 全過的是同一顆（這次只重編了 wb_serve 的膠水），門檻沒改。3 ms 這條在共用機器上會隨負載失敗；離 20 ms 的預算還很遠。8 個機台檔的 SHA256 前後相同，`D:\HT9045_Log` 152 個檔不變。
- OFF 不編 ui\native，不用重編。MotorTest（St02）沒有目標位置欄，沒有動。
