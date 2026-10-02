---
name: ht9050-st01-evaluations
description: >
  St01（Steven01，資料讀寫）工程線的「評估案」：Steven 還沒決定、做之前要先查清楚範圍、歸屬、風險與工作量的題目，
  由 ST01-E 派工程師唯讀查證後寫成一份 reference，每份在
  D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md 對應一題 Q 等 Steven 決定。
  20260927 夜間第一批四份：在 Steven01 用沙盒跑 wb_serve 把沒跑過的網頁探針跑掉（wbserve-sandbox-run.md）、
  設定頁「開窗那一刻」記 Enter／重查開頁權限（window-open-edge.md，R108 延伸）、網頁版 7 個要密碼的設定怎麼做
  （q45-web-password.md，Q45 實作設計＋10 個子題）、產生器分頁表與開頁分頁照 golden（page-control-tabs.md，R105／R106 細化）。
  Use when：Steven 說「做評估案」「先評估」「寫成待決策」、要新增一份評估、回頭查某題為什麼這樣建議、Steven 回了評估題之後要開工、
  要知道沙盒跑 wb_serve 會寫哪些真檔、開頁時機（editlist.get 在開站就發生）、網頁密碼確認設計、TPageControl 分頁值。
  關鍵字：評估案, 評估, 待決策, decisions-pending, Q49, Q50, Q51, 沙盒, sandbox, wb_serve 真檔, 探針沒跑, webprobe,
  開窗時機, Enter 事件, 視窗總表, window registry, 開頁閘, 重新登入, DoPassword, 廠商密碼, SG_PW, 一次性亂數, HMAC,
  分頁, TPageControl, ActivePage, ELSetPageOrder, 產生器, gen_editlist。
---

# St01 評估案

> 讀者：Steven（題目與結論）、St01 工程師（開工前的查證底稿）。寫法照 Steven 20260927 21:xx：「給我的決策文件裡面，相關的檔案都要使用絕對路徑，不要使用代號；功能也是要白話說明，淺顯易懂的方式」。

## 1. 目前的評估案

| 檔案 | 在評估什麼（白話） | 對應題號 | 狀態 |
|---|---|---|---|
| [references/wbserve-sandbox-run.md](references/wbserve-sandbox-run.md) | 在 Steven01 這台跑網頁伺服器（wb_serve）又不改到機台真檔的做法，好把今天寫好沒跑的網頁探針跑掉（建議：整夾備份、跑完逐檔還原，分四輪） | Q50 | 等 Steven |
| [references/window-open-edge.md](references/window-open-edge.md) | 網頁的設定頁在開站時就在背景載好，所以「開窗記 Enter」「開窗查權限」都發生在開站；要不要改成操作員真的按開窗那一刻（建議：St01 先改成看視窗總表的「打開」那一下，另請 Jimmy 把網頁引擎改成開窗才要資料） | Q49 | 等 Steven |
| [references/q45-web-password.md](references/q45-web-password.md) | golden 7 個「要密碼才能改」的設定，網頁版怎麼問、密碼怎麼送、日誌不留密碼；含 10 個子題（建議：重新登入的 3 點現在做，其餘 4 點維持改不了）；§10 實作紀錄（20260930，B5） | Q45（子題 Q45-1～Q45-10） | Steven 已裁（按建議）；甲 3 點已實作（C++ `699dc06d`，頁面待 commit） |
| [references/page-control-tabs.md](references/page-control-tabs.md) | 設定頁的分頁標籤：開頁停在哪一頁要不要照 golden、產生器要不要替每個分頁控制列出頁數、點分頁時的程式翻哪幾支 | Q51（子題 Q51-1～Q51-5，R105／R106 細化） | 等 Steven |
| [references/page-state-array.md](references/page-state-array.md) | C++ 用一張表記住所有網頁畫面（90 列）現在開還是關，跟網頁雙向同步；BCB 判斷畫面開沒開的 fShow／Showing／Visible 全部改讀這張表（Steven 20260928 Q51＋「fShow 改用這個陣列，要先做好」） | Q51 已裁決；設計裡的 Q-P1～Q-P3 問 Steven | 設計完成，等 Q-P1～P3 再實作 |
| [references/stream-by-open-page.md](references/stream-by-open-page.md) | 只替「開著的畫面」送資料：C++ 依頁面表跳過關著的頁的 tag 整理與串流（今天每拍整理約 7,100～8,100 個 tag，其中 pci1203.* 5,886 個只有平常關著的 1203 設定頁在看；HW.IoSetView 關窗後仍每 200 ms 要 io runtime、HW.teach 每 1 s 要 motor runtime，golden 關窗就停計時器）；推算省約 95% 位元組、15% 訊息；14 項永遠不能停（含非網頁消費者 ELA／SECS SV／REST／TCP 7016／7017）；原生六頁不用登記進頁面表 （Steven 20260930 15:3x「有開的才進行通訊」＋原生頁一節） | Jimmy 同意方向＋四個條件（§0.1）；寫程式前 Jimmy 逐段同意（§10） | 評估完成（第 1 階段先量），等 Jimmy 逐段回 |

