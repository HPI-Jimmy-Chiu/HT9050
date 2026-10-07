# Kernel 版本差異與未閉合部分

本層來源與保存範圍見 [入口](index.md)／[manifest](source-manifest.json)。DoSystemMessage兩版body相同，只證明所選排程文字與local輪轉相同，不代表刷新來源、UI、安全或實際機台行為相同。

| 所選ShowRunLabel附近差異 | V906 | V912 | 界線 |
| --- | --- | --- | --- |
| 函式進入計數 | 開頭增加W906_ShowRunLabel_Count | 未見此句 | 是本處差異，不是UPH計數或pause次數 |
| Alarm早退 | MyMessageBox／fNote用W906_FormShowing | 直接讀fShow，另含fSecsAlarm && Visible | helper／頁面表／所有Alarm流程未讀完，不能判等價 |
| EMG說明中的PLC判斷 | Enable_PLCSafety_IO | IsSafePLCIOInstall() | 所選條件不一樣，未驗證安裝／runtime／IO |
| PAUSE後FTP nearby條件 | fFTPClient->bShow判斷被#if 0閘住 | 保留該判斷 | 此段在所選UPH賦值之後；不據此推定所有FTP或UPH等價 |

2298個追蹤cpp／h的限定盤點是ShowRunLabel(字面形狀，排除.svn／docs／tests並mask註解／字串。13個原文命中檔只留下四個形狀（兩版各定義與DoSystemMessage內呼叫）；清單SHA與guard見manifest。這不涵蓋巨集／別名／跨root，也不是所有UPH writer、完整caller或可達性已驗證。

四個已讀區段僅涵蓋兩版入口／早退，以及暫停開始與附近先行分支；兩個SystemStart結構僅保存已讀接合。ShowRunLabel完整UI body仍未讀完。所有caller／callee、旗標生命週期／同步、上層tick與FlushFlag更新、MainProc串接、CSV／DB／SECS／UI consumer與落盤、容量／site／校正及作用中客戶／機型仍待查。

與 [狀態子樹](../index.md)、[writer子樹](../../writers/index.md) 共同補同一 [UPH Skill](../../../../SKILL.md)；先前原稿／metadata／裁決／資源／相容入口保留。沒有執行C++、build、API／LIVE、UI、Home、Profiler、磁碟writer、機台或runtime。
