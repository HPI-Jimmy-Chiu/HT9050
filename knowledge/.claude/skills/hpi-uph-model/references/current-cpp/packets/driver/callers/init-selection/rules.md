# 連線規則hook與callback

定位HandlerTesterConnect.cpp的W906_TesterConnectRulesInstall及五個W906_Ctc*函式；manifest保留完整原文，包括歷史版本註解。這裡只描述讀到的body，不重新核准任何機台行為。

| callback | 已讀條件與效果 |
| --- | --- |
| W906_CtcD1Access／bRemote | AccessLevel達LevelSet.AccessLevel[8]、bRemote為true，或I40啟用且LastSet為OFF_LINE、再滿足工程師等級／one-cycle條件，返回對應布林 |
| W906_CtcD2IcRefuse／SOFT_SIMULTE | 沒有SOFT_SIMULTE時，one-cycle旗標false且HasICUnderMachine或HasAnyICInMachine成立就返回true；只有Mode==10且Msg==true才另顯示MES1646；有宏時不做這組IC檢查，返回false |
| W906_CtcD3ManualSort／I27 | I27啟用且尚非ManualSort時，設bRunManualSortMode=true、記MES2156並返回true；其餘false |
| W906_CtcD6To2DSort | 清ManualSort，ModifyTester(_2D_SORT)、記MES2155；bOffLineBin成立才ReadParam／ReadFile |
| W906_CtcD7AsmOnLine／SingleSite | SingleSite依iAutoSiteMapRunStartMode==0分別設ContinuStart或ContinuRetest，更新UI文字並記RecordProcess；其他mode依VTESTFunction及bAutoSiteMappingOpenSite決定是否SetMainRunStartMode(rsmAutoSiteMap) |

RulesInstall每次指派五個Hook，不在本body呼叫callback。沒有選读ChangeTesterConnect的完整caller、null seat／事件路徑、權限全系統或原始V912body，不能把上述局部條件當全機台切換安全性結案。

D7的來源註解標示V912追蹤記錄沿用／golden差異；本單元保留註解，不把它當成這次已重新查證的裁決或實機證據。SOFT_SIMULTE的IC拒絕分支與 [NI／Sim driver選擇](../../lifecycle/selection.md) 是不同條件。

回 [本層索引](index.md)、[前置](preinit.md)、[界線](limits.md)。
