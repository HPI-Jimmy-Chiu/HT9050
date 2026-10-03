# Ops Daily Worklog（每日工作日誌）

> **20261003 起一律用 RD5 統一格式**（Steven 20261003 定案，全員適用，包含 Steven 的日報和 AI 代寫的日報）。
> 舊的 8 段固定格式（今日目標／變更觸及範圍／關鍵決策／風險與阻塞／經驗留存／明日接續點／收尾檢查）不再使用。
> 舊的 A（8 段固定）／B（敘事）格式網站仍讀得到，舊日報不用改；新寫的一律用統一格式。
>
> **正本**在入口網站 repo（GitLab `honprec/rd/rd5/9050motionview`）：
> `<repo>\.claude\skills\rd5-daily-report\SKILL.md` §2 格式、§3 寫作規則、§3.5 推送前的格式檢查，
> 範本 `<repo>\.claude\skills\rd5-daily-report\references\template.md` §統一格式。
> 本檔照抄正本；兩邊不一致時以正本為準。

## 適用時機
- 每日記錄 HT9045 / GPIB9045 / RS232Standard 等開發工作（一天一份）
- 作為週報、除錯報告的原始素材來源（細節放 ChangeLog 或報告，日報只寫結果、卡點和連結）

## 輸出路徑與命名

| 目錄 | 檔名格式 |
|---|---|
| `<repo>\public\Docs\Daily\<EnglishName>\` | `{YYYYMMDD}.md`（當天附件用 `{YYYYMMDD}_<說明>.md\|html\|pdf`） |

> `<repo>`＝自己本機 RD5 入口網站 repo 的 clone 位置，`<EnglishName>`＝組織表上的英文名（見 SKILL.md「輸出位置設定」）。
> 所有專案（HT9045 / GPIB9045 / RS232Standard）共用同一目錄，不再依專案分目錄。

## 輸出格式
- 純 **Markdown**（`.md`，UTF-8），**不放 Logo**；HTML 版由入口網站的 `node tools/md2html.js <檔>` 產生（選用）

## 統一格式（Steven 20261003 定案）

**所有人一律用這個格式**。範本見 [template.md](../../templates/ops-daily-worklog-template/template.md)（照抄正本 template.md §統一格式）。
一篇控制在**一頁內**（md 約 3 KB；最多 8 KB）。**只能有這四個 `##` 段落**，其他內容放 ChangeLog 或報告；段落裡可以用 `###`。

```markdown
# YYYYMMDD 工作日誌 — <英文名>
客戶／機種：…　版本／分支：…

## 一句話            ← 2 行內：今天的主線與結果
## 今日完成          ← 表格：項目｜客戶｜結果｜狀態｜連結
## 卡點／需要協助    ← 表格：項目｜卡在哪｜從哪天開始｜需要誰；沒有就寫「無」
## 明天接續          ← 最多 3 項
```

- 推翻前一天的結論時，在標題下第一段寫「**更正**：…」，不要只在內文裡改。
- 要記很多 AI session 的人（例如 Steven）：每個 session 的細節放 ChangeLog 或交接檔，日報只寫結果、卡點和連結。

**頁面摘要怎麼取**：入口網站取 `## 一句話` 那段（最多 180 字）。所以一定要有「一句話」，首頁與清單才有可讀的摘要。

## 蒐集當天內容（AI 代寫時）

依序找：使用者口述 → 當天 git log / diff（`git log --since=midnight --author=<名字>`）→
當天 ChangeLog → 對話紀錄。只寫**實際做了、實際驗證過**的事；沒驗證的照下面五種狀態寫（待上機／待客戶…），不要籠統寫「待確認」。

**AI 代寫時**：推送前的檢查只看格式。內容要本人看過：自己就能確認的「待確認」先確認掉，AI 寫錯的地方改掉，再推。

## 寫作規則
- **寫結果，不寫過程**：不放 Claude 視窗或對話經過（幾點問了什麼、AI 怎麼答）。
- **細節放報告**：根因推理、程式行號、測試數據放異常修正紀錄表、提案表或 ChangeLog，日報只寫一句結論加連結。
- **狀態只用五種**：完成／待上機／待客戶／待裁決／進行中（另可用「取消」）。
- **延後的事要標天數**：同一項從第二天起寫「（第 N 天）」，主管才看得出拖了多久。
- **卡點一定寫「從哪天開始、需要誰」**：這是主管最需要的欄位。
- 跨人的描述**寫名字不寫代名詞**（Steven / Jimmy，不寫你我他）。
- 具體勝過空泛：寫 MR 編號、commit hash、數字、測試結果。
- **當天下班前寫好**，最晚隔天上午；不要事後一次補好幾週。真的要補寫，在「一句話」註明「補寫」。
- ⚠ 入口網站**內網免登入**：不寫帳號密碼、token、個資、客戶聯絡人的 email 或電話、客戶機密數值；客戶名稱可用代碼。
  不小心貼出 token 要當下撤銷，這件事不用寫進日報。

## 推送前自我檢查（Steven 20261003：「不用阻擋同事的報告」）

推送前在入口網站 clone 跑一次：

```
py tools/check_daily.py --changed      # 這個分支相對 origin/main 改到的日報（commit 之後跑）
py tools/check_daily.py <檔>           # 指定的檔（還沒 commit 也能跑）
```

規則只有一份：入口網站 repo 的 `tools/check_daily.py`。只檢查日期 ≥ 20261003 的 `public/Docs/Daily/<英文名>/YYYYMMDD.md`，舊日報不受影響。
這是**自我檢查**：列出的項目照本檔改好再推。入口網站那邊的處理方式：

| 時機 | 不合格時 |
|---|---|
| Claude 在入口網站 repo 執行 `git push` 前（repo 的 `.claude/settings.json` hook） | 推送**暫停一次**，錯誤清單交給 Claude → 照 rd5-daily-report 調整（事實保留、細節移 ChangeLog）→ 重新 commit → 再推。同一份內容再推一次就放行，不會卡住 |
| 不經 Claude 的推送（`git config core.hooksPath tools/githooks`，每個 clone 設一次） | 只列出不合格項目，照常推送 |
| GitLab MR（check-portal） | 只在 job log 列出，MR 照常自動合併 |

檢查項目：
- 第一行 `# YYYYMMDD 工作日誌 — <英文名>`，日期跟檔名一致
- 只有四個 `##` 段落，名稱與順序：一句話 → 今日完成 → 卡點／需要協助 → 明天接續
- 「一句話」不空、180 字以內
- 「今日完成」是表格、有「狀態」欄，每列狀態是 完成／待上機／待客戶／待裁決／進行中／取消 之一
- 「卡點／需要協助」是有「從哪天開始」「需要誰」兩欄的表格（每列兩欄都要填），或只寫「無」
- 「明天接續」最多 3 項
- 檔案不超過 8 KB；沒有「視窗經過」「對話經過」這類段落；沒有 token（`glpat-`、`Bearer`）或 `password=` 這類字樣

## 放上入口網站

照 SKILL.md「寫完之後」與正本 rd5-daily-report §4：從最新 `origin/main` 開分支、只 `git add` 自己的
`public/Docs/Daily/<EnglishName>/`、推送並開 MR。`node tools/md2html.js` 只轉自己的檔（不帶參數會轉整個網站）。
**節奏：一天推一次**（Steven 20260929）——晚上推當天的，或隔天上班推前一天的。

## 模板
`../../templates/ops-daily-worklog-template/template.md`
