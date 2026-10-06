# AutoStart／AutoClean共同項與差異

| 共通概念 | 依入口／配置不同的部分 |
|---|---|
| 機台啟動仍須走實際Start與守衛 | OLP ProcessBuffer、7016 HTSET、Web按鍵不是同一條call chain，reply也不同 |
| 清潔先選模式、初始化Task，再由生產流程推進 | 手動、interval、Lot／socket／yield等觸發按旗標分流；不是每個UI或Host都已接同一份本體 |
| OneCycle與AutoClean互相協調 | 目前手動本體保留V912的iOneCycle!=0早退，其他部分沿906；有料先清機、無料過位置／HOME檢查 |
| Clean Pad、Kit、HP2、Fix3、CleanAir有不同資料／機構 | eCKPos、TestIF／TestIF_File、CosFunction、教點與站點配置不能靠HT9050名稱推定 |
| 告警回應與機構停止分別確認 | [Alarm共用入口](../../hpi-alarm/SKILL.md)說明kcode、畫面／實體鍵與Yield分流 |
| HT9050也在共同入口內 | [乾跑／機型](machines/index.md)先確認tick是否被W906_Ht9050DryRunOn接管，再追正常階梯的AutoClean |

舊AutoStart的五階段是協定／外部Agent角色說明，外部GTK重試與Info Mismatch不等於C++全量實作。[目前啟動](runtime/start.md)、[目前清潔](runtime/cleaning.md)只列來源碼已核對的邊界。

若疑似機台設定或工單不一致，依目前snapshot規則先確認來源與同步狀態；本次Skill整理不更改機台參數、不發啟動／清潔指令。
