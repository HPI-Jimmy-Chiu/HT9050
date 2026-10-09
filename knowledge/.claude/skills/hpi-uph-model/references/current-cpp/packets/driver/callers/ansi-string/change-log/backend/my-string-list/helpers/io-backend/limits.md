# 機型、版本、建置與保存範圍

[上層](index.md)；[原文清單](source-manifest.json)。來源以 commit、Git blob 與 byte hash 定位；新 reader 的完整定義及原始 bug／size／ownership banner保留，外層 preprocessor stack為空。5既有 writer／probe正文逐字重核，不算新完成。

| 軸 | 本輪依據與剩餘 |
| --- | --- |
| HT9050／其他 Handler | 兩個 reader 定義沒有 MachineType／客戶分支；只有實際 caller 採用時才適用。未完成所有呼叫端，因此不推定各機台都會讀同種 log。 |
| 客戶 | MyStringList 的 HANA、FT／RT、Cypress、N10、Jam 與2D差異沿用前批，共用 reader 不加入新客戶或班別規則。 |
| 版本 | V906 選定 C++ 來源；V912、客戶 V899 及最新913仍需各自來源。原始 golden／translated banner 保留為歷史，不是新913 RTL、BCB6或實機驗證。 |
| 編譯器 | build.bat 的 MINGW_BIN、mingw lane 明確選 g++／MinGW Makefiles；CMakeLists.txt 的 HT9045_CXX_STANDARD 預設17並允許特定 lane 覆寫。這是建置入口原文，不是本輪執行建置或確認所有部署使用同一編譯器。 |
| CRT／OS | 官方 Windows 契約與 Microsoft CRT 規格分開；未核部署所連結的 msvcrt／UCRT／其他實作，也未核全部 preprocessor、flags、locale／_fmode。不能因 MinGW 使用Windows而自行套用全部 Microsoft CRT行為。 |
| runtime | 沒有建立／讀寫／刪除測試log，未同步／修改system、config或機台，沒有執行任何reader／writer。 |

## 待續

- ReadDataFromFile 的完整 callers、free ownership、NULL／短讀使用、檔案生命週期；CheckFileIsEmpty 的 caller 判斷是否反向使用。
- Win32／CRT實際開檔成功、bytes／sharing、超大檔、pathname與encoding、buffer併發、flush／硬體持久性。靜態未核結果不是已重現故障。
- RS232獨立 writer、Lotfile／upload、HANA form與其餘 caller；時區／DST與clock失敗規格。
- 最新913／V912／客戶V899與機台版本差異、UPH／S8公式及經授權的實機驗證。

父 IO 文只 append 更新路由，保留其原始待續口徑並由本單元補 reader／條件式契約；42歷史 manifest、metadata、裁決正文、資源與相容入口不改寫。僅 Skill references 與交付文件，未修改來源或執行程式。
