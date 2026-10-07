# HT9050 概念動畫與 UPH 的界線

## 差異

| 項目 | Concept／Main.MotionView9050 | UPH 計算頁 |
|---|---|---|
| 週期 | `buildOps` 的 `CYCLE`、`PRIME_LEN`、`KIT0_LEN`；`cycleLen` 隨 `phase` 選值 | `schedule` 取穩態起始間隔 `P` |
| Hot 入料 | `IN_KIT` 從 `SHT_BACK.b` 開始，接 `IN_PICK`、`IN_HP` | HP FIFO／Soak／各資源可用時間 |
| Ambient | portal Concept 由 `TEMP_HOT` 選分支，`IN_PICK` 自 0 開始，`IN_AMB.a=max(IN_PICK.b,SHT_BACK.b)` | portal UPH `p.hot` 分支 |
| 主鏈 | `SHT_IN→IDX_PICK→SHT_BACK→TEST→OSHT_IN→IDX_PLACE→OSHT_OUT→OUT_PICK→OUT_PLACE` | Out P&P 與下一顆測區可能重疊 |

HT9045 repo Concept 與 Main 的指定 `buildOps` 只有 Hot 配料路徑。
portal Concept 另有 `inArmAmbientAt`，以 Loader／In Shuttle 示意位置插值；這不是 encoder／氣缸實測。

## 零秒與傳參

`buildOps(dur)` 只接受 `dur[key] > 0`，否則用 `STEP_DEF` 的預設秒數。
UPH `readDur` 允許動作欄為 0；主值 `TEST` 另覆寫。因此零秒傳入 iframe 時，計算與動畫可用不同的秒數。
本批記錄差異，沒有改 HTML。

UPH `run` 把動作秒數與盤形寫入 iframe hash；portal 另傳 `temp`、`soak` 並用 `ANIM_HOT` 選動畫模式。
`seqCycle` 顯示單顆動畫主鏈／入料分支長度，與 `calc` 的 `P` 分開。
部分等待、暖機與批次重疊不能只由畫面循環長度推算。

## 查證來源與安全

六份檔案版本見 [來源](../resources.md#html-來源)，定位 `buildOps`／`cycleLen`／`inPhaseAt`／`inArmAmbientAt`／`seqCycle`。
本批沒有播放頁面、觸發 SIM／LIVE、驗證 iframe DOM 或實機；幾何與時間僅作來源說明。
