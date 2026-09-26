# W5 Teach 畫面完善化 —— 進度

> AI(W906-W5-TEACH) 20260925 起。使用者 20260924 晚：「MotorTest畫面功能完善化後，接著要把Teach畫面完善化，可存讀參數，
> 這些參數也必須和C++裡的變數是同步的」。做法定案（週末計畫 §0.6）：「**採用 Steven 的 C 路**（golden 表單橋，05f2695b），不另外發明」。
> 規格：`.claude/skills/ht9045-json-bridge/references/write-inventory.md` 一之二（具名替身）、§四「Teach | teach.ini | D（TECH_* SaveToFile）＋C（elTeach）… → Teach.cpp」。

## §1 分兩步

| 步 | 內容 | 狀態 |
|---|---|---|
| **W5-a** | 存讀參數：WS `editlist.get`／`editlist.save` tag=`Teach` → `FileRW/Teach.cpp`（golden `TfTeach` 的 FormShow 資料部分＋`btnSaveClick`）。網頁 `HW.teach.html` 改走 C 路 | ✅ 本檔同一顆 commit |
| W5-b | 教導頁的運動鈕（Set／Go／SetTo／jog／move／home）接進 W4 的 `motor.access`，照 golden `uteach.cpp` 的互鎖（`CheckCanMove`、`IsCanQuickJogMove`、軟體極限） | 覆核 14 條已修（§6）；探針待主迴圈跑 |

## §2 資料從哪來、存到哪去（golden `uteach.cpp`）

| 類 | golden | 筆數 | 移植樹 |
|---|---|---|---|
| D | `TECH_PARA`（登錄表 `forms/fTeachRegistry.cpp`，TEACH-W1 產生） | 登錄表全部 | 每筆 = 一個 `Tech.*` 變數 ＋ `teach.ini` 的 `[馬達 Alias] Key`；讀 `TECH_PARA::ReadFromFile`、寫 `SaveToFile` |
| D | `TECH_TWOPARA` | 同上 | 同上（兩個變數） |
| D | `TECH_SUCKPARA`（`teInArm`／`teOutArm`／`teSortArm`，golden :285-292） | 16＋16＋2 | `[InArmZSub]`／`[OutArmZSub]`／`[SortArmZSub]` |
| C | `elTeach`（`InitialTeachEditList`，golden :3131-3358） | **203** | `FileRW/Teach.gen.inc`（`tools/gen_teach_editlist.py` 從 golden 產生；元件改成具名替身 `filerw::EL<TEdit>("TfTeach", 名稱)`，其餘逐字） |
| 另 | `Gerneral.ini [Shuttle] CHECK_RANGE／iInShtZRange`、`teach.ini` 旋轉背隙兩鍵 | 4 | `btnSaveClick`／`SaveFile` 原樣 |

**同步保證來自 golden 流程本身**：存檔 = 頁面值套進替身 → `UpdateTempTech`（替身 → `Tech.*`）→ `SaveFile`（變數 → 檔）→ `ReadFile`（檔 → 變數）→ 再鏡像到替身。
頁面重讀看到的 TECH_* 值是從 **C++ 變數**鏡像出來的（golden `ReadFromFile` 尾端 `SetEdit->Text=*Parameter`；移植樹 `SetEdit` 是 NULL，由 `MirrorToProxies` 代做）。

頁面 387 個 `sysFields`（B 路接線）**全部**被 C 路涵蓋（0 個遺漏），C 路的 ~530 個元件 id 也全部在頁面上（20260925 以腳本對 `ht9045_wire_hwteach.js`／`HW.teach.html` 量）。
多出來的是接線檔標「契約有、本機實檔沒有」的鍵 —— B 路讀不到它們，C 路從 `Tech.*` 變數給值。

## §3 改了哪些檔

| 檔 | 內容 |
|---|---|
| `FileRW/Teach.cpp`（新） | `FileRW_Teach_Boot／Page／Save`；`IC_UpdateTempTech`／`IC_SaveFile`／`IC_btnSaveClick` 逐段對 golden 行號 |
| `FileRW/Teach.gen.inc`（新，產生的） | `IC_InitialTeachEditList`（203 筆）、Key→元件對照 354 筆、吸嘴三組 |
| `tools/gen_teach_editlist.py`（新） | 產生器（讀 golden cp950；行號與筆數有 assert） |
| `tools/wb_serve.cpp` | `editlist.get／save` 收 `tag=Teach`；開機在 `FileRW_IniConfig_Boot` 之後註冊 elTeach（golden `TfTeach` 建構子，讓開機的 `ReadTechData→ReadFile` 讀得到 `Teach.*`）；`teach.ini` 的寫者只剩 C 路（B 路 `system.file.put` 回 409）。全部同行附加，行號不變 |
| `CMakeLists.txt` | wb_serve 加 `FileRW/Teach.cpp`（同行） |
| `web/page/ht9045_wire_engine.js` | `GOLDEN_BRIDGE['HW.teach.html']='Teach'`；各結構的存檔確認題（`GB_SAVE_Q`） |
| `tools/webprobe/w5_teach_probe.py`（新） | 端對端探針（§5） |

## §4 偏離與限制（逐條）

