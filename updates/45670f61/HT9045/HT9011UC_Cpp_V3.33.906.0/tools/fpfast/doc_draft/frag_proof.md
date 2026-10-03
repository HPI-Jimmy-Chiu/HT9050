**證明結果**（oracle g++ 6.3.0；每一條唯一的編譯指令「加／不加」各編一次，逐位元組比 obj，再用 objdump 逐區段比反組譯與內容；1003 10:1x～12:2x，筆電）：

| 組態 | 比對的編譯指令 | 逐位元組相同 | 只差 `__DATE__`／`__TIME__` | 程式碼不同 | 失敗 |
|---|---:|---:|---:|---:|---:|
| SIM（模擬，Debug `-g1`） | 1104（涵蓋 1593 條 GNU 編譯、932 個原始檔） | 1101 | 3 | **0** | 0 |
| SHIP（出貨，`-DW906_NO_SOFT_SIMULTE=ON`） | 1097（涵蓋 1593 條、932 個原始檔；C++ 1094、C 3） | 1094 | 3 | **0** | 0 |
| SIM `-O3` 抽查 | 24（18 個原始檔） | 24 | 0 | **0** | 0 |

* 只差時間的三支在兩個組態都是 `TempCtrl/TriTemp.cpp`、`cObserver.cpp`、`WebBridgeTags.cpp`（`.rdata` 裡的編譯時間字串）；把日期與時間固定後重編，三支都逐位元組相同（`recheck.py`：RESULT ALL EXPLAINED）。
* 兩邊編譯的回傳碼、stderr、stdout 全部相同；g++ 6.3.0 對 C 與 C++ 都安靜接受這個旗標，沒有新的診斷。
* SHIP 那一輪：反組譯比對 122,499 個區段（11,736,320 行），`objdump -s` 內容比對 256,318 個區段。結果檔：`tools/fpfast/results/proof_SIM.*`、`proof_SHIP.*`、`recheck_SHIP.txt`。
