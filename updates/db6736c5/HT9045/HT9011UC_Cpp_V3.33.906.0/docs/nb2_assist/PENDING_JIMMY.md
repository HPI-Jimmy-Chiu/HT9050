# 待 Jimmy 總表（NB2 輔助 session 彙整，依急迫性排序）

> 這一頁只放**要你決定的事**；每一條都附出處報告（都在 `docs/nb2_assist/`）。
> NB2 不會替你做任何決定，新電腦也被告知「標『待 Jimmy』的不要自己做」。
> 最後更新：20260926 11:1x（🔴 1 件未解；🟡 13 列，其中 Q6-D1／Q8a／Q7／Q8b-J3 已處理，只剩觀察點）。

> ✅ **安全提醒（更新 08:3x）**：網頁馬達測試（W4）在 R21～R25 列的安全項，新電腦已在 `02f22d05`／`9dd66076` 補完，NB2 R26 抽查承重四條到位。剩下的是**實機驗證**：DS402 歸零方向與完成判斷、到位判斷、SHUTTLE_FLOODGATE 實際值。在 HT9050 旁試運動時要有人在場。

## 🔴 急（可能已經在機台端發生，或下一步就會踩到）

| # | 決定什麼 | 為什麼急 | 選項 | 出處 |
|---|---|---|---|---|
| ~~R58-IMC~~ ✅ | 已由 RULINGS_20260926 第 6 條裁決：ini 不改、程式裡繞（表上 MTestZ1 是 PCI1203 時記憶體當成 1），機台實作 | — | — | README R58、R63 |
| ~~R63-OOP~~ ✅ | **使用者 0926 11:1x 選 A**：和第 6 條同條件、同一顆 commit，Z2 那一項只在 Enable 時算；由機台做（csystem.cpp 在 04 包收進 main 前筆電不動；機台樹 G04 已開） | — | — | README R63、R65 |
| ~~R52-通道~~ ✅ | 已解決：main 在 `72e8d4fe` 合進 v906/nb2-assist（R30～R63），`56bbf785` 讓 night-loop 開場就 fetch 這支分支 | — | — | README R64 |
| ~~R46-COM1~~ ✅ | 已解決：`3e1aec8d` 讓 HT9050 不探測、不開 RS232 扭力埠（NB2 R64 覆核過） | — | — | README R46、R53、R64 |
| R42-量 | 請機台端只讀回報：`D:\GPIB9045\system\general.ini` 的 Model；`Gerneral.ini` 的 `ZSafePos`、`INDEX_SUCKER_TYPE`、`USE_46_SUCKER_DB／SENSOR_DB` | ⚠ 19:36 起（`212c8e1d`）開機會照 golden 讀 `ZSafePos`（筆電檔案值 50）⇒ 機台端合併之後，入料／出料臂 Z 的網頁 HOME（`WebMotorAccess.cpp:1081` case 500）和 Z 上升（`mymotor.cpp:4449/5355`）都會停在**機台檔案裡的值**，不再是 20。要先知道那個值；Model 決定 HT9050 是用哪個機種身分在跑 | 量了再決定 | README R42、Q8b／Q10 附錄 |

## 🟡 翻譯方向（決定之後新電腦才能往下做）

