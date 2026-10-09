# 固定來源、保存與更正

[上層](index.md)；[manifest](source-manifest.json)；[有限census](symbol-census.json)。
固定來源`8cd0be50e14294b42774510c7fa8accc51b511d6`的WebBridgeServer.cpp／.h；活文件以version/file、function與變數定位，offset只供此pin保存核對。
10完整CPP／226函式原文行；13 reader頁含三context，NowMs原文只作context不計第11函式，常數與atomics不增加完整body數。
先前ReceiveInto dispatch尾段保持原樣；此單元新增完整body，與60manifest去重，先前24文件Git blob不改。
原header320行、Conn與decoder／encoder／handshake仍沿用既有保存頁與metadata／資源，不複製另一套Skill。

## Intake更正記錄

前intake註記曾寫：CloseAllSockets「closes ... listener/wake」且「does not itself reset ... liveWs_」。
完整正文實際為conns_.clear()後liveWs_.store(0)，關listener；沒有wake_清理或ctrlOwner_ reset。
原錯誤註記保留於manifest的intake_correction.superseded、OPS intake的superseded_findings，明確標示已被更正；它不是程式原文或人工裁決。
此更正不改C++，Start／Stop／Wake的thread／socket收尾仍待獨立查讀。

## 適用範圍

兩檔comment／string遮罩後spellings只是有限詞法census，不是全語意call graph、全caller或thread safety證明。
本10 body沒有MachineTypeChoice／customer分流；HT9045／9046／HT9050共享來源證據，不推定每台部署相同路徑或runtime參數。
cfg的maxConnections、pollIntervalMs、maxSendBacklog與timeout只核來源／header契約；部署值、Winsock／Sync平台、browser與機台現場另查。
V912 BCB6／Big5與golden913不是這份V906 UTF-8 C++17通訊原文；歷史HTML UPH模型亦不因本傳輸單元完成而變成容量／實機驗證。
尚餘Start／Stop／Wake／platform、PumpSnapshot／PumpOutgoing、命令／ACK／browser／crypto、UPH容量／版本客戶與S8／846、相容入口退役核對。
本次只整理Skill／文件、逐hash核原文／引用；沒有build、執行程式、socket／runtime／機台測試、寄信或改共用checkout。
