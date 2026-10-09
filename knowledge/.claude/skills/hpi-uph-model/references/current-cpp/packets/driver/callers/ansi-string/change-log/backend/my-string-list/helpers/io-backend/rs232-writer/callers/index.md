# RS232 記錄 caller 與持久化邊界

[上層 writer](../index.md)；本輪新增 21 完整 C++ function、2 region，共 23 摘錄，
讀 7 份來源檔；既有 `rs232std::WriteDataToFile` 原文不重算新完成。
來源釘在 `7f1e937f2dcecba0c1937dea3731d90132f22803`，限定 V906 移植樹的 tester-side RS232Standard。

| 要查的問題 | 入口 |
|---|---|
| ShowCommData、先檢查再加入、append 與失敗清空 | [通訊緩衝](buffering.md) |
| Bin log、Stop／Teardown／析構與路徑 | [關閉與檔案分流](lifecycle.md) |
| 同名 writer、全樹字面普查、版本／機型與未查項 | [證據及適用範圍](evidence.md) |
| 選定正文及 hash | [來源清單](source-manifest.json) |
| 2049 份來源檔的 blob／hash 及 WriteDataToFile 字面命中 | [普查資料](writer-census.json) |

通訊文字→RS232 專用 TMyStringList→MySaveToFile，以及 Bin memo→SaveBinData
是本輪已定位的兩條保存路徑。不要將它們接到同名裸資料 Win32 writer，
也不要把可見的 Memo 資料、文字格式或 CMake 收錄視為磁碟保存、實際 link 或機台驗證。
上游各模式／checkbox 的完整呼叫圖、upload 與部署行為仍待查。
