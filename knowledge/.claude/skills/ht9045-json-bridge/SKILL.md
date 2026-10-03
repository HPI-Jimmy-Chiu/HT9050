---
name: ht9045-json-bridge
description: >
  HT9045 V906 移植樹的 JSON ↔ C++ 結構橋接層（設定／生產／mot-io／alarm／動作／event log 六條通道＋開機配置廣播）
  的分工、API 形狀、通訊縮減規則與驗收 gate。使用者 20260923 定案：C++ 讀檔由
  JerryYang 翻譯；讀完後 struct→JSON、HTML Save 後 JSON→struct→存檔→再讀檔、
  生產數值回傳、mot/io 串流、alarm 雙向，全部由 Steven 提供 C++ function。
  型別表由 golden 的 ReadIniData/WriteIniData 產生（offsetof 由 C++ 出，Python
  不猜），實例綁定手寫，頁面投影沿用 pagewire 三元組。Use when：設計或實作
  /api/struct、struct.put、FieldDesc、Binding、TestIF_File／DeviceForm_File／
  LevelSet 的 JSON 化、IO 位元打包、alarm 事件、判斷「畫面顯示的值是不是機台在用的值」。
  關鍵字：JSON 中介層, struct→JSON, JSON→struct, /api/struct, struct.put, FieldDesc,
  Binding, offsetof, TestIF_File, DeviceForm_File, LevelSet, SYSTEM_TEST_IF,
  SYSTEM_DEVICE_FORM, LAST_LEVEL_SET, LAST_GENERAL_SET, ReadIniData, WriteIniData,
  HTEditList, SaveAllFile, DoIniDataToForm, SaveSetupFile, vclcompat, dryRun,
  snapshot patch, io.di, io.do, 位元打包, Dialog-bridge-contract, modal.answer,
  dialog.response, act.*, 動作通道, btnClearCountClick, Clarn_Data, counter.clear,
  ReadWriteIni, ReadLastSetIni, SaveLastSetIni, IniConfig, config.ini, elConfig,
  Config.Configuration.html, Config.DIOInterFaceCFG.html, cUnitConvert, DoStructUnitConvert,
  RecordProcess, NewRecordProcess, RecordChangeLogProcess, MyDBIProcess, log.event, log.tail, event log,
  machine.defines, cfg.resync, cfg.ver, 開機配置, SOFT_SIMULTE, MachineType.h,
  通訊縮減, 表⑧, FILEIO_BRIDGE_STATUS, DoIniDataToForm, /api/form, S12, 頁面改讀 C++,
  存檔後重讀, Chiller Temp, ATC.ini, iATC_MODE_TYPE, golden 表單橋, C 路, A 形狀, C 形狀,
  gen_formbridge.py, gen_editlist.py, tools/formbridge, FileRW, --only, _hand_kept.py, E031_FormBridgeFullRun, editlist.get,
  editlist.save, PageDesc, PageRegistrar, _EditPage.h, EL<T>, 具名替身, Ld_UldDelayTime,
  W906_SecurityBoot, iMaxLevelItem, GATE (SEC1), iDecimalPoint, G1 驗收, s12c_page_probe.py,
  s12_form_probe.py, _integrated.txt, GOLDEN_BRIDGE, CRouteOwner, kOwned, W906_DoReadLastData,
  KeepNewerOverlap, 多寫者, MainTempOffsetTail, 關窗尾段, blocks not hit, _find, ATKRecipeInfo,
  HT9045_TESTERCOMM, HT9045_ref 退場, ACTForm, Winway, Monitor, CfgTrayPlate, MainBoot, MainBackup, MainClose,
  MainRecord, W906_BackupSetupFileBody, W906_SaveRunModeBody, 函式指標安裝座, WebCmdGuard, busy:, W906_CMDGUARD_MS,
  同一行插入, [W906] 偏離
---


# HT9045 JSON ↔ C++ 橋接層

> **狀態（20260923 17:00）**：**S0～S7 已實作**（S0 已推送 `a165cc0`；S1～S7 於 20260923
> 完成並提交，見同日第二顆 commit）。S8～S11 未開工。
> ⚠ **今天的 C++ 全部唯讀**：`struct.put` 只到 dryRun，`dryRun=false` 一律拒絕；
> `JsonBridge/` 零個寫檔呼叫；`log.event` 的三個 golden 入口今天都不落地。配方檔不會被動到。
>
> ---
> **⚠ 狀態更新（20260923 20:30）—— 上面那兩句話從這一刻起都過期了，但不改寫原句（§十二 的格式）。**
> **S8～S11 已實作**（未 commit，留在工作區）。逐期：
> - **S8**：`prod.*` 16 個 tag（`JsonBridge/ChanProduction.cpp`）。靜止時對 patch 的貢獻
>   **實測 0 bytes**（30 秒窗口，唯一流量是既有的 `pump.*` 心跳 56×65 B）。
>   ⚠ **做法與本檔 §八 S8 那一列的字面相反**，理由見該列的更正段。
> - **S9**：`io.di`／`io.di.valid`／`io.do`／`io.do.valid`／`io.ver`／`io.di.ports`／`io.do.ports`
>   ＋ `motor.axes`／`motor.count`／`motor.ver`（`ChanIo.cpp`／`ChanMotor.cpp`）。
>   ⚠ 這台機器的 binary 沒有 `HAVE_PCI1203`，`Pci1203Monitor()` **物件在但 0 個 port**，
>   所以 `io.di`／`motor.axes` 目前**全是 null**（不是全 0 的 base64）。
>   ⚠ 512 個舊 `pci1203.di<N>`／`do<N>` **刻意沒刪**（`web/js/pci1203/view.js` 還在讀），
>   所以今天是兩套並存，線上總量反而 **多 870 bytes**；真正的減量要與 web 同一個 commit 才拿得到。
> - **S10**：`alarm.*` 7 個 tag ＋ `GET /api/struct/alarm.tail`（`ChanAlarm.cpp`）。
>   **實測跑完一次完整往返**（raise → `dialog.response{"RETRY:BtnStart"}` → clear，`active` 1→0）。
>   ⚠ 每筆事件的 `stopAllMotor` 一律 **null**：golden `note.cpp:805-808` 會 `StopAllMotor()`，
>   而移植樹的 `StopAllMotor()` 逐字是 `{}`（`aHotPlateSubstrate.cpp:1255`），true/false 都是謊話。
> - **S11**：`act.main.clarnData`＋`act.counterClear.exe`（`counter.clear` 收編為別名）。
>   `Clarn_Data` 191 行**從零逐字翻譯**（原本是空殼，§十二 R6 說對了）。
>
> ⛔ **S11 打破了上面那條「今天的 C++ 全部唯讀」**，而且是**使用者 20260923 明示裁決**的結果
> （「`InstallClarnDataBody()` 維持開機無條件武裝，照 golden 行為，不要改成旗標控制」），不是疏忽。
> 具體後果：`tools/wb_serve.cpp` 開機呼叫 `InstallClarnDataBody()` 之後，移植樹既有的
> **20 個 live `fMain->Clarn_Data()` 呼叫點**（全部 28 個，8 個在 `#if 0` 內）
> 從 no-op 變成真的會清計數並 `WriteLastDataFile()` → 寫 `D:\HT9045\system\lastdata.dat`。
> **該檔不在版控裡**，被蓋掉沒有 git 可以救；跑之前先自己備份。
> 其中 `WebStart.cpp:1352/:2355/:2367` 三筆全在**網頁 START 路徑**上且都是 live。
> 逐筆清單、量法、以及「為什麼不能用往回找最近 `#if 0` 的寫法」在 `forms/fMain.h` 的安裝座註解。
> 本 skill 記錄分工、裁決、量到的事實、API 形狀與驗收 gate，讓 Steven／JerryYang／Jimmy 三邊照同一份做。
> 移植缺口（值為 null 的真正原因）另見 `references/porting-gaps.md`。
> 數字全部來自實測（表⑧ `web/page/ScreenShots.html`、`scratchpad/gen_fileio_bridge_status.py`），
> 不是推估；重跑產生器即更新。
>
> ---
> **⚠ 狀態更新（20260924 深夜，Steven）——C 路（golden 表單橋）兩個產生器落地，五工程師平行分工進行中。**
> `tools/gen_formbridge.py`（A 形狀）的表單設定拆成 `tools/formbridge/<Class>.py`（一表單一檔 ＋ `--only`，
> 多人平行作業不衝突）；`tools/gen_editlist.py`（C 形狀）做出第二個 `HTEditList` 結構
> `Ld_UldDelayTime`，第二個以後共用新增的 `FileRW/_EditPage.h`。順帶解了一個會讓 C 路頁面整頁不可改的
> 環境缺陷（GATE (SEC1) 查表半解閘）。**這批全部進行中，尚未整合驗收**，細節與新增表單的步驟見
> `references/generators.md`；逐結構完成度見 `references/write-inventory.md`。
>
> ---
> **⚠ 狀態更新（20260926 17:xx，Steven 團隊，HEAD 8fad1522 對程式核對）**：C 路 `tools/editlist/_integrated.txt`
> **32 個結構**、24 頁在 `GOLDEN_BRIDGE`；A 形狀只剩 HotPlate（`TFTestIF` 等 `f89be4ce` 退役）；`D:\HT9045_ref` 退場、
> 產生器改讀主 repo V912（`3e0ebb92`）。§〇 速查表、§〇之二、§九 14～15、§十一 表已改成現況；被取代的舊句在
> `references/archive/SKILL_superseded.md`。**C 路逐結構總表的單一出處**：skill `ht9045-html-json` 的
> `references/route-c-golden-bridge.md` §6。
>
> ---
> **⚠ 狀態更新（20260927 02:xx，Steven 團隊 St01，HEAD 227b79db 對程式核對；原句不改寫）**：C 路 `_integrated.txt` **35 個結構**
> （＋`ACTForm`／`Winway`／`Monitor`，`217e7e5e`，都沒有頁面）；手寫 FileRW 多四支（`CfgTrayPlate`／`MainBackup`／`MainClose`／`MainRecord`），
> 以及「forms 門面 → 只編進 wb_serve 的本體」的函式指標安裝座（`generators.md` 廿七）；伺服器防連點 `WebCmdGuard`（`busy:`，
> skill `ht9045-html-json` `web-bridge-json-contract.md` §1.3）；`tools/wb_serve.cpp` 共用檔的插入慣例（`generators.md` 廿八）；
> 這一輪 St01 照建議先做的 `[W906]` 偏離列在 `porting-gaps.md` 二十二（每條附 todo ★ R 題號與 commit）。§〇 新增四列（標 🆕0927）。

