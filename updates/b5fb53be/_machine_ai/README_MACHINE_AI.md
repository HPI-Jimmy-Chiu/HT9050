# 給機台端 Claude：更新包 5（GitLab main `b5fb53be`，相對更新包 4 `0b506e82`）

> 筆電端 Claude 20260926 13:5x 產生。**先套更新包 3（`updates/db6736c5/`）、4（`updates/0b506e82/`），再套這一包。要不要套由 Jimmy 決定**（Jimmy 正在機台上驗證，推上 GitHub 不代表要馬上 pull）。
> 94 檔，底稿 `base_0b506e82\`。**跟前幾包一樣排除 OBJROOT 那 4 個檔**（`build.bat`、`.vscode/launch.json`、`tools/pe_truncation_check.ps1`、`tools/webprobe/f5_contract_probe.cjs`）——機台照舊用自己的 build_integ_* 目錄。

## 這一包是什麼

**你們（EastSun 機台）的修改已經合進公司 main 了**（`20494aea`：GitHub `machine/integ-ioweb` 6cb99ec 的 C++ 0001～0013＋web 0001～0007，**TEMP-DOORS 0014 不含**）。
所以這 94 檔裡大部分就是你們自己的檔，Check 應該會顯示「LOCAL 已相同」。真正新的只有下面這些：

| 類 | 檔 | 對機台的影響 |
|---|---|---|
| 警報框一次只跳一個 | `HAlarm.cpp`、`halarm.h`、`tools/wb_serve.cpp`（:627、:693 同行附加）、`tests/test_halarm.cpp` | 照 golden `TfNote::FormClose` 的 `Alarm->Clear()`（note.cpp:2531）：答完一個警報框就清掉同一次排隊的其他碼。以前兩個氣缸同時逾時會跳兩個框、停兩次機 |
| 權杖 | `web/page/HW.MotorTest.html:1610`、`web/page/ht9045_recipe_client.js:457`、`tools/webprobe/token_idle_selftest.cjs` | Light Scale 進行中也不還權杖（跟你們 :1091 keepAlive 的 busy 一致）；你們的 `keepAlive()` 重拿權杖後補掛閒置計時（以前工作結束後永遠不還） |
| 註解 | `csystem.cpp`（G03 前 4 行） | 你們的註解寫「G03 在 HT9050 維持關閉」，但你們合 56bbf785 時收下了 `#if 1`（golden）——程式是開的。main 保留 golden 的開，註解改成說明這件事（NB2 R66 §3 量到判斷式只在第一次 Index 歸零前為真） |
| 測試 | `tests/CMakeLists.txt` 檔尾 | 5 個 HT9050 IO 測試在「舊表」（`machines/HT9050/IO_Table.csv` 1063 行）時設 Disabled。**你們的現場表不是 1063 行，所以在機台上照常跑** |
| 編譯 | `WebBridge/Sync.h:151` | `::Sleep` 參數轉 DWORD，行為不變 |
| 文件 | INBOX、TO_STEVEN、CHAT_JIMMY 等 | — |

## ⚠ 請機台端做的（Jimmy 同意後）

1. **把你們的 `machines/HT9050/IO_Table.csv`（IOWEB-P7 的現場表，1124 列、ISABase 3 的列 Lane 1）推到 GitHub**（`machine/integ-ioweb` 或新分支都可以）。
   理由：你們的 patch 只涵蓋 `HT9011UC_Cpp_V3.33.906.0/` 與 `web/` 兩個子樹，`machines/` 在外面，所以這張表沒跟著合進 main；
   main 上還是舊表（1062 列、Lane 0），你們改成對照現場表的 5 個測試在筆電兩組態都紅，筆電只好暫時 Disabled（INBOX 第 50 列）。
   `Mot_Table.csv`、`Pci1203Io.ini` 若也改過，一起推。**不要推 `system\`、`config\` 的正本**（那是執行期目錄）。
2. **TEMP-DOORS（暫時關的 8 個安全門）只在你們的樹上**，main 沒有；上線前在你們那邊 revert（INBOX 第 47 列）。
3. NB2 R66／R67 的時序提醒（給第 2、6 條）：
   * 第 6 條（INDEX_MOTION_CARD 當成 1）必須在 `HSys.LoadMotData()`（`cinitial.cpp:3874`）**之前**生效；否則 `TMOTDATA` 已把 Z1 改寫成 `"SMC"`、板號 -1，Index 四軸會被建成停用的 SMC 軸。
   * `rs232.cpp:950` 扭力判準看 `MOT[MTestZ1].CardType=="PCI1203"`，同樣依賴上面那個時序；不然扭力會回到 RS232，而那個埠開機時已經不開，逾時後回 2。
   * **R66-GALI（待 Jimmy／EastSun 決定）**：golden Index 流程 105 個 `MOT[MTestZ1].Gali_*` 活呼叫點直接走 Galil 層、不轉交馬達物件，第 2 條「甲」只接 `TMyEtherCatMotor` 的話，Z1 的生產移動到不了 1203。NB2 建議在 `Gali_*` 層依 CardType 分流。
   * R63 A 不夠：`ht9045_motor` 沒有 `HAVE_PCI1203`，第 6 條生效後 Z1 的伺服燈寫死 false，要從 1203 監看器取。

## 步驟

同更新包 3、4：Check（唯讀）→ EastSun 同意 → 備份 → Apply（LOCAL 三方合併）→ **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證（main `04a2c66f`，兩組態全量 gate）：出貨 187 項，失敗＝基準 3（config_db、config_loaders、GA1_ReadGeneralIni）＋5 個 Disabled；
模擬 187 項，失敗＝基準 18 項＋5 個 Disabled；test_halarm [I]（真的跑 ProcessAlarm）修後 1 個框、對照組 2 個框；
基準內失敗測試的子檢查與上一版逐行相同；system\ config\ 586 檔 0 變動。
（兩件環境雜訊：`IniFiles_Win32Diff` 的 exe 被防毒隔離，重新連結後通過；`WebMotorAccess` 模擬組態 -j 8 下逾時一次，單獨 0.82 s 通過 —— 你們若也遇到請記下來。）
