---
name: hpi-mnetlog-split
description: 提案（20261007，只提案、還沒實作，等 Jimmy 派工）：把 MNetLog（MNet／IO／FTP 錯誤紀錄，golden Motor/myMN200motor.cpp:2146）從 TfMain 拆成獨立模組，**只做 V906 C++ 樹**。附 golden 0618 與 V906 的位置、呼叫點數量、「拆出去能少幾個 include main」的實測，以及一個更重要的發現：V906 現在 MNetLog 一行都沒有寫出去（本體的閘理由已過期＋兩個檔各自放了空殼）。Use when：問 MNetLog 在哪、誰在用、為什麼 D:\HT9045_Log\MNetLog 沒有檔、要拆 main.h／fMain.h 的相依、要做這張卡。關鍵字：MNetLog, slMNetLog, mmoMNet, tsMNetLog, MNet Log, D:\HT9045_Log\MNetLog, myMN200motor.cpp, LogObjects.cpp, W906_CreateLogObjects, TMyStringList, AddTextWithDateTime, main.h, fMain.h, include 相依, extern bool MNetLog, TfFTP, MyLaneIo, AutoClean。
---

# MNetLog 獨立模組提案（V906 C++ 限定）

> 起因：Steven 20261007 10:1x「我想把 main.cpp 裡面的 mnetlog 獨立出來，這樣就不用大家都去 include main.h」「只針對 c++ 版本做這個變更」「做成一份提案的 skill」「然後請 jimmy 派工」。
> 量測：ST02-M 20261007 10:1x，golden＝`HT9011UC_Code_V3.33.906.0_20260618`（BCB6），V906＝`HT9011UC_Cpp_V3.33.906.0`（行號照 GitLab main `5af1df2d`）。腳本 `C:\AI_TempFile\st02m\count_mnetlog.py`、`mnetlog_include.py`。
> **BCB6 樹（V912／V899／golden）一律不動。**

## 1. 一句話結論

- 「拆出去能少幾個 include main」：**幾乎省不到**——golden 只有 1 支（定義所在的 `Motor/myMN200motor.cpp`）是純為了 MNetLog 才 include main.h；呼叫端本來就是用自己手寫的 `extern bool MNetLog(AnsiString)`，不靠 main.h。
- **真正的價值在別處**：V906 今天 MNetLog **完全沒有落檔**（見 §3）。拆成獨立模組＝順便把它修好，讓 MNet／IO／FTP 的錯誤紀錄照 golden 寫進 `D:\HT9045_Log\MNetLog`；另外把 6 處手寫 extern 收成一個標頭、測試不用帶 TfMain 就能連結。

## 2. golden 0618（BCB6）現況

| 項目 | 位置 |
|---|---|
| 函式本體 | `Motor/myMN200motor.cpp:2146` `bool MNetLog(AnsiString Message)`：寫 `fMain->slMNetLog->AddTextWithDateTime()`，再把同一行加進主畫面的 `fMain->mmoMNet`（超過 5000 行清空） |
| 紀錄物件 | `main.h:1478` `TMyStringList *slMNetLog`；`main.cpp:1516-1517` 建立（`D:\HT9045_Log\MNetLog`）、`main.cpp:11991` 刪除；畫面分頁 `main.h:560` `tsMNetLog` |
| 其他 main.cpp 用法 | `:25222`、`:26382` 呼叫；`:26413-26418` 複製作用中的 MNetLog 檔、`:26526` robocopy 近 2 天（hang-up 收集） |
| 宣告 | 6 處手寫 `extern bool MNetLog(AnsiString Message);`：adam6024.cpp:56、HS_Function.cpp:2046、AutoClean/AutoClean.cpp:43、EtherCAT/MyEtherCAT.cpp:233、Motor/myMN200motor.h:101、ProductionInfo/TfFTP.cpp:11 |
| 呼叫點 | **129 處／13 檔**：TfFTP 58、MyLaneIo 17、uLotInfo 14、uhome 12、myMN200motor 8、adam6024 6、mymotor 6、cMyDB 2、main 2、HS_Function／AutoClean／HANA_ART／MyEtherCAT 各 1 |

