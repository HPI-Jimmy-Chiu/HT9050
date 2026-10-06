# 客戶資料的權威與用途

| 資料 | 權威／入口 | 用途與界線 |
|---|---|---|
| CC符號、程式碼數值與別名 | [各版本MachineType.h索引](versions/index.md) | V912／V906各自查；不改定義或推論商務身分 |
| UI客戶名稱與Factory來源 | [目前名稱查表](../../hpi-config/references/runtime/customer-name.md) | HSys.cpp::FileRW_HSys_CustomerName、CUSTOMER_CODE、rgCustomerList；UI顯示與RunInfo.Factory的caller仍分開 |
| 報告用代理商、地區、語言、客戶縮寫 | [客戶碼權威表](../../make-report-skill/references/customer-code-table/customer-code-table.instructions.md)／[報告碼管理入口](../../make-report-skill/customer-code/customer-code-manager/SKILL.md) | 這份報告表是唯一權威；不能從CC英文名、國家字樣或程式註解猜代理商與語言 |
| 新客戶或改名的雙向同步 | [Config客戶樹](../../hpi-config/references/customer/index.md) | 原流程保留；本批不新增CC、不執行產生器、不改報告表 |
| 某台真正作用中的客戶／機型 | [機型與runtime](machines.md) | 按當台資料、來源時間及裁決查；Model與CUSTOMER_CODE不互代 |

例如CC_PTI在已核對的V906定義中是957；這不表示每台HT9050都是該客戶。807／808／898名稱路由與舊版UI列表的差異，仍以Config的來源版本與caller界線為準，不把兩張表改稱已同步。

本索引連回原表，沒有另建代理商／地區鏡像。所有舊客戶管理原文、metadata與資源保持在各主題保存樹；原文歷史行號不作當前主定位。
