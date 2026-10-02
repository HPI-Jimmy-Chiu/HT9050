# write-inventory.md —— 已失效內容封存

> 這份檔案只收「已經失效、不再需要追蹤」的舊內容，從 `references/write-inventory.md` 移出來的。
> 每段開頭寫：移出日期、原本在哪一節、為什麼失效。**原檔對應位置留著標題／表格框架，不留這裡的舊句。**
> 不要刪這份檔案裡的任何段落——這是稽核用的封存，不是垃圾桶。

---

## 移出日期 20260926｜原本在「二、逐結構清單」→ `LevelSet`（LAST_LEVEL_SET，levelset.dat）表 → `SetLevelSet` 那一列

**為什麼失效**：舊句說 `system.levels.put` 「要改走 SetLevelSet（含鉗制）」是待辦語氣，且沒有
提到 SEC-W1／SEC-W2 兩個 GATE 的狀態；S64（commit `8c5ea501`）已經把 `system.levels.put` 換成
呼叫新檔 `WebLevelSet.cpp` 的 `W906_LevelSetPut`（照 golden `TfSecurity::FormClose` 全流程：
`Insufficient(29)` → `GetLevelSet` → 驗證 → 套值 → 三條鉗制 → 備份 → `SaveJamLevel` →
`SetLevelSet` 整塊 1024 bytes → 重讀逐位元組比對），`cSecurity.cpp` 的 SEC-W1（`GetLevelSet`
建檔）／SEC-W2（`SetLevelSet` 寫入）兩個 GATE 都已解開。現況見
`references/write-inventory.md`「二、逐結構清單」`LevelSet` 列與 ChangeLog
`CHANGES_20260926_Steven.md` §11.27。

**原句**：

> `SetLevelSet` | D | `cSecurity.cpp:1511` | levelset.dat | ⚠ wb_serve 已有 `system.levels.put`
> （`wb_serve.cpp`）；golden 觸發者 `TfSecurity::FormClose`（含 `[87]`／`[129]`／`[86]` 三條
> 鉗制，`file-io-mechanisms.md` §C.2）、SECS S125F4 ⏳ 對照

---

## 移出日期 20260926｜原本在「四、golden `cprod.h` 全域變數的讀寫盤點」表 → `LevelSet` 那一列

**為什麼失效**：舊句寫「`system.levels.put` 要改走 `SetLevelSet`（含鉗制）」是待辦語氣；
S64（commit `8c5ea501`）已經做完，見上一段的說明與 ChangeLog §11.27。

**原句**：

> `LevelSet` | `system\levelset.dat` | B | `GetLevelSet` cSecurity.cpp:1474 | `SetLevelSet`
> :1511（FormClose :465） | TfSecurity | live；Bindings.cpp:85 已綁；`system.levels.put` 要改走
> SetLevelSet（含鉗制）

---

## 移出日期 20260926｜原本在「二、逐結構清單」→ `TestIF_File` 表 → `TfSetup::SaveSetupFile` 那一列

**為什麼失效**：舊句說「頁面未接」「`fSetup->Init()` 之後呼叫 `FileRW_Setup_Boot()`」是當時（0925
下午）的狀態；`SetUp` 頁後來完整整合並通過回歸（commit `cf530be8`，`CHANGES_20260925_Steven.md`
§12.3／§13.1：讀、寫、換 Test Mode 三組 ALL PASS），原句已經不成立。

**原句**：

> `TfSetup::SaveSetupFile` | ~~A~~ **C**（20260925 改走，`16f463b8`） | `cSetUp.cpp:3655`（133 鍵） |
> HandlerCondition.Data（另寫 Contact／Temperature／configByRecipe） | ✅ `ReadFile` | ⏳
> `tools/editlist/TestIF_File_SetUp.py`／`FileRW/TestIF_File_SetUp.cpp`：僅開機建替身
> （`fSetup->Init()` 之後呼叫 `FileRW_Setup_Boot()`），**頁面未接**——golden 存檔每次送 ATC7 指令
> `@CH_ENABLED` 給溫控器、頁面缺 `rgSensor1..24`。舊 A 形狀 `tools/formbridge/TfSetup.py` 已退役