**include main.h 的情況**（全樹 185 支 include main.h）：呼叫 MNetLog 的檔裡，只有 `myMN200motor.cpp` 是「include main.h、而且除了 MNetLog 之外沒用到 fMain」；其他（AutoClean 75 處、uLotInfo 164 處、cMyDB 32 處、HS_Function 53 處、mymotor 13 處…）本來就大量用 fMain，拆了 MNetLog 也還是要 include。⇒ **省 1 支**。

## 3. V906 現況（重點：今天一行都沒寫出去）

| 位置 | 狀態 |
|---|---|
| `Motor/myMN200motor.cpp:2516` 真正的本體 | **整段 `#if 0`**，直接 `return true`。閘的理由（:2513）寫「forms/fMain.h 沒有 slMNetLog／mmoMNet」——**已過期**：`forms/fMain.h:1277` 有 `slMNetLog`，`LogObjects.cpp:114` 的 `W906_CreateLogObjects()` 在 wb_serve 開機時就建好它（AI(W906-LOGOBJ-W7) 20260927，Steven W7＝A）。只有 `mmoMNet`（VCL 畫面上的記錄框）真的沒有。 |
| `AutoClean/AutoClean.cpp:189` | 檔內 `static bool MNetLog(AnsiString){ return false; }` 空殼（註解「not yet translated」） |
| `ProductionInfo/TfFTP.cpp:179` | 同上的檔內空殼——**FTP 的 58 處紀錄全部丟掉** |
| `Adam6024Pressure_St02.cpp:591` | 經函式指標 `g_pAdamPressNetLog`（:576，預設 `AdamPressDefaultNetLog`）轉送，可被測試換掉 |
| `MainTimer3.cpp:81` | 只有宣告，:341 呼叫真的那支 |
| `MyLaneIo.cpp:57` | 已改接真的那支（AI(W906-W3-LANEIO) 20260924），註解寫「slMNetLog 建好後就會自動落檔」——但本體的閘沒拆，所以還是沒落檔 |
| `cStateRecord.cpp:1110` | GATE(W906-STATEREC-G5)：hang-up 收集時複製作用中 MNetLog 檔那段閘住，理由同樣是「slMNetLog 不存在」——也已過期 |

V906 呼叫點：**116 處／12 檔**（另 17 處已註解掉）；全樹 100 支直接 include `fMain.h`，呼叫端多半沒有直接 include（經其他標頭拿到 fMain），所以 V906 也省不到幾個 include。

## 4. 提案做法（兩步，一張 MR 或分兩張）

**第 1 步：讓它照 golden 落檔（行為修正，照翻）**
1. `myMN200motor.cpp:2516` 拆閘：照 golden 寫 `fMain->slMNetLog->AddTextWithDateTime(Message)`（先判 `slMNetLog!=nullptr`，ctest 裡沒有建）；`mmoMNet` 那三行**改成不顯示在主畫面**（Steven 1007 12:0x「先改成不用顯示在main」），原位加註 `// TODO(W906-LOGVIEW): …`，列進 §8 待辦，之後換位置顯示。
2. `AutoClean.cpp:189`、`TfFTP.cpp:179` 的檔內空殼退役，改呼叫真的那支。
3. `cStateRecord.cpp:1110` GATE G5（hang-up 時複製作用中 MNetLog 檔）理由過期，照 golden 打開。