1. golden 確認框 `MessageDlg("Sure to Save? (確定要存檔?)")`（只在 `fAllMotorHome==false` 時問）→ `filerw::ELAsk`，題目用 golden 原字串；網頁一律先問操作員，答案帶給伺服器。
2. `btnSaveClick` 的 D63（Index Y 尋相）與 AOI（上下檢查座標抄到 `FrmAOI`）兩段沒有移植的相依 → 條件成立時回報 `ELTodo`（頁面會顯示「golden 還有沒做到的步驟」），不假裝做了。
3. `TECH_*::SaveToFile` 寫 `*Parameter` 不寫元件文字（使用者 20260919 裁決 A4=(b)）；本檔先 `UpdateTempTech`，結果與 golden 相同。
4. ⚠ **`tech.dat` 沒有寫**（待裁決，夜間報告 §0 第 6 件）。golden 無條件整塊寫。**TECH 的版面依版本不同**（NB2 R24，`tools/nb2_assist/struct_layout_across_trees.py`，重跑一致）：
   V899＝3792 bytes（這台的檔就是它寫的）；golden 906／移植＝3872（V899＋結尾 20 個 SortArm int，對 V899 前綴相容）；
   V912＝3872 但把 `M_In/Out_iRotateA_Backlash` 從 byte 724 搬到結尾（大小相同、第 161 個欄位起錯位 8 bytes）。
   ⚠ **更正**：第一版寫「兩邊欄位相同，差在編譯器的對齊」是錯的（我比的是 golden 906 對移植，兩者當然相同；3792 那個檔是 V899 寫的）；
   第一版的「檔案大小相同才寫」兩邊都錯（V899 的 3792 安全卻不寫；V912 的 3872 不安全卻會寫）⇒ 改成**一律不寫**並回報。
   `teach.ini` 是三個版本共同的真實來源（量產 exe 只在缺 `Update2` 鍵時才讀 `tech.dat`）。
5. `InSHZDownRange`：golden 只在 `In_Shuttle_Auto_Latch==eInSHAutoLtc` 時由 FormShow 填（`uteach.cpp:1961`）；不成立時元件是 .dfm 的空字串
   （`uteach.dfm:6394` 沒有 `Text =`，VCL 載入時不會拿 Name 當文字），存檔 `atoi("")=0` 寫進 `iInShtZRange`（`:2280`），再讀回來也是 0（`:2281`）
   —— **golden 同**，照翻。端對端實測：這台（非 Latch）存一次 → `Gerneral.ini [Shuttle] iInShtZRange` 250 → 0。
   量產 exe 在同一台機器上按存檔也是這樣，所以不是新行為；但日後改成 Latch 機台時範圍會是 0 而不是預設 250 ⇒ golden 潛在 bug，記在夜間報告 §2，不自己改（兩種改法都偏離 golden）。
6. `ReadFile` 每次都會重寫 `teach.ini`（`HTEditList::ReadEditTextFromFile` 結尾 `TMemIniFile::UpdateFile`，golden 同），`TECH_PARA::ReadFromFile` 的 `CheckAndReadIniData` 會補寫缺的鍵（golden 同）。
7. `fTeachPara.cpp` 的 `W906-TEACH-W1-AUTOCLEANPICK` 閘：前提「`Teach.*` 的載入鏈不存在」在 wb_serve 裡已不成立（開機註冊了 elTeach），
   但 ctest 與其他 exe 沒有註冊 ⇒ 在那些行程裡 `Teach.iAutoCleanPick==0` 仍是假前提，解閘會讓 ctest 寫量產 `teach.ini`。**閘維持**，理由改寫在閘註解。

## §5 驗證

**gate（20260925 06:4x–06:59）**：出貨組態 173 項 4 失敗 = 基準（`GA1_ReadGeneralIni config_db config_loaders dfm2rc_idempotent`）；
模擬組態 173 項 19 失敗，集合與 R21 那次**逐項相同**。

**端對端**（`build_b1dbg/wb_serve.exe --seconds 90 --port 8047`＋`tools/webprobe/w5_teach_probe.py`；先 `realfile_guard.py snap`，驗完 check → restore → check（39 檔 same）→ drop）：

| # | 驗什麼 | 結果 |
|---|---|---|
| 1 | 沒開頁就存 | 409「reload page」✅ |
| 2 | 開頁 | techPara 207／techTwoPara 90／techSuckPara 32／general 4／**elTeach 203** ✅ |
| 3 | 頁面值 == `teach.ini` | TECH_PARA 189 個全等；elTeach 139 個全等（double 筆畫面是 `0.000`、檔案 `0.000000`，比數值）✅ |
| 4 | 改 `setEditOutRG` 0 → 1、答「是」 | `saved:true`，問了 golden 原題，`todo` 列出 tech.dat 沒寫；`teach.ini [MOutRotateG] setEditOutRG=1` ✅ |
| 5 | 重開頁 | 顯示 1（由 C++ 變數鏡像）✅ |
| 6 | 改 2、答「否」 | `saved:false`，檔案仍 1，重開頁回 1 ✅ |
| 7 | 存回 0 | 檔案 0 ✅ |
| 8 | B 路 `system.file.put tag=teach` | 409 owned by C route ✅ |

**對真實檔的副作用**（第一輪 check，還原前量的）：

| 檔 | 變化 | 誰造成 |
|---|---|---|
| `system\teach.ini` | 98 節、鍵值**0 增 0 減 0 改**；只多了每節後一個空行（98 個） | `HTEditList::ReadEditTextFromFile` 的 `TMemIniFile::UpdateFile`＝Delphi `TMemIniFile` 存檔格式（golden 同） |
| `system\Gerneral.ini` | `[Shuttle] iInShtZRange` 250 → 0 | `btnSaveClick :2280`（§4-5，golden 同） |
| `system\tech.dat` | 不變 | §4-4（當時還是「大小相同才寫」；這台是 V899 的 3792 所以沒寫。之後改成一律不寫） |
| `config\config.ini`、`Contact.Data`、`machinerecord.dat` | 補鍵／開機寫入 | wb_serve 開機（IniConfig／配方載入／golden 開機記錄），與 W5 無關 |