---

## 移出日期 20260926｜原本在「二、逐結構清單」→ `TestIF_File` 表 → `TfAutoAlignment` 存檔那一列

**為什麼失效**：檔名／行號／鍵數全部引用錯誤。舊句寫 `AutoAlignment/AutoAlignment.cpp:426`
（11 鍵），20260926 查證（commit `c675594d`）確認 golden 的存檔鈕
`spbSaveClick` 實際在 `AutoAlignment.cpp:1573`，22 鍵；`:426` 附近不是這個函式。另外舊句沒有
提到 0925 稽核把「編譯進去的是哪一支檔案」也搞錯了（誤以為是 `cAutoAlignment.cpp`，那支其實是
死碼、不在 `HT9045.bpr` 裡）。

**原句**：

> `TfAutoAlignment` 存檔 | A | `AutoAlignment/AutoAlignment.cpp:426`（11 鍵） |
> HandlerCondition.Data | ✅ | ⏳（沒有頁面） |

---

## 移出日期 20260926｜原本在「四、golden `cprod.h` 全域變數的讀寫盤點」→「Gerneral.ini（形狀 E）」表 → `THandlerSystem::SaveSystemSet` 那一列

**為什麼失效**：舊句記的是 0924～0925 之間的中途狀態（「進行中」「`web/page/HW.HandlerSys.html`
這次未定案、未推送」「未整合」「頁面缺 `rgTrayArmType`，要 Steven 決定」）。`HandlerSys` 後來
選 (b) 補上 `rgTrayArmType`、9050GPIB 改善 A＋B 都做了，整合並通過回歸（commit `cf530be8`，
`CHANGES_20260925_Steven.md` §12.1／§12.3／§13.1：讀寫 ALL PASS），現況見
`references/write-inventory.md` 〇 表 HandlerSys 那一列。

**原句**：

> `THandlerSystem::SaveSystemSet` | `HandlerSys.cpp:579` | 277 | HW.HandlerSys | 進行中：
> `tools/formbridge/THandlerSystem.py`（形狀 E，走法同 A 形狀）＋ `FileRW/HSys.cpp` 已產生；
> 第八輪審查標為 H-1——`display` 會補寫 `Gerneral.ini` 等量產共用檔，且跑在 HTTP 執行緒上，**暫不
> 註冊進 `wb_serve`**，網頁開頁能不能寫要 Steven 決定（見 `generators.md` 十一、十六）；
> `web/page/HW.HandlerSys.html` 這次未定案、未推送（見 ChangeLog §46.1），頁面現況仍是檔案鏡像
> `system.file.put`，尚未接上。20260925 Steven 設計指示：「參考 BCB 原本的作法
> `SYSTEM_MODULAR::ReadGeneralIni()`、`THandlerSystem::FormShow`」——開頁照 golden（含補寫
> `Gerneral.ini`／RS232／GPIB 缺鍵）；golden 遇 GPIB `general.ini` `Model` 讀取失敗是
> `MessageDlg`＋`Application->Terminate`，網頁版改成送告警訊息＋拒存、**不結束程式**；golden
> `TfMain` 建構子讀的 `EP_Install`／`InOutArmPickerUseMotor`／`ION_FAN_TYPE` 要翻成開機函式
> （見 `generators.md` 十三通用陷阱）。20260925 改 C 形狀交件：`tools/editlist/HSys.py`→
> `FileRW/HSys_C.cpp`（`bHandlerModel` 失敗＝ELMessage＋拒存，不 Terminate），**未整合**；
> 頁面缺 `rgTrayArmType`，HW.HandlerSys.html 要 Steven 決定

---

## 移出日期 20260926｜原本在「🆕 待派佇列（20260926）」整節

