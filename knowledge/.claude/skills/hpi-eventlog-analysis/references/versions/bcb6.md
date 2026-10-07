# BCB6與外部分析器

原Skill的BCB分析器來源為SVN Rev891／FileVersion 20.25.891.0，TfrmELA及TAnalysisProcessThread的架構、命名與限制見[原正文](../ela/original-entry.md)。此批未打開外部SVN工作副本或exe，完整知識按原來源／日期保留，不能視為目前磁碟最新版或實測。

原外部分析器保持Big5／BCB6／pre-C++11與元件唯讀規則。V912是目前量產修正目標，V899／golden等只讀；本題不修改任何原始碼。main2db的V912 InterfaceSYS::SendCommand_EventLog正文局部查證見[來源](../runtime/source-review.md)，不代表已查啟動看門狗、O10或所有送出端。