| # | 決定什麼 | 出處 |
|---|---|---|
| ~~R64-CYL14~~ ✅ | **使用者 0926 11:1x 選 A**：第 11 條只修 75 號（MaxCylinderItem 295→296、C_LoadRobotX 搬到 295），123～136 那 14 組不動 | README R64、R65 |
| R39-B | golden 906 自己的 4 種缺陷（V912 已修）改回 906 後要不要保留 V912 的修法：操作員鎖 A02 沒 `return`（13 條存檔路徑）、2 個 `==` 打字、次數欄位夾 0～100。A 照 906／**B 保留 V912 修法（建議）**／C 只修操作員鎖 | README R39、附錄 §4 |
| Q6-D1（✅ 已由第 38 條裁決） | 使用者：「忠於翻譯」⇒ 寫入改走獨立執行緒、DCB 套 SPComm 預設、讀取閒置時 Sleep(1)。⚠ Q6 報告的殘留風險：沒有照 SPComm 的 `ReadIntervalTimeout=100` 組包，Panasonic 的 ACK＋ENQ 若被拆成兩次交出，`ptreot==ACK && ptrenq==ENQ` 會不成立（`golden:rs232.cpp:1596/:938`），扭力寫入會一直重試。上機驗扭力時要看這一點 | README R42、Q6 附錄 §6 |
| Q6-D2 | 真機組態開埠失敗：**A 照 golden 丟例外（建議）**／B 靜默轉 SIM／C 回 false 並寫 log | 同上 |
| Q8a（✅ 新電腦 `afbcb6b8` 已開到 2560） | 已解決 HT9050 的 M108。建議補一條 CI 斷言：每支 1203 軸 `BoardID*10+Port` < 陣列大小、`Port<10`（README R47） | README R47 |
| Q8b-J1 | 開機缺 `INDEX_SUCKER_TYPE` 鍵：A 寫 1／B 寫 0 並警告／**C 照 golden 問人（建議）**。`212c8e1d` 已把開機整段閘住（泵先），這題等解閘時再決定 | Q8b 附錄 |
| Q8b-J2 | D44 泵翻完前，HandlerSys 存檔會即時設 `INDEX_SUCKER_TYPE`（`HSys.gen.inc:3579`，全樹唯一活的寫入點）：A＋C 照 golden，並通知機台端先別在負壓機上存這頁／**B 過渡期閘掉（19:5x 改建議）**。理由：`212c8e1d` 已判定「泵翻好前載入會讓 WAR1604 永久失效」並閘住開機；只閘一半會給人已經擋住的錯覺，而且任何一次存檔（不管改哪一欄）都會觸發。B 的改法是一筆 tuple，NB2 已乾跑驗證並附 patch（README R50、R54） | Q8b 附錄、README R50／R54 |
| Q8b-J3（✅ 新電腦 `212c8e1d` 已處理） | 直接讀，開機印出 `ZSafePos=`（筆電 50）。剩下的是機台的檔案值，見 🔴 R42-量 | Q8b 附錄、README R50 |
| Q10-#2 | HT9050 的 Setup 頁要不要顯示 Octal kit 勾選框：A 加／B 不加／**C 先問 EastSun（建議）** | Q10 附錄 §4 |
| Q10-#3 | 開機讀 DIO 檔：A 現在翻／**B 和 atester TTL 段的解閘綁在一起（建議）**／C 不處理 | Q10 附錄 §4 |
| Q8c | EP 比例閥翻譯（使用者已說「要」）：① EP-2 壓力寫出比照 S3 要人在場（**建議是**）② EP-1 tick 阻塞照 golden（**建議**）③ `EP_Install=0` 真機 REALLY 模式擋歸零照翻（**建議**，先確認機隊）④ DLL 執行期載入（**建議**）⑤ ADSMOD 死碼照翻放 `#if 0`（**建議**） | README R44、Q8c 附錄 §8 |
| Q11 | 扭力改走 1203（有條件可行）：J1 HT9050 沒有 Z2 怎麼處理（**建議 a：記 log 後回 1**）／J2 歸零時放開 Z1 成 300（**建議 b**）／J3 SDO 失敗訊息（**建議新寫**）／J4 見 🔴 R46-COM1／J5 `iMaxPreasure=120` 先沿用／J6 流程觸發的寫入可不可以不經站號確認；EastSun E1～E6（寫哪個物件、單位、6077h 正負號與 PDO、300 放開、位置偏差警報） | README R46、Q11 附錄 §7 |
| Q7（已由第 39 條裁決：全部加） | 第 39 條照做。只剩一個上機觀察點：TrayArm `iSafePos=49750` 在 HT9050 若教導值不在 49750 正確一側，移到 Empty 會被擋（歸零 case 1310）。被擋時再決定是否只拿掉 `mymotor.cpp:3522` 這一處 | README R45 |
| R35 | golden 引擎 500 ms 一拍（B13）→ 氣缸每步 1.5～5.6 s。A 維持／**B 照 golden 自己的執行緒（建議）**／C 10～20 ms | README R35 |