**為什麼失效**：這節列的待派項目，在 20260926 這一天陸續由這些 commit 做完，原句已經全部
過期：P8 四小項（`c913d5e5`：S58 PE模式、S59 DUT on/off、S60 早已完成、S61 TC401 Heater）、
`TfMagazine`／`TfFixAICCD` 的讀寫（`c79ee4e9`：S62／S63）、`SetLevelSet` 觸發者盤點
（`8c5ea501`：S64）、`WriteLastDataFile` 的 20+ 觸發者盤點（`1ba00a68`：S65）、
`TZteach`／`TfProductionInfo`（`58bd9425`：S68／S70；`TFrmAOI` 仍在進行中，S69）、
`IniConfig.gen.inc:8307` `SetTestRunMode` GATE（`5bbbb31f`：S66）。現況見
`references/write-inventory.md`「🆕 待派佇列」現版（只剩真正還沒做的）。

**原句**：

> * P8 剩 4 小項：`bHasEnteredPEModel`（PE 模式，`sbPEModelClick` `main.cpp:33741`）、
>   `bTestSiteUse`（DUT on/off，`mtDutOnOffMouseUp` `main.cpp:30341`）、`iAutoCleanShuttle`
>   （併 Cleaning）、`TC401HeaterControl`（V912 新功能，要先翻 `g_tHeaterInsInfo`）
> * `TfMagazine`、`TfFixAICCD` 的讀寫
> * `SetLevelSet` 觸發者盤點（`TfSecurity::FormClose` 三條鉗制）
> * `WriteLastDataFile` 的 20+ 觸發者盤點（哪些頁面還沒照 golden 觸發，見上方「`LastSet`」列）
> * 移植樹整檔不在的：`TZteach`（Position Offset.Data）、`TFrmAOI`（AOI.Data）、
>   `TfProductionInfo`（AutoCalSuckZ.Data）——量大，排後
> * `IniConfig.gen.inc:8307` `SetTestRunMode` GATE（等 `Status.CounterSel` 交件，可能也動
>   `IniConfig` 設定）
> * `Rotate`／`AutoAlignment`／`LaserSensor`／`TrayMapping` 的存檔函式已翻譯（`c675594d`）但
>   還沒有頁面觸發點——細節見「二、逐結構清單」對應列，不在此重複

---

## 移出日期 20260926（15:3x）｜原本在「二、逐結構清單」→ `TestMode`（SYSTEM_TEST_MODE）表 → `SaveTestMode` 那一列

**為什麼失效**：舊句說「golden 16 個呼叫點，移植樹只接 2 個」是 0925 之前的狀態。S88
（commit `295bc768`）逐一對照後，已接 9 個（含這次補的 `sbTempOffsetClick` 尾段）；
`TfBuilder::bSaveAllFillOrFile` 查證後 golden 本身走不到，不算缺口；剩下 7 個交 Jimmy／
Steven02。現況見 `references/write-inventory.md`「二、逐結構清單」`TestMode` 列與
`scratchpad\frw_s88\report.md`。

**原句**：

> `SaveTestMode` | B | `cprod.cpp:3314`（27 鍵） | TestMode.Data | — | ⚠ 本體一致；golden
> **16 個呼叫點**，移植樹只接 2 個。缺的觸發者：`TfMain::ChangeSetUpFile`、
> `ChangeTesterConnect`、`RunICModeChange`、`ShowTestHeadComp1`、`sbTempOffsetClick`、
> `DoReadLastData`、`TfBuilder::bSaveAllFillOrFile`、`DoTrayFeedProcess`

---

## 移出日期 20260926（15:3x）｜原本在「二、逐結構清單」→ `IniConfig`（config.ini）表 → `SaveTasterInfo`／`SaveEventLogAutoSaveInfo`／`SaveRmsInfo` 三列

