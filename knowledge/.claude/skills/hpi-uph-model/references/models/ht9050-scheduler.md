# HT9050 事件排程與解析路徑

本批只靜態讀 HTML；模型是頁面演算法，不是已核對的機台排程器。
來源：[HT9045 UPH](../resources.md#html-來源)、[portal UPH](../resources.md#html-來源) 的 `readDur`／`readParam`／`paths`／`schedule`／`calc`／`run`。

## 差異與解析路徑

`S = SHT_IN + IDX_PICK + SHT_BACK`；`W = TEST + OSHT_IN + IDX_PLACE + OSHT_OUT`。
HT9045 repo 頁只有 Hot。portal `readParam` 由 `pTemp` 產生 `tempMode`／`modes`；
`run` 用 `modeOf` 對各模式呼叫 `calc`。

| 路徑 | Hot | portal Ambient |
|---|---|---|
| 入料 `inpp` | `IN_KIT + IN_PICK + IN_HP` | `IN_PICK + IN_AMB` |
| 測區 `zone` | `S + max(IN_KIT, W)` | `S + max(IN_AMB, W)` |
| 出料 `outpp` | `OUT_PICK + OUT_PLACE` | 同式 |
| `soak` | `(pSoak + inpp) / nHP` | `null`；不經 HP |
| `tray` | `max(pLT, pULT) / nTray` | 同式 |

`readParam` 以 HP cols×rows 和 Tray cols×rows 四捨五入後計乘積，至少為 1；這只描述輸入處理，不能證明實機容量。
上述 `paths` 用來標 `bottleneck`；最後產能 **用排程器 `sc.P`**，不能將路徑最大值冒稱完全等於排程結果。

## 排程與等待

`schedule` 保存 `armFree`、`shtHome`、`zoneFree`、`oshtReady`、`outArmFree`；
`hpEnd` 與 `kitStart` 分別記 HP 補回與下一顆取料起始時間。

- Hot：HP FIFO 的 `hpEnd[n-nHP] + pSoak`、入料臂可用與 Shuttle 回家共同限制 `tk`。
  `IN_KIT` 後補 `IN_PICK`、`IN_HP`；每 `nTray` 顆依 `pLT` 延後補料。
- Ambient：先 `IN_PICK`，可與測區平行；`IN_AMB` 要等 `shtHome`，無 HP／Soak 路徑。
- 測區鏈順序更新，Out Shuttle／Out Arm 可用時間另限制 `toi`、`top`；`pULT` 在每盤出料末端加等候。
- `waits` 是迭代中的等待 **次數**，不是秒數、機台 log 或完整損失占比。

`N = max(80,14*nHP)`；Hot 平均最後 `8*nHP` 個起始間隔（以 `N-1` 為上限），portal Ambient 平均最後 24 個。
`calc` 以 `max(sc.P,0.001)` 防零除；`uph = 3600/P*(pYield/100)`，沒有 site 倍乘。
不能據此認定指定迭代長度對所有輸入都已證明收斂；本批未執行數值測試。

## 待補與實測對照

示意秒數、HP／Tray 容量、各機台資源互鎖、site-off／retest、良率口徑及故障／換盤損失須以實際版本與量測補。
`readDur` 對動作欄取不小於零，再覆寫主值 `TEST`；不能把 HTML `min` 屬性當成全面數值驗證。
和 [動畫序列](ht9050-animation.md) 的零秒／暖機差異一起看，避免把動畫總長帶入排程產能。