**第 2 步：獨立模組（Steven 要的拆分）**
1. 新增 `MNetLog.h`（只宣告 `bool MNetLog(AnsiString Message);`）＋`MNetLog.cpp`（本體從 myMN200motor.cpp 搬過來，原位留一行註解指過去、附 golden 出處）。
2. 6 處手寫 `extern` 換成 `#include "MNetLog.h"`。
3. 紀錄物件仍由 `W906_CreateLogObjects()` 建、掛在 `fMain->slMNetLog`（golden `main.cpp:26414` 的 `GetFileName()` 與 cStateRecord 都還要拿它）；`MNetLog.cpp` 只透過一個小的取用函式拿這個指標，不 include `fMain.h`。⇒ 測試連結 MNetLog 不必帶整個 TfMain。

**測試**：新 ctest——沙盒 `W906_HT9045LOG_ROOT`，建 LogObjects 後呼叫 MNetLog，檢查 `MNetLog` 資料夾當天檔有那一行；`slMNetLog==nullptr` 時不當掉、回 true；反向（閘關回去／空殼放回去）要紅。另跑 TfFTP、MyLaneIo、AutoClean、cStateRecord、Adam 相關既有 ctest，兩組態。照 RULINGS_20261005 #6 先 machine_sync。

## 5. 風險與要先決定的

- **會開始寫檔**：`D:\HT9045_Log\MNetLog\YYYY\MM\…`（golden 行為）。FTP 失敗、IO 讀寫失敗時會多寫紀錄——要列進人工審核 B（看得到的行為改變）。
- **檔案擁有者**：`forms/fMain.h`、`Motor/myMN200motor.cpp`、`AutoClean.cpp`、`cStateRecord.cpp` 多半是筆電或別人的檔，`TfFTP.cpp` 要確認是誰的——**動手前在 FROM_*.md §1 認領、先問擁有者**；一次改到 6～8 支檔的同一兩行，挑兩批之間的空檔，避免跟開著的 MR 撞。
- `mmoMNet`（畫面記錄框）在網頁沒有對應：這次不做；要做是另一張網頁卡。
- 行號會位移：文件／skill 裡指 `myMN200motor.cpp:2516` 的地方要跟著改。

## 6. 給 Jimmy 的選項（派工用）

- **A（建議）**：第 1＋2 步派給 St02-E（目前閒置），一張 MR，先在 §1 認領上面列的檔、問擁有者。
- **B**：只派第 1 步（讓紀錄真的落檔，改動最小），第 2 步之後再說。
- **C**：先不做，只把 §3 的過期閘理由登記進閘清單。

> 1007 11:1x 筆電選 **A** ⇒ 卡 W-143（St02-E，排在 W-142 之後）。

## 7. 其他 TfMain 紀錄物件——比照辦理（Steven 1007 12:0x「還有其他很多個類似的 log…能把 log 從 main 拆開改成獨立 cpp 的，就這樣做」）

golden `main.h` 有 **20 個** `TMyStringList *` 紀錄物件（V906 在 `forms/fMain.h`，由 `LogObjects.cpp` 的 `W906_CreateLogObjects()` 建，W7＝A 0927）。下表是 **main.cpp 以外**直接用 `fMain->slXxx` 的地方（快速審查，ST02-M `log_audit.py`／`log_audit2.py`）：

| 紀錄物件 | 現成的包裝函式 | golden 在 main 外的使用（檔:次） | V906 使用 |
|---|---|---|---|
| slMNetLog | `MNetLog()`（myMN200motor.cpp） | myMN200motor:1 | ＝本提案 §1～§6（W-143） |
| slHeaterLog | `HeaterLog()`（cpublic.cpp） | cpublic:2 | cpublic:3、fDTME08.h:1 |
| slIndexYMaxMinShift | `LogIndexMaxMinPos()`（cpublic.cpp） | mymotor:1、cpublic:3 | cpublic:3 |
| slTTLLog | — | cpublic:1 | cpublic:1 |
| slAutoSiteMapLog | — | ainarm_SearchPickPlate:3、ainarm_SearchPlacePlate:1、uhome:1 | 同 golden |
| slTriTempDoorlog | — | TempCtrl/TriTemp:26 | 只有 LogObjects 建 |
| slDewPointLog | — | TempCtrl/TriTemp:2 | — |
| slInputDataLog／slLotInfolog／slSocketIdProductData | — | uLotInfo 各 2 | forms/fLotInfo |
| slProdRecordLog／slTimeData | — | cMyDB 各 2 | cMyDB 各 2（slTimeData 另 ElaOee.h） |
| slRecordRunState | — | HS_Function:1 | fHS.h:1 |
| slTestLog | — | cObserver:1、cprod:2 | 同＋fObserver.h |
| slTorqueLog／slTorqueLogNew | — | BarCode:4、OCR:2、atester:2、rs232:3／rs232:2 | rs232 |
| slJamAlarmLog | — | note:2 | — |
| slQtyLog、slTorquMUClog、slUploadFile | — | 只在 main.cpp | LogObjects／rs232 |