## 📌 新電腦夜間報告 §0

第 1～6 件使用者都已回覆（見下方「已裁決」兩批）。

* **第 7 件（氣缸常數 75 撞號）**：NB2 R51 量過。這組氣缸在 HT9045.exe 裡沒有任何程式會動到（唯一的使用者 `MR\acatchcassette.cpp` 沒編進任何專案）⇒ **建議 A，不用查機隊**。

## 🟢 流程／文件

| # | 決定什麼 | 出處 |
|---|---|---|

---

### 使用者 20260925 13:3x 裁決（NB2 轉達給新電腦，請記進 `docs/RULINGS_20260925.md`）

| # | 事項 | 使用者原話 | 誰做 |
|---|---|---|---|
| R32 | S12 三支產生器（gen_editlist／gen_formbridge／gen_sjson）與 editlist 設定檔的 golden 仍指 V912，要不要照 §15 改回 906 | 「R32 那題選 A，照 §15 全部改回 906」 | 新電腦：照 README R32 的改法、驗法做。換算表在 `RD5軟體_NB2量測_S12產生器golden仍指V912_改回906換算表_20260925_123539.md` |

### 使用者 20260925 10:1x 裁決（第二批；NB2 轉達給新電腦，見 README R29）

> ⚠ **單一出處是 `docs/RULINGS_20260925.md`**（新電腦記錄，使用者 10:3x／11:2x 當面回覆，比本表晚）。衝突時以它為準：開機權限照 golden Operator（§4）、YES/NO 照 golden 跳網頁框（§10）、uPadInterface 延後（§12）。下表對應三列已作廢。

| # | 事項 | 使用者原話 | 誰做 |
|---|---|---|---|
| 2 | 確認對話框替身自動回答 | 「先維持自動，未來改善，我現在首要目標是先讓機台運作起來」 | 維持現狀（回 0＝自動同意）；網頁是否通道列為日後改善 |
| 5（夜 §0-5） | 1203 馬達 Direction 慣例 | 「方向交給驅動器（EastSun 的做法）」 | 新電腦：選 B（不看 Mot_Table Direction，方向歸驅動器 Pn000；HT9050 馬達表 Direction 改 0 屬機台檔，照規矩先備份） |
| 14 | EP 電控比例閥子系統 | 「要」 | 新電腦：排進翻譯 |
| 3 | TfMain 建構子沒讀的機台鍵（INDEX_SUCKER_TYPE／EP_Install／ZSafePos／INOUT_ARM_PICKER_USE_MOTOR…） | 「要翻」 | 新電腦：翻（R14 提醒：D44 泵在前，否則 WAR1604 會被 bIndexCheck1 永久關掉） |
| 9（夜 §0-3） | RS232 實體面板（uPadInterface） | 「web畫面需要有，可先從steven提供的參考…如果缺少web都要告知」 | 新電腦：C++ 照翻＋網頁畫面先用 Steven 的參考；缺的網頁畫面列進「Steven 缺件清單」（通知方式見 `STEVEN_NOTIFY_PROTOCOL.md`） |
| 夜 §0-2 | 開發期權限一律最高（真機組態） | 「目前先這樣處理，我們會手動告知關閉」 | 新電腦：維持現行作法，上線前由使用者手動告知關閉 |
| 7 | M108 MCCDY MotorID 1080 > `m_Axishand[999]` | 「依據建議」 | 新電腦：把陣列開大（確定是越界錯誤，≥90% 修正，寫明偏離 golden） |
| 8 | HT9050 1203 IO 位址超出 TLaneIO 維度 | 「依據建議」 | 新電腦：維度開大（golden 沒有 1203，不算偏離） |
| 夜 §0-6 | tech.dat 寫不寫 | 「不寫」 | 已是現狀（`4ea858da`） |
| 夜 §0-1 | 共用區 7z 密碼字串散在版本庫 | 「依據新電腦建議 A」 | 新電腦：照 A |
| 夜 §0-4 附帶 | 寫信給 EastSun | 「不用」 | 不寄 |
| 15 | `dfm2rc_idempotent` 算不算常駐失敗 | 「讓新電腦查」 | 新電腦：查它為什麼又失敗 |
| 16 | CLAUDE.md「這台筆電 ⇒ HAVE_PCI1203 關」 | 「依據建議」 | 新電腦：改寫成「沒裝 Advantech SDK 的機器」（CLAUDE.md 不是 NB2 的檔） |
| 17 | NB2 實跑一次 wb_serve 量 COLD-1 | 「好」 | ✅ NB2 R30 已跑：沒卡時 0.08 秒就返回、不會等 90 秒；真實檔 44 個 snap／check／restore／drop 完成 |
| 18 | 通知 JerryYang cOffSet 活 ODR 已被 W3-8 解掉 | 「順便CC Steven，寄信通知」 | NB2：擬信→使用者確認收件人與內容→寄出 |
| 5／10／11／12／13（C 類） | D44 泵放哪、三個閘、OCRInsp 替身、SckArtState、JerryYang commit 兩個矛盾 | 「都由新電腦判斷」 | 新電腦：照「忠於 golden；≥90% 才修並寫明」自行判斷 |