## §6 W5-b：教導頁運動鈕（覆核 14 條的修正＋第二輪覆核 17 條，20260925）

> AI(W906-W5-b) 20260925。C++ 拒絕字串與 `tools/webprobe/w5_teach_probe.py` 裡原本寫「夜間報告 §0」的地方，全部改指到**這一節**。
> 第二輪覆核（R-W5B-1…8、W5B-R1…R9）的處理在各小節標「（第二輪）」；R-W5B-7 查證不成立（§6.8）。

### §6.1 使用者裁決（20260925，照做）

| # | 題目 | 裁決 | 落點 |
|---|---|---|---|
| 1 | 真機組態下教導頁運動鈕的 golden 互鎖 `IsCanQuickJogMove → CheckShuttleCanMove → ShowErrorMessage("WAR16435", K_RETRY)` | **照 golden 跳框並等待**（tick 迴圈會停到操作員回答） | `forms/fTeach.cpp` 本體不改；答得到嗎見 §6.5 |
| 2 | 怪按鈕（golden 按下去會拿「另一張清單同號教導點」） | **暫定拒絕**；選項 C＝逐顆驗證後改成動自己那一列（可能之後採用） | 單一常數 `kTeachQuirkPolicy`（`WebMotorAccess.cpp`）；清單見 §6.4 |

### §6.2 改了哪些檔

| 檔 | 內容 | 對應覆核 |
|---|---|---|
| `tools/gen_teach_editlist.py` | 第 4 段：移植樹登錄表與 golden 建構子**各自**剝註解、遞迴解析 if／else，逐列比對（含建構子最後一個引數 **Visible**）；正則法再數一次；建構子 OnClick 覆寫（`:271-274`）；FormShow Tag 覆寫（`:1518-1521`）；（第二輪）.dfm 父子關係＋`Visible／TabVisible／Enabled／Tag`，golden 全檔的 `->Visible／->TabVisible／->Enabled` 賦值（建構子與 FormShow 帶 if／for 條件，其他函式一律「執行期會變」），算出每一顆教導處理函式按鈕的「靜態確定按不到」與 .dfm Tag（`W5B_BTN`），`InitialFormOncetime`（`:5998-6014`）另外登錄的 18 顆也標出來；`--check` 模式 | W5B-1、W5B-8、R-W5B-2、R-W5B-3 |
| `WebTeachButtons.gen.inc`（產生的） | `W5B_ROW` 多一欄 vis；新增 636 列 `W5B_BTN` | 同上 |
| `FileRW/Teach.gen.inc` | **沒有變**（重產後逐位元組相同） | — |
| `tests/CMakeLists.txt` | ctest `TeachButtonsGen`（`gen_teach_editlist.py --check`）；（第二輪）註解補 W5B_BTN | W5B-1、8 |
| `WebMotorAccess.h／.cpp` | golden Tag 語意解析；照處理函式分派；`fields` 驗證；EditPtr 回報；編碼器基準；背隙方向；回傳碼。（第二輪）golden ActiveMotorIndex 回報（`activeMotor`／拒絕尾巴 `active=`）；golden 速度狀態模型 `g_goldenPct`＋頁面速度事件；按不到／未登錄按鈕的判定；拿掉手動教導的死人開關、改成「手動教導中 START 拒絕」；教導頁 teachSet 不再過同軸那一道；運轉中（SystemStart）一律不動；每個命令清 `fAllMotorHome`；HOME 抬起照 golden 抬起分支；歸零完成設 `iLastRotatorDirP`；ORG 極性未量＝不明；GetTechPos 的 `ActiveMotorIndex==-1`；GoButton020 欄位先驗；背隙值驗證 | W5B-2…14；第二輪 R-W5B-1／2／3／4／5／6／8、W5B-R1…R6、R8、R9 |
| `WebMotorAccessLive.cpp` | `GoldenTeachRegistry`（產生表求值＋與 `fTeach` 逐列核對；（第二輪）帶 seq／vis 與 W5B_BTN）；`IsCanQuickJogMove` 的兩個掛鉤；Contec 分支；（第二輪）`sensorType`、`GoldenClearAllMotorHome`、`GoldenStopAll("uteach.home")`、`GoldenJog(pct<0)` 不設速度、`W906_MotorAccessStartBlocked` | W5B-2、3、6、14；第二輪 |
| `forms/fTeach.cpp／.h` | `IsCanQuickJogMove` 本體同行替換（行號不動）：SOFT_SIMULTE 的 `return true` 只在「被移動的不是真的會動的 1203 軸」時生效；1203 軸的 Z 在原點讀監看器 ORG | W5B-3 |
| `tools/wb_serve.cpp` | （第二輪）`start.run` 與告警框按 START：手動教導中**拒絕啟動**（同行改寫，行數不變）；上一版的「先結束手動教導並自動開伺服」拿掉 | W5B-R5 |
| `FileRW/Teach.cpp` | （第二輪）開頁（`FileRW_Teach_Page`＝golden FormShow 的資料部分）`fAllMotorHome=false`（golden `:1606`，同行附加） | W5B-R3 |
| `web/page/HW.teach.html` | 位置讀 `/api/struct/motor/*`；只送 C++ 載入過的整數欄位；（第二輪）按 Set／Go 不再先選 teach-access.json 第一個點的馬達、不改 motors，選哪一軸照 C++ 回的 `activeMotor`；速度事件（操作員改 edtSpeed、操作員選馬達）；結束被拒時重新查詢；查詢失敗會重試（權杖在別的分頁） | W5B-4、5、7、13；第二輪 R-W5B-1、R-W5B-4、W5B-R1、W5B-R6 |
| `tests/test_web_motor_access.cpp` | 第 (12) 段用真的產生表；（第二輪）每一條的測試（見 §6.7） | W5B-11；第二輪 |
| `tools/webprobe/w5_teach_probe.py` | [11]–[11e]；（第二輪）EditPtr 尾巴格式、死登錄改成「按不到」、新增未登錄按鈕 | 第二輪 |