**為什麼失效**：三句都是 0925 之前的待辦狀態，且 `SaveRmsInfo` 那句把 golden 實際寫的區段
（`[Server]`／`[RMS]`）誤植成「`[Product*]`」。S89（commit `295bc768`）逐一對照後：
`SaveTasterInfo`／`SaveEventLogAutoSaveInfo` 的觸發者都已接上或確認 golden 本身是死碼；
`SaveRmsInfo` 的觸發者（`TfMain::FormClose`）仍未接，列為待 Steven 決定 Q2。現況見
`references/write-inventory.md`「二、逐結構清單」`IniConfig` 表對應三列與
`scratchpad\frw_s88\report.md` 第 1／7 節。

**原句**：

> `SaveTasterInfo` | B | `cprod.cpp:3229`（7 鍵） | config.ini `[Taster]` | ⚠ 本體一致；
> 觸發者 `TfConfiguration::SaveConfiguration`、`TfFTPClient::btSafeTasterNameClick`
> **0 處接上**
>
> `SaveEventLogAutoSaveInfo` | B | `cprod.cpp:3177` | config.ini `[Event Log]` | ⚠ 本體
> 一致（`FormatString`→`FormatDateTime` 等價）；觸發者 `TfConfiguration::FormClose`、
> `TfObserver::bAutoSaveEventLog` 4 處全在 `#if 0`
>
> `SaveRmsInfo` | B | `cprod.cpp:3150` | config.ini `[Product*]` | ⚠ 本體一致；觸發者
> `TfMain::FormClose` 未接

---

## 移出日期 20260926（15:3x）｜原本在「其餘結構」表 → `LastSet` → `WriteLastDataFile` 那一列

**為什麼失效**：舊句列出「golden 觸發者 20+ 處」的清單但標「⏳ 逐一對 live 狀態」，是待辦
語氣。S65（commit `1ba00a68`／`c317ca30`）已經把這 23 個活呼叫點逐一對照完成（10 處已接），
不再是待辦。現況見 `references/write-inventory.md`「其餘結構」`LastSet` 列、ChangeLog
`CHANGES_20260926_Steven.md` §11.25 與 `scratchpad\frw_s65\audit.md`。

**原句**：

> `WriteLastDataFile` | D | `cprod.cpp:1910` | lastdata.dat＋backup＋backup2 | ⚠ 本體已翻
> （`cprod.cpp:2001`）；golden 觸發者 **20+ 處**：`TfConfiguration::SaveConfiguration`／
> `edSoftSpeed0Change`／`edOCRTrayLotChange`、`TfStartCondition::sbSaveClick`／
> `spbExitClick`、`TfTowerLight::RGB00Click`、`TfSortCT::pnlAuto1DblClick`、
> `TfMain::FormShow`／`FormClose`／`UpdateMainOperateMode`／`Clarn_Data`（S11 已接）／
> `SetLotState`／`ProcessARTMessage`／`ShowTestHeadComp1`、`CheckContinusStartIsReady`、
> `DoART_AfterCleanOut`、SECS S7F4、`TfSCKART::AccessFile`、TCP 命令… ⏳ 逐一對 live 狀態

---

## 移出日期 20260926（15:3x）｜原本在「三、順序」第 3 點

**為什麼失效**：這句原是待辦事項（「golden 觸發者逐一對 live 狀態，缺的由產生器…改寫」），
現在四個都做完了：`SetLevelSet`＝S64、`WriteLastDataFile`＝S65、`SaveTestMode`＝S88、
`config.ini` 三支＝S89。

**原句**：

> 3. 形狀 B／D 的觸發者：`SaveTestMode`、`WriteLastDataFile`、`SetLevelSet` 等，golden
> 觸發者逐一對 live 狀態，缺的由產生器從 golden 改寫成 `act.*`

---

## 移出日期 20260926（17:xx，HEAD 8fad1522）｜第三批：唯讀普查第 4 節列的過期敘述，逐條對程式核對後更正

**共同原因**：同檔〇表（`:25-33`）早已把這些結構標成讀寫通過，下面各列卻還停在 20260924～0925 的「進行中」
狀態，前後矛盾；另有幾列的「⏳ 未移植」其實已有程式。來源：St01 唯讀普查（20260926 17:xx，HEAD 8fad1522）第 4 節，
每一條都重新看過程式（行號寫在現版各列）。以下依原位置列出原句。

