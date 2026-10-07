> 保存來源：`.claude/skills/cpp_build/SKILL.md`，main `2db43115d`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../common.md)。

> **最新建置分流（Jimmy 20261007）**：F5、一般 build.bat 與 Ctrl+Shift+B 只建 wb_serve 及必要依賴，BUILD_TESTING=OFF，不跑測試／probe／PE 驗證工具。開發驗證仍保留：明確執行 build.bat gate/test，使用獨立 build_tests 或 build_tests_<工作代號> 目錄，BUILD_TESTING=ON；禁止使用應用程式／F5 目錄。W906_TEST_BUNDLES 只是打包方式，不能當成關閉測試。下文歷史「一般建置含 tests」描述依本裁決更新；完整規則見 [F5 與開發驗證共用規則](../../../../../docs/handoff/RD5軟體_F5與開發驗證建置規則_20261007_134727.md)。未執行測試或零測試不得回報 gate 通過。

<!-- preserved-content:start -->

# V906 C++ 建置（cpp_build）

`bcb_build` 管 BCB6 量產版；這份只管 V906 C++ 移植樹 `HT9011UC_Cpp_V3.33.906.0`。
初版：St02-M 20261004（Steven 要求「做個新的 cpp build skill」）。**有更好的做法請直接補進第 6 節**。

## 1. 環境

| 項目 | 現況 | 說明 |
|---|---|---|
| 編譯器 | MinGW g++ 6.3.0（`C:\MinGW\bin`） | oracle 線，**不換**；機台用 WinLibs i686 g++ 16.2.0 |
| CMake | 4.0.2（`C:\CMake\bin`） | |
| 產生器 | 找得到 `ninja.exe` 就用 Ninja，否則 MinGW Makefiles | 只在**第一次 configure** 決定；要換先 `build.bat clean` |
| 建置目錄 | `<repo>\Obj\V906\<dir>`（原始碼樹外） | `V906_OBJ_ROOT` 可改 |
| 加速 | `cmake/W906_FastBuild.cmake`（PCH＋ccache）**寫好了但沒接上**：CMakeLists.txt 沒有 include 它（0921 `1f509315` 刻意不掛），所以目前 PCH、ccache 都**沒有生效** | 見第 4 節 |

## 2. 怎麼建

`build.bat`（在 `HT9011UC_Cpp_V3.33.906.0\`，從哪裡跑都可以）：

| 指令 | 做什麼 |
|---|---|
| `build.bat` | 增量建置（需要時才 configure），日常「還編得過嗎」 |
| `build.bat serve` | **只建 wb_serve**（F5 需要的就這個，不建 ~340 支測試 exe） |
| `build.bat gate` | configure＋建置＋ctest，回報工作時引用的數字 |
| `build.bat test` | 只跑 ctest |
| `build.bat clean`／`prune [-y／N]` | 刪預設建置目錄／列出（或刪）舊的 build_* |

環境變數：`V906_BUILD_DIR`（換目錄）、`V906_GENERATOR`、`V906_CMAKE_ARGS`（第一次 configure 的 -D）、`V906_OBJ_ROOT`、`V906_CTEST_ALL=1`、`V906_CTEST_ARGS`。

**兩組態都要建**：sim（預設，SOFT_SIMULTE 開）與 ship（`-DW906_NO_SOFT_SIMULTE=ON`，習慣放 `build_ship`）。
直接用 CMake 也可以：`cmake --build <建置目錄> -j 16`；在 bash 背景跑時 stdin 接 `< /dev/null`（見第 5 節）。

## 3. 「完成」的標準——單一 cpp 編過 ≠ 模組完成

同一個 cpp 會被編進主程式的大型靜態庫（ht9045_sm、ht9045_forms…）**和**很多支只挑部分檔案的 ctest 目標；功能多半靠別人檔案裡「同一行加一個呼叫」接進主迴圈；兩組態的 `#ifdef` 不同。所以實際的完成標準是：

1. **兩組態、全部目標**完整建置 0 錯誤，PE 檢查過（錯誤數用 `: error:|fatal error:|FAILED:|undefined reference` 算）。
2. 新全域符號 `nm` 各只出現一次；exe 的 **DLL 匯入**沒有意外增加（W58 曾把 WININET 拉進 ELA_Ftp）。
3. **掛點真的接上**：呼叫寫在 `//` 註解前面；被呼叫的函式讀到底，本體不在 `#if 0`／閘裡。
4. 有 ctest，資料**種在被測函式真正讀的源頭**，附**反向檢查**（拿掉改動要紅）。
5. 代跑兩組態綠；會被牽動的**計數類測試**（START_SitesCensus、FShow_Audit 基準、SECS 數量、相鄰行釘子）一起改。
6. 對 main 試合（`git merge-tree` 只印一行）、筆電 gate 綠才進 main；上機要看的列人工審核。

## 4. 加快編譯

### 4.1 先看實測：慢在打包與連結

機台派工 5（EastSun 1003，i7-14700、7.7 GB RAM、原始碼與建置目錄在 5400 轉 HDD、WinLibs 16.2.0、MinGW Makefiles、-static）：