### §6.3 偏離 golden 的每一處與理由

| # | golden | 這裡 | 理由 |
|---|---|---|---|
| D1 | `SetButton020Click`／`SetButton064Click` 逐軸「HomeFlag（或馬達 -1）→ 關伺服」，第 2 軸擋下時第 1 軸已經關伺服、函式直接 return（`:3541-3547`／`:3677-3684`） | 兩軸都先檢查完才開始關伺服 | ≥90% 確定 golden 在這裡是坑：留下一軸沒有伺服、沒有對話框、也不開回來 |
| D2 | `btnHomeClick`（`:2133-2198`）：電源關 → return；`CheckCanMove／IsCanQuickJogMove` 不過、`ActiveMotorIndex==-1`、磁性尺軸 → **不停**就 return；抬起：INDEX_MOTION_CARD==0 的 Index 四軸 `Gali_Command("ST")`，其他 `StopAllMotor()`；然後 `SetSpeed(1)` | 電源那一道照 golden（抬起也擋）；抬起**一律**停（不看 CheckCanMove／IsCanQuickJogMove、-1、磁性尺）。（第二輪 R-W5B-6）抬起改照 golden 抬起分支，不再借用 btnStopClick：Galil Index 軸只送 `Gali ST`；其他 `StopAllMotor()`（**沒有** btnStopClick 那個 `MOT[MTestY1].Gali_Command("ST")`）＋EastSun 監看器開的 1203 軸逐軸補停（golden 的 StopAllMotor 碰不到那些 handle，譯法同 DoStop）＋取消歸零工作；最後 `SetSpeed(1)` | 停止只會更安全；`IsCanQuickJogMove` 可能跳阻塞框，停止不能等它 |
| D3 | 怪按鈕照 Tag 讀另一張清單（§6.4：實際是讀到清單外或 row 0） | 拒絕（裁決 2，`kTeachQuirkPolicy=kTeachQuirkRefuse`） | 使用者裁決 |
| D4 | jog／MoveP／MoveN 的 999999 保護與軟體極限比的是畫面 `edtNowPosition` 的字 | 用 C++ 讀的位置（1203＝監看器 cmdPos；手動教導後的 ServoAlarmOn 軸＝編碼器，見 D8） | 畫面字串在網頁上可能是舊值或「—」 |
| D5 | Go 鈕 `iNewPos=atoi(EditPtr->Text)`：空白或非數字 → 0 照走 | 頁面只送 C 路載入過或 C++ 位置寫進來的整數欄位；C++ 缺欄位／非有限整數就拒絕。（第二輪 W5B-R8）GoButton020 兩軸的欄位在**第 1 軸動之前**一起驗（這一道本來就是移植樹自己加的，放前面不影響 golden 語意；軟體極限照 golden 逐軸）；（第二輪 W5B-R9）背隙畫面值同樣要是有限整數，目標＋背隙超出 int 就拒絕 | 沒載到值時目標是 0 |
| D6 | `GoButton020Click` 的 Enable 檢查在第二個迴圈、第 1 軸已經在走之後 | 兩軸的解析／1203 層／Enable 先過才動 | 動不了的軸不先動另一軸 |
| D7 | `IsCanQuickJogMove` 在 SOFT_SIMULTE 建置第一行 `return true`；1203 軸的狀態讀 golden `MOT[].ScanMotorStatus()／Led[iHomeLed]` | 被移動的是「1203 且控制層可用」的軸時，SOFT_SIMULTE 也跑本體；1203 軸的「Z 在原點」讀 EastSun 監看器樣本的 motionIO ORG 位元；沒樣本／命令後還沒新樣本＝不在原點；Mot_Table Enable=0＝在原點。（第二輪 W5B-R2）**ORG 極性未量之前一律「不明＝不在原點」**，見 D14 | W5B-3 |
| D8 | `TMyMotor::ServoOnOff(true)` 在 PServoAlarmOn 的軸上 `MySleep(200)`＋`PCIL132_ResetPos` | 不發明驅動呼叫（EastSun `Pci1203Control.h:222`：`Acm_AxSetCmdPosition` 刻意不做）；改成記下這一軸，之後以編碼器為基準 | W5B-6 |
| D9 | `fTeachShow->ShowModal()` 擋住整個畫面 | （第二輪 W5B-R5／R6 改寫）**沒有死人開關**：連線斷掉、重新整理、權杖被收回、任何告警（含 kcode==0 的通知）都**不**結束手動教導、**不**自動開伺服（golden 的對話框也不會自己關，伺服只在操作員按確定／取消後開 `:3393-3396`）；重新載入的頁面用 `teachSet {query:true}` 把對話框叫回來（查詢被拒會每 3 秒重試，最多 5 次）；**手動教導中 START（`start.run` 與告警框的 START）一律拒絕**（golden：ShowModal 開著時畫面上的 START 按不到；網頁上擋不住別的分頁，只能擋在 C++）；教導頁除了確定／取消與查詢都擋；MotorTest 對同一軸的按鈕也擋；教導頁自己的確定／取消**不**再被「同軸」那一道擋（W5B-R1 的 blocker）；停止永遠不擋 | 上一版的死人開關會在操作員的手還推著軸時自動開伺服，而開伺服會不會把軸拉回舊命令位置（Q3）還沒量 |
| D10 | 教導頁 HOME 對 Direction=1 的軸不特別處理 | 同 W4 uMotorTest：HOME 不用位置，Direction=1 不擋；只有臂 Z（歸零後要絕對移動到 ZSafePos）擋 | R21 safety 摘要寫「Direction=1 拒絕」不涵蓋 HOME |
| D11 | （第二輪 W5B-R4）`sbTeachingClick`（main.cpp:27827）進頁時 `if(SystemStart) return;`，教導頁／MotorTest 的按鈕本身不再看 SystemStart | 每一個 `motor.access` 都看：運轉中一律拒絕（停止、ui、teachSet 查詢／結束放行） | 網頁上的頁面在運轉中照樣開著（別的分頁、告警框按 START）；golden 的不變量是「運轉中這兩頁不存在」，把入口那一道搬到每個命令是它的等價 |
| D12 | （第二輪 W5B-R3）golden 進教導頁 FormShow `:1606`、離開 main.cpp:27845、MotorTest 關閉 `:2439` 都 `fAllMotorHome=false` | 教導頁 C 路開頁（`FileRW_Teach_Page`）清；另外每一個教導頁／MotorTest 的 `motor.access` 都清 | 網頁看不到「開頁／關頁」；golden 在 MotorTest 只能從教導頁進，清的時機比 golden 晚（第一個命令）但效果一樣：之後的 START 會先 Home by Start（WebStart.cpp `fAllMotorHome==false` → `Home("Home by Start")` → return false） |
| D13 | （第二輪 R-W5B-2）golden 的按鈕是否看得見由建構子 Visible 引數、.dfm、FormShow 約 600 行依組態的 Visible 設定決定 | 只判「靜態確定按不到」（29 顆：28 顆在 128 種組合都按不到、`btnSetSortZSafeHeight` 在沒裝分類臂的 64 種組合按不到；包括建構子 Visible=false 登錄的 14 顆、.dfm 隱藏的父物件從沒被設回 true（`pnlInPicker_Go`、`Panel16`、`grpScannerAOI_Pick`、`pnlOutWaitRobot`）、FormShow 無條件隱藏的 `Set／GoButton070、071`、死登錄 202／203、.dfm 隱藏的 OCR 兩顆）→ 拒絕「golden 按不到」；**依組態才決定的可見性沒有模擬**（當成按得到） | 要模擬就得把 FormShow 的條件式（含 `fMain->cInplace->…`、區域變數、`grpAOICtrl->Controls[i]` 迴圈）翻進來；待決 Q7 |
| D14 | （第二輪 W5B-R2）golden InitMotor（myEthercatmotor.cpp:252）依 SensorType 設 `CFG_AxOrgLogic`，ScanMotorStatus 的 `Led[iHomeLed]` 直接讀 ORG 位元（`:868`「because set CFG_AxOrgLogic by SensorType」） | EastSun 監看器開軸沒有設這個屬性 ⇒ 位元的極性是卡片當下的值。`kPci1203CardOrgLogic`（`WebMotorAccess.cpp`）＝在機台上量到的卡片值，**-1＝未量 ⇒ 1203 軸的「在原點」一律不明＝不在原點**（fail-closed）；量到後：卡片值 == golden 的 `SensorType ? 0 : 1` 位元照讀，不同就反相。拒絕訊息會說「原點狀態不明」而不是只有 golden 的「請先讓 Z 軸在 home」 | 旁證：golden `:864-867` 被註解掉的舊寫法 `if(bSensorType) Led=Flag; else Led=!Flag;`（沒設 OrgLogic 時的讀法）顯示卡片預設下 SensorType=0 的軸位元是**反**的；HT9050 被這道互鎖讀的 Z 軸（M03／M14／M22）與 Y 軸（M01／M20）SensorType 都是 0。上一版直接讀位元＝很可能反向（fail-open） |
| D15 | （第二輪 R-W5B-4）golden 的速度是 MOT 物件的狀態：ScrollBar1／edtSpeed 改變、選馬達（UpdateMotorTeachMonitor 1%）、HOME 抬起（1%）、Go 鈕（1%／20%）、MotorTest 會設；jog（TMyMotor::JogP 不用 Speed 引數）與 MoveP／MoveN／MoveTo **不設** | 1203 軸：C++ 記 golden 的速度（`g_goldenPct`），教導頁每次 jog／移動前把這個值重送一次（值不變）；這個行程還沒設過就用 1%。非 1203 軸：golden MOT 物件自己記，jog／移動不設速度。頁面在操作員改 edtSpeed 或選馬達之後，下一個命令帶 `speedEvent` | 1203 軸在 EastSun 監看器的 handle 上，卡上的 PTP 速度會被 W4C-3 的「歸零前改成歸零速度」蓋掉，所以要重送；「還沒設過 = 1%」是 golden 選馬達時的值 |