### 「二、逐結構清單」→ `TestIF_File` 表 → `TFTestIF::SaveSetupFile`＋`spbSaveClick` 列

**為什麼失效**：A 形狀 bridge 在 `f89be4ce` 退役；讀檔端改由 C 路 `FileRW/TestIF_File_TesterIF.cpp` 轉 golden
`ReadTestIFFile`，開機／換配方 `tools/wb_serve.cpp:3231`（`8af13c07`）。

> | `TFTestIF::SaveSetupFile`＋`spbSaveClick` | A | `cTesterIF.cpp:337`／`:1320` | Tester.Data | `ReadTestIFFile` GATE (F-5) | ⚠ bridge 完成（ctest 24 項），讀檔端缺 → `sourceGap`（`porting-gaps.md` 十四） |

### 同上 → `DeviceForm_File` 表 → `TfContact::SaveSetupFile` 列的狀態欄

> 進行中：`16f463b8` 先整合顯示（唯讀）；`a9636d9c` 移植力量公式（`CalculateTotalAirForce`／`GetMaxIndexForceLimit`／`GetMinForce`／`CountDieForceKg` 與 DFM 事件處理器），存檔前 `DF_DeriveBeforeSave` 依 golden 事件順序重算衍生欄位（`Torque`、`Force Per Pin N/G`、`Double Force`…），只在缺 `ContactForce` 表時才拒存（詳見 `generators.md` 十四）；寫入測試（改 `G=50`）**尚未跑完**。原 A 形狀 `tools/formbridge/TfContact.py` 已退役（`b2aea32f`）

### 同上 → `Temperature` 表 → `TfTemp_Set::SaveSetupFile`＋`spbSaveClick` 列（形狀、讀檔端、狀態三欄）

**為什麼失效**：`322d68a3` 已整合（C 形狀）且 ATC.ini 已讀（`porting-gaps.md` 十三結案），〇表 Temp_Set 列 `bc935659` 讀寫通過。

> | `TfTemp_Set::SaveSetupFile`＋`spbSaveClick` | A | `uTemp_Set.cpp`（249 鍵） | Temperature.Data、Tester.Data | ✅ `ReadTempFile`，但 ⚠ ATC.ini 沒讀（`porting-gaps.md` 十三） | 進行中（20260925）：分兩塊——表單固定元件（一般具名替身）與執行期動態產生的 `TMyTempPanel *myTempPal[tcTotalCount]`（JSON 直接交換，不建面板具名替身，詳見 `generators.md` 十五）；主軸 `ReadTempFile(bUpdateAll)`／`DoIniDataToForm(bUpdateAll)`／`spbSaveClick`。**前置**：先補 golden 讀 `system\ATC.ini`（`iATC_MODE_TYPE`），否則每次開機 `Chiller Temp` 仍會被錯誤鉗制範圍夾成 5（先修十三這條再繼續，不要跳過）。20260925 交件（**未整合、未推**）：`forms/fATCHandlerSide.cpp` 新增 `W906_ReadATCIni()`、`tools/editlist/Temperature.py`→`FileRW/Temperature.cpp`。整合步驟：`new TfTemp_Set()` 後 `FileRW_Temperature_Boot()`；`Init()` 後依序 `FileRW_Temperature_BootPanels()`、`W906_ReadATCIni()`，都在 `ReadTempFile(true)` 之前；拿掉 `ReloadRecipeDocAfterSave` 的 "temperature" skip；temperature.data owner gate；引擎 map `Setup.Temp_Set.html`→`Temperature`。風險：iATC_MODE_TYPE 0→33 影響十幾處執行期（要跑 ctest）、存檔寫量產共用 Gerneral.ini、SOFT_SIMULTE 下 Tester.Data InitialDelay 會被寫成 5 |

### 同上 → `TestMode` 表 → `SaveTestMode` 列的最後一句

