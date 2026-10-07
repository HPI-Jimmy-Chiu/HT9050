# 共同建置證據與路由

本頁整理兩支原Skill的共同工作順序，不宣稱BCB6與V906使用同一個build命令。先核對repo／commit、來源樹、工作樹和工具鏈，再讀[版本索引](versions/index.md)；Handler型號不能代替版本或編譯器選擇。

BCB6來源遵守Big5／pre-C++11／VCL及.bpr；V906移植來源UTF-8，CMake的預設HT9045_CXX_STANDARD為17，WinLibs專線有14的設定，按實際cache／旗標記錄。量產修正目標V912；V899與golden等歷史樹只讀，不能因建置範例就取得修改它們的授權。

日常增量或單一目標適合縮短回饋；交付證據另記兩組態／受影響目標、ctest、caller與link／PE結果。沒有實際跑就寫未執行，不拿原量測當這台通過，不把編過單一cpp等同完整模組／實機完成。V906完整標準見[原cpp_build正文](cpp/original-entry.md)。

整理文件僅檢查保存、引用、metadata與工具byte，不安裝工具、跑build／ctest或建立／修改機台runtime。未來真正測試依專案機台同步、備份／真檔check與還原規則；建置出貨、程式MR與本對話Skill文件的合併授權各自適用，不借Skill交付授權合併機台程式。

BCB原文有廣域Stop-Process清理範例，完整保留在[歷史清理文件](bcb/references/powershell-cleanup.md)；本入口不把它提升為可直接關閉其他session的流程。三支BCB工具仍在舊scripts位置逐byte保存，完整原命令與相對路徑見[BCB原文](bcb/original-entry.md)。
