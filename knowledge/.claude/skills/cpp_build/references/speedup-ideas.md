# V906 C++ 加快編譯——大家的做法

有好方法就加一列（最新的放最下面）。量測請寫清楚：哪台機器、哪個建置目錄、前→後。沒量過寫「未量」。
試過沒效、或有副作用的也請記下來，免得別人重踩。

| 日期 | 誰 | 做法 | 量測 | 注意事項 |
|---|---|---|---|---|
| 20260921 | 筆電（Jimmy） | `cmake/W906_FastBuild.cmake`：PCH（Tier A god headers、Tier B vclcompat）＋ccache launcher | Config.cpp 4.32→2.39 s、AMR.cpp 4.61→1.40 s；ccache hit 約 0.7 s | .gch 不可重現、帶絕對路徑 ⇒ 用 PCH 的檔只在 .gch 沒變時才 hit；不做 unity build（ODR 陷阱）。⚠ **CMakeLists.txt 沒有 include 它**（`1f509315`「刻意不掛 ⇒ 整套惰性」），所以 main 上目前沒有生效（St02-E 1004 08:5x 查到） |
| 20261003 | NB2-1 | !159：PE 截斷檢查每個檔開啟最多等 `-OpenTimeoutMs`（30 s），防毒鎖住報 LOCKED | gate 不再卡 30 分鐘 | 只改檢查工具，不碰防毒 |
| 20261004 | NB2-1 | !167：防毒拒絕（Win32 225／226）或列出後消失的 exe 報 AV-BLOCKED，不讓整個檢查失敗 | — | 同上 |
| 20261004 | St02-E | 增量建置改用 `cmake --build <dir> -j 16`（bash、stdin `< /dev/null`），之後跑 !159 的 PE 檢查（`lane_cmake.sh` 做法） | 未量（避開 build.bat 舊 PE 檢查卡住） | — |
| 20261004 | St02-M | STEVEN-NB3 用 winget 裝 ccache 4.14.1（user scope） | 未量 | **只裝不會生效**：St02-E 試 `cmake -UW906_CCACHE_EXE <dir>` 重新 configure，輸出沒有 `W906 fastbuild` 那行——要先把 W906_FastBuild.cmake 接上（SKILL 4.2） |
| 20261004 | St02-E | 比對每支 exe 的 DLL 匯入改用只讀 PE 標頭的小腳本（`pe_imports.py`），不用 objdump | 未精確量（objdump 跑 349 支 exe 超過 40 分鐘才改寫） | 用來做「新依賴只出現在預期的 exe」檢查（!174 的 WINMM） |
| 20261004 | St02-E | 實驗：`W906_FastBuild.cmake` 用 `-DCMAKE_PROJECT_INCLUDE=` 臨時接上（不改 CMakeLists.txt）；STEVEN-NB3、sim、main `fb608e5e`、-j16、三個獨立建置目錄 | 全新建置：一般 1393 s；接上 ccache 冷 1374 s（hit 27%）；熱 1249 s（hit 74%）⇒ 只快 1～10%。改一個 .cpp：一般 1726 s＝1 次編譯＋240 次連結，時間幾乎都在連結（同樣工作量另一次 934 s，受機器負載影響大） | ⚠ 接上後改一個 .cpp 重建**失敗**：MinGW 6.3 `internal error in mingw32_gt_pch_use_address ... MapViewOfFileEx`（PCH 映射失敗，ccache 記 Uncacheable）。目的檔比對 6 支：5 支相同；fMain.cpp.obj 198 行差異全是 .rdata 字串位移，遮掉後 0 差異（字串集合相同）。⇒ 不建議照現況接上；要加速先處理連結（NB2 那列） |
| 20261004 | NB2-1（卡 (j)，MR !178，第 143 包已進 main） | `W906_FAST_INCREMENTAL`（預設 OFF）：靜態庫改 thin archive（`ar qcT`）；並建議 F5 建置目錄改用 **Ninja** | NB2（SSD、WinLibs 16.2、wb_serve、改一個 .cpp、-j 6、各 3 次、新目錄）：MinGW Makefiles OFF 27～28 s／ON 27～32 s；**Ninja ON 8～10 s、Ninja OFF 8～9 s**；全新建置 Makefiles 389～391 s、Ninja 301～315 s；libht9045_sm.a 37,979 KB→1,828 KB（thin） | SSD 上的好處全來自 Ninja（不產生 objects.a、用 response file 連結）；thin archive 在機台 HDD 上才省得到那 25 s 的 159 個物件複製。`CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS` 在 MinGW Makefiles 下拿不掉 objects.a（試過）。gate（build.bat：Ninja、g++ 6.3.0）不設這個選項。機台建議：`build_integ_ship_x86` 用 `-G Ninja -DW906_FAST_INCREMENTAL=ON` 重建（NB2 README R218） |