---

## 〇、現況速查（20260923 21:30 skill 維護盤點加。**先讀這張表，再讀下面任何一節**）

⚠ 本檔的慣例是**更正時不改寫原句**（見 §十二），好處是可稽核，代價是
「現在到底是哪樣」散在**五層**裡。這一節是那五層的**單一結論**，只講現況，不講歷史。

| 問題 | 現況（2026-09-23 收盤；**20260924 Steven 更新的列標 🆕**） | 權威出處 |
|---|---|---|
| 🆕 頁面有在用 C++ 的值嗎 | ✅（20260926 HEAD 8fad1522）**C 路 24 頁**（`web/page/ht9045_wire_engine.js:1038-1061` `GOLDEN_BRIDGE`，`WS editlist.get`＝golden `FormShow`）＋ **A 形狀 1 頁**（`Setup.HotPlate.html`，`/api/form`＋`form.save`）；S12 第一型 `/api/form` 的 4 頁全被取代。`/api/struct` 的結構綁定仍沒有頁面用，但 IO／馬達頁用 `/api/struct/io/*`、`/api/struct/motor/*`（`HW.IoSetView.html:433`、`HW.MotorTest.html:227`） | skill `ht9045-html-json` `route-c-golden-bridge.md` §3、§6 |
| 🆕 `DoIniDataToForm()` 還要翻嗎 | ~~✅ 要~~ ⛔ **不翻（20260924 下午再裁）**：「直接使用 BCB 的原檔，設計成 JSON bridge」「不要管 cpp 版本的，直接參考 bcb 版本做成 JSON bridge + 寫檔；讀檔的部分如果沒有實作的，就列入待辦」→ **S12 第二型**：`tools/gen_formbridge.py` 從 golden 原檔產生 `WriteFile/<結構>.cpp`（一個結構一支、BCB 表單分 function）。第一型之後退場 | `decisions.md` 二之二、**二之三**；`phases.md` S12 第二型；`WriteFile/README.md` |
| 🆕 寫檔做到哪 | ✅（20260926）`tools/editlist/_integrated.txt` **~~32~~ → 35 個結構**（~~31~~ → 34 支 `gen_editlist.py`＋`Teach`；20260927 `217e7e5e`），逐頁讀寫驗收見 `write-inventory.md` 〇；還沒做的看同檔「🆕 待派佇列」。A 形狀 `TFTestIF` 已退役（`f89be4ce`），`porting-gaps.md` 十四結案 | `write-inventory.md` 〇、`route-c-golden-bridge.md` §6 |
| 🆕🆕（20260926 更新）兩個產生器分別做什麼、怎麼加一個新表單 | **新表單一律 C 形狀**：`tools/editlist/<struct>.py`（一結構一檔）→ `python tools/gen_editlist.py --only <struct>` → 手寫 `FileRW/<struct>.cpp`（`PageDesc`＋開機函式）→ 驗收過才加進 `_integrated.txt`。A 形狀 `tools/gen_formbridge.py` 只剩 `TfHotPlate`。`Teach` 用 `tools/gen_teach_editlist.py`（讀 golden **V906** 樹） | `generators.md` 二、三、廿四 |
| 🆕（20260926）加新結構時最容易踩的 | 多寫者「本頁沒改的欄位不把舊值蓋回」（S57／S90）、golden 關窗後才跑的尾段（S88）、`.py` 寫死 golden 行號會靜默對錯行、取代字串反斜線要寫四個、`ATKRecipeInfo` 是 NULL、e2e 要 `HT9045_TESTERCOMM=0` 才不起測試機引擎、開機呼叫別插在 `//` 註解後面 | `generators.md` 十七～廿四 |
| 🆕（20260926）網頁存檔會不會在運轉中改參數 | ⚠ **會**：`editlist.save`／`PageSave` 不檢查 `SystemStart`，golden 設定鈕只在停機設定模式按得到（`main.cpp:29030-29036`）。SAFETY 相關，**交 Jimmy 決定**放哪，不要自己加 | `porting-gaps.md` 二十一 |
| 🆕🆕（20260924 深夜）C 路頁面整頁不可改，先查什麼 | GATE (SEC1) 的查表半解閘了嗎（`cSecurity.cpp` 檔尾 `W906_SecurityBoot()`：`iMaxLevelItem` 對不對、有沒有在 `FileRW_IniConfig_Boot()` 之前呼叫） | `generators.md` 四、`decisions.md` 二之四 |
| 🆕🆕（20260924 深夜）C 形狀第一次存檔差幾個位元組算不算過 | 算，只要差異是「golden 正規化格式」（`iDecimalPoint=6`）或「golden 補鍵」；第二次原值存檔才要求位元組完全不變 | `generators.md` 五 |
| 🆕 S5、S6 做完了嗎 | ❌ **沒有**。S5（`LAST_GENERAL_SET`／`IniConfig`／`TfDIOFrom`）沒有型別表也沒有綁定；S6 `struct.put` 只到 dryRun（`StructApply.cpp:252` "persist not implemented yet"）。下兩列的「✅」不含這兩期 | `phases.md` ⛔更正（20260924） |
| 🆕 讀檔的值是真的嗎 | ✅ JerryYang `ca4e903` 補齊八支 `ReadFile` 並接上 `DoStructUnitConvert()`。實測 QPM5577_8：`deviceForm.file` `IndexContact=[-134,-134.28]`，`.live` `[-13400,-13428]`；三個綁定已改 `sourcePorted:true` | `porting-gaps.md` 四 結案段 |
| 🆕 開機會寫配方檔嗎 | ⚠ **會**（golden 行為）：golden 的補鍵（Contact／Tester／Temperature／Binasgn*_ART 等）。20260924 那條「`Chiller Temp` -20 被改成 5」已修（`322d68a3`，`W906_ReadATCIni` 在 `tools/wb_serve.cpp:3123`；十三 結案，未重新做開機前後 SHA256 複驗）。另外開機還會寫系統檔：`Gerneral.ini [Version] Ver` 戳記（`:3886`）、`IniConfig.bShowLotInfo` 時寫 `lastdata.dat`（`:4162`）。測試前先備份配方夾與 `system\` | `porting-gaps.md` 十三；`write-inventory.md` 二「`TfMain::FormShow`」列 |
| S0～S7 做完了嗎 | ✅ 已實作並 commit（S0 = `a165cc0`，S1～S7 同日第二顆）⛔ **20260924：S5、S6 除外**，見上 | 檔頭 |
| S8～S11 呢 | ✅ 已實作並 commit（`8deffd1`）。§十三 是它留下的 12 條待辦 | §十三 |
| 「今天的 C++ 全部唯讀」還成立嗎 | ❌ **不成立**。S11 的 `InstallClarnDataBody()` 開機無條件武裝，20 個 live 呼叫點會真的寫 `D:\HT9045\system\lastdata.dat`（該檔**不在版控**）。使用者明示裁決，不是疏忽 | 檔頭 20:30 更新段 |
| IO 縮減拿到了嗎 | ❌ **還沒**。512 個舊 `pci1203.di<N>`／`do<N>` 刻意沒刪，兩套並存，線上反而**多 870 B**。要與 `web/js/pci1203/view.js` **同一顆 commit** 才拿得到 | §六 ⛔更正、§十三 #3 |
| `io.di`／`motor.axes` 為什麼全是 null | 這台機器的 binary 沒有 `HAVE_PCI1203` ⇒ 0 個 port。**null 不是 bug，是誠實** | 檔頭 20:30 更新段 |
| Alarm 的 NonStop 分類誰做 | **HTML 做，C++ 不做。**C++ 只送事實 | §六 ⛔更正（#10 結案） |
| `alarm.stopAllMotor` 是什麼值 | ✅（20260926 核對）**恆 true**：`c3c459f2`（20260924）起告警照 golden 停機、`StopAllMotor` 兩個多載統一；每筆事件 `stopAllMotor:true`、`alarm.stopMotorPorted:true`（`JsonBridge/ChanAlarm.cpp:211`／`:185`）。舊答案「恆 null」已過期 | §十三 #5（`open-todos.md`） |
| `MyDBExecSQL` 對 null db 會當掉嗎 | ❌ **不會，一層 crash 路徑都沒有。**`bUseMDB` 短路在前；sqlite 3.7.7.1 對 NULL db 回 `SQLITE_MISUSE` | §4.7 ⛔更正（#12 結案） |
| `act.main.clarnData` 有寫進 sqlite／EventLog 嗎 | ❌ 沒有。呼叫解析到 3 參數版 → `uHGemEquipment.cpp:3483` 純轉發 → `aHotPlateSubstrate.cpp:1264` 計數空槽；`cMyDB.cpp:1000` 真本體整段在 `#if 0` | §十三 #1 結案 |
| 值是 null 的**真正**原因去哪查 | `references/porting-gaps.md`（~~12~~ → ~~13~~ → ~~第一～二十一節，20260926~~ → 第一～二十二節，20260927；附型態 A／B／C 分類與複驗指令；十、十三、十四已結案；二十二＝St01 這一輪照建議先做的 `[W906]` 偏離清單） | §十一 |
| 🆕 20260926 C 路已接的結構與頁面有哪些 | `tools/editlist/_integrated.txt` **~~32~~ → 35 個結構（20260927）**，其中 24 個有頁面（`GOLDEN_BRIDGE`）、`ContactForce` 有 `PageDesc` 但頁面還沒接、~~7~~ → 10 個沒有頁面（Rotate、AutoAlignment、FixAICCD、Magazine、AutoCalSuckZ、AOISetup、IniConfig_OCR；20260927 加 ACTForm、Winway、Monitor）；A 形狀另有 `HotPlateForm_File`。**逐結構的 tag／頁面／開機建替身／開機與換配方讀檔／檔案擁有者總表只在一處維護** | skill `ht9045-html-json` `references/route-c-golden-bridge.md` §6（HEAD 8fad1522 核對）；驗收見 `write-inventory.md` 〇；沒頁面的建頁備忘見 `pending-pages.md` |
| 🆕 20260926 S48／S49 歸誰 | Steven 20260926 09:2x～09:3x 裁決：「S48 Jimmy 在做了，我們不要做，停掉」「S49 也是 Jimmy 的工作」——**Steven 這邊不做** | CHANGES_20260926 §8.4；唯讀調查報告 `D:\docs\ops\weekly\2026\09\20260926\RD5軟體20260926_093427_S48_S49_唯讀調查報告_交給Jimmy.md` |
| 🆕 20260926 筆電上第 42 條 IO 解除為什麼測不到 | 筆電 `Gerneral.ini` `IO_CARD_TYPE=1` → `[BOOT] IO_Table rows=0、Sen named/enabled=0/40`，`sim.di.set` 找不到任何感測器名字；程式已在 `8af13c07`，但這台機器上測不到（Steven 擋下實測） | CHANGES_20260926 §11.4；CHANGES_20260926 §8.3 第 1 項 |
| 🆕0927 手寫 FileRW（不走產生器）有哪些、forms 門面怎麼叫到它們 | `MainBoot`／`MainBackup`／`MainClose`／`MainRecord`／`CfgTrayPlate`／`MainClick`／`Zteach`，全部列在 `CMakeLists.txt:3404`、只編進 `wb_serve`。forms 門面（`ht9045_forms`）要叫它們時用**函式指標安裝座**：`forms/fMain.cpp:458` 的 `W906_BackupSetupFileBody`／`W906_SaveRunModeBody`（預設 0＝原本的 no-op），wb_serve 開機 `tools/wb_serve.cpp:4111` 明確呼叫安裝函式；ctest 不裝＝no-op、不寫真檔 | skill `ht9045-html-json` `route-c-golden-bridge.md` §6 表註；`generators.md` 廿七；`write-inventory.md` 二「`TfMain` 開機／關程式／計時器的手寫 FileRW」 |
| 🆕0927 在 `tools/wb_serve.cpp` 插一行要注意什麼 | **同一行接在行尾、不移動行號**；**不要接在別人登記的行**（S85「拿不準就看 git 合併會不會撞到同幾行」；`6db687d4` 把 S113 從 Jimmy 的 `:4598` 搬到 St01 自己的 `:5953`）；那一行有 `//` 註解要插在 `//` 前面（十七） | `generators.md` 廿八、十七 |
| 🆕0927 網頁送指令回 `busy:` 是什麼 | 伺服器防連點 `WebCmdGuard`（S107-3，`2ae40ffe`）：同一個 `cmd`＋`tag`＋`value` 在上一條完成後 400 ms 內又到 → 不執行、ack `ok:false`、`error` 以 `busy:` 開頭，**不是失敗**；探針要重送同一指令設 `W906_CMDGUARD_MS=0` | skill `ht9045-html-json` `web-bridge-json-contract.md` §1.3 補註 |
| 🆕0927 這一輪 St01 照建議先做、偏離 golden 的地方 | 16 條（例 R50 沒讀到機型不裝 `SaveRunMode`、R43 S113 不看 `SystemInitialOK`、R25 DailyJamRate 一個行程只存一次；R49 `SaveRmsInfo` 空值守衛已於 `61c96910` 關掉、恢復照 golden），每條附 todo ★ 題號與 commit；Steven 可推翻 | `porting-gaps.md` 二十二 |

