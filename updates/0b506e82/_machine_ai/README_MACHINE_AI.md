# 給機台端 Claude：更新包 4（GitLab main `0b506e82`，相對更新包 3 `db6736c5`）

> 筆電端 Claude 20260926 12:3x 產生。**先套更新包 3（`updates/db6736c5/`），再套這一包。Jimmy 說可以再套。**
> 內容全是 Steven 的：cMyDB CSV 版（P0 sqlite 退役＋P2 AlarmCode 目錄）與 Tester 通訊（GB 戰役）P0 骨架。24 檔，底稿 `base_db6736c5\`。

## 內容

| 類 | 檔 | 對機台的影響 |
|---|---|---|
| cMyDB P0 | `cMyDB.cpp`、`CosFunction.cpp` | sqlite 路徑以 `W906_CMYDB_SQLITE`（預設 0）關掉；`dbReadOnly`／`dbReadWrite` 仍定義（初值 NULL），cObserver 照常判「沒有 DB」。使用者 0923／0926 裁決：Handler.db3／sqlite3 退役、只做 CSV 版 |
| cMyDB P2 | `AlarmCodeCatalog.cpp/.h`、`AlarmCodeUpdater.cpp`、`ship/Error/AlarmCodeList.txt`（`ship/.gitattributes` 標 -text，保持 CRLF）、`tools/gen_alarmcode_catalog.py` | MDB Updater 的 AlarmCode 靜態目錄轉成 C++＋CSV；**不會寫機台的 D:\HT9045\Error**（只在出貨包裡） |
| Tester 通訊 P0 | `TesterComm/*`（SyncMailbox、TesterCommHub、TesterCommThread、TesterEngine.h）、`MessageDef.cpp`（版本字串改 12.13.905.0） | 骨架，**還沒接 wb_serve**，不會開任何執行緒 |
| 建置 | `CMakeLists.txt`（新 target `ht9045_testercomm`、`ht9045_db` 加新檔）、`tests/CMakeLists.txt`、新測試 `test_mydb_csv_alarmcode.cpp`、`test_testercomm_ipc.cpp` | 要重新 configure |
| 文件 | `docs/CMYDB_*`、`docs/TESTERCOMM_PORT_LEDGER.md`、`docs/ELA_20260926_WEB_CONVERSION_PLAN.md` | — |

筆電驗證（合進 main 後、兩組態全量 gate）：出貨 184 項 3 失敗＝基準；模擬 184 項失敗集合＝基準 18 項；`MyDB_CSV_AlarmCode`、`TesterComm_IPC` 兩組態通過；`cMyDB.cpp.obj` 對 `sqlite3_*` 未解析參照 0。

## 步驟

同更新包 3：Check（唯讀）→ EastSun 同意 → 備份 → Apply（LOCAL 三方合併）→ **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`