**照 golden（不是偏離，但值得知道）**：
* 按鈕做什麼由 **golden 處理函式**決定：`SetBtnPreciserOpen／Close`（`GoButton140Click`）按下去是**移動**；`SetButtonIn*LtcSen*`（`MotorTrayXClick`）只選馬達。
* 多列按鈕照 golden Tag：`GoButton068` 動 **MTestZ2** 到 `setEdLoadCellZ2`；`SetButton064／065` 在 4 軸機台是（Y1,Y2）／（Y2,Y1），3 軸機台是（Y1,Y1）且第 2 軸跳過。
* `SetButton060-063` 是建構子 `:271-274` 覆寫成的 `SetButton140Click` —— 正常手動教導。
* `SetButton140` 對 MTrayBracketZ／MMagazine 不關伺服，但對話框之後**照樣** `ServoOnOff(true)`。
* Contec 卡且 `MotorType==0` 的單軸 `GetTechPos` 讀 `ReadPos()`。
* （第二輪 R-W5B-1）**選取的馬達跟著 golden `ActiveMotorIndex`**：GoButton140／SetButton140（含 ep1Picker 重映射）一進來就改、即使之後被拒；GoButton020 第一個迴圈跑完停在 `MotorSelect[1]`；SetButton020／064 **不改**（頁面維持原本的選取）；MotorTrayXClick 改成它的軸。頁面照 C++ 回的 `activeMotor` 改選，SetTo／jog／移動才會作用在 golden 的那一軸。
* （第二輪 R-W5B-8）SetButton020／064 的確定：頁面當時沒有選馬達（golden `ActiveMotorIndex==-1`）⇒ golden GetTechPos 第一行 return，什麼都不寫（ack `taughtNothing`）。
* （第二輪 R-W5B-5）歸零完成照 golden `TMyMotor::MotorHome` case 20（`mymotor.cpp:1721`）設 `iLastRotatorDirP=true`（旋轉站背隙補償讀它）。

