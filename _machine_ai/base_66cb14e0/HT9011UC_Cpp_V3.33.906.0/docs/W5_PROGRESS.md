# W5 Teach 畫面完善化 —— 進度

> AI(W906-W5-TEACH) 20260925 起。使用者 20260924 晚：「MotorTest畫面功能完善化後，接著要把Teach畫面完善化，可存讀參數，
> 這些參數也必須和C++裡的變數是同步的」。做法定案（週末計畫 §0.6）：「**採用 Steven 的 C 路**（golden 表單橋，05f2695b），不另外發明」。
> 規格：`.claude/skills/ht9045-json-bridge/references/write-inventory.md` 一之二（具名替身）、§四「Teach | teach.ini | D（TECH_* SaveToFile）＋C（elTeach）… → Teach.cpp」。

## §1 分兩步

| 步 | 內容 | 狀態 |
|---|---|---|
| **W5-a** | 存讀參數：WS `editlist.get`／`editlist.save` tag=`Teach` → `FileRW/Teach.cpp`（golden `TfTeach` 的 FormShow 資料部分＋`btnSaveClick`）。網頁 `HW.teach.html` 改走 C 路 | ✅ 本檔同一顆 commit |
| W5-b | 教導頁的運動鈕（Set／Go／SetTo／jog／move／home）接進 W4 的 `motor.access`，照 golden `uteach.cpp` 的互鎖（`CheckCanMove`、`IsCanQuickJogMove`、軟體極限） | 下一步 |

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