**本檔的更正層（知道去哪找就好，不必全讀；20260923 續拆後全文都在 `references/`，`SKILL.md` 原處只留 stub）**：
① 檔頭「狀態更新 20:30」（本檔） ② §六 ⛔更正（IO 算術、Alarm 分工）→ `references/wire-reduction.md` ③ §八 ⛔更正（null/0 區分）→ `references/phases.md`
④ §4.7 ⛔更正（MyDBExecSQL）→ `references/api-shape.md` ⑤ §十二（審查員 12 條逐條查證）→ `references/review-corrections.md` ⑥ §十三（12 條待辦）→ `references/open-todos.md`。

⚠ **章節編號不照檔案順序**（十、十二、十一、十三），且 §四 沒有 4.4。
  **不要重新編號** —— `§4.5`／`§4.6`／`§4.7`／`§六`／`§十二` 有 **10+ 處被 `.cpp`／`.h` 註解引用**
  （20260923 grep 實測），改號會讓那些引用全部變成懸空指標。

---

## 〇之二、新增一個表單的讀寫，先看這裡（20260924 深夜；20260926 Steven 團隊依現況改寫，舊版在 `references/archive/SKILL_superseded.md`）

一個結構（配方檔／設定檔）要接上「C 路」（golden 表單橋，與 B 路檔案鏡像並存，定義與**全部結構的總表**見
skill `ht9045-html-json` 的 `references/route-c-golden-bridge.md` §6），照這個順序：

1. **一律走 C 形狀**（具名替身，`tools/gen_editlist.py`）——不論 golden 是逐鍵 `WriteIniData` 還是 `HTEditList`；
   A 形狀（`tools/gen_formbridge.py`）只剩 `TfHotPlate` 在維護，`TFTestIF` 等已退役（`f89be4ce`）。都是**直接改寫
   golden BCB 原檔**（20260924 下午裁決，`references/decisions.md` 二之三）。
2. **設定**：新增 `tools/editlist/<struct>.py`（一結構一檔，給 `prefix`），行號一律用 `_find(meth, regex)`／`L(meth, text)`
   定位、不寫裸數字（`generators.md` 二十）；`python tools/gen_editlist.py --only <struct>` 產生 `FileRW/<struct>.gen.inc`
   （**不要手改**）；手寫入口 `FileRW/<struct>.cpp`（`PageDesc`＋`PageRegistrar`、開機函式、`SaveFlow`／`Reload`）。
