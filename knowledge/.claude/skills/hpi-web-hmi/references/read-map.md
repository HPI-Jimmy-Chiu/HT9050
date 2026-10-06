# 逐題閱讀地圖

先分版本、機台與客戶，再選最小分支；各樹都有original-entry.md與原references，長文按原章節繼續下探。

| 問題 | 第一步 | 連動 |
|---|---|---|
| 開站JSON／資料來自哪裡 | [JSON](json/index.md) | [目前main](runtime/index.md)、[Config](../../hpi-config/SKILL.md) |
| 新頁／DFM／元件／鍵盤／主題 | [畫面](pages/index.md) | 原display／widgets／proxy規範與generator來源 |
| editlist.get／save、form.event、Cowner | [橋接](bridge/index.md) | [JSON路C](json/references/route-c-golden-bridge.md)與[Config](../../hpi-config/SKILL.md) |
| Recipe／Lot／QA | [LotInfo／Recipe](../../hpi-lotinfo-recipe/SKILL.md) | 本Skill僅保留Web／橋接原文與分工 |
| fShow不對、關窗／縮小／瀏覽器斷線 | [視窗](windows/index.md) | [目前main](runtime/index.md)；不把hub放開jog與全關STOP混用 |
| 登入、改密碼、設定／告警重驗 | [登入](auth/index.md) | caller、CC與位元組格式；不讀實際密碼本作文件驗證 |
| AlarmCode／CSV／event log | [紀錄](logs/index.md) | [Alarm](../../hpi-alarm/SKILL.md)、原EventLogAnalyzer入口 |
| native六頁與HTML監看分工 | [原生](native/index.md) | 對應編譯期旗標、最新裁決與實作進度 |
| HT9050頁面與其他機型差異 | [機台](machines/index.md) | [MotionView](../../hpi-motionview/SKILL.md)、[IO](../../hpi-io-control/SKILL.md)、[Motor](../../hpi-motor-control/SKILL.md) |

舊入口保持相容路徑；原文的「已推／未做／測過」保留原日期、commit或分支，不從那些表推算20261006全量完成度。來源未在Git時先標缺件；不補造內容或借別人的進度當自己的驗證。
