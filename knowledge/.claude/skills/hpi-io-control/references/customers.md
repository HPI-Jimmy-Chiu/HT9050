# IO客戶條件索引

| 客戶／配置 | 函式／變數 | 差異 | 驗證界線 |
|---|---|---|---|
| CC_Greatek | fiosetview Close按鈕／ShowModal | 原VCL IO頁的Close按鈕條件 | 保存原案例；未重驗V912表單，不套HTTP頁 |
| ReeR／Schneider安全PLC | ePLCbase／Enable_PLCSafety_IO／SnSafeMode | 匯總EMG／Door是否有實體點與表格停用前提不同 | 按原裁決／表格與實體配置查，本批不修改安全PLC或IO表 |
| HT9050配置 | W906_GpibModel／MachineTypeChoice／IN_ARM_VC4_GROUP | 型號強制In／Out群組，Index仍獨立 | V906本批只讀核對；不是客戶碼全量盤點 |

其他客戶code在InitialSafeDoor、IO GUI、氣缸alarm與退出guard的差異未做全量掃描，保留原文與檢查入口。