**比照辦理的做法（建議，等 Jimmy 派工）**
1. 物件照舊由 `W906_CreateLogObjects()` 建、掛在 `fMain`（main.cpp 自己的程式照 golden 不動）。
2. 新增 `LogObjects.h`：每個紀錄物件一個小取用函式（例 `TMyStringList* W906_HeaterLogObj()`），本體在 `LogObjects.cpp`（只有它 include `fMain.h`）。
3. main.cpp 以外的呼叫端，把 `fMain->slXxx->` 同一行換成 `W906_XxxLogObj()->`（行數不變），include `LogObjects.h`；**該檔若除了紀錄之外不再用 fMain，就拿掉 fMain.h 的 include**（實際省幾個在派工時逐檔量，golden 那邊大多數呼叫端還有別的 fMain 用途）。
4. 現成的包裝函式（HeaterLog、LogIndexMaxMinPos、MNetLog）搬到同一個模組或各自獨立 cpp，呼叫端不變。
5. 一個物件一個 commit 或一小組一張 MR；每張 ctest：沙盒 `W906_HT9045LOG_ROOT`、建 LogObjects 後寫一行看檔、反向。

## 8. 待辦：改成「不顯示」的紀錄（之後換位置顯示）

Steven 1007 12:0x：「把這些改成不顯示的 log，要註記起來，之後再換個位置顯示，列成 todo」。golden 主畫面（`main.h`）有 4 個記錄框（TMemo）被紀錄寫入；V906 沒有 VCL 畫面，先**只寫檔、不顯示**，程式原位一律加 `// TODO(W906-LOGVIEW): golden 顯示在 fMain-><框名>（主畫面）；V906 先不顯示，之後換位置顯示（skill hpi-mnetlog-split §8）`。

| # | 記錄框 | golden 寫入處 | 內容 | V906 現在 | 之後放哪（待定） |
|---|---|---|---|---|---|
| L1 | `mmoMNet` | Motor/myMN200motor.cpp:2151-2153（MNetLog） | MNet／IO／FTP 錯誤訊息，超過 5000 行清空 | 整段閘住 ⇒ W-143 改成不顯示＋TODO | 待定（網頁紀錄頁？） |
| L2 | `mmo1`（主畫面） | atester.cpp:2037-2077（TTL 測試資料）、main.cpp:17130-17135（超過 20 行清空） | 測試機 TTL 收發資料 | atester.cpp 有 5 處參照，狀態待查 | 待定 |
| L3 | `mmoTrayStepMotor` | Motor/TrayStepMotor.cpp:546-550（收到的資料，超過 1024 行清空）；main.cpp:9405 接上、:11970-11972 解除 | 盤步進馬達通訊 | 待查 | 待定 |
| L4 | `mmoVibrationMotor` | main.cpp:33976-33982（滿 1000 行先 **SaveToFile** 再清空） | 振動馬達紀錄 | 待查 | 待定——**注意：這個框本身就是落檔緩衝**，不顯示時要保留存檔，不能整段拿掉 |
| L5 | `MemoIndexPosLog`（主畫面 IndexArmYPos 頁，golden 0618 main.dfm:15626） | main.cpp:33649-33662（TfMain::AddIndexPosLog；LogIndexMaxMinPos 與 mymotor 的 InitialMaxMinValue／RecordIndexPosition 都寫它） | Index Y 指令／編碼器／教導位置的最大最小值 | W-150 第 2 片：`IndexPosLog.cpp` 的 W906_AddIndexPosLog 改成不顯示＋TODO | 待定——**跟 L4 一樣，這個框本身就是落檔緩衝**（bSave 或超過 1024 行就 SaveToFile 到 IndexPos\YYYY\MM_IndexPosLog\Z1UpZ2Down_*.logs），緩衝與存檔照留 |