### §6.4 怪按鈕清單（裁決 2 的材料；golden 實際會讀到什麼）

判準（第二輪擴充）：處理函式讀的清單（140 → `TechPara[Tag]`；020／064 → `TechTwoPara[Tag]`）≠ 設定這顆按鈕 Tag 的那一列所屬的清單；**或**這台機台沒有登錄它（golden FormShow 不設它的 Tag，Tag＝.dfm 的值，預設 0）。先排除「golden 靜態確定按不到」的（D13）。對 7 個機種條件原子的全部 128 種組合量過（`WebTeachButtons.gen.inc`）：

**A. 已登錄、讀另一張清單（6 顆，每一種組合都讀到清單外）**

| 按鈕 | golden 處理函式 | 它自己的教導點（TECH_PARA） | Tag（依機種） | golden 實際讀的 |
|---|---|---|---|---|
| `SetBtnPADView_Z`／`GoBtnPADView_Z` | Set／GoButton020Click | MOutArmZE、`setEditPADViewZ` | 102–119 | `TechTwoPara[Tag]`，清單只有 45／50 列 ⇒ **越界（未定義行為）** |
| `SetBtnBGAView_Z`／`GoBtnBGAView_Z` | Set／GoButton020Click | MOutArmZE、`setEditBGAViewZ` | 103–120 | 同上 |
| `SetBtnScannerAOI_Z`／`GoBtnScannerAOI_Z` | Set／GoButton020Click | MOutArmZE、`setEditScannerAOIZ` | 176–193 | 同上 |

（上一版列的 `SetBtnTopViewKit_Zup`／`_Z` 是建構子 `Visible=false` 登錄的 —— golden 按不到，移到 D13。）

**B. 移植樹從來沒有登錄、golden 看起來按得到（36 顆）**

| 按鈕 | golden 處理函式 | .dfm Tag | golden 實際讀的 |
|---|---|---|---|
| `SetInSmartSetupButton`／`SetOutSmartSetupButton` | SetButton020Click | 0 | `TechTwoPara[0]`＝MInArmX／MInArmY、`setEditPreciserX／Y`（關 X／Y 伺服、手推、確定把編碼器寫進 **Preciser** 的欄位） |
| `GoInSmartSetupButton`／`GoOutSmartSetupButton` | GoButton020Click | 0 | `TechTwoPara[0]`：MInArmX／MInArmY 以 20% 走到 Preciser X／Y |
| `SetInSmartSetup_ZButton`／`SetOutSmartSetup_ZButton` | SetButton140Click | 0 | `TechPara[0]`＝MInArmZA、`setEditInZSafeHeight`（關 ZA 伺服、手推、寫進 InArm Z 安全高度） |
| `GoInSmartSetup_ZButton`／`GoOutSmartSetup_ZButton` | GoButton140Click | 0 | `TechPara[0]`：MInArmZA 以 1% 走到 InArm Z 安全高度 |
| `GoBtnLoadPort1Z`～`4Z`、`GoBtnLoadPortBufferZ` | GoButton140Click | 0 | `TechPara[0]`：MInArmZA 走到 InArm Z 安全高度 |
| `SetBtnLoadPort1Z`～`4Z`、`SetBtnLoadPortBufferZ` | SetButton140Click | 1004 | `TechPara[1004]` ⇒ **越界（未定義行為）** |
| AOI 上下檢查 18 顆（`sb*65`、`Gosb*65`、`sb(Set|Seted)TopBtn*`） | Set／Go 020／140 | 0 | golden 只在 `InitialFormOncetime`（`USE_Scanner_AOI_Inspection==TopBottomInstall`）登錄它們，那時是正常教導點；移植樹沒有 FrmAOI（缺相依），一律沒登錄 ⇒ 拒絕並說明 |

