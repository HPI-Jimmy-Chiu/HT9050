# 目前Web寫者與舊旗標

唯讀基準main `f57d93f15`，HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp，沒有呼叫HTTP／WS或寫入任何檔案。

## allow-system-write的現況

gAllowSystemWrite目前初始化true，main解析--allow-system-write僅接受舊參數、不改選擇。原20260915段落說「必須帶此旗標」保存作歷史，不能宣稱現在仍是一道有效閘。這是目前程式與其既有裁決註解，本批沒有擴大任何操作授權。

## owner仍有作用

system.file.put經FindSysFile／SysFilePath再查CRouteOwner；非dryRun而有owner時回409 owned by C route。kOwned把config.ini／lastset.ini／configByRecipe.ini交FileRW/IniConfig，Gerneral.ini交HSys，teach.ini交Teach；config.ini的Visible另有IniConfig_CounterSel共享寫者。列出或dryRun成功不證明B路apply會通過。

相應C路用editlist.get／editlist.save與tag；editlist.save在SystemStart或SoftStart為真時先拒，之後重查OpenGateRefused的開頁條件。不能只以寫入旗標或對話框曾開過判許可；Other command的guard另核對caller，不把此規則套到所有form.save／動作入口。

Config與Recipe的寫者分工見 [LotInfo／Recipe owner](../../../hpi-lotinfo-recipe/references/runtime/recipe.md)與[原C路規格](../../../ht9045-html-json/references/route-c-golden-bridge.md)。原文的「實際寫入／備份」是前次授權與實作狀態，不要求本次Skill重整執行它。

最新main `02ed3e735`的設定鏡像與E-043 WIP紀錄差異見 [20261006整合補充](main-update-20261006.md)，來源核對與待答分開。