| 情境 | 時間 |
|---|---|
| 改一個 .cpp 按 F5 | 約 **61 秒** ＝ 編譯 3 秒＋重打包 libht9045_sm.a（159 個 .o）25 秒＋重打包 wb_serve 的 objects.a（`--whole-archive`）22 秒＋連結 11 秒 |
| 改 MachineType.h | 重編 400～600 個檔（-O0 245 秒、-O2 352 秒） |
| 全部 -O2 | 約 21 分 |

STEVEN-NB3 1004：合 main 後動到 fMain.h ⇒ 兩組態幾乎全部重建，約 45 分。

### 4.2 寫好了、但還沒接上的（`cmake/W906_FastBuild.cmake`，20260921）

> ⚠ **目前沒有生效**（St02-E 1004 08:5x 查到、St02-M 確認）：main 的 CMakeLists.txt 沒有 `include(cmake/W906_FastBuild.cmake)`，也沒有 `CMAKE_PROJECT_INCLUDE`；0921 收尾 commit `1f509315` 寫明「CMakeLists 刻意不掛 ⇒ 整套惰性」，條件是先量測＋objcompare。
> build.bat 只會把找到的 ccache 放上 PATH、印快取統計；**真正把 ccache 當 launcher、開 PCH 的是這支 .cmake，沒 include 就都不會發生**（configure 輸出不會有 `W906 fastbuild` 那行）。
> 啟用方式（擇一）：(a) CMakeLists.txt 加一行 include（筆電的檔，要筆電同意＋先量測＋objcompare）；(b) 不改原始碼、只對單一建置目錄試：第一次 configure 加 `-DCMAKE_PROJECT_INCLUDE=<root>/cmake/W906_FastBuild.cmake`（未試過，試了請記在 speedup-ideas）。
> **St02-E 1004 實測（14:05）：不建議照現況接上。** 全新建置只快 1～10%（一般 1393 s；接上後 ccache 冷 1374 s、熱 1249 s），改一個 .cpp 的時間幾乎都在 240 次連結；而且接上後增量重建在 MinGW 6.3 會**隨機失敗**：`internal error in mingw32_gt_pch_use_address ... MapViewOfFileEx`（GCC 在 Windows 上的 PCH 映射問題）。目的檔比對：程式相同，只有 .rdata 字串位移不同。⇒ 若要接，**MinGW 6.3 上 PCH 要關**（`-DW906_PCH=OFF`），只留 ccache；真正要加速的是連結（4.4 第 1 項）。數字見 `references/speedup-ideas.md`。

下面是這支 .cmake 接上之後會做的事（0921 在筆電量的數字）：

- **PCH**：cmydef.h／cprod.h／MachineType.h（Tier A）、vclcompat 總標頭（Tier B）。單檔編譯 -45%～-70%；`tools/pch_eligibility.py` 只對本來就 include 這些標頭的檔案開。
- **ccache**：沒變的檔案直接取快取（約 0.7 秒），結果與真編位元組相同；`depend_mode`、`base_dir`、`sloppiness=pch_defines,time_macros` 由 CMake 自動設定。
- **刻意不用 unity build**：樹裡有 uPlateInfo、TMyKitSuck 兩個 ODR 陷阱和沒 `#undef` 的檔內巨集。

### 4.3 每台都裝 ccache（裝好備用；要等 4.2 接上才會生效）

```powershell
winget install --id Ccache.Ccache --scope user
```

- 裝完開新的 shell；build.bat 和那支 .cmake 都會自己去 `%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ccache.Ccache_*` 找。
- **只裝 ccache 不會變快**——要 4.2 的 .cmake 被 include（或用 4.2 (b) 的 `CMAKE_PROJECT_INCLUDE`）。接上之後，既有建置目錄要重新 configure（或開新目錄），configure 輸出要看到 `W906 fastbuild: ccache ON`。
- 驗證：建兩次後 `ccache --show-stats` 要有 hit。
- 注意：`__DATE__`／`__TIME__`（cObserver.cpp、WebBridgeTags.cpp、TempCtrl/TriTemp.cpp）可能拿到第一次編譯的時間；要可信的版本戳記就 `-DW906_CCACHE=OFF`。
- 安裝紀錄：STEVEN-NB3 1004 08:2x 裝 4.14.1（F-Secure 沒擋，`ccache --version` 可執行）；**加速效果還沒在這台量過**——量了請補第 6 節。

### 4.4 提案（依效益排序；NB2 卡 (j)「機台編譯太慢」是負責的卡）