3. **接線（五處，缺一不可）**：① `tools/wb_serve.cpp` 開機照 golden `CreateForm` 順序呼叫 `FileRW_<X>_Boot()`
   （別插在同一行的 `//` 註解後面，`generators.md` 十七）；② golden `DoReadLastData` 有讀的，照 golden 行號插進
   `W906_DoReadLastData`（開機＋換配方共用）；③ 驗收過才把名字加進 `tools/editlist/_integrated.txt`（進 build）；
   ④ `web/page/ht9045_wire_engine.js` 的 `GOLDEN_BRIDGE` 加「頁面 → 結構」；⑤ 會寫的檔加進 `CRouteOwner` 的 `kOwned`
   （擋 B 路直改）。做完把一列補進 `route-c-golden-bridge.md` §6 總表。
   ⛔ 20260927 補：①②⑤ 都在共用檔 `tools/wb_serve.cpp`——**同一行接在行尾、不移動行號，不要接在別人登記的行**（S85；`6db687d4`），
   見 `generators.md` 廿八。沒有頁面的結構（例 `ACTForm`／`Winway`／`Monitor`）④ 不用做、`PageDesc` 也不用登記（`generators.md` 廿五）。
4. **怎麼驗收**：`tools/webprobe/s12c_page_probe.py`（`--page`／`--struct`），G1 判定分兩段（第一次原值存檔只允許
   golden 正規化差異，第二次要位元組不變，`generators.md` 五）；不需要測試機通訊時啟動 `wb_serve` 前設
   `HT9045_TESTERCOMM=0`（`generators.md` 廿三）。⚠ 20260926 起 Steven 裁示暫停 build（S51），只做語法檢查、實測累積待補。
5. **先查這幾個陷阱**：多寫者（`generators.md` 十八）、golden 關窗後的尾段（十九）、取代字串反斜線（廿一）、
   `ATKRecipeInfo` NULL（廿二）、golden `TfMain` 建構子裡沒做的初始化（十三）、衍生欄位重算（十四）。
6. 若頁面元件整頁都不可改，先查 GATE (SEC1) 的查表半解閘有沒有跑（`cSecurity.cpp` 檔尾
   `W906_SecurityBoot()`）。

**完整操作步驟、產生器欄位、平行分工規則、驗收細節，全部在 `references/generators.md`**——
本節只是導覽，不要在這裡展開細節。逐結構「做到哪」見 `references/write-inventory.md`。

---

## 一、分工（使用者 20260923 定案）

### 設定相關（配方／系統檔）

| # | 動作 | 負責 | 對應 API／函式 |
|---|---|---|---|
| 1 | C++ 讀檔（檔案 → 全域結構） | **JerryYang**（翻譯 golden 的 `ReadFile()` 族） | golden 的 `TfXxx::ReadFile()`／`ReadTestMode()` 等，逐字翻譯 |
| 2 | 讀完後 struct → JSON 給 HTML | **Steven** | `ToJson(binding, projection)` → `GET /api/struct/<binding>` |
| 3 | HTML 按 Save，JSON 送到 C++ | **Steven** | `WS struct.put {binding, values, dryRun}` |
| 4 | C++ 翻譯 JSON 內容（JSON → 結構） | **Steven** | `FromJson(binding, json, dryRun) → ApplyResult` |
| 5 | 翻譯好的內容存檔 | **Steven** | `Persist(binding)`：**從結構直接寫檔**，golden ③ 的存檔鉗制搬進 `Clamp` 鉤子（見 §五） |
| 6 | 存檔後再次讀檔 | **Steven** | 呼叫 #1 的 `ReadFile()`，再 `ToJson` 回給頁面 —— 這一步同時是驗收 |

### 生產相關

| 動作 | 負責 | 對應 |
|---|---|---|
| C++ 生產數值回傳 HTML | **Steven**，提供 function | 沿用執行期 tag 串流（snapshot ＋ patch），**只在變動時送**（§六）。producer 規則見 §4.8：golden 的 `Show*()`／`Update*()`（86 個函式、599 處 Caption 賦值）每個變成一個 `Stage*()`，**結論欄位一起出**，格式字串進 schema |

### mot／io 相關

| 動作 | 負責 | 對應 |
|---|---|---|
| C++ 發 JSON 給 HTML | **Steven**，提供 function | `io.di`／`io.do` 位元打包、`motor.axes` 定序陣列（§六） |

### 動作相關（第五條通道，使用者 20260923 補：「透過畫面去清除數據的操作有加入嗎？」—— 原本沒有）

| 動作 | 負責 | 對應 |
|---|---|---|
| HTML 按鈕 → C++ 執行 golden 的事件處理器本體 → 存檔 → 再讀 → patch 回頁面 | **Steven** | `WS act.<單元>.<動作>`（§4.5）；C++ 端翻譯 golden `*Click` 的**非 VCL 部分**，VCL 刷新那幾行改成 `Reload`＋tag patch |

量到的規模（golden V912）：事件處理器 1,733 個；**527 個（30%）會改全域結構或呼叫 Clear／Reset／Save／Update／Do* 類函式**；直接落地存檔的 58 個。wb_serve 現有 14 條 WS 指令，只有 `counter.clear`（7 個 family，對應 `cCounterClear.cpp:399 spbExeClick`）是成形的動作族。

### event log 相關（第六條通道，使用者 20260923 補：「html 的操作會需要有 event log，也需要設計一個 JSON」）

| 動作 | 負責 | 對應 |
|---|---|---|
| HTML 操作 → C++ 留痕（sqlite ＋ 文字 EventLog） | **Steven** | `WS log.event {kind, msg, debug?, alarmCode?}` → C++ 呼叫 golden 的 `RecordProcess`／`NewRecordProcess`／`RecordChangeLogProcess`（§4.7） |
| C++ 事件 → HTML 顯示（Data.Observer 類頁面） | **Steven** | 既有 `GET /api/text/<root>/YYYY/MM/…` 唯讀讀檔 ＋ 新增 tag `log.tail`（最近 N 筆的環形緩衝，patch 推送） |

### alarm 相關

| 動作 | 負責 | 對應 |
|---|---|---|
| C++ 發 JSON 給 HTML | **Steven**，提供 function | 事件物件，不輪詢；沿用 `web/JSON/Dialog-bridge-contract.json` v1.3.0 |
| HTML 發給 C++ | **Steven** | **`dialog.response`**（Q30-8 已於 20260923 自行裁決，見 §二）；`modal.answer` 伺服端保留為別名，JS 端不再送 |

---

## 二、裁決紀錄（20260923，使用者）

> **全文移至 `references/decisions.md`**（20260923 續拆，逐字保留，**編號不變**；同檔另收 §十 待裁決）。
> 一句話：golden 改用 **V912**；`_NET` 不開；JSON／API／移植樹一律 **UTF-8 直通、不轉碼**；② `DoIniDataToForm()` 廢除、③ 改成「JSON→結構→寫檔→讀檔→JSON」；
> `*.live` 開唯讀；Q30-8 線上一律 **`dialog.response`**（`modal.answer` 伺服端別名）；動作第一刀 **`Clarn_Data`**；`IniConfig` 走結構但**存檔格式照原本**；
> 陣列 `GET` 稠密 `[...]`／`put` 稀疏 `{"3": v}`／二維巢狀。沿用：GL「照 golden 不偷吃步」、W906-ZEROARG、A1。
>
> **20260924（`decisions.md` 二之二）**：⛔ 推翻「② 廢除」——「`DoIniDataToForm()` 就等於是 C++ 發送 JSON 給 HTML」；
> widget↔欄位對照由 C++ 的 `DoIniDataToForm()` 決定（不由 Python/JS 抽）；`recipe.doc.put` 存檔後重跑該文件的 `ReadFile()`＋`DoStructUnitConvert()`。

---

## 三、量到的事實（改設計前先讀）

