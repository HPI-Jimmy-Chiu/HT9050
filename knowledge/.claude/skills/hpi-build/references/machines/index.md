# HT9050與其他Handler共同／差異

| 範圍 | 選路與界線 |
|---|---|
| HT9050機台或開發機 | 先讀版本與實際toolchain／cache；WinLibs機台數字、nonoracle／C++14配置與oracle gate分開，不由HT9050名字判定這台工具已裝 |
| HT9045／HT9046等BCB6量產機 | .bpr／Big5／VCL工具鏈；型號、站點、driver/import lib與共享輸出路徑另核對，不能用V906產物覆蓋BCB6 EXE |
| 其他V906機型／開發工作樹 | 與HT9050共用來源入口與sim／ship區分，但runtime裝配、Model、CC、IO／軸配置和測試適用性另確認 |

共同的是建置證據流程，不是相同硬體、工具鏈或測試基準。runtime快照與[source](../runtime/current-source.md)都要記版本；未知配置保留缺口，機台sync／備份／還原只在真正測試時依專案流程執行，本批不操作。
