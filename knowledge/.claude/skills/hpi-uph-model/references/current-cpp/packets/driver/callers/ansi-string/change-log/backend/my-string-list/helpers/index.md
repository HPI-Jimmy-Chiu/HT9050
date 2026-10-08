# MyStringList下層：目錄、檔案與全域時間

承接 [MyStringList writer／filename](../index.md)。本層保存Handler共用目錄與全域時間helper，以及vclcompat的filesystem／path正文；將「函式回傳」「呼叫者是否核結果」與「實際落盤」分開。

| 問題 | 文件 |
| --- | --- |
| MyForceDirectories的路徑分類與回傳碼 | [目錄helper](directory.md) |
| exists／mkdir／remove／path拆解及Windows巨集 | [filesystem](filesystem.md) |
| member、全域、Yesterday與初值 | [時間](time.md) |
| 共用CRT writer、Win32呼叫形狀與查證界線 | [IO界線](io-boundary.md) |
| 原文保存、機型／版本適用性與待續 | [範圍](limits.md) |
| 11完整cpp、12region、11來源與既有context證據 | [保存清單](source-manifest.json) |

## 範圍與同題共通項

本層11新完整cpp加12宣告／歷史／macro region，共23新摘錄；2個WriteDataToFile與2個Decode正文是既有單元的context重核，不重算新完成。完整來源byte、摘錄及body hash記在保存清單；這不是完整common／cpublic／SysUtils翻譯完成。

| 軸 | 讀法 |
| --- | --- |
| HT9050與其他Handler | 選定helper正文沒有MachineType客戶分支；實際caller採用Handler共用函式時才適用。各機台啟用項與輸出路徑仍須查caller及部署，不因無分支而推定都會寫相同檔。 |
| 客戶／班別 | N10昨天folder、SG Jam的日期旗標、HANA／FT RT等差異仍在上層caller；共用helper不新增08:00／20:00班別規則。 |
| 版本 | 本輪為V906 C++17／UTF-8選定來源。V912、V899、最新913與部署版本需另附來源；保留20260626／20260716／20260721／20260728／20261007註記為歷史。 |
| 執行期 | 本輪沒有讀寫、安裝或同步機台設定，沒有建立／刪除log或呼叫writer。 |

僅Skill與交付文件的靜態查證，未建置、執行C++、test、runtime、BCB6 oracle、913比較或實機驗證。
