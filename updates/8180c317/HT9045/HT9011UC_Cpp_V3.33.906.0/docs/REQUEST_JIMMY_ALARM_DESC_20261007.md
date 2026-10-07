# 請 Jimmy 處理：所有警報都要顯示說明（EastSun 20261007）

> EastSun 1007 原話：「請jimmy徹查 所有異常有那些沒有說明顯示的 全部都要有」、「之後畫面不要只顯示錯誤碼 要顯示錯誤碼對應的說明」。
> 機台端只做了只讀盤點，**程式沒有改**，交給筆電這邊做，修好後照一般更新包流程推過來。
> 盤點清單：`docs/alarm_audit_20261007.csv`（1,228 個代碼，一列一碼）。

## 機台上看到的現象

- 告警框（`web/page/Alert.Note.html`）上方的訊息行（ShowMessageEdit1）只顯示代碼，例如 `JAM31304`。
- 下方的說明框（reDescription）是空白。
- 例：JAM31304＝氣缸警報 31000＋304＝C_InPnPDrop1。
  - golden 訊息行會顯示 `Cylinder C_InPnPDrop1 push or pop error!`，說明框會顯示自動產生的 `English\JAM31304.dat`。
  - 機台兩個都沒有。

## 原因（兩個顯示問題＋資料缺口）

### 1. 非馬達警報的訊息行一律只顯示代碼（影響 1,075 碼）

- `tools/wb_serve.cpp:380` 送出的 `display.message = W906_AlarmMsgWithErrPart(code)`，內容是代碼本身加上 errPart。
- `web/page/dialog-page.js` renderAlarm 把 `d.message` 放進 ShowMessageEdit1。
- `web/page/ht9045_alarm_motionview.js` fillFromCode 只有在 message 為空時，才改查 AlarmCodeList-index.json。message 永遠不會是空的，所以一直顯示代碼。
- golden（note.cpp ErrShowToForm）是 `ShowMessageEdit1 = AlarmCodeMap[Code]`，加上 `" : "+errPart`，重複時再加 `" (Again!!)"`。資料來自執行中的 `D:\HT9045\Error\AlarmCodeList.txt`。
- 移植樹在 `forms/fNote_ShowError.cpp:272` 已經算出 `sAlarmMes`，只是沒有送給網頁。
- 只有馬達告警框（`g_w906MotorNoteMsg`）帶的是 golden 那句話。

### 2. 說明框只讀一份靜態的網頁 JSON

- 機台上沒有任何 `.dat`：`D:\HT9045\Error` 底下沒有 `Chinese\`、`English\`、`Korea\`、`Singapore\` 資料夾。
- `web/JSON/Alarm-description.json`（951 碼＋MOT0～MOT8）和 `AlarmCodeList-index.json` 是 09-09 產生的快照，之後是手動補的。
  - 不會跟著執行中的 AlarmCodeList.txt 更新。
  - 有 436 碼的短訊息跟 live txt 不一樣。

### 3. 各家族的缺口（數字見 csv）

| 家族 | 碼數 | 缺說明（任何語言） | 說明 |
|---|---|---|---|
| 馬達 `WAR24nnnk` | 153 | 153 | golden 依「種類」讀 `<Lang>\MOT<k>.dat`（note.cpp:4429/4485）。JSON 已有 MOT0～MOT8，但頁面拿 `WAR24nnnk` 去查，查不到 |
| 氣缸 `JAM31nnn` | 285 | 25（JAM31295～31320） | golden cMyDB.cpp:1866-1916 開機時依範本產生短訊息和 `English\JAM31nnn.dat`；移植樹的範本在跑，但 `English\` 資料夾不存在，vclcompat `TStringList::SaveToFile`（vclcompat/TStringList.cpp:396-397）開檔失敗時靜默返回 |
| 溫度 `WAR15nn`／`WAR151nn` | 142 | 20（WAR15151～15170） | golden cMyDB.cpp:1925-1934 只產生短訊息 |
| 其他（literal／陣列／門／公式等） | 648 | 363 | golden 的 `.dat` 也沒有這些碼，golden 畫面上一樣是空白 |

- 不在網頁短訊息清單裡的有 81 碼：
  - 48 碼在 live txt 裡有，例如 JAM31295～31320、WAR2851 等；
  - 33 碼兩邊都沒有，連 golden 都會顯示「Unknown Alarm Code」，例如 WAR0149（ainarm9045.cpp:2779）、WAR0945～0956（OCRInsp.cpp）、WAR31004～31006（uTemp_Set.cpp）。清單見 csv 的 `inLiveAlarmCodeListTxt=0` 列。
- 網頁清單有、live txt 卻沒有的有 9 碼（MES16441、WAR16120、MES0922x 等）。照第 1 點改成送 golden 訊息之後，這 9 碼會變成「Unknown Alarm Code」，要先補進 AlarmCodeList.txt。

## 盤點建議的做法（給你參考，由你決定）

1. 訊息行照 golden：wb_serve 送 `AlarmCodeMap[code]`（或 `sAlarmMes`），加上 errPart 和 Again。
2. 說明照 golden 的順序找：
   - `Error\<Lang>\<Code>.dat`；
   - 找不到就用 `English\`（note.cpp:4483）；
   - 馬達用 `MOT<k>.dat`。
3. 氣缸範本 `SaveToFile` 前先建 `Error\English` 資料夾（MyForceDirectories），這樣 285 個氣缸說明就會自動產生。
4. golden 也沒有說明的那 363 碼，加上清單都沒有的 33 碼：EastSun 要求「全部都要有」。這部分要你決定是手寫說明，還是沒有說明時把訊息行顯示在說明框（golden 的氣缸範本也是用訊息當第一行）。
5. 1203 卡的錯誤碼（例如 `card returned 0x8000002D`）也請附上名稱和說明。例：0x8000002D＝InvalidAxDistance，0x80000011～14＝InvalidAxParVelLow／VelHigh／Acc／Dec，來源 `EtherCAT/vendor/AdvMotErr.h`。

## 機台端已經做的（不用重做）

- web ae69cdd RTFDESC：說明框原本把 RTF 原始碼直接印出來，現在會轉成 TRichEdit 顯示的文字（`dialog-page.js` 檔尾的 `HT9045RtfText`）。
