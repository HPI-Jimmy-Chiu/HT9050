# 封包宣告查證界線

本單元完成四來源檔的十三片段保存、四個完整 VM／MV 宣告的欄位對照、UPH=132 的定義／展開與四全域符號宣告；這是局部靜態來源整理，沒有結案完整 UPH 或 transport。

仍待查：

- ABI：`sizeof`／`offsetof`、型別寬度、對齊、packing、HWND、編譯旗標與接收程序版本。選讀 header 沒有 `#pragma pack`，不能推定 include 鏈或編譯環境也沒有。
- VM／MV 的初始化、指標設置與解除、跨執行緒同步、覆寫／截斷／字串終止，以及所有 producer／consumer。
- GPIB driver 宣告與實作、命令 parser、讀取 `MSG_CMD_NAMES`／`MSG_CMD_COUNT` 的 caller，以及實際送達／ACK／錯誤。
- HT9050 與其他 Handler 的版本部署、機型／客戶分派、啟用 site、UPH 計數單位、容量與現場量測／校正。

未執行 C++、build、API、Profiler、GPIB 指令或機台；未寫入 system／config／機台快照。來源原註解保留作歷史，不把舊知識或本次靜態對照稱為實機驗證。

20261008：上一批 !321 的30檔內容已確認進入 main `77887765d`，提交祖先與逐檔位元組核對通過，自己的工作樹已 pull 並重驗。本子樹原先從獨立 main 建立；兩批整合不代表完整 UPH／transport 或實機語意已查完。

原文、metadata、HTML、裁決與相容入口保留；SKILL.md 主體不增加。回 [封包索引](index.md)、[目前 C++ 索引](../index.md)。
