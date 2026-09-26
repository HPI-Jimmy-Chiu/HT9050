# W2 進度表 —— 逐步解除 `#if 0`（週末計畫 `docs/WEEKEND_PLAN_20260925.md` W2）

> 規則（計畫書 W2 節）：一顆 commit 一類前提；解閘前**重跑那個閘的前提**（nm／編譯探針，不 grep）；忠實優先；
> 客戶碼專屬的閘跳過；行為變更單獨量（出貨＋模擬兩組態 gate，另外逐條比對每支 ctest 的 FAIL／PASS 子項）。
> START／PAUSE 探針照使用者 20260924 晚的規則留到清單最後。

## 驗證怎麼做（每一顆都一樣）

1. 前提重驗：真方法／欄位用 `nm --defined-only` 找到定義所在的 .obj；改完後確認呼叫端 .obj 多出對應的 `U`（真的連到真方法）。
2. 同行改：`#if 0`／`#endif` 換成 `AI(W906-W2-…)` 註解，或 `#if 0`→`#if 1`（有 `#else` 的），行數不變（其他文件的行號引用不動）。
3. worktree 增量建置兩組態 gate：失敗集合逐項等於基準（出貨 3 項、模擬 18 項）。
4. 逐條子項比對：`scratchpad/subfails.py` 抓每支測試的 FAIL 行與 PASS 數，改前改後比集合。
5. ctest 會寫 `system/machinerecord.dat` ⇒ 跑前快照 587 檔、跑後照備份還原並核對 0 變動。

⚠ 到目前為止**每一顆的子項比對都是 0 差異** —— 意思是現有 ctest **沒有走到**這些路徑，不是「測過而且對」。
行為要等清單最後的 START／PAUSE 探針與機台實跑才驗得到（機台更新包第三包 README 已寫明：第一次跑 Index 自動流程要 EastSun 在旁）。

## 解了哪些

| # | commit | 範圍 | 前提為什麼死了 | 行為會變嗎 |
|---|---|---|---|---|
| A | `a5e0db2a` | aTester_Front／Rear、csystem：IC 料況的 MoveSuckData／SetHasNullIcToNullIc／SetUnuseToNullIC 空替身與 3 個閘 | A4-6（8ff6c754）把兩套 TMyKitSuck 合一成 golden 佈局，真方法都在 | **會**：TestSuck ↔ 飛梭的格子資料搬移原本被丟掉 |
| B | `afc9e3c7` | 同兩支檔的生產紀錄、Index 計數、送氣時間紀錄 15 個閘＋2 個替身 | LastSet 替身退役、MyProductionRecord.cpp 有真本體、TMySucker 有送氣欄位 | 只多紀錄；4 個 AddErrorRecord 仍連到空替身（G-5） |
| C | `28b7d86b` | 3 個「陷阱 5」閘：Rear CopyFrom、兩個 AddIndexPickVacuum | 同 A | CopyFrom 會（黏料檢查前把 TestSocket 資料抄回來）；其餘只多紀錄 |
| D | `033358a2` | A4-6 同類 10 處：ainarm2 ClearAllError／SetNeedDestroy、asortarm SetNeedDestroy、Front／Rear bNeedCheck、Rear HasNotTestYet、CopyFrom＋SetAllToNullIC、aRotateKIT_In HAS_NO_IC、32Site 真空紀錄 | 同 A | **會**：bNeedCheck 讀回真值、asortarm 空格時把吸嘴標成要破壞（原 else 與 golden 不同）；HAS_NO_IC 目前走不到（外層 DutNum 被 tRotateShim 閘住） |
| E | `0dcc9c2c` | 翻 SetTestRunMode（golden main.cpp:1117-1338）＋ModifyTester 換成 golden 本體 | 不是閘，是 W1 普查點名的空殼：5 個活的呼叫點 | **會**：切 tester 上下線時真的改 LastSet.iTester 並重算 iTestRunMode（分 bin） |
| F | `b8d4e501` | MyProductionRecord G-5 homecoming：AddErrorRecord 空替身（aHotPlateSubstrate.cpp:942）退役、真本體解閘；common.cpp 加 `W906_PRODLOG_ROOT` ctest 接縫，tests/CMakeLists.txt 6 組 ENVIRONMENT 全帶上 | 替身與真本體同一個 archive，只是沒人換；G-5 登記自己寫明「要不要解是真的決定」——依使用者「V906 要上線、全部動作都執行」解，先放 ctest 圍堵 | **會**：30 多個 JAM／WAR 呼叫點在機台上會把該顆 IC 的位置寫進紀錄並寫一列到 `D:\HT9045_Log\Production_Log\<YYYYMM>\*.csv`（golden 行為） |
| G | `04df7785` | MyProductionRecord G-6 homecoming：SaveRecordCleanPad 空替身（aHotPlateSubstrate.cpp:944）退役、真本體解閘；common.cpp 加 `W906_CLEANPADLOG_ROOT` 接縫，6 組 ENVIRONMENT 全帶上 | 同 F | **會**：Auto Clean 的吸嘴清潔紀錄寫進 CleanPad_Log（golden 行為，含 golden 自己的「重試那一列欄位不符」bug，照留） |
| H | `eed5033b` | csystem.cpp GATE G03：Galil index 警報條件補回 `IsIndexMotorOutOfPower()==false`（golden :4007） | 函式已定義（機台端請筆電做） | 斷電時不再多跳 Index 位置錯誤訊息；HT9050 之後因第 6 條不進這一臂 |
| I | `760c13d2` | csystem.cpp GATE h4-G4：CheckNozzleEventFinish「吹氣完成才可以歸零」互鎖 | 閘註解要求與 iNozzleEvent 同一波開，A4-6 已帶進 | **會（安全互鎖恢復）**：有吸嘴在破壞中就不歸零，else 分支的 Lock 與各手臂重推活了 |