**C. 依機種才登錄的（97 顆，例 `Set/GoBtnAuto1ZUp`、`Set/GoButtonInX340`、`sbAlignInZAe`、`btnGoSortArm*`）**：沒登錄的那些組合下 golden 讀 `TechPara[0]`（MInArmZA／InArm Z 安全高度）或 `TechTwoPara[0]`（MInArmX／Y／Preciser）。這些組合下 golden 多半也用 FormShow 的條件把它們藏起來（D13 沒模擬），所以實務上按不到的居多；C++ 一律當怪按鈕拒絕，訊息照實寫出 golden 會讀的那一列。

⇒ A 類與 SetBtnLoadPort*Z 在 golden 是未定義行為；B 類其餘與 C 類在 golden 是「動到別的教導點」（多半是 InArm ZA 或 InArm X／Y）。選項 C（`kTeachQuirkPolicy = kTeachQuirkOwnRow`）只對 A 類有意義（改成照它自己那一列做 SetButton140／GoButton140）；B、C 類沒有自己的教導點，兩種政策都拒絕。

### §6.5 裁決 1：WAR16435 的框答得到嗎（量過）

**答得到，沒有死鎖** —— 前提是有一個載入 `ht9045_dialog_host.js` 的頁面連著（見下面「前提」）。路徑（全部是現有程式，這次沒改）：

1. `fTeach->IsCanQuickJogMove()` → `CheckShuttleCanMove` → `ShowErrorMessage("WAR16435", K_RETRY, MMSystem)` → `canary_support.cpp:105` 呼叫 `W906_ShowErrorMessage_Hook`。
2. 這個掛鉤在 wb_serve 開機 `server.Start()` 之後**無條件**裝成 `ForwardShowErrorMessage`（`tools/wb_serve.cpp:3848-3849`）。
3. `ForwardShowErrorMessage`（`tools/wb_serve.cpp:433-656`）先照 golden 停機（`W906_AlarmStopLikeGolden`：StopAllMotor、SoftStop／SoftStart／SystemStart=false、`MotorAccessOnAlarm` 停 1203 全軸＋取消工作；（第二輪）**不再**結束手動教導），然後 `PostQuery`＋寫告警信箱，進 `for(;;){ Sleep(100); g_pumpQueue->drain(...) }` —— 在 tick 執行緒上自己排空命令佇列。
4. 命令佇列是 WebBridgeServer 的 socket 執行緒在填；`modal.answer`／`dialog.response` 免操作權杖（`WebBridgeServer.cpp:1446`）。
5. 收到回答 → `ClearQuery`、信箱退役 → `return K_RETRY` → golden `CheckShuttleCanMove` 回 false → 教導鈕照實拒絕。

**前提（第二輪 W5B-R7 補記）**：
* (a) 框要有人畫得出來：全網站只有 `web/background.html:305` 載入 `page/ht9045_dialog_host.js`（`git grep` 驗過）。**單獨打開 `HW.teach.html`（或探針直接連 WS）時沒有人畫這個框** ⇒ tick 迴圈停在等待迴圈裡，只剩 `motor.stop` 會處理，直到有人用 `modal.answer`／`dialog.response` 回答（或打開 background.html）。
* (b) `modal.answer` **不帶按鍵**（例如探針或自動化只回 `"RETRY"` 當值、沒帶 pressed）會被當成按了 START（`tools/wb_serve.cpp:620` `if (pressed.empty() || pressed == "BtnStart")`）→ `StartFromWeb("fNote::Start 1")`。也就是：自動化回答教導頁跳出的框，會嘗試讓機台開始跑。

會發生、但都是 golden 同型或既有裁決的副作用（**不是死鎖**）：

| 副作用 | 說明 |
|---|---|
| 整條 tick 迴圈停到回答為止 | MainProc、1203 監看器輪詢、tag 發布、`MotorAccessTick` 都停；停機已在第 3 步做了；等待中 `motor.stop` 照樣會處理（`wb_serve.cpp:652`） |
| 觸發它的那個 `motor.access` 要等回答後才 ack | 超過 15 秒頁面先顯示 `no ack within 15000ms` |
| 答 **START**（或 `modal.answer` 沒帶按鍵）會走 `StartFromWeb("fNote::Start 1")` | golden `TfNote::Start` → `fMain->Start`。（第二輪 W5B-R3 更正）上一版寫「golden 同」但移植樹當時沒有清 `fAllMotorHome`：golden 進教導頁就清（`:1606`），所以 golden 在這裡會 **Home by Start 然後 return false**，不會直接去生產。現在教導頁開頁與每個教導頁命令都清 ⇒ 同 golden。手動教導中按 START 另外會被拒（D9） |
| 閘門擋下之後要再按一次 | golden `CheckShuttleCanMove` 回答後仍回 false |

SOFT_SIMULTE 組態下同一個框也會出現，條件是「被移動的是真的會動的 1203 軸」（D7）。

### §6.6 待決（要使用者或機台）