**為什麼失效**：`ChangeTesterConnect` 已由 Steven02 移植（`forms/fMain.cpp:1096`，`SaveTestMode()` 在 `:1233`；`979eac6b`，
併入 main `7f332938`），已接從 9 個變 10 個。

> 剩下 `ChangeTesterConnect`、`ShowTestHeadComp1`、`RunICModeChange`、`DoTrayFeedProcess`×3（S79）交 Jimmy／Steven02，詳見 `scratchpad\frw_s88\report.md` 第 1 節

### 同上 → `IniConfig` 表 → `SaveLastSetIni` 列的狀態欄、`TfMain::cbSetupFileNameChange` 整列

**為什麼失效**：(1) `FileRW/IniConfig.cpp` 的 `editlist.get` 已跑 golden `FormShow`（`IniConfig.cpp:226-257`），
〇表 Configuration 列讀寫通過，頁面在 `GOLDEN_BRIDGE`（`web/page/ht9045_wire_engine.js:1039`）；
(2) `cbSetupFileNameChange` 寫 `elConfig` 那一段只在 `CUSTOMER_CODE==CC_SCK` 才跑（主 repo V912 `main.cpp:25361-25375`），
`WebRecipeChange.cpp:398-399` 已標客戶專屬，依 S25 不做——不是「`main.cpp` 不在移植樹」的待辦。

> ⚠ 20260924 `FileRW/IniConfig.cpp`＋`IniConfig.gen.inc`：讀 ✅（開機實測）；寫：`editlist.save` 已接 golden FormClose 全流程，拒存規則實測過；缺 golden `FormShow` 顯示端（94 個替身值）→ 尚未做 G1。頁面 `Config.Configuration.html` 尚未改接（仍走 `system.file.put`，見 §四之二）
>
> | `TfMain::cbSetupFileNameChange` → `elConfig` | C | `main.cpp:25331` | config.ini | ⏳（`main.cpp` 整支不在移植樹） |

### 同上 →「其餘結構」表：`UserDefForm_File`、Tray Assignment、Offset 族、`Tech`、`ESD_GENERAL`、`MachRec` 六列

> `UserDefForm_File` 狀態欄：進行中：`tools/editlist/UserDefForm_File.py`、`FileRW/UserDefForm_File.cpp`／`.gen.inc` 已產生（`FileRW_TrayForm_Boot()`），驗收結果**待確認**，不要標成完成
>
> Tray Assignment 狀態欄的一句：修正後回歸測試仍剩 `Fix3/Direction` 1→0 一項差異，查證中，**不要標成完全通過**。
>
> Offset 族整列：| Offset 族（`*ArmOffSet_File`、`Offset_File`） | `TfOffSet::SaveSetupFile` | A | `cOffSet.cpp:1470`（48 鍵） | Position Offset.Data | 進行中：`tools/formbridge/TfOffSet.py` 已建（五工程師平行分工「Contact＋OffSet」那一組的 OffSet 半邊）；十一、下一波**不**把它排進 C 形狀轉換，暫留 A 形狀，尚未整合驗收。20260925 Steven 設計指示：主軸 `ReadFile`／`spbSaveClick`／`DoIniDataToForm(iNowOffsetSel, SpecialMode)`——「全部 offset 透過 JSON 整包傳輸，但點到某個按鈕才顯示指定的項目」，做法是**開頁時對每個選取跑一次 `DoIniDataToForm` 收成整包**，存檔對改過的每組套值後跑 golden `spbSaveClick`，詳見 `generators.md` 十五。20260925 改 C 形狀交件：`tools/editlist/Offset_File.py`→`FileRW/Offset_File_C.cpp`（`_KitSuck.cpp` 的 `FileRW_ArmSuckDims`／`FileRW_ArmSuckHasIC` 已隨 `fe4f4d1d` 推送），**未整合** |
>
> `Tech` 狀態欄：⏳ 移植樹無此函式（現況：`FileRW/Teach.cpp:147` `IC_SaveFile`，tech.dat 刻意不寫）
>
> `ESD_GENERAL` 狀態欄：⚠ 本體一致；觸發者 SECS `S2F15_UpdateNewEquipmentConstant` 在 `#if 0`（現況：`S2F15` 本身是活的，`uHGemHT9045.cpp:3249`；閘住的是裡面 `:3715` 那一段，GATE [E8]）
>
> `MachRec` 狀態欄：⏳ 對照（現況：`cinitial.cpp:7632-7639` 註記三個呼叫點 `LIFTED 20260925 (W3-12c)`）