## 卡在哪些（刻意沒做）

| 項目 | 為什麼 | 下一步 |
|---|---|---|
| run.*／runmode.* 6 個 tag 的發布端 | 要動 `WebBridgeTags.cpp` 的 tag，機台端 0926 要求先不要動（INBOX 第 32 列） | 等第五份 supplement 回來 |
| tRotateShim 的 DutNum／RotationTime* 閘（cinitial 6 處、aRotateKIT_In／Out） | tRotateShim 仍是 aHotPlateSubstrate.h 的精簡替身，A4-6 沒動到它 ⇒ 前提仍成立 | 另外評估旋轉站 TRotate 的翻譯 |
| `MyDBIProcess` 2 參數 vs 3 參數（asortarm GATE(13)、mymotor B1-1） | 別的前提（cMyDB 簽名），與 A4-6 無關 | 另外量 |
| W5aG 旗標（cinitial.cpp:3926） | 決策題：值已是 true，要不要把條件式改回 golden 原文 | 等使用者 |
| 氣缸逾時警報（mycylin SetAlarm） | 等 NB2 的 HAlarm 原始碼（需求單 Q12） | — |
| PLC 安全 IO 閘 W7a-I3（cinitial InitHontechHardware） | 安全關鍵 | 佇列 |
| N1-G2e `fStartCondition->LoadCylinderLife()`（cinitial.cpp:16957，開機載入氣缸壽命預警） | 閘理由「沒有宣告、表單沒翻」已過期：`TfStartCondition::LoadCylinderLife` 已翻（cStartCondition.cpp，nm T）；但全樹**沒有 `fStartCondition` 全域實例**（forms/fStartCondition.h:272「INTEGRATION-PENDING」，Command.cpp:10250／:15084 的 `->fShow` 在閘裡）⇒ 要先決定單例怎麼接（靜態 new 會在 main 前跑建構子＝陷阱 4），而且載入的是警報相關資料 | 下一個整合題：照 fSmartDiagnostic 的做法建實例前，先量 TfStartCondition 建構子碰哪些全域 |
| START 戰役 §5.3 剩下的 E／F／G | E：ATC 來源未移植；F：fShow 等使用者回答三題（`docs/FSHOW_WEB_VISIBILITY_DESIGN.md`）；G：CheckEmployeeID 照翻會送空白憑證 —— 都是卡住或決策題（C 列 W5-B 0923 早已解，`602b7679` 對帳更正） | — |
