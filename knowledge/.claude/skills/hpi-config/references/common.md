# 共同資料層與差異

| 資料層 | 共通查證方式 | 另分條件 |
|---|---|---|
| config.ini／IniConfig | 欄位→HTEditList綁定→讀寫caller | UI可見／可編輯／固定值／ReadFromFile、客戶與版本 |
| Gerneral.ini／HSys | ReadGeneralIni讀入機型與硬體 | 機台實際配置、GPIB general.ini與Handler Gerneral.ini的不同路徑 |
| configByRecipe.ini | elConfig_byRecipe與作用中Recipe | 分支決定落在全機或配方，不由Caption顏色自行推定全部欄位 |
| teach／IO／Mot tables | Alias、欄位、單位與讀取函式 | 軸／卡／機構、升版與缺鍵寫入副作用 |
| 客戶碼／名稱 | CC常數、DoCustomerFunction與名稱caller | 機型不是客戶、名稱查表版本不同於UI列表版本 |
| 手冊與資料源 | 原欄位MD／YAML／i18n／scripts分工 | 資料源是否在Git、受眾、產出版本與固定舊路徑 |

同一主題涵蓋HT9050與其他Handler，差異看 [機型](machines/index.md)與[客戶](customer/index.md)。Recipe本身的生命週期與寫者見 [LotInfo／Recipe](../../hpi-lotinfo-recipe/SKILL.md)，Index／Tray／Shuttle設定耦合依各共用Skill核對。

本次是文件重整，不新增CC、不補鍵、不呼叫讀取函式、不修改runtime／機台鏡像。歷史案例／風險／裁決完整保留，不直接改稱目前缺陷或已上機通過。
