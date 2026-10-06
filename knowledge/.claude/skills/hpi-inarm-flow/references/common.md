# InArm共同介面與機型分界

基準main `372b91908`，20261006。共同的是查證方式與資料介面；狀態機、吸嘴幾何、盤參考與飛梭機構按配置分派。

| 項目 | 可共用的判讀 | 機型／模式分流 |
|---|---|---|
| 入口 | DoInArm guard→DoInArm_9045 dispatcher→特定狀態機 | USE_PICKER_COUNT先選Single Picker，其他由iInArmType；不從函式9045字樣排除9050 |
| 流程方向 | 一般入料由Loader／HotPlate到Shuttle；ASM／AutoClean有特殊取還料 | Hot／常溫／ASM與One Cycle／Clean Out不能混用Task語意 |
| 資料／真空 | Item／HAS_NULL_IC／HAS_TRY_SUCK_IC與GetStatus分開；動作等待與失敗分開 | 8／16／32-site映射、iWhichSht／iWhichKit與單Picker各追實際設定 |
| Teach／Pitch | 先辨基準軸、盤基準與單位，再追偏移／映射 | 多吸嘴E／F基準、AxEx／AxxG／ACEG；HT9050教導A且四嘴合一顆，見機型層 |
| HotPlate | 空位搜尋、落點幾何、帳本booking與吸嘴資料是不同證據 | 2x4／16-site／單Picker不能套同一落點矩陣；歷史鐵律須帶原前提 |
| 成功與互鎖 | caller、回傳、Task與實際輸出分開 | main仍有gate／stub／可編譯但未接入路線；文件保存不等於上機驗證 |

[目前main](runtime/current-main.md)／[HT9050](machines/ht9050.md)／[其他配置](machines/other-models.md)／[流程原文](flow/original-entry.md)／[吸取原文](vacuum/original-entry.md)。
