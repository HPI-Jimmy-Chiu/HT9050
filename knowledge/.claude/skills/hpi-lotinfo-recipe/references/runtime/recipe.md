# 目前main的Recipe讀寫邊界

唯讀基準main `f57d93f15`，檔案在HT9011UC_Cpp_V3.33.906.0。本次未讀機台密碼資料、不下載／修改工作檔、不執行API或Handler。

## 可列出與可寫不同

tools/wb_serve.cpp::RecipeRoute只接受GET／HEAD，EnumRecipeDocs枚舉作用中Recipe的文件；寫入走recipe.doc.put dispatch。它以document tag找實檔，僅使用sections/<key>/raw字串；傳输的JSON UTF-8不代表.Data應改UTF-8。

CRouteOwner依文件找golden Form bridge擁有者；非dryRun寫入時如已有owner，回409 owned by C route。包括configByRecipe、Contact、Temperature、ArmCondition、Binasgn等已接C路文件；應走其FileRW／editlist／form保存入口。原文「每個.Data／.ini都可讀可寫」是早期B路能力描述，不能超越目前owner gate。

## ApplyIniGuarded／notFound

ApplyIniGuarded先拒rawValue內CR／LF，先做kRecipeWriteDryRun計數。dry、!ok或changed==0提前返回，不落盤；有changed且notFound>0時拒整批並回ok=false，其他才進kRecipeWriteApply。所以changed==0且有notFound時不能只看ok，仍要按ack的notFound處置；沒有寫入不代表所有欄位都匹配。

保存後重讀與golden記憶體的owner分工另查原C路規格，Skill本批只路由，沒有改owner表或API。 [原路由規格](../../../ht9045-html-json/references/route-c-golden-bridge.md)與客戶／欄位相容資料保留既有權威。

## TestMode的資料含義

cprod.cpp::ReadTestMode先判CosFunction.bLastSetInSetUpFile，false時只從LastSet.bUseTestSocket複製iCloseSiteMap並返回；其後才GetRecipeFileName(TestMode.Data)讀Tester Connection、Temperature Mode、Running Mode。TestMode.iTestConnection預設ON_LINE，不能直接把原欄位表的DIO／GPIB值域當現在定義；回答需跟下游ModifyTester／enum核對。

機台快照／工單與.Data欄位名是不同層，[機型](../machines/index.md)先確認作用中文件來源，不以開發機預設資料斷言機台設定有錯。
