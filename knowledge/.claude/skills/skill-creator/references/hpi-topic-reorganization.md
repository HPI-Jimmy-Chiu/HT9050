# HPI 主題 Skill 整理規範

依據：[ST01 整理提案 20261005，含 13:2x 更正](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/v906/steven-handoff/docs/handoff/ST01_AGENT_SKILL_REORG_PROPOSAL_20261005.md) §2～§4。
Steven 20261006 指定 ST-GPT 接手；S1／S2 供 Steven／Jimmy 審查，後續批次依 Steven 最新指示持續整理並分批推送。
同日補充：主體最多 300 行、其餘使用 references；大型主題採樹狀分層；同一主題整合 HT9050 與其他機型的共同項／差異；不因 S2 尚未審查而停止。

## 1. 入口與四種內容

一主題一支 `.claude/skills/hpi-<topic>/`，同時管理 HT9050 與其他機型的共同項／差異；不按機型建立重複 Skill。機型名放 reference。正本仍只在 `.claude/skills/`。

| 位置 | 內容 | 分界 |
|---|---|---|
| `SKILL.md` | 這是什麼、讀檔路由、必記安全條件 | 最多 300 行，盡量更短；保留 name／description 與觸發詞；細節按需讀 |
| `references/common.md` | 經程式核對、不依機型分支的行為 | 有 MachineTypeChoice／機型旗標分流，只留分流指標 |
| `references/ht9045.md`、`ht9050.md`、`ht9046-ls.md` 等 | 該機型差異、流程、互鎖與驗證界線 | 檔頭差異表；標題順序固定為差異、入口與流程、安全、查證來源 |
| `references/customers.md` | 真正由客戶碼分流的功能 | 不把機型專屬機構或一般設定開關當成客戶功能 |

沒有差異才在路由表寫「同通用」而不開檔；未核對寫「未核對，先讀實際分派」，不能把缺文件當成沒差異。
小型主題可用四份 reference；大型主題依 common／流程／協定／硬體／機型／客戶等用途分層，每層以短索引直達下一層，原文長篇再按章節拆葉節點。
跨機台 API／驅動器／協定手冊按主題放 `references/protocols/`、`hardware/` 等分支；機型細節放 `references/machines/ht9050/` 或 `references/ht9050/`。保留既有可用路徑，不為目錄形式搬第二次。

## 2. common 與版本判準

從執行入口追到函式／Task 的機型判斷，確認同一路徑再抽 common。名稱、Task 數字與停位名相同不代表機構相同。
V906 移植樹為 C++17／UTF-8；V912 為 BCB6／Big5；golden 906 與 V899 只讀。
保留舊文件來源版本、案例與推論界線；不可把 V897／V904 的描述默默改稱 V906／V912 已驗證。
活文件引用「版本／來源樹＋檔名＋function 名稱＋關鍵變數」，流程需要時再加 Task/case 或條件；不以程式碼行數作主要參照（Steven 20261006 最新補充）。
歷史搬移件可保留原行號，但明標歷史定位，新摘要與查證來源改用 function／變數。

## 3. 客戶表

固定六欄：`客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台`。
函式層級合併相同分支，保留確實不同的 Task／條件。只查 V906 就寫「906 已核對；912 未核對」，不推論為兩版相同或 912-only。
本批手工核對樣板列；全樹 customer_scan.py 與跨主題索引屬 S8，另依認領實作。日後腳本產骨架不覆寫人工說明，另列孤兒／待補清單。

## 4. 搬移與相容路徑

先核對最新 main 與 origin/v906/steven-handoff；在 FROM_STEVEN §1 登記確切檔及區段、先推再做，使用乾淨獨立工作樹。
只動認領主題；他人現況板、通訊三組與 night-loop 不在飛梭樣板範圍。
盤點原 SKILL 正文與每份 reference，建立舊→新表。搬移保留內容、來源及安全規則，只因位置改變修正真正的 Markdown 連結。fenced／inline code 內的協定字串、Markdown 範例與路徑逐字保存，不當作活連結重寫。
舊 frontmatter 可能有 Claude 的長 description／applyTo 等既有欄位；保留觸發詞並明列 Codex 驗證例外，新 canonical 入口則須通過驗證。
無法重新核實的知識保存在明標版本的歷史文件，由機型 reference 路由；不能擅自刪掉或提升為 common。
舊 SKILL 保留原 frontmatter 觸發詞，正文改短指標；仍有引用的舊 reference 路徑保留短指標並直達新正文。
新路由是活入口，原文保存件供查證而不形成第二套活規則。
提案「保留一週後刪」仍需審查與引用盤點；本批不刪空殼。由整合者確認已部署、callers 已更新、已通知原擁有者，再在後續批次撤除。

## 5. 驗證與交付

檢查 frontmatter／命名、UTF-8／控制字元、whitespace、每個舊正文保存、相對連結、來源函式／Task 與認領範圍。
純文件不為流程而編譯或啟動機台；回報區分文件檢查與機台驗證。
每批推送前重新 fetch main，檢查相關 Skill、規格、裁決及來源碼，整合後重新驗證，使用短分支＋Draft MR；不直接推 main、不 force push。
Steven 最新指示已允許續做 S3 與後續批次，不以 S2 待審為開工阻塞；MR 審查與合併仍由 Steven／Jimmy 裁決。他人正在認領的相同區段需避開，缺資料只暫停依賴該資料的部分，其他可獨立主題繼續。
溫控與 EP 壓力分開為不同主題，但每個主題內整合所有機型；跨主題功能互相路由，不因過去提案同組就誤稱 EP 是溫控器。
每批交付文件與 ST-GPT 正式交接補 Change Log、日報素材；每小時回報實際 Git／MR／目前批次與下一步，不把 Draft 推送寫成已合併。
完成後在 FROM_STEVEN §2 寫 hash、MR、驗證與 human-review 分類。
