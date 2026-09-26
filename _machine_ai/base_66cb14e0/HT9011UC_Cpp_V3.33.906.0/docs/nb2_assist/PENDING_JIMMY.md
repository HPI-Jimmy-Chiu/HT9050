# 待 Jimmy 總表（NB2 輔助 session 彙整，依急迫性排序）

> 這一頁只放**要你決定的事**；每一條都附出處報告（都在 `docs/nb2_assist/`）。
> NB2 不會替你做任何決定，新電腦也被告知「標『待 Jimmy』的不要自己做」。
> 最後更新：20260925 09:4x（使用者裁決三件，見「已裁決」）。

> ✅ **安全提醒（更新 08:3x）**：網頁馬達測試（W4）在 R21～R25 列的安全項，新電腦已在 `02f22d05`／`9dd66076` 補完，NB2 R26 抽查承重四條到位。剩下的是**實機驗證**：DS402 歸零方向與完成判斷、到位判斷、SHUTTLE_FLOODGATE 實際值。在 HT9050 旁試運動時要有人在場。

## 🔴 急（可能已經在機台端發生，或下一步就會踩到）

| # | 決定什麼 | 為什麼急 | 選項 | 出處 |
|---|---|---|---|---|
| 2 | **確認對話框的替身要回 0 還是 2** | 回 0（今天）⇒「確定要儲存測距數值？」「Initial Start???」「Check bin setting?」**被自動同意**（其中一題會直接存 Shuttle 測距校正值）；回 2 ⇒ 這幾題會停住，但網頁上沒有地方可以回答，`bInitialStartNeedAsk` 開著會永遠擋啟動 | 0／2／先做網頁阻塞式 YES/NO 通道（仿 `ShowErrorMessage` 泵） | R15 `RD5軟體_NB2_對話框替身自動答案普查_20260925_031414.md`；R14 S2 |

## 🟡 翻譯方向（決定之後新電腦才能往下做）

| # | 決定什麼 | 出處 |
|---|---|---|
| 3 | `INDEX_SUCKER_TYPE`、`EP_Install`、`ZSafePos`、`INOUT_ARM_PICKER_USE_MOTOR` 等 **`TfMain` 建構子的機台鍵要不要現在翻**（含 golden 缺鍵時的 MessageBox 與強制寫 1）。今天全部恆 0 ⇒ 負壓機／雙 EP 機分支全死、每台都被當汽缸 picker。⚠ 要先有 D44 泵，否則 WAR1604 會被 `bIndexCheck1` 永久關掉 | R14 S3、P2 §0.2 |
| 5 | D44 泵放在 `TfiosetviewShim`（NB2 建議）還是把全域 `fiosetview` 換成真的 `Tfiosetview`（新增 forms→sm 相依）；`TfMain::Timer1Timer` 的暫停泵在 web 架構下落在哪個執行緒 | R14 P1、§4 |
| 7 | M108 `MCCDY` 的 MotorID＝1080 超出 `m_Axishand[999]`（golden 同）：照 golden 保留、改表的 BoardID、還是改陣列大小 | R2 `RD5軟體_NB2回覆Q4_W3馬達段預勘_20260924_222022.md` |
| 8 | HT9050 的 1203 位址超出 `TLaneIO` 宣告維度（IP 最大 179 > 64、Port 31 > 4），1203 開關 `Status()` 恆常數（golden 同）：照 golden 或另設維度 | R1 覆核 R1-11 |
| 9 | uPadInterface：7 件（HT9050 的 `ControlPanelMode`、`Main232` 節拍、模擬組態會打開真 COM 埠、網頁除錯頁、手動送出、六個閘先解還是等串列埠擁有者、golden 缺陷要不要登錄） | R2 `RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md` |
| 10 | `atester_ProcessCount.cpp:830-1011`（161 行）要不要開；`ainarm2.cpp:1376` 被 TRAP 3 政策擋，是否已被 20260918 裁決取代；`csystem.cpp:16309` 照 golden（連無防護 NULL 取值）還是加防護 | R6 `RD5軟體_NB2開閘清單_主流程52閘_20260925_000516.md` §5 |
| 11 | `OCRInsp.cpp` 的 `fOCR`／`fLotInfo` 替身：要不要先把幾個判斷式轉呼叫真物件（試編 OK） | R11 `RD5軟體_NB2巨集接縫拆除清單_20260925_013855.md` |
| 12 | `W5SCKART_ACCESSFILE`：它對到的 `fSCKART.cpp:69` 本身也是替身，要先決定 `SckArtState` 由誰持有 | R8 `RD5軟體_NB2替身換真方法清單_20260925_003420.md` |
| 13 | JerryYang `8bfbab2f`：`G-TM-SCANLINE` 因會寫 `Gerneral.ini` 維持閘住，與同顆「照翻不拆」矛盾（RJ-11）；Q-4 開閘方向與 `AI(W906-T4-INIFMT)` 裁決不同（RJ-12） | R3 `RD5軟體_NB2覆核_W3-8_W3-9_8bfbab2f_20260924_230846.md` |
| 14 | EP（adam6024／APAX）子系統的翻譯範圍與排程（RS232 扭力寫入那半已裁決，見下方） | R14 P3 |

## 📌 新電腦夜間報告 §0 還在等你的（不在本表重複，只列指標）

`HT9011UC_Cpp_V3.33.906.0/docs/NIGHT_REPORT.md` §0：第 1 件（7z 密碼字串散在版本庫）、第 2 件（開發期權限一律最高在真機組態怎麼落實）、
第 3 件（RS232 實體面板要不要現在翻；同本表第 9 條）、第 5 件（1203 馬達 Direction 慣例）、第 6 件（tech.dat 要不要寫）、第 4 件附帶的「要不要寫信給 EastSun」。

## 🟢 流程／文件

| # | 決定什麼 | 出處 |
|---|---|---|
| 15 | **`dfm2rc_idempotent` 到底算不算常駐失敗**：CLAUDE.md 20260917 說不算，但 W3 系列已有 **4 顆** commit 把它列進「與基準相同」的失敗集合 | R1 R1-10、R3 RB-12、R13 RM-10 |
| 16 | CLAUDE.md「這台筆電 ⇒ `HAVE_PCI1203` 關 ⇒ not linked」只對沒裝 SDK 的 JIMMYCHIU-NB 成立；NB2 裝了 SDK，建出來的 wb_serve 是武裝的。要不要改寫成「沒裝 SDK 的機器」 | R1.5 `RD5軟體_NB2_ODR普查_20260924_212746.md` §3 |
| 17 | 要不要讓 NB2 實跑一次 wb_serve，量「有 SDK、沒卡」時 COLD-1 的開機延遲（照備份 → 驗證 → 刪備份）。NB2 在輔助角色下不跑 wb_serve | 同上 |
| 18 | `cOffSet.cpp:63` 的活 ODR（JerryYang `8bfbab2f` 加的）已被 W3-8 合一順帶解掉；要不要通知 JerryYang | R1 `RD5軟體_NB2預勘_W3吸嘴段_20260924_210127.md` §A |

---

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
