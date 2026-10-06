# HT9050 Index／FinePitch

## 差異

HT9050 僅 Index Z1；水平定位由 Shuttle 獨立 X 處理。Index1 的軸接口與部分 Shuttle 軸不同，不能把 1203／Galil 驅動泛化到整台。共同項與其他機型的差異見 [共用表](../../common.md)。

## 入口與流程

- [FP／F1／F2 流程、工作卡與原裁決索引](ht9050-index-fp-flow.md)
- [Auto Height／Contact Test 當前版本筆記](autoheight-contact-test-ht9050-current.md)
- V906 source 有 `DoTestHeadMotorFP()`，但目前 `csystem.cpp` 尚未呼叫 FP 入口；原文內的永久路線是裁決／待交付項，不能當成 dispatcher 已切換。
- 通用與 FP 入口共享 `iTestHeadMotorTask`，分流時須一起追 cursor，避免不同狀態機混跑。

## 安全

- W-44 的 Steven 原話、socket 外安全區方向與 Index 優先權保留在 [原文件第9節](ht9050-index-fp-flow.md#9-20261006-steven-q129q133steven-1006-082x-回-frank01-的-f-3f-7連同-f-1f-2decisions-decided20261006-082x-steven-裁決)；若 anchor 隨原標題改變，以文件導覽的第9節為準。
- 下壓條件是 socket 外安全區；不可改寫成兩支 Shuttle 必須精準停在原點。方向由各自 `sign(Right-Left)` 判斷；安全帶與 shake 關係依原裁決／提案分開說明。
- 本次整理保留原裁決全文，沒有宣稱所有互鎖已完成。

## 查證來源

原 FP 文件保留每次裁決／驗證的版本與日期；當前 V906 dispatch 及 EP stub 查證見 [共同項](../../common.md)。