（`fMotorTest->mmo1`〔mymotor.cpp:2941、uMotorTest.cpp:2075〕是馬達測試頁自己的框，不是主畫面，不在此表。）

## 9. W-150 第 1 批（20261007，St02-E，MR 見 FROM_STEVEN）

照 §7 做了 6 個物件：`LogObjects.h:27` 加取用函式 `W906_TimeDataLogObj`／`W906_ProdRecordLogObj`／`W906_TestLogObj`／`W906_JamAlarmLogObj`／`W906_TorqueLogObj`／`W906_TorqueLogNewObj`（本體 `LogObjects.cpp` 檔尾；W906_CreateLogObjects 之前與 ctest 裡回 nullptr）。

| 物件 | main.cpp 以外的呼叫（golden 0618 → 移植樹） | 這批做了什麼 |
|---|---|---|
| slTimeData | cMyDB.cpp:491-492 → cMyDB.cpp:650-653 | 同一行換成取用函式（行為不變） |
| slProdRecordLog | cMyDB.cpp:559-560 → cMyDB.cpp:729-732 | 同上 |
| slTestLog | cObserver.cpp:2228 → cObserver.cpp:3193；cprod.cpp:2905-2912 → cprod.cpp:3066-3075 | 兩個過期閘照 golden 打開（JSCK OEE，CC_SCK／[N28]）；cprod 的路徑加 W58 Q5 `W906_SimNetPathBlocked`——模擬版不套網路路徑（SimNet/SimNetMask.cpp:79「N28＝本機紀錄」的前提由這道守門補上） |
| slTorqueLog | rs232.cpp:1783 → rs232.cpp:868 | 註解掉的那句照 golden 打開（國際牌 RS-232 >2000 位元組異常封包） |
| slTorqueLog（其他）／slTorqueLogNew | BarCode.cpp:1780/1856/1927/2003、OCR.cpp:621/1365、rs232.cpp:3250/3899、:4513/:4599 | **呼叫端還沒翻**（Barcode_1..4ReceiveData、CommOcr*ReceiveData、Comm4／cmATC ReceiveData、TimerHPCardTimer）——只有取用函式，等那些函式翻進來直接用 |
| slJamAlarmLog | note.cpp:6777（TfNote::SaveErrEventLog） | 呼叫端還閘著（forms/fNote_ShowError.cpp N-7）——只有取用函式 |
| atester 的兩處 | atester | 留給 Frank01 那一批 |

- 這 6 個在 golden 都只寫檔、不顯示在主畫面 ⇒ 沒有 §8 的 TODO(W906-LOGVIEW)。
- include 省不掉：cMyDB 40、cObserver 14、cprod 12、rs232 53 處其他 fMain->（跟 §2 的判斷一樣）。
- 坑：單獨編 cMyDB.cpp／cprod.cpp 的測試（test_ga1_cmydb、test_ga1_cprod）要在舊空行補取用函式的替身；cprod.cpp、rs232.cpp 要另外 include `Public/MyStringList.h`（設成員／呼叫方法需要完整型別）。
- ctest `St02_W150LogSplit`：取用函式＝fMain 的物件、MyDBIProductionData 經取用函式寫進沙盒 ProductRecord、TestLog 寫檔、W58 判斷（模擬擋 UNC、出貨不擋）、cObserver／cprod／rs232 原始碼釘子＋全樹沒有活的 `fMain->sl<這 6 個>`。

