# 散在其他 skill 的溫度事實索引（20261001）

> 路徑都在 `D:\HT9045\.claude\skills\` 底下。這裡只列「哪個 skill 記了什麼」，內容以原 skill 為準；新的 V906 溫度現況請記在本 skill 的 SKILL.md §2，原 skill 連過來。

## 1. 兩個 ATC skill 的重疊（要 Jimmy 決定）

- **`ht9045-atc`**：golden BCB6 的 ATC 程式：`ATC_Handler_Side` 檔、`iATC_MODE_TYPE` 表（20～70）、API、五個已知 ACK／fall-through 缺陷、新增命令怎麼做、
  kevin 0415 擋 ATC 6.0／7.0 的條件、0423 的 1127／1128 白名單與 prime 期間輪詢、ATC 3.6 回報成 35。references：`Handler_Command_Report.md`、
  `ATC_Command_Payload.md`、`ATC_Control_Command_Diff_Table.md`、`MultiZone_EnableChannel_Bug.md`。說明欄格式正確。
- **`ht9045-atc-interface`**：TCP 封包格式與範例、137 個命令摘要、site／通道對應（`iSiteToATC`／`iSiteToOfs`、`dATCTempOffset` 索引、V899 的筆誤）、`SetATCOffset` 呼叫路徑、除錯情境；
  `references\atc-commands.md`（xlsx 全表）是它獨有的。
- **重疊的經過**：Jimmy `8e6faae8`（20260915）把同事的 `ht9045-atc` 併進 atc-interface 當 references（「一個問題一個入口」），所以有 `from-colleague-20260915.md` 與 `colleague-*`；
  St02 `72bec5a0`（20260926）又從 `.github\skills` 把 `ht9045-atc` 整份複製回來，等於撤銷合併。現在 `ht9045-atc\SKILL.md` 與 `from-colleague-20260915.md` 只差搬移註記；
  四個 references 有三個跟 `colleague-*` 位元組相同，`ATC_Command_Payload.md` 只差 BOM；atc-interface 自己的 SKILL.md 沒有連到任何 `colleague-*`。
- **20261001 已修**：atc-interface 的說明欄原本是沒加引號的一行、內含「Use when:」（YAML 不合法），清單上顯示成「HT9045 Handler ? ATC Interface 知識庫」（? 是亂碼的 ↔），所以很少被挑中；
  已改成 `>` 折行、↔ 修好。`hangup-intake` 把 ATC 問題導到 atc-interface，這點沒改。
- **兩個都沒寫 V906**：`W906_ReadATCIni`（`322d68a3`）、H-011 `W906_ATC_PORTED=0`、`SetATCOffset` 沒有定義、`SendCommToATC7` 沒有呼叫端——記在本 skill SKILL.md §2。
- **建議（待 Jimmy）**：留一個。以 `ht9045-atc` 為底（內容多、格式對），把 atc-interface 獨有的 `atc-commands.md` 與 §4～7（封包、對應、呼叫路徑、除錯）併進去，刪掉重複的 `colleague-*`
  與 `from-colleague-20260915.md`，atc-interface 留一個指過去的殼，並改 `hangup-intake` 的導向。範圍：只管 Handler↔ATC 協定與 `ATC_Handler_Side` 行為；廠牌、COM 埠、bthermo 歸 heater-control。
  這會推翻 Jimmy 0915 選的入口，所以要他同意才動。

## 2. 其他 skill 裡的溫度事實

| skill／檔 | 記了什麼 |
|---|---|
| `ht9045-json-bridge\references\porting-gaps.md` | §一 填 `bUT150Install[]` 的程式（`Index16Heater`，931 行）不存在；§二 `HeaterThread` 沒啟動；§三 GATE(T1) 擋住 WAR15 升級（SAFETY），`cTemperFrom` 從 V908 翻、不是 V912；§八 15 個通道永遠收不到 `SetTemp`；§九 `bTemperatureReady` 初值不對；§十三（已結）ATC.ini 沒讀，開機把 Chiller Temp 從 -20 改成 5 |
| `ht9045-json-bridge\references\api-shape.md` §4.8 | `StageThermo` 送的 `temp.zone.*` 是 null；舊的 `temp.pv`／`zone.*` 與新 tag 並存，要在同一顆 commit 一起改 |
| `ht9045-json-bridge\references\pending-pages.md` | DUT on／off 的 ATC7 沒有呼叫端；`TimerACTTimer` 會改 `fTempOffSet`（SAFETY）；WinWay 頁 |
| `ht9050-construction\references\decisions-decided.md` | Q14、Q15、Q34 全文（D-1～D-9、底層 ①～⑧） |
| `ht9050-construction\references\rulings-index.md`、`todo.md`、`phase-gates.md` | S61、S91（ATC offset 被夾成 0 的修正）、S109；C-002（DTM 超溫）、F-003、H-011；Gate 9 |
| `ht9045-st02-workflow\references\w9-remote-temp-offset-plan.md` | GPIB `SETTESTOFFSET_` 寫 `Temperature.Data`，S3 閘已開；沒有 `SetATCOffset`，ATC 留著舊 offset；START 之前 `iSiteToOfs` 全 0 |
| `ht9045-st02-workflow\references\p4-cmydb-w7-plan.md` | `HeaterLog` 的掛勾；`HeaterSVLog` 仍擋 |
| `ht9045-general-ini`（SKILL、`handlersys-mapping.md`、`database-mapping.md`） | 規格書上 `USE_ATC_MODE`／`ATC_SYSTEM_USEHEAT`／`USE_16_HEATER` 的組合；`HEATER_CTRL_TYPE` 預設 KT4H；`SetHeaterTemp_Max*` 預設 60；`ATC_SYSTEM_PORT` 強制 1234 |
| `ht9045-auto-temp-offset`（20261001 RogerYang 加入） | ATK 溫度自動補償 by FTP（Config [N31]）：流程、`SetTempOfs_yyyymmddHHMMSS.ini` 格式、Arm＝0／1／2／-1、GPIB 端要 Handler 送 `iLotStatus` 才生效（§5a）、NN mode `RefreshTempData()` 回 0 套不進去（§5c）、`iSiteToOfs` 覆寫套到隔壁 site（§5d）——都是 P260908-ATK-H9-02 定案。V906 移植樹沒有 `AutoTempOfsByFTP`（本 skill SKILL.md §2 LotInfo 那列） |
| `ht9045-recipe\references\Temperature.Data.md` | `Temperature.Data` 逐段說明（含 `[ATC]`、`[Index]`、Boost、LB）——⛔ 有錯，見該檔開頭與本 skill 的 temp-set-and-lotinfo.md |
| `ht9045-config\references\config-fields-L.md`、`flowcharts\L\L43-power-follow.md` | [L] 組：L11 ATC 保護、L43 Power Follow |
| `ht9045-lotinfo-flow\SKILL.md` §8 | 各 ATC 型怎麼顯示 ATC 溫度；L11 告警 WAR15301～15304 |
| `ht9045-alarm-dismissal\SKILL.md` §7.5 | WAR15 整個前綴加 WAR1637 在必停清單（`neverNonStop`） |
| `ht9045-secsgem\SKILL.md` | 71 個 `eTempControll` enum 全表；溫度 SV／EC 的寫法（`ShowTempComp`、`dSingleTempLimit`） |
| `ht9045-io-control\references\exit-shutdown.md` | `EndHeaterThread` 什麼都不做；`SwHeaterRelay.Off()` 在模擬版是空的，繼電器一直通電；ATC STOP 沒接 |
| `ht9050-hw\references\temp-dtm-map.md` | HT9050 用台達 DTM 走 Ethernet，3 站 24 通道，對到 `eTempControll`；程式裡沒有 HT9050 的通道表 |
| `ht9050-st01-evaluations\references\page-control-tabs.md`、`wbserve-sandbox-run.md` | Temp_Set 的 `pgcTempOffsetChange`（`f14484e1`）；HSys Heater 分頁假的頁數待辦；溫度頁存檔寫死 `config\ATC.ini`；`--config` 會切加熱繼電器並送 ATC7 |
| `ht9045-html-json\references\route-c-golden-bridge.md` | Q14＝B 重播不寫檔；存 Configuration／Yield 會切繼電器並送 ATC7 |
| `ht9045-html-version`（SKILL、`main-split.md`、`production-runtime.md`） | HeaterView 與 TemperFrom 頁沒資料時顯示 `---`；兩個 ATC.ini；浸泡時間與設定溫度的 JSON |
| `ht9045-state-record-analysis\SKILL.md` | ATK MultiZone 案例（r895 對 r896）；決定走哪個溫度分支的 ini 鍵 |
| `gpib-command-list`、`ht9045-gpib-bridge`（SKILL、`gb-p8-bringup-plan.md`） | `SETTEMP`／`SETSOAK`／`SETTESTTEMP` 對到 `MSG_CMD`；寫入命令不要先送 |
| `ht9045-v899\references\module-index.md` | V899 的 `TempCtrl\`（DT4848／KT4H／TMC401／UT100／WT404／TriTemp）、`EJ1N\` |
| `pre-release-check\references\patterns.md` | `cTemperFrom.cpp:1600` 一行 `==` 沒有作用；uLotInfo 3Sigma 的 `malloc`／`delete` 不配對 |

`st-wave-loop`、`ht9045-index-flow`、`ht9045-uph-model` 只是順帶提到，不列。
