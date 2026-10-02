# 筆電（Jimmy 這邊）目前的雙向溝通對象（20261001 14:3x，給 Steven／St01 畫 AI 組織圖；15:1x 照「Jimmy 人員與 session 確認」session 逐筆核對更正 6 處）

> Steven 1001 13:4x：「要問一下Jimmy, 是不是有其他人加入了這個大家庭」「他目前手下跟多少人進行雙向溝通」。下表是筆電這邊 git 交接管道的實況
> （main 的 `docs/handoff/` 與各 `*-handoff` 分支）。**session ID 筆電只知道自己的**；其他人的請各自寫在自己 FROM 檔的 §0（CHAT_JIMMY 14:3x 已請大家回報），St01 從那裡收。

| 名稱 | 誰／哪台 | 角色 | 管道（筆電寫／對方寫） | 雙向？ | 開始 | 備註 |
|---|---|---|---|---|---|---|
| Jimmy | 本人 | 最終裁決 | 直接跟筆電對話；裁決寫進 `HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_YYYYMMDD.md` | 是 | — | |
| 筆電 | Jimmy 的 Claude（JIMMYCHIU-NB） | 整合：兩組態 gate、合 `main`、推 GitLab `main` 與 GitHub 機台更新包；906 移植樹 | 寫 main 的 `TO_*.md`、`CHAT_JIMMY.md` | — | 0922 | session ID `c21900ca-daa9-47bd-a642-cd9e56e1b224`（1001 13:4x 重開機後的新 session；重開機或關視窗就會換，前一個是 `9190c0f6-6ccd-4722-a0db-a8f189dce683`） |
| NB2 | 舊筆電的 Claude（JIMMYCHIU-NB2） | 輔助：工具、審查、R 系列 | NB2 推 `v906/nb2-assist`（`docs/nb2_assist/README.md`、`URGENT.md`）；筆電每輪讀 | 是（NB2 讀 main） | 0924 20:0x | 也會開 MR（例 MR !47 SCREEN-TOKEN） |
| St01 | Steven01 | 資料讀寫轉檔（FileRW、cbridge） | `TO_STEVEN.md`（main）／`FROM_STEVEN.md`＋`CHAT_ST01.md`（`v906/steven-handoff`） | 是 | 0926 | 工作分支 `v906/steven-cbridge-review6`、`v906/st01-*` |
| St02 | Steven02（STEVEN-NB3）：ST02-M／ST02-E／ST02-E2 | 測試介面（TesterComm、GPIB、網頁） | 同上，聊天 `CHAT_ST02.md` | 是 | 0926 | session ID 已在 FROM_STEVEN §4（1001 12:57）回報 |
| Kevin | Kevin | K-01 golden 0618 模擬版；K-02 審 Frank 910 的 Index | `TO_KEVIN.md`／`FROM_KEVIN.md`＋`CHAT_KEVIN.md`（`v906/kevin-handoff`） | 是 | 0929 | |
| Jerry | JerryYang | 翻譯（讀檔；J 系列：J-6 SOCKETSENSOR、J-7 WAR0154 已修、J-8 吸嘴格子內容沒發布到網頁、J-9 TestSocket 156 處閘內盤點） | `TO_JERRY.md`／`FROM_JERRY.md`（`v906/jerry-handoff`） | 是 | 交接 0929 18:24（移植樹 commit 從 0924 起） | |
| Ifor01 | Ifor 的 Claude（IFOR-NB2） | 加熱鏈 I-01、溫控通訊層 I-03、RotateKit I-04、G-031 差異清單 I-02 | `TO_IFOR.md`／`FROM_IFOR.md`＋`CHAT_IFOR.md`（`v906/ifor-handoff`） | 是 | 1001 09:1x |**溫度（尤其 ATC）的問題請教 Ifor 本人**（Jimmy 1001 17:1x：「溫度有問題可以請教Ifor，他是溫度專家，尤其是ATC」）：Ifor01 問筆電的溫度題，溫度專業的部分交 Ifor 本人定，只有動到筆電的檔（例：serve loop 的節拍）才轉 Jimmy |
| Frank01 | Frank 的 Claude | 9050 流程導入（Index、Shuttle 先）；910 樹參考分支 `ref/frank-910-9050` | `TO_FRANK.md`／`FROM_FRANK.md`（`v906/frank-handoff`；CHAT 檔還沒開） | 是 | 1001 11:3x | |
| ES02 | EastSun 的筆電 | 測試（筆電＋上機驗證），測到問題就修、交筆電整合；St01／St02 的上機清單；HTDESIGNER 外掛 | `TO_ES02.md`／`FROM_ES02.md`（`v906/es02-handoff`；CHAT 檔還沒開） | 是（1001 14:25 起） | 1001 13:5x | Steven 1001 09:4x「需要上機驗證的, 都是請Eastsun處理」 |
| RogerYang | RogerYang | skills 合併（17 支 `skills/rogeryang-*`；MR !25～!30、!35～!45） | 只有 MR，沒有交接檔 | 否 | 1001 | MR 已開；Jimmy 1001 15:0x「照建議，兩題都選 A」⇒ 繼續放著（RULINGS_20261001 第 28 條） |
| 機台端 | HT9050 機台（DESKTOP-QCMVRND，有 PCIE-1203 卡） | 跑 GitHub 機台更新包、上機驗證 | 去程：GitHub `HPI-Jimmy-Chiu/HT9050` 的 `updates/<版號>/`；回程：GitHub orphan 分支 `machine/integ-ioweb`（機台推 format-patch，筆電 `git am`，RULINGS_20260926 :73、:105）。0925 曾用 Remote Control 文字轉交（RULINGS_20260925 :81-83） | 是（兩個方向都走 GitHub） | 0925 | 機台要不要套包由 Jimmy 決定；「機台端與筆電端進度同步」是筆電這邊轉達視窗的名字，不是機台的 session ID |

> **範圍**（Jimmy 1001 15:0x，RULINGS_20261001 第 29 條）：只畫 HT9045／HT9050 專案的成員；只在 RD5 入口網站寫日報的同事（KenHeish、Chrischen、Maurice、StevenHong、Mengfan）不放。
> **筆電的 session**：只列整合／夜間迴圈這一個（上表「筆電」那列，`c21900ca-…`，ht9045-b0）。筆電另有單一任務的 session（例：資料夾容量清查、人員確認 `63e17803-…`），只跟 Jimmy 對話、不跟同事溝通、做完就關，不列。
> 人名旁邊的 4 碼（例 3420）是分機（FROM_FRANK.md:4），不是員工編號，表上不寫。