0. ~~先把 4.2 接上~~——**1004 實測後降級**：PCH＋ccache 對全新建置只快 1～10%、對「改一個檔」幾乎沒幫助（時間在連結），且 PCH 在 MinGW 6.3 會讓增量重建隨機失敗。若仍要接，只接 ccache（`-DW906_PCH=OFF`）；優先做下面第 1 項。
1. **改連結方式**（改一個檔最有效，可省約 47 秒的重打包）：ht9045_sm 改 thin archive 或 OBJECT library；用 response file（`CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS`）取代每次重打 objects.a。
2. **Ninja（實測最有效）**：build.bat 已支援（找得到 ninja.exe 就用；安裝 `winget install --id Ninja-build.Ninja --scope user`），新目錄才會生效。NB2-1 實測（!178）：改一個 .cpp 的 F5 建置 MinGW Makefiles 約 28 s → **Ninja 約 9 s**，全新建置 391 s → 302 s；Ninja 不產生 objects.a、直接用 response file 連結。機台 HDD 上再加 `-DW906_FAST_INCREMENTAL=ON`（thin archive，!178 已在 main）省掉重打包。
3. **建置目錄放 SSD／NVMe**：`V906_OBJ_ROOT` 指到 SSD 上。
4. **開發時只建需要的目標**：`build.bat serve`，或 `cmake --build <dir> --target wb_serve test_xxx`；推之前才兩組態全建。也可考慮測試 exe 移出 ALL、共用 test_bootstrap 做成 OBJECT library。
5. **ccache 每台都裝**（4.3）：換分支、新 gate 目錄時最有感。
6. **長期：熱門標頭瘦身**（前置宣告、少用的 include 移出 cmydef.h／fMain.h）。拆 MachineType.h 要 EastSun 裁決（MW-1），不要自己拆。

每一項都要在**新的建置目錄**量前後對照（表格放 MR 說明），而且 gate 的結果不能變。

### 4.4a St01 1006 實測與 Steven 的決定（1006 14:0x）

- St01（NVMe、22 執行緒、Ninja、只建 wb_serve）：g++ 6.3 全新 469 s／改 1 個 .cpp 51 s／碰 `cprod.h` 407 s（429 檔）；WinLibs 16.2 全新 647 s／改 1 個 .cpp **21 s**／碰 `mymotor.h` 357 s（245 檔）。細節在 `references/speedup-ideas.md` 20261006 兩列。
- **Steven 1006 決定**：①第一階段（Ninja、只建目標、建置目錄放 SSD、ccache）直接做——Jimmy 同日 !249 也把 Ninja 推給全員（`tools/ninja_setup.ps1`）；②**編譯器要換成 WinLibs 16.2**（機台已在用）——oracle 線的切換由筆電排 gate 對照後執行，4.5 第 2 點的「不換」自此作廢、改成「換之前要有兩組態 gate 與 ctest 失敗清單對照」；③標頭瘦身與 Motor／IO 隔離的評估在 `references/header-slimming-and-motor-io-isolation-20261006.md`（結論：先做 S0 註解規則與 cmydef.h 三拆，DLL 不做；B 部分結論要併進 hpi-motor-control／hpi-io-control 的通用節）。

### 4.5 不要做

- **不要自己替防毒加排除**——公司資安軟體，IT 決定（gate 的偶發逾時也跟它的 CreateFile hook 有關）。
- ~~不要在 oracle 線換掉 g++ 6.3.0。~~ Steven 1006 14:0x 決定換 WinLibs 16.2（見 4.4a）；在筆電完成 gate 對照、切換之前，oracle 線照舊用 6.3。
- 不要開 unity build（見 4.2）。

## 5. 踩過的坑

| 坑 | 怎麼避 |
|---|---|
| PowerShell 背景跑 build.bat，cmake 停在 Generating 之後 | 用 bash，stdin 接 `< /dev/null` |
| PE 截斷檢查遇到防毒鎖住的 exe 一直卡 | NB2-1 !159 的 `-OpenTimeoutMs`；!167 把防毒拒絕／消失的 exe 標 AV-BLOCKED |
| 測試 exe 名稱有 setup／install／update／patch，Windows 要求提權 ⇒「Not Run」 | 名稱避開這四個字 |
| 機器忙時測試逾時（端點安全的 CreateFile hook，St01 E-040） | 單獨重跑判定，不要當成程式錯 |
| exe 在建置與 ctest 之間消失，ctest 顯示 Not Run「Could not find executable」 | 先看 Windows「應用程式」事件記錄 **event 25（FSecure-EPP File scanning）**：1004 08:23 F-Secure 把剛連結好的 `build_ship\tests\test_cJSON.exe` 誤判成 Trojan.TR/W32.Evo 隔離（accessor `ld.exe`），之後幾分鐘每支在跑的測試都卡約 2 分鐘。重建後再跑；要不要報 IT 由 Jimmy 決定，不要自己改防毒 |
| 原始碼掃描類測試（FShow_Audit、START_SitesCensus…）沒列進代跑，gate 才紅（!165，1004） | 讀表單 fShow 或動 forms/*.h 的 MR，推之前自己先跑這些腳本 |
| 新呼叫加進別人的檔，讓「只編那個檔」的測試目標連結失敗（S-20 FormTryLock） | 先查有哪些目標編那個檔 |

## 6. 大家的好方法（請直接補）

格式與目前的清單在 `references/speedup-ideas.md`：日期｜誰｜做法｜量測（前→後，哪台、哪個目錄）｜注意事項。
沒量過的寫「未量」，不要寫成已證實。

<!-- preserved-content:end -->