> **全文移至 `references/measured-facts.md`**（20260923 續拆，逐字保留，**編號不變**）。
> 物理順序照原檔 3.1→3.2→3.3→3.6→3.4→3.5（不是錯，不要重排）。子節索引：
>
> | 節 | 一句話 |
> |---|---|
> | §3.1 三段式 → 一段半 | golden ①`ReadFile` ②`DoIniDataToForm` ③`SaveSetupFile`；新架構 ①不變、②'`ToJson`、③'`FromJson`+`Persist`。③ 讀的是**控制項**，不能逐字翻；唯一會遺失的是存檔鉗制與值格式 |
> | §3.2 三套持久化 | ini 文字（`ReadIniData`／`WriteIniData`）、`HTEditList` 文字、二進位 blob；只認第一套會漏掉核心配方檔 |
> | §3.3 三個實例 | `_File`（畫面單位 mm）→ 執行中（馬達單位 ×100）的套用點是 `DoStructUnitConvert()`；移植樹開機被 `#if 0`／stub 擋住；橋接層只碰前兩層 |
> | §3.6 `IniConfig` ↔ `config.ini` | `ReadLastSetIni`／`SaveLastSetIni` 名不符實，主體是 `IniConfig`（`elConfig->Add()` 1,581 筆）；`HW.HandlerSys.html` 對 `LastSet` 是 0 處 |
> | §3.4 結構大小 | **剝註解後**配對：`SYSTEM_TEST_IF` **788**／`SYSTEM_TEMPERATURE` **223**／`LAST_GENERAL_SET` 387／`SYSTEM_DEVICE_FORM` 66…；權威是 `tools/gen_sjson.py` |
> | §3.5 橋接層真實狀態 | 表⑧ 第三版 57 單位、553 個讀檔欄位只有 3 個出 JSON（0.5%）；`/api/recipe` 是檔案鏡像不是結構；1203 一個 port 是**一個位元組**（`:1099`） |

---

## 四、API 形狀

> **全文移至 `references/api-shape.md`**（20260923 續拆，逐字保留，**編號不變**）。
> 含原 `references/producers-and-layout.md` 的 §4.8／§4.9 —— 該檔已整檔併入並刪除。子節索引（`.cpp`／`.h`／`.py` 註解引用的 §4.3／§4.5／§4.6／§4.7／§4.9 在那份檔裡同名同號；§四 本來就沒有 4.4）：
>
> | 節 | 一句話 |
> |---|---|
> | §4.1 核心切分 | 型別表由結構宣告產生（**`offsetof` 一律 C++ 出**）、實例綁定 `kBindings[]` 手寫（寫檔路徑綁在這裡）、頁面投影沿用 pagewire 三元組 |
> | §4.2 端點 | `GET /api/struct/`／`<binding>/schema`／`<binding>?page=&since=`＋`WS struct.put {binding, values, dryRun}` → `ApplyResult`；拒寫 **all-or-nothing** |
> | §4.3 C++ function 簽章 | `ht9045::sjson::ToJson`／`FromJson`／`Persist`／`Reload`＋`StageProduction`／`StageIo`／`StageMotor`／`EmitAlarm`（Steven 提供） |
> | §4.5 動作通道 | `act.<單元>.<動作>`；第一刀 **`act.main.clarnData {tag:0..12, msg, dryRun}`**（`Clarn_Data` 191 行、46 個 golden 呼叫點）；5 條規則：一處理器一條、**守衛一律回報**、dryRun 只跑守衛、副作用照 golden 叫並標 `sideEffectsSkipped`、與資料通道共用 `Reload` |
> | §4.6 開機配置廣播 | `machine.defines`（唯讀，只由 C++ 出）／`machine.hsys`／`cfg.ver`／`cfg.resync`（第 15 條 WS 指令）；HTML 在 `def.*` 到齊前不渲染 |
> | §4.7 event log 通道 | `log.event {kind, msg, debug?, alarmCode?}`／`log.tail` 一律走 golden 落地器 `RecordProcess` 族；⛔更正：**`MyDBExecSQL` 沒有任何 crash 路徑**（`bUseMDB` 短路＋sqlite 3.7.7.1 NULL 守衛），P8 第一步仍是把 `MyDBOpenDB` 帶進開機序列 |
> | §4.8 執行期顯示 producer | golden 是「定時器 → `Show*()` → 算完再寫 Caption」（86 個函式、599 處賦值）；送的是**算完的結論**；S7 補記：`StageThermo` 第一刀 71 通道全 null、沒碰 `ShowThermo` |
> | §4.9 檔案佈局 | **不塞 `WebBridgeTags.cpp`**；一結構一產生檔（`gen/`）、一通道一手寫檔（`Chan*.cpp`）、綁定表一檔；⛔更正：`ht9045_jsonbridge` library **沒有做**，全部直接進 `wb_serve` 來源清單 |
| 🆕 S12 `/api/form`（20260924） | `GET /api/form/`（清單）／`/api/form/<Page>` → `{page, form, available, runs, widgets:{id:{text\|checked\|itemIndex\|caption\|position\|down\|visible\|enabled}}, assignedCount, skipped}`，只送這次 `DoIniDataToForm()` 有賦值的屬性。本體 `JsonBridge/FormJson.cpp`、widget 表 `tools/gen_formjson.py` → `gen/form_*.gen.cpp`。`recipe.doc.put` 的 ack 多 `reload` 欄。細節見 `phases.md` S12 第一波結果（尚未併入 `api-shape.md`）。⛔ 20260926：這一型已沒有頁面用得到；設定頁實際用的是 C 路 `editlist.get`／`editlist.save`，形狀見 skill `ht9045-html-json` `route-c-golden-bridge.md` §3 |

---

## 五、寫方向（#3 → #6）

> **全文移至 `references/write-path.md`**（20260923 續拆，逐字保留，**編號不變**）。
> 一句話：**沒有控制項。**`JSON ─FromJson(dryRun)→ 暫存副本 ─Clamp*→ Persist（ini／editlist／blob 三選一）→ ReadFile() → ToJson 回頁面`。
> 8 條規則：dryRun 先跑並回報 `clamped`；先備份再原子替換；**`Persist` 之後必跑 `Reload`**；字串 UTF-8 直通、`char[]` 截斷不留半個字；`*.live` 唯讀；`Clamp*` 純函式；`vclcompat/` 不在這條路上；
> `editlist` 存檔三條硬規則（順序＝`elConfig->Add()` 呼叫序、一律經 `TMemIniFile`、值字串照 `SaveEditTextToFile`），G1 判定＝與 golden 存出的檔 `cmp` 零差異。

---

## 六、通訊縮減（我們有 JSON 結構設計權）

> **全文移至 `references/wire-reduction.md`**（20260923 續拆，逐字保留，**編號不變**；含兩段 ⛔更正）。
> 一句話：**API 語意不變，線上少送**——value-only、`?page=` 投影、`?since=` 304、put 只送改動欄位、生產只在變動時進 patch、`io.di`／`io.do` 位元打包、`motor.axes` 定序陣列、alarm 事件物件不輪詢、HTTP 拿全量／WS 只送 patch、橋接層自己分塊（`kWsChunkBytes`）；`null` 與 `0` 仍要分得開。
> ⛔ 兩段更正：**IO 一個 port 是一個位元組**（`io.di` 320 B → base64 428 字元，另有 `*.valid` 在位平面 40 B；減量 **10.9 倍**不是 ~100 倍，且今天 512 個舊 tag 刻意未刪、兩套並存反而**多 870 B**）；
> **Alarm：C++ 只送事實，NonStop 分類在 HTML**（`AlarmChannel.h:63-66`），`stopAllMotor` 恆 null、另送恆 false 的 `alarm.stopMotorPorted`。

---

## 七、驗收 gate（每個結構、每條通道一致）