| # | 事 | 為什麼要人 |
|---|---|---|
| Q1 | 怪按鈕選項 C（§6.4 A 類） | 裁決 2 還沒回；golden 沒有定義好的行為可以照翻 |
| Q2 | （第二輪改寫）1203 卡片的 `CFG_AxOrgLogic` 是多少：Z 軸停在原點與離開原點各讀一次 motionIO bit 4，再讀卡片的 `CFG_AxOrgLogic`（EastSun 監看器目前不讀這個屬性），把結果填進 `kPci1203CardOrgLogic`（0 或 1） | **在填之前，有 1203 卡的機台上教導頁凡是 golden 要檢查「Z 在原點」的移動（InArm／OutArm X／Y、飛梭、TrayArm 等）一律被擋**（fail-closed，訊息說原點狀態不明）。另一個作法是請 EastSun 監看器開軸時照 golden 設 `CFG_AxOrgLogic`（動同事的模組，要使用者決定） |
| Q3 | 手動教導結束開伺服那一刻，驅動器會不會把軸拉回推之前的命令位置（EastSun 的 SvOn 不重設命令位置，D8） | 要在機台上量。（第二輪）死人開關拿掉之後，自動開伺服只剩「操作員按確定／取消」一條路 |
| Q4 | 以編碼器為基準的清單只增不減 | 要看 Q3 的結果 |
| Q5 | 開機後改了機種條件時，產生表的求值會和開機建的 `fTeach` 登錄表對不上 ⇒ 教導鈕全部拒絕 | golden 的登錄表也是開機建一次 |
| Q6 | SOFT_SIMULTE 建置在有 IO 卡的機台上，`Cylinder[]`／`Sen[]` 讀的是不是真值 | 這次沒有機台 |
| Q7 | （第二輪）FormShow 依組態的按鈕可見性（約 600 行）沒有模擬（D13） | 網頁照 .dfm 畫出全部按鈕；golden 在沒裝那個功能的機台上會藏起來。模擬它要翻 FormShow 的條件式；或由頁面照 C++ 回報的可見性藏按鈕 |
| Q8 | （第二輪，既有的閘）golden `VerifyMotorAction`（uteach.cpp:5346-5426，csystem.cpp:16963 每拍呼叫）在教導頁／MotorTest 開著時：有馬達在動就把教導頁按鈕全部鎖住、安全門一開就 StopAllMotor —— 移植樹是 SAFETY-GATE `W906-T6-VERIFYMOT`（csystem.cpp:30114，缺相依），網頁教導頁沒有這兩道 | 翻它要 `fTeach／fMotorTest->fShow`、`btnHome->Down` 等畫面狀態的網頁等價物 |
| Q9 | （第二輪）§6.4 B／C 類（未登錄的教導鈕）要不要在頁面上藏起來 | golden 對它們的行為是「動到 row 0」或未定義；頁面藏按鈕屬於 Steven 的畫面 |

### §6.7 驗證

| 項 | 結果 |
|---|---|
| 產生器 `--check` | （第二輪）OK：312 列登錄（TechPara 260＋TechTwoPara 52）、624 顆按鈕、建構子 OnClick 覆寫 4 顆、FormShow Tag 覆寫 4 顆、條件原子 7 個；.dfm 教導處理函式按鈕 636 顆（50 顆任何組態都沒登錄、12 顆父物件／Enabled 靜態確定按不到、14 顆建構子 Visible=false）；移植樹與 golden 逐列相同（含 Visible 引數）；`FileRW/Teach.gen.inc` 逐位元組不變 |
| 單元測試 `test_web_motor_access`（靜態連結，-Wall -Wextra 0 警告） | （第二輪）309 passed／0 failed（上一輪 274；第二輪新增：golden ActiveMotorIndex 回報、速度模型（HOME 抬起後 1%、速度事件、GoButton020 後 20%、jog 不用 edtSpeed、非 1203 不設速度）、SystemStart 閘、fAllMotorHome、START 在手動教導中拒絕、斷線／告警不結束手動教導、查詢找得回來、教導頁確定鈕帶教導軸不被擋、ORG 極性真值表、歸零完成 iLastRotatorDirP、HOME 抬起不送 MTestY1 Galil ST、GetTechPos -1、GoButton020 欄位先驗、背隙 NaN／溢位、按不到的 14＋未登錄按鈕） |
| 突變測試（`build_w5b_probe/mutbuild.py`） | （第二輪）40 個突變（上一輪 20 個：18 個照新碼保留、改 pattern；deadman_off（那個功能拿掉了）與 org_always_home（改成 r2_org_polarity_ignored／r2_org_no_invert）換掉；新增 22 個對應這一輪每一條修正），40／40 讓測試失敗，每個都跑完輸出摘要（沒有當掉）；每個突變失敗 1–38 條 |
| 增量建置 | （第二輪）`build_nosimg`（出貨組態）與 `build_b1dbg`（模擬組態）的 `wb_serve` 都編過、連結成功；重編的是 `wb_serve.cpp`、`WebMotorAccess.cpp`、`WebMotorAccessLive.cpp`、`FileRW/Teach.cpp`，0 個警告；`ctest -R ^(WebMotorAccess|TeachButtonsGen)$`（build_nosimg）2／2 Passed |
| 頁面內嵌 script | `node --check` 通過 |
| 端對端探針 `w5_teach_probe.py` | **沒有跑**（這一輪不啟動 wb_serve，由主迴圈跑；跑之前先 `realfile_guard.py snap`） |
| ctest 全套 | 沒有跑（主迴圈跑） |

### §6.8 第二輪覆核查證不成立的一條

* **R-W5B-7（btnStop 被綁兩次）**：`teachBindMotionButtons()`（先跑）用 `teachOn('btnStop',…)` 綁停止鈕並設 `data-acc="1"`；`teachBindTechButtons()`（後跑，在 `teach-access.json` 載完的 `.then` 裡）對 `goButton=btnStop` 那一列找到的是**同一個元素**（`elByTitle` 找 title／`getElementById` 退路都是 `id="btnStop"` 那一顆，頁面上只有一顆），條件 `!goEl.getAttribute('data-acc')` 不成立 ⇒ **不會**再綁成教導 Go 鈕，按 STOP 不會改選馬達、也不會多送 teachGo。C++ 端 `btnStop` 仍照實拒絕（處理函式 `btnStopClick` 不是教導處理函式）。
