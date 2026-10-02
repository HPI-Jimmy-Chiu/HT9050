# ChangeLog §12 範本

> §12 是 Steven 看進度的地方（ST01-E 核對後把 12.1～12.4 原文轉給 Steven），也是 Jimmy 看「交給 Jimmy」清單的地方（12.5）。
> 12.1～12.4 每輪**從來源重新組**再整段覆蓋；12.5 累加。`<…>` 是要填的地方，其餘照抄。
> 來源：ST01-E 的派工訊息（範圍、推送狀態、工程師、Steven 回覆、ST01-M 文字、上一輪更正）、git、decisions-pending.md／decisions-decided.md、RULINGS。

## 12.1 已推送

**規則**：標題寫本輪終點 hash；第一段寫推送狀態＋本輪重點（只詳述這一輪）；第二段把更早的批次壓成一句話指回 §11；最後一行寫通知信狀態。本機還有沒推的 commit 就照實寫（`origin/v906/steven-cbridge-review6..HEAD`＝N），不要寫「已推送」。

```
## 12.1 已推送到 `<本輪終點>`

`origin/v906/steven-cbridge-review6` 已推送到 `<本輪終點>`（`origin/v906/steven-cbridge-review6..HEAD` = <N>；`HEAD..origin/main` = <M>）。`<上一輪終點>` 之後新增 <K> 顆（09-27 <hh:mm>～<hh:mm>，見 §11.<NN>a～<x>）：<重點 1：做了什麼＋題號＝選項（S 編號）＋主要檔案絕對路徑＋hash>；<重點 2>……

更早的批次（<關鍵字，例：S121／S122 裁決、S95～S117 生產資料收尾、NUMCMP、S123～S163 大批裁決>，見 §11.<aa>～§11.<bb>）不重複列，細節都在對應章節。
6 小時總結信：暫停（見 §11.31／S87；Steven 20260926 20:4x 改看待決檔），有要 Steven 決定的一律寫進 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`，不逐次寄信。
```

## 12.2 進行中／即將開始

**規則**：只列 ST01-E 派工訊息說「在跑」的工程師、在等結果的 build／ctest／代編、剛交件待 ST01-E 核對的東西。每條寫：誰（或哪一題）、做什麼、產出在哪、見哪一節。上一輪在這裡、這一輪做完的，移到 12.1 敘述，這裡寫一行「已完成（hash，見 §11.NN）」即可。

```
## 12.2 進行中／即將開始

* <上一輪在跑、這輪做完的>——**已完成並推送**（`<hash>`，見 §11.<NN>）。
* **工程師在跑**：<題號／項目>（<做什麼>，產出 <絕對路徑或「交件後由 ST01-E 放到固定位置」>）；<…>。
* **等結果**：<ST01-M 替 St02 代編 `<hash>`：模擬組態 <結果>，出貨組態 <在跑／結果>>；<ctest 名稱> <在跑>。
* **剛交件、等 ST01-E 核對**：<…>。
```

## 12.3 排隊還沒開始

**規則**：每條寫「項目——等誰／卡什麼（見哪一節）」。已經不用排的，這輪就拿掉（或留一行 ⛔ 說明去向，下一輪再拿掉）。最後兩段固定：「往後排」（Steven 13:5x 裁決往後排的那一批）、「不算 St01 隊列」（St02 認領的）。

```
## 12.3 排隊還沒開始

* **<題號＝選項（S 編號）>**：<要做什麼>——<等誰／卡什麼>（見 §11.<NN>）。
* **<S 編號>**（<一句題目>）——等 **<Jimmy／St02／S72…>**（<原因>）。
* ……

**往後排**（Steven 13:5x 裁決：讀寫檔優先，網頁與次要項目往後排）——<清單>。完整清單、狀態與出處見
`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`／
`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`。

**不算 St01 隊列**：<St02 認領的項目>，St01 不碰（見 §11.<NN>）。
```

## 12.4 待 Steven 決定

**規則**：以 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 為準，**每個 `### Q..`／`### R..`／`### W..` 標題都要列到**，分三組：

1. **要 Steven 決定的 Q**（「要 Steven 決定或確認的 Q」那一組）
2. **已照建議先做、還要 Steven 點頭的 R**（「已照建議做、但還要 Steven 點頭的 R」那一組）——這些程式已經照建議做了，Steven 沒意見就維持現狀；可以併成一條，逐題寫一句題目。
3. **St02 的 W**（「St02（Steven02，測試通訊）」那一組，ST01-M 代登記）

另外掃一次 decisions-decided.md 裡寫「⚠ 還要 Steven 回」的追問，有的話列在最後並寫明在哪一題底下。每條寫：題號、一句題目、選項或 St01／St02 的建議、出處（S 編號或 todo ID）、「等 Steven 選／確認」。

```
## 12.4 待 Steven 決定

待 Steven 決定的題目以 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 為準（已決的在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`，不必再問）。目前 decisions-pending.md 有 <Q 題數> 題 Q、<R 題數> 題 R、<W 題數> 題 W：

* **<Q34>**：<一句題目>（RULINGS_20260926 <S154>）——<選項與建議>。等 Steven <確認方案 D／選 A 或 B>。
* **<Q44>**：……
* **<R60～R72>**（已照建議先做、Steven 可推翻）：<R60 一句>、<R61 一句>、……沒意見就照現狀。
* **<W36>**（St02，ST01-M 代登記）：<一句題目>——St02 建議 <B>。
* <decisions-decided.md「⚠ 還要 Steven 回」的追問：在 Q35 底下，網頁重新連上後「繼續生產」……>

沒列到的一律以 decisions-pending.md 為準；本節和 decisions-pending.md 對不起來時，下一輪記錄員核實再回報。
```

條目數核對：`grep -c "^### [QRW][0-9]" D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md` 的數字＝本節逐題（或併條裡逐題點名）的題數。

## 12.5 交給 Jimmy／其他 Steven session

**規則**：累加，只加不刪。新條目加在最上面、開頭標 🆕；每條寫：交給誰、什麼事、為什麼屬於 Jimmy（或 St02）的範圍、目前狀態（「已轉達（FROM_STEVEN `hash`），等 Jimmy 回」或「待轉達」）、見哪一節。做完或不用做的，在原條目前面標 ⛔ 已完成／⛔ 已結案，舊字加刪除線，寫明依據。ST01-M 已經在 FROM_STEVEN 轉達的，要寫「已轉達」不是「待轉達」。

```
* 🆕 **<項目>**（<題號＝選項（S 編號）>）：<要 Jimmy 做什麼／知道什麼>——<屬 Jimmy 範圍的原因>。已轉達（`FROM_STEVEN` 分支 `v906/steven-handoff` `<hash>`，<hh:mm>），等 Jimmy 回（見 §11.<NN>）。
* ⛔已完成（`<hash>`，<時間>）：~~<原條目文字>~~ <現況>。
```

待確認：舊條目上的 🆕 要不要在下一輪拿掉，目前的做法是沒拿掉（20260927 14:2x 的 12.5 裡新舊 🆕 都在）；ST01-E 沒交代就維持現狀。