| Gate | 條件 | 怎麼量 |
|---|---|---|
| G1 往返 | `GET` → 改一欄 → `struct.put` → **檔案位元組 diff 只有那一行** → `Reload` 後值一致 | `git diff --no-index` 舊新檔；`ApplyResult.bytesDiff` |
| G2 語意 | `ToJson(_File)` 的每個欄位 == `ReadFile()` 之後結構的值（不是檔案的值） | 對 `ReadFile()` 的修正規則各造一個踩線配方（`X Division=1`、`bHotPlateMove1CM=false` …） |
| G3 編碼 | UTF-8 直通：`GET` 出來的字串位元組 == 檔案裡的位元組；含中文別名欄位；`char[]` 截斷不得留下半個字元 | 取 `IniData\Data\` 底下所有配方跑一輪，另造一個別名剛好卡在 `sizeof(Alias)` 邊界的配方 |
| G4 拒寫 | 任一欄不合法 → 整筆不寫、檔案位元組不變 | 故意送越界值 |
| G5 縮減 | 每條縮減都有前後 bytes 量測值 | `wb_publish --pump` ＋ 抓 WS 流量 |
| G6 表⑧ | 該結構的 `fieldsStaged/fields` 變成 n/n | 重跑 `gen_fileio_bridge_status.py` |

---

## 八、分期

> **全文移至 `references/phases.md`**（20260923 續拆，逐字保留，**編號不變**；含 S8 ⛔更正）。
> 一句話：依相依順序 S0 開機配置廣播 → S1 event log → S2 產生器＋`SYSTEM_DEVICE_FORM` → S3 `LAST_LEVEL_SET`（⛔ 前提被 §十二 R2 推翻）→ S4 `SYSTEM_TEST_IF` **788** 欄 → S5 `LAST_GENERAL_SET`＋`IniConfig`＋`TfDIOFrom` → S6 寫方向 → S7 `StageThermo` → S8 生產差量 → S9 IO／Motor → S10 Alarm → S11 `act.main.clarnData`。
> 括號內舊代號 P0'～P9 與 S 序同義。全部 ✅ 已實作（見 §〇）；每期共用 §七 六個 gate。
> ⛔ **20260924 更正**：S5、S6 **沒有**實作（見 `phases.md` 更正段）。另新增 **S12 頁面改讀 C++**（`/api/form/<Page>` ＝ C++ 跑 `DoIniDataToForm()` 出 JSON；存檔後重讀），使用者列為優先。
> ⛔ S8 更正：「只 stage 有變動的」照字面做會把 `0` 送成 `null`（`TagSnapshot.h:155-158` 契約）—— 實作改成**每 tick 全 stage、靠 diff 減量**，另送 `prod.changed`／`prod.ver`／`prod.live` 與差量 `prod.<x>.d`；G5 實測靜止 30 秒 `prod.*` 進 patch 的 tag 數 = 0。

---

## 九、陷阱（今天踩過或量到的）

1. **量結構大小有兩個坑，要同時避開**：(a) 用 regex 會從更早的 `typedef struct` 一路吃過來；(b) **大括號配對前沒有先剝註解**會咬到註解裡的 `{` —— `cprod.h:2156` 行尾註解就有一個，`SYSTEM_TEST_IF` 因此被量成 351（實際 788），漏掉 437 個欄位而且毫無徵兆。兩個坑我都踩過。原文：**用 regex 量結構大小會錯**：`typedef\s+struct[^{]*\{(.*?)\}\s*NAME;` 把 `LAST_LEVEL_SET` 量成 859 欄，實際 1 欄。用大括號配對。
2. **用結構名判「已出 JSON」會高估**：`TfBarCode`／`TfRFID` 寫進 `IniConfig`，`IniConfig` 整體有被 stage → 誤判 full。一律 `Struct.field` 粒度，且先剝註解。
3. **只認 `ReadIniData` 會漏第二套**：`TfLd_ULd`／`TfLaserSensor`／`TfVacuumUnit` 純 `HTEditList`。
3b. **只認「區段是字串常數」會漏 247 讀／383 寫**（20260923 實測）：`cOffSet.cpp` 的區段是 `CapStrInput[i]`、`BarCode.cpp` 159 處、`uLotInfo.cpp` 65 處。目標是 `Obj[i]->SetXxx(ReadIniData(...))` 的還有 38 個。產生器已改成接受表達式（報表以 `<expr>` 標示），但 **`<expr>` 的區段名要靠人或執行期展開**，`FieldDesc` 不能直接從它產生——Offset 族的 `FieldDesc` 要另外用 `CapStrInput[]` 陣列展開。
3d. **第五種呼叫形式 `ReadWriteIni(...)` 與 `HTEditList::Add()` 註冊式 IO，表⑧ 第二版都沒算**：16 個 `ProcessLastSetIni_*` 全用前者、`TfConfiguration`（1,581 筆）與 `TfDIOFrom` 用後者，所以 `IniConfig` 這個全樹最大的設定結構在表⑧ 裡**沒有自己的一列**。第三版產生器要補這兩種。
3e. **`ReadLastSetIni`／`SaveLastSetIni` 名不符實**：主體是 `IniConfig`↔`config.ini`，`LastSet` 只沾到 1 個欄位＋開頭呼叫 `ReadLastDataFile()`。看名字排工作會排錯結構。
3c. **Offset 族整個不在移植樹**：`cOffSet.cpp`（135 讀／97 寫）、`AutoTeach/InOutArmZteach.cpp`（11 寫）、`ArmOffsetData.cpp`（13 寫）三檔在 `HT9011UC_Cpp_V3.33.906.0` 不存在。`DoArmOffsetConvert()` 的來源因此沒人填。見 `references/file-io-mechanisms.md` §F。
4. **`_File` → 執行中的套用點是 `DoStructUnitConvert()`，不是「沒有」**：第一版 grep 漏了 `memcpy`。寫了 `_File` 之後要照 golden 叫它一次；移植樹目前開機沒叫（`#if 0`／stub `#define`），先查這個再談 JSON。
5. **`SaveSetupFile` 從 VCL 讀值**：新架構沒有 VCL，所以它不能逐字翻譯；**它裡面的鉗制與格式字串是唯一會遺失的東西**，逐表單搬進 `Clamp*` 與 `FieldDesc.fmt`，並用 G1 抓漏。
6. **`/api/recipe` 不是結構**：它是檔案鏡像；`ReadFile()` 有修正規則。
7. **`--dry` 不是 no-op**：三個啟動器已於 20260923 移除，不要加回去。
8. **`setup.inf` 已移出版控**：pull 會刪本機那份，pull 前備份。
9. **`.cmd`／`.bat` 必須 CRLF**：09-22 有工具把 2,249 檔轉成 LF，`build.bat` 因此失敗過。
10. **Python 直譯器**：PATH 上的 `python` 是 Store 殼，靜默 exit 49；用 `C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe`。
11. **build 前確認 PATH 含 `C:\MinGW\bin`**：20260924 實測某些 session 的 PATH 沒有它，`cmake --build` 會**靜默失敗**——`cc1plus` 以 `exit 0xC0000135`（DLL not found）結束、不印任何錯誤訊息，容易誤判成「build 過了」。build 之前先 `$env:PATH = "C:\MinGW\bin;" + $env:PATH`（PowerShell）或 `export PATH=/c/MinGW/bin:$PATH`（bash）。
12. **C 路頁面整頁 `editable:false`，不代表權限鏈翻錯**：先查 GATE (SEC1) 的查表半解閘有沒有跑（`cSecurity.cpp` 檔尾 `W906_SecurityBoot()`，開機要在 `FileRW_IniConfig_Boot()` 之前）。golden `Insufficient(iType)` 只看 `iMaxLevelItem` 與 `LevelSet.AccessLevel[]`，這個沒補，`Insufficient(任何 iType>0)` 恆 `false`，golden 轉出來的 `grp->Enabled=Insufficient(n,false)` 全部變停用——20260924 實測 `Setup.Ld_ULd.html` 23 個替身全不可改，一開始看起來像是權限鏈或容器巢狀判斷翻錯，其實是這半顆 gate 沒解。⚠ `iMaxLevelItem` 這個數字＝golden 當時的權限項目筆數（20260924 是 180），golden 加權限項目要跟著改，見 `generators.md` 四。
13. **C 形狀（`HTEditList`）第一次原值存檔，檔案位元組會變，不一定是 bug**：golden `HTEditList` 的 `iDecimalPoint` 預設 6，一律用 `0.500000` 這種格式寫檔。如果檔案是舊版或 B 路寫的（`0.500`，位數不足，或缺 golden 會補的鍵），**第一次**用 C 路把原值存回去會出現「正規化」差異：同一個鍵數值相等（只是字串格式不同）、或 golden 補上了缺的鍵。這是**唯一**允許的差異；同一個鍵數值真的不同，或**第二次**原值存檔位元組還在變，才是真的缺陷。驗收步驟見 `generators.md` 五。
14. **（20260926）C 路新結構的七個坑，全文在 `generators.md`**：十七 開機呼叫插在 `//` 註解後面會被註解掉；十八 兩頁寫同一鍵要「本頁沒改的欄位不把舊值蓋回」，快照不可在 reload 拍（`FileRW/_EditPage.cpp:164`）；十九 golden 關窗後才跑的尾段在網頁沒有觸發點；二十 `.py` 寫死 golden 行號，golden 在同一方法內加減行時產生器**不報錯而對錯行**（只有 A 形狀會核對起行文字）；廿一 取代字串的反斜線 Python 要寫四個；廿二 `ATKRecipeInfo` 在移植樹是 NULL；廿三 `wb_serve` 開機約 1 秒會起測試機引擎，e2e 設 `HT9045_TESTERCOMM=0`。
15. **（20260926）網頁存檔不看運轉狀態**：golden 設定鈕只在停機設定模式按得到，`editlist.save` 沒有這道閘——SAFETY 相關、交 Jimmy 決定（`porting-gaps.md` 二十一），不要自己加。
16. **（20260927）forms 門面要叫只編進 wb_serve 的本體：用函式指標安裝座，不要直接呼叫、也不要 static init 自我登錄**：直接呼叫會讓每一支連進 `fMain.o` 的 ctest 連結失敗；static init 自我登錄的 TU 放在 `.a` 裡根本不會被抽出來（`forms/fMain.h:1326-1332`）。做法與實例（`W906_BackupSetupFileBody`／`W906_SaveRunModeBody`，`forms/fMain.cpp:458`）見 `generators.md` 廿七。
17. **（20260927）探針 400 ms 內重送同一指令會拿到 `busy:`，不是伺服器壞了**：伺服器防連點 `WebCmdGuard`（S107-3）；不要它就在啟動 wb_serve 前設 `W906_CMDGUARD_MS=0`（skill `ht9045-html-json` `web-bridge-json-contract.md` §1.3）。
18. **（20260927）A 形狀產生器沒有檔案層原文插入點**：C 路可以在 `.py` 的 `members` 寫 `#define fConfiguration (W906_CfgTrayPlate())` 接到共用物件，A 形狀只能在那一行前宣告同名區域變數（`7d490f7c`），見 `generators.md` 廿六。

---

## 十、待裁決

> **全文移至 `references/decisions.md`**（20260923 續拆，逐字保留，**編號不變**；與 §二 同檔）。
> 一句話：8 條**全部已裁**（20260923）——陣列形狀、`dialog.response`、WS 上限不裁數字改分塊、`*.live` 開唯讀、`LAST_GENERAL_SET` 進 P2b、`DoStructUnitConvert()` 不擋 P0（未解閘就標 `liveNotApplied:true`）、`Clarn_Data` 先做、`IniConfig` 走結構。
> 目前**沒有**未裁決事項；新的待裁決請接在這段 stub 之下，裁完再移到 `decisions.md`。

---

## 十二、20260923 審查更正（審查員獨立查證 12 條宣稱的結果）

> **全文移至 `references/review-corrections.md`**（20260923 續拆，逐字保留，**編號不變**；R1～R9 與「我自己的更正」整段照搬）。
> 一句話：審查員獨立重量 §三，主要結論站得住、**9 處細節要改（R1～R9）**：R2 移植樹 `cSecurity.cpp` 180 筆整段在 `#if 0`（S3 前提沒了）、R4 `SaveAllFile` 是 11 個表單且 2 個破例、R5 `RecordProcess` shim 要分三種談、
> **R6** 移植樹 `Clarn_Data` 是空殼（S11 等於從零翻譯）、R7 `wb_publish --pump` 已退役（G5 要重寫）、**R8** `--dry` 只隔離 C++ loader（`Persist` 寫的是真檔）、R9 ×100 欄位移植樹只翻 6/10。
> ⛔ 我自己的更正：對 `SYSTEM_TEST_IF` 的「反向推翻」是錯的，**788 欄才對**（腳本未剝註解就配對，咬到 `cprod.h:2156` 行尾註解裡的 `{`）；`a165cc0` 的 commit 訊息與 13:47 那封信方向是反的。尚未查證：`DoAutoCleanKit` 解閘後 `DoStructUnitConvert` 的可達性。

---

## 十一、相關檔案與 skill

### 本 skill 的 references（共 ~~**6**~~ → ~~13~~ → **16** 份（20260926 實數：加 `pending-pages.md` 等；另有 `archive/` 放移出的舊內容）；20260923 skill 維護盤點更正——原本寫「先讀這兩份」，當時就已經有 5 份；同日 21:50 續拆再加 8 份、併掉 `producers-and-layout.md` 1 份）

**按用途挑，不要全讀：**

| 想知道 | 讀哪份 |
|---|---|
| 某個值為什麼是 null、要排哪個翻譯波次 | `porting-gaps.md`（~~12 條~~ → 第一～二十一節（20260926），型態 A／B／C 分類 ＋ 複驗指令；二十一＝網頁存檔不看運轉狀態，SAFETY 待 Jimmy；⛔ 20260927 → 第一～二十二節，二十二＝St01 這一輪照建議先做的 `[W906]` 偏離） |
| 🆕（20260926）**C 路每個結構接在哪**（tag、頁面、PageDesc 入口、開機建替身、開機／換配方讀檔、`CRouteOwner` 擁有的檔）；`editlist.get`／`editlist.save` 的 JSON 形狀與檢查順序 | skill `ht9045-html-json` 的 **`references/route-c-golden-bridge.md` §6（總表，單一出處）、§3（JSON↔HTML）** |
| 某個檔案／結構今天橋到哪一步 | `table8-fileio-bridge-status.md`（表⑧，產生器出的） |
| golden 的讀寫檔到底怎麼運作 | `file-io-mechanisms.md` |
| 畫面單位 ↔ 馬達單位怎麼換 | `unit-convert-layer.md` |
| 本樹的註解要怎麼寫 | `comment-convention.md` |
| §4.8 執行期顯示 producer／§4.9 檔案佈局的全文 | ~~`producers-and-layout.md`（20260923 從 SKILL.md 拆出）~~ → **`api-shape.md`**（同日續拆時整檔併入，原檔已刪） |
| 使用者裁決的原話與理由（golden／`_NET`／編碼／Q30-8／陣列形狀／`IniConfig`…）＋ 8 條已裁的待裁決 | `decisions.md`（§二 ＋ §十） |
| 改設計前要先知道的量測事實（三段式、三套持久化、三個實例、`IniConfig` 族、結構大小、橋接層現況） | `measured-facts.md`（§三） |
| 端點、`FieldDesc`／`Binding` 形狀、C++ 簽章、`act.*`／`cfg.resync`／`log.event` 三條通道、producer、檔案佈局 | `api-shape.md`（§四 全部，含 §4.8／§4.9） |
| `struct.put` 之後 C++ 怎麼寫檔（dryRun／`Clamp*`／`Persist`／`Reload`、`editlist` 格式三條硬規則）——⛔ 這條管線沒實作，實際寫方向是 C 路（檔頭補註） | `write-path.md`（§五） |
| 線上為什麼這麼送、IO 位元打包的正確算術、Alarm 的 C++／HTML 分工、WS 分塊規則 | `wire-reduction.md`（§六） |
| S0～S11 各期做什麼、為什麼這樣排、S8 的 null/0 更正與 G5 實測 | `phases.md`（§八） |
| 審查員查證出的 9 處錯誤（R1～R9）與 `SYSTEM_TEST_IF` 788 欄那條反轉 | `review-corrections.md`（§十二） |
| S8～S11 留下的 12 條待辦、處理進度、#8 重寫 | `open-todos.md`（§十三） |
| 🆕 **總共有哪些結構要寫檔、各由誰寫、四種形狀（表單／自由函式／HTEditList／二進位）、做到哪** | **`write-inventory.md`**（20260924，以結構為主軸；表⑧ 只算 ini 一套且以單位列） |
| 🆕 某個結構的寫檔 bridge 在哪支 cpp、寫哪些檔、能不能存檔、讀檔端缺什麼 | ⛔ 20260926：`HT9011UC_Cpp_V3.33.906.0/FileRW/README.md` 是 `gen_formbridge.py` 出的，**只列 A 形狀（現在只剩 HotPlate）**；C 形狀 ~~32~~ → 35 個結構（20260927）改看 skill `ht9045-html-json` `route-c-golden-bridge.md` §6 |
| 🆕🆕（20260926 更新）產生器（`gen_editlist.py`／`gen_formbridge.py`／`gen_teach_editlist.py`）怎麼用、怎麼加一個新結構、SEC1 gate、golden 小數格式的 G1 驗收、多寫者規則、關窗尾段、寫死 golden 行號的風險與換 golden 核對步驟、反斜線、`ATKRecipeInfo` NULL、`HT9045_TESTERCOMM`、各產生器讀哪一棵 golden | **`generators.md`**（SKILL.md 只留導覽，操作細節全在這裡；十八～廿四 是 20260926 新增；廿五～廿九 是 20260927 新增：沒有頁面的 C 路結構、檔案層原文插入點、函式指標安裝座、`wb_serve.cpp` 插入慣例、別人翻好之後拿掉 `replace`） |


| 檔 | 內容 | 維護方式 |
|---|---|---|
| `references/file-io-mechanisms.md` | 三套讀寫檔機制（ini／`HTEditList`／二進位 blob）的**實作層**對照：值格式、預設值行為、Change Log 副作用、鉗制位置、移植樹現況、自由函式與讀寫總指揮順序 | 手寫，量測日 20260923 |
| `references/table8-fileio-bridge-status.md` | 表⑧ 的文字版：57 個讀寫檔單位逐列（機制／①②③ 行號與鍵數／移植狀態／JSON 中介層逐欄位）＋已出 JSON 的欄位清單 | **產生器輸出，不手改**；與 `ScreenShots.html` 表⑧ 同一次執行寫出 |
| `references/porting-gaps.md` | **JSON 橋接層擋在哪裡**：~~9 條~~ → 第一～二十一節（20260926）「值是假的、因為 golden 那段還沒翻譯」或「網頁少了 golden 的閘」的缺口，逐條給型態（A 沒翻譯／B 翻了沒人叫／C 被閘擋住）、證據、影響到哪些 JSON 欄位、排序建議、複驗指令。⛔ 最大一條是 `main.cpp` 的 `Index16Heater` 等（`bUT150Install[]` 的填值來源 99% 在裡面；第一節 20260926 註明「整支不存在」只剩檔名成立）；十、十三、十四已結案；二十一 SAFETY 待 Jimmy；二十二（20260927）St01 這一輪照建議先做的 `[W906]` 偏離清單 | 手寫，量測日 20260923 起；給 Jimmy 排移植波次用 |
| `references/unit-convert-layer.md` | `cUnitConvert.h` 全部七個函式：`_File`（畫面單位）→ 執行中（馬達單位 ×100）的整包 memcpy＋逐欄轉換＋選擇邏輯；golden 24 個呼叫點；移植樹「本體已翻、開機被 `#if 0`／stub 擋住」的現況 | 手寫，量測日 20260923 |
| `references/decisions.md` | SKILL.md §二 裁決紀錄 ＋ §十 待裁決（8 條全部已裁）的全文 | 20260923 續拆，逐字；編號不變。新裁決先寫 SKILL.md §二 stub 之下，再移入 |
| `references/measured-facts.md` | SKILL.md §三 量到的事實（§3.1～§3.6）的全文 | 20260923 續拆，逐字；編號不變；更正時另起更正段 |
| `references/api-shape.md` | SKILL.md §四 API 形狀（§4.1～§4.9）的全文，含原 `producers-and-layout.md` | 20260923 續拆，逐字；編號不變；被 `.cpp`／`.h`／`.py` 註解引用最多的一份 |
| `references/write-path.md` | SKILL.md §五 寫方向（#3 → #6）的全文 | 20260923 續拆，逐字；編號不變 |
| `references/wire-reduction.md` | SKILL.md §六 通訊縮減的全文，含 IO 算術與 Alarm 分工兩段 ⛔更正 | 20260923 續拆，逐字；編號不變 |
| `references/phases.md` | SKILL.md §八 分期（S0～S11）的全文，含 S8 ⛔更正 | 20260923 續拆，逐字；編號不變 |
| `references/review-corrections.md` | SKILL.md §十二 審查更正（R1～R9 ＋ 788 欄反轉）的全文 | 20260923 續拆，逐字；編號不變 |
| `references/open-todos.md` | SKILL.md §十三 S8～S11 待辦（12 條 ＋ 處理進度 ＋ #8 重寫）的全文 | 20260923 續拆，逐字；編號不變；待辦結案時在那裡更新進度表 |
| `references/generators.md` | 🆕（20260924 深夜；20260926 更新）C 路（golden 表單橋）產生器的操作手冊：C 形狀 `gen_editlist.py`（`tools/editlist/<struct>.py`＋`--only`＋`_integrated.txt`＋共用 `FileRW/_EditPage.h`）、A 形狀 `gen_formbridge.py`（只剩 HotPlate）、`Teach` 的 `gen_teach_editlist.py`；怎麼加一個新結構；SEC1 gate；golden 小數格式的 G1 驗收；平行分工規則；審查結果；十七～廿四 陷阱（開機呼叫位置、多寫者、關窗尾段、寫死行號與換 golden 核對、反斜線、`ATKRecipeInfo`、`HT9045_TESTERCOMM`、golden 樹一覽）；廿五～廿九（20260927：沒有頁面的 C 路結構、檔案層原文插入點、函式指標安裝座、`wb_serve.cpp` 插入慣例、別人翻好之後拿掉 `replace`） | 手寫，`decisions.md` 二之四、`write-inventory.md` 互相引用；不與 skill `ht9045-html-json` 的 `route-c-golden-bridge.md` 重複（那份定義 A/B/C 三路、JSON 形狀與**結構總表**，這份只講產生器操作與陷阱） |
| `references/pending-pages.md` | 🆕（20260926）**還沒有網頁的表單建頁備忘**：Rotate／AutoAlignment／LaserSensor／TrayMapping ScanLine／Magazine／FixAICCD／PE 模式鈕／DUT on/off／AOA offset（頁面已建但功能受限）／Alert.Password Event Log／ProductionInfo／TZteach／AOI／OCR（S86）／ContactForce（S57，頁面已存在但是純靜態）、20260927 加 Configuration Tray／HP 分頁（S98）／ACTForm（S108）／Winway（S109）／Monitor（S110），每個表單固定欄位（建議頁面檔名、golden 檔、移植入口、PageDesc、讀檔時機、存檔觸發鈕、必送欄位、元件 id、頁面事件、已知陷阱、待 Steven 決定、待 Jimmy 前提、commit） | 手寫，Steven 20260926 交辦；找不到的欄位標「待確認」，不猜 |
| `references/clear-buttons-st01.md` | 🆕（20261002）**畫面上的清除鈕**（St01 接的 act.*）：Contact CT 的 Count Clear／格子點兩下（E-022 CK-1／CK-2）、Observer 的 Clear Time Data（E-021 OB-9）——golden 行號、golden 疑點、移植樹的分派點、測試名 | 20261002 從 RogerYang 的 `ht9045-clearcount-flow` 搬來（那支 skill Jimmy 在 main 撤回 7287f4d3），原文沒改 |

- 表⑧ 網頁版：`web/page/ScreenShots.html`；資料 `web/page/screenshot_meta.js` 的 `FILEIO_BRIDGE_STATUS`；產生器 `HT9011UC_Cpp_V3.33.906.0/scratchpad/gen_fileio_bridge_status.py`（同時寫出上面那份 `.md`）
- 現有橋接：`HT9011UC_Cpp_V3.33.906.0/WebBridgeTags.cpp`（2,194 行手寫 staging，開頭的 null-vs-0 規則）、`WebBridge/TagSnapshot.h`、`WebBridge/TcpTagLink.cpp`（snapshot／patch 型別）、`tools/wb_serve.cpp`（`/api/recipe` :431、`levelset` :1359、`system.levels.put` :1475）
- 頁面接線：`HT9011UC_Cpp_V3.33.906.0/tools/pagewire/`（三元組 `[文件, 區段, 鍵]`）
- 對話框契約：`web/JSON/Dialog-bridge-contract.json` v1.3.0、`web/config/AlarmNonStop.json`
- VCL shim：`HT9011UC_Cpp_V3.33.906.0/vclcompat/`
- 1203：`EtherCAT/Pci1203Monitor.h:403-405`（DI 320／DO 192 的 enum；~~`:383`~~ 是註解開頭，20260923 更正）；tag 層 `:1637-1638`；`byteData` 型別 `:1099`
- 相關 skill：`ht9045-html-json`（四大類 JSON 與 B 路端點）、`ht9045-recipe`（11 個 `.Data` 檔與全域對應）、`ht9045-config`（`HTEditList`／`IniConfig`）、`ht9045-alarm-dismissal`（NonStop 對話框契約）、`ht9045-array-audit`（`LastSet` 陣列邊界）、`gl-wave-loop`（忠實 vs 偏離判準）

---

## 十三、S8～S11 留下的待辦（高級審查員第二輪，20260923 21:03）

> **全文移至 `references/open-todos.md`**（20260923 續拆，逐字保留，**編號不變**；含處理進度表、#8 重寫、原 12 條待辦表）。
> 一句話：審查員第二輪 12 條，使用者裁示**不擋 commit、轉成待辦**。✅ 已處理 #1（`MyDBIProcess` 鏈是 3 參數→`uHGemEquipment.cpp:3483`→`aHotPlateSubstrate.cpp:1264` 空槽，`cMyDB.cpp:1000` 在 `#if 0`）／#4（行號漂移 5 處）／#7（`porting-gaps.md` 補十～十二）／#10（§六 Alarm 分工）／#11（ctest 基線）／#12（`MyDBExecSQL` 無 crash 路徑）；
> ⛔ #8 前提作廢重寫：真問題是 `web/JSON/Alarm-dialog-request.{json,js}` **被 track**，建議 B（`git rm --cached`＋`.sample`）但那是 Jimmy 的檔、要點頭才動；
> ⏸ 未做：#2（`act.main.clarnData{dryRun:false}` 實跑寫入，需人在機邊備份 `lastdata.dat`）、#3（刪 512 個舊 `pci1203.*` tag 要與 `web/js/pci1203/view.js` **同一顆 commit**）、#5（等 `StopAllMotor()` 翻譯）、#6（測試涵蓋缺口）、#9（`AckJson` `ok=false` 形狀要與 web 側一起裁）。