> 題號與狀態以 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`／`decisions-decided.md` 為準；Steven 回覆後在這張表改狀態，並在該份 reference 的「目前狀態」補裁決編號（RULINGS_20260926 的 S 編號）。

## 2. 怎麼寫一份評估案

1. **只查不改**：工程師唯讀查證（`git -C D:\HT9045 show <hash>:<path>` 讀固定版本、golden 用 cp950 解碼），只寫一份 `references\<主題>.md`；不 build、不跑 wb_serve、不寫 git。
2. **結構**：0 一句話結論 → 1 背景／現況（白話＋絕對路徑＋commit 附主要檔全路徑）→ 查證（每個說法都附 `D:\...\檔名:行號`）→ 選項 A／B／C（做法、要改的檔與主人、風險、工作量、能驗到什麼、一個具體例子）→ 建議 → 要 Steven 決定的題目全文（背景／選項／St01建議／例子／目前狀態）→ 沒查證／推論的地方。
3. **寫法**：每一處路徑寫全（不寫 `:行號` 或 `tools\wb_serve.cpp` 半路徑）；內部代號先白話再放括號；先講機台上／畫面上發生什麼、操作員看到什麼。
4. **禁止**：任何密碼值（golden 寫死的、SG_PW.ini、login.dat）不寫進檔案、訊息、日誌（RULINGS_20260925 S41、RULINGS_20260926 S55）；只能描述長度、結構、在哪一行。
5. **登記**：ST01-E 核對後把題目放進 decisions-pending（Q 題），在本檔第 1 節補題號，commit＋push；ST01-M 登記 skill-registry 並重產快查表（先問 ST01-E）。

## 3. Steven 回覆之後

- 題目整段搬到 `decisions-decided.md`（附 S 編號）；reference 的「目前狀態」改成裁決結果＋日期。
- 評估案裡的設計就是開工底稿：派工時把 reference 路徑寫進 prompt，要求照「選定的選項」那一節做，偏離要回報。
- 做完後照 Steven「做完就更新 skill」：實作細節寫進對應的領域 skill（例：`D:\HT9045\.claude\skills\ht9045-html-json\`、`D:\HT9045\.claude\skills\ht9045-login\`），這裡只留評估與結論。

## 4. 相關

- 待決／已決題目：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`、`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`
- 網頁伺服器慣例（測試縫、視窗總表、驗證規則）：`D:\HT9045\.claude\skills\ht9045-html-json\references\wbserve-conventions.md`
- C 路（C++ 照 golden 算好設定頁的值再送網頁）：`D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md`
- 登入與密碼：`D:\HT9045\.claude\skills\ht9045-login\SKILL.md`