### 同上 →「Gerneral.ini（形狀 E）」表 → `rgHeaterTypeClick`…`TASESendMessage::*` 那一列的狀態欄

> ⏳（現況：`rgHeaterTypeClick` 在 `FileRW/HSys.cpp:792-815` 重播、`btnSaveClick` 在 `FileRW/Teach.cpp`、`InitialGaliDelayCount` 本體 `Motor/myGALILmotor.cpp:1042` 缺呼叫者）

### 「四、golden `cprod.h` 全域變數的讀寫盤點」表：`TestIF_File`、`DeviceForm_File`、`UserDefForm_File[4]`、`TrayForm`、`Temperature`、`BinSelect[8]`、`Offset_File`、`TTLCfg`、`tAOISetup`、`Teach` 各列的最後一欄

> `TestIF_File`：ReadTestIFFile GATE F-5；fSetup／YM ReadFile live；Auto clean load/save GATE。`FileRW/TestIF_File.cpp` 只有 TFTestIF 半邊 → **擴充**（TfSetup／YM／Cleaning）
>
> `DeviceForm_File`：ReadFile live；A 形狀 bridge 已退役（commit `b2aea32f`）→ C 形狀進行中：`16f463b8` 顯示（唯讀），`a9636d9c` 移植力量公式＋存檔前重算衍生欄位（見二、`generators.md` 十四），寫入測試未跑完
>
> `UserDefForm_File[4]`：ReadFile live；顯示＋寫未移植 → **UserDefForm_File.cpp**
>
> `TrayForm`（句尾）：仍剩 `Fix3/Direction` 一項差異查證中
>
> `Temperature`：讀寫全 live（移植樹真碼）→ 只需 JSON 橋，porting-gaps 十三先修
>
> `BinSelect[8]`（中間一句）：**讀頁探針 FAIL**（頁面沒打到 editlist.get），未完成。
>
> `Offset_File`：讀 live；顯示＋寫未移植 → **Offset_File.cpp**
>
> `TTLCfg`（句尾兩句）：探針未跑起來（檔名空格），未驗。`InitDIOStstus`（TTLCfg→Prod.DIOCfg＋TTL 輸出）移植樹沒有，存檔 ack 列 ELTodo，20260925 已寄信請 Jimmy 排實作
>
> `tAOISetup`、`ScannerAOIIF`：未移植 → **AOISetup.cpp**
>
> `Teach`：ReadFile／SaveToFile live；SaveFile 未移植 → **Teach.cpp**

### 「〇、頁面讀寫總表」→ DIOInterFaceCFG 列的「還差什麼」欄

**為什麼失效**：Jimmy `0ca03ee6`（20260925 15:25）已把 golden `TfMain::InitDIOStstus` 移植到 `cDIOStatus.cpp`，DIO 設定頁
存檔後接上（`FileRW/TTLCfg.cpp:267-270`）；只有開機那一處仍閘著（`cDIOStatus.cpp:91`）。

> `InitDIOStstus` 未移植（已通知 Jimmy，存檔 ack 列 ELTodo）

### 「🆕 待派佇列」→ S67 那一行

**為什麼失效**：Steven 13:5x 裁示「不重要，往後排」（`RULINGS_20260926.md` S86～S87 表的 S67 列）。

> * S67 tech.dat（`TfSmartSetup`）——與 S75（`TfTeach::SaveFile`）同一個檔，暫不派，待分工。