## 10. W-150 第 2 片（20261008，St02-E，分支 `v906/st02-w150-logsplit2`）

盤點（main.cpp 以外還碰 fMain 紀錄物件的地方，分組）與認領全文：St02-E 的 `C:\AI_TempFile\st02e-scratch\w150\claim_s2.md`；St02-M 在 FROM_STEVEN §1 登記（8466e5cf）。

| 物件 | 這片做了什麼 |
|---|---|
| slTTLLog | golden `TTLLog`（0618 cpublic.cpp:489-512）搬成 `TTLLog.cpp`（ht9045_sm：它讀 SW[]，globals／db 看不到）；經 `W906_TTLLogObj()`。退役兩個閘：cDIOStatus.cpp:71（0618 main.cpp:24373，出貨組態才跑）、MainTimer3.cpp G15（:25493）。cpublic.cpp:669-692 的原文留著當對照（閘照舊）。golden 只寫檔 ⇒ 沒有 LOGVIEW |
| slIndexYMaxMinShift | golden `LogIndexMaxMinPos`（0618 cpublic.cpp:1582-1599）＋`TfMain::AddIndexPosLog`（main.cpp:33634-33664）搬成 `IndexPosLog.cpp`（`W906_AddIndexPosLog`，§8 L5）；經 `W906_IndexYMaxMinShiftLogObj()`。退役 cStateRecord.cpp G9（0618 main.cpp:26675）。cpublic.cpp:1818-1835 的原文留著當對照 |
| slQtyLog | MainClarnData.cpp:45 的 `fMain->slQtyLog` 換成 `W906_QtyLogObj()`（測試用的延遲建構照留） |

- **還沒做（之後的片）**：FileRW/MainClose.cpp（St01 的檔）關站表的 12160 `TTLLog("Close")`（0618 main.cpp:11642-11644）與 12446 `LogIndexMaxMinPos("Program closed")`（:11929）照舊標 missing。
- **最大／最小值暫時都是 0**：餵值的是機台端——`Motor/mymotor.cpp` 的 RecordIndexPosition（:2646 閘住）、InitialMaxMinValue／EncoderTeachingMaxMinCount（:2845-2846 空殼），只有 Galil（myGALILmotor.cpp:4395）會設；`asendic_Loader.cpp:273`／`:303` 還是 TU 內空殼。那幾支補上時直接用 `IndexPosLog.h` 的 W906_AddIndexPosLog 與 W906_IndexYMaxMinShiftLogObj。
- **其他人那一組**：Frank01——ainarm_SearchPickPlate.cpp:247／:303／:381、ainarm_SearchPlacePlate.cpp:4809、uhome.cpp:1112 的 slAutoSiteMapLog（V906 是 `TfMainSiteMapLog` 替身 forms/fMain.h:191，沒有寫檔）、atester T17 的 TTLLog×8 與 §8 L2；Ifor01——TempCtrl/TriTemp.cpp:336-343 的 W7TT_FMain 替身（slDewPointLog[3]／slTriTempDoorlog，28 處，沒有寫檔）、forms/fLotInfo.cpp:6637 自己 new 的 slLotInfolog（寫死 D:\HT9045_Log\LotInfo，沒走 W906_HT9045LOG_ROOT 沙盒）。
- ctest `St02_W150LogSplit2`：取用函式在建立前是 null、建立後＝fMain 的物件；TTLLog 只在 TTL_MODE（＝0，TestIF.iTestType 的預設值）或 "Close" 時記；LogIndexMaxMinPos 兩行進 IndexMaxMin 檔與緩衝；W906_AddIndexPosLog 的 bSave 與第 1026 行先存 1025 行；三個退役的閘是活的呼叫、cpublic 的對照原文還閘著、全樹沒有活的 `fMain->sl<這 3 個>`。