### 使用者 20260925 09:4x 裁決（NB2 轉達給新電腦，見 README R28）

* **第 1 條 HT9050 要不要當 HT9046 家族** → 使用者：「**他的確是HT9046家族，透過machine type來分類**」。
  ⇒ 新電腦夜間報告 §0 第 4 件＝選 A。NB2 的理解（R18 的三類）：凡是分派裡列了 `Type_HT9046` 的都加 `Type_HT9050`（G 類 7 處、H 類 4 處）；
  只給 `Type_HT9046_LS`／`Type_HT1032`、**刻意不含 HT9046** 的 L 類 4 處（Y-latch、START 8 picker）不加。若使用者的意思不同，以使用者為準。
* **第 4 條 Index Z 扭力上限在出貨組態沒寫進驅動器** → 使用者：「**要對齊原BCB6版本做法**」。
  ⇒ `atester.cpp:6126`（Arm 0）／`:6757`（Arm 1）的 `COM2->iWriteAndCheckMotorTorque` 照 golden 翻成真的 RS232 寫入，不再用回 1 的替身。
* **1b V912 的 TECH 欄位搬家** → 使用者：「**你的協助任務範圍是針對906 C++部分，其他版號不要處理**」。⇒ 從本表移除。
  906 這邊的 tech.dat 已由新電腦 `4ea858da` 照 R24 改成一律不寫（夜間報告 §0 第 6 件仍待使用者回 A／B／C）。
* **第 19 條 sgTimeData 11／10 欄** → 新電腦 `feacf1a8` 已回到 golden 的 10 欄，移除。

### 已經被處理掉的（新電腦 20260925 採用）

* ~~第 6 條 W4 MotorTest 四個衝突（JOG／HOME／SetPos(0)／SetExtDrive）~~ → **本來就不該列**：使用者 20260924 晚的通則「控制精神和底層必須按照 Eastsun 的」（`docs/WEEKEND_PLAN_20260925.md:52`，`e335c78f` 18:49）已裁決，NB2 R1（21:0x）沒讀到。新電腦 `3878bcc9` 照 EastSun 做（`docs/W4_PROGRESS.md` §3）。見 R19 `RD5軟體_NB2覆核_W4-a_3878bcc9_MotorTest停止伺服_20260925_050258.md` §2

* R12 RA-01 的吸嘴清零（SetMyKitSuckItemAmount 2×8 分支加 HT9050）→ `bd90b948`（04:09，附有修／沒修對照組）

* Q5 的「先開 n4-1 再開 47 個還原閘」→ `8445ed2f`、`c22dcb12`
* R3 RJ-03 開機讀檔順序 → `93042c72`（Bin 的位置還沒修，見 R12 RB-01）
* Q4 的 W3-10 預期值 → `39f3a04d`
