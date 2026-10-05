# 派工、ST01-M 文字、交件範本

## 1. ST01-E 派記錄員（Agent 工具）

**規則**：範圍終點寫死 hash；列出這一輪的所有輸入，記錄員不用自己猜。Agent 參數：`model: sonnet`（S53 原話「派一個 sonnet 文書記錄員」）、背景執行。派之前：`git -C /d/HT9045 fetch`；把三份檔各複製一份到 session 暫存資料夾（交件後 diff 用）。

```
記錄員這一輪的工作：照 D:\HT9045\.claude\skills\ops-st01-clerk-report\SKILL.md 與它的 references\ 五份檔，更新三份紀錄：
D:\RD5-Portal\public\Docs\ChangeLog\Steven\CHANGES_YYYYMMDD_Steven.md、D:\HT9045\docs\ops\daily\YYYY-MM-DD.md（repo 日報）、
D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md。只改這三份，不做 git 寫入、不 build、不開子代理。

1. 範圍：<上一輪終點>..<本輪終點>（<N> 顆，<hh:mm>～<hh:mm>）。本輪終點固定是 <本輪終點>；寫的時候 HEAD 往前了也不要自己延伸。
   新節號：§11.<NN>（上一節 §11.<NN-1>）。
2. 推送狀態（ST01-E <hh:mm> fetch 過）：origin/v906/steven-cbridge-review6..HEAD = <n>；HEAD..origin/main = <m>。
3. 工程師：
   - 在跑：<誰／題號：做什麼；產出位置>
   - 本輪交件：<題號：結論一句；產出位置；ST01-E 核對結果>
   - 預定但還沒派：<…>（寫「預定」，不要寫成已派）
4. Steven 本輪的回覆：<原話＋時間＋在哪邊（對話／St02 那邊／經 ST01-M 轉述）＋ RULINGS 編號>；沒有就寫「無」。
5. ST01-M 的項目（原文照貼，寫進 §11.<NN> 最後一節與 §12.2／§12.5）：
   <ST01-M 給的文字>
6. ST01-E 對上一輪的核對更正（在原處用「⛔ 更正（ST01-E <hh:mx> 核對）：」補記；§12 直接照正確的寫）：
   <…>；沒有就寫「無」。
7. 其他交代：<例：這輪起開新檔／某一大批裁決併成一列還是拆列>；沒有就寫「無」。

交件照 references\dispatch-and-handin.md §3 的五項格式。沒有新 commit、沒有交件、沒有 ST01-M 項目時只回「無新進度」。
```

## 2. ST01-M 給 ST01-E 的文字（SendMessage）

**規則**：ST01-M 不直接寫三份檔；自己的交接動作整理成下面的格式，SendMessage 給 ST01-E，由 ST01-E 貼進派工訊息第 5 項。5 小時檢查點（ST01-M 排程 `23 */5 * * *`）就是用這則訊息觸發，附一句「請派記錄員並 push／pull」。

```
[ST01-M → ST01-E] 5 小時檢查點／交接項目 <YYYYMMDD hh:mm～hh:mm>
- handoff 分支 `v906/steven-handoff`：`<hash>` <hh:mm> FROM_STEVEN §<x>（St01 → <Jimmy／St02>）：<內容一句>
- 共用工作樹 D:\HT9045 的 commit：`<hash>` <hh:mm> <檔的絕對路徑>：<改了什麼（例：代登記 todo H-029、decisions W36）>
- 代編：St02 `<hash>`：模擬組態 <build exit／ctest n/n>，出貨組態 <…>
- 要進 §12.5 的：<項目>（已轉達，等 <誰> 回）
- 要進 §12.4 的：<新 W 題：題號＋一句題目＋St02 建議>
```

## 3. 記錄員交件（→ ST01-E）

**規則**：五項都要有；沒有內容寫「無」。§12 原文照貼，不要摘要。

```
記錄員交件：<上一輪終點>..<本輪終點>（<N> 顆，09-27 <hh:mm>～<hh:mm>），新節 §11.<NN>

① 改了哪些檔
- D:\RD5-Portal\public\Docs\ChangeLog\Steven\CHANGES_YYYYMMDD_Steven.md：§11 摘要表第 <NN> 列；新增 §11.<NN>（a～<x>）；§12.1～§12.4 整段重寫；§12.5 新增 <k> 條（開頭字：…）、改標 ⛔ <j> 條（開頭字：…）；⛔ 更正 <i> 處（§… 第 … 行附近）
- D:\docs\ops\daily\20260926.md：檔尾新增「09-27 <HH:MM> 更新」一段；⛔ 更正 <…>
- D:\HT9045\.claude\skills\ht9050-construction\references\rulings-index.md：最後更新；主表改 <列號…>、新增 <列號…>；最近變動加 1 行、刪掉最舊的「<那行開頭的時間>」那行，現在 20 行
② 狀態計數：✅<a>／⏳<b>／📝<c>／❓<d>／➡<e>／—<f>（合計 <T>；上一輪 ✅…；差異：<哪一列從什麼改成什麼>）
③ §12 原文
   12.1：<全文>
   12.2：<全文>
   12.3：<全文>
   12.4：<全文>
   12.5（本輪新增／改標的條目）：<…>
④ 待確認：<寫不準或來源互相矛盾的地方，每條附兩邊的絕對路徑與行號／節號>
⑤ 新教訓：<這輪踩到、建議 ST01-E 補進 references/checklist.md C 段的>
```

## 4. 無新進度

**規則**：`<上一輪終點>..HEAD` 0 顆、派工訊息沒有交件、沒有 ST01-M 項目、沒有 Steven 回覆 ⇒ 三份檔都不動，只回一行：

```
無新進度（<上一輪終點>..HEAD 0 顆；沒有工程師交件、ST01-M 項目、Steven 回覆），三份檔沒動。
```

只有推送狀態變了（例：ST01-E 推了、`HEAD..origin/main` 變了）也算無新進度；下一輪有內容時再一起寫進 §12.1。
