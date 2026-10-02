# HT9045_CONFIG 欄位速查：群組 [P] Tray（Tray / 料流）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Pxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 87 個欄位，分屬 74 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| P04 | must be turned on and [P24] must be turned on | `cbP04` | `bP04ColorIsEmptyUnloader` | bool | `bP04ColorIsEmptyUnloader` |  | ECBool | Color stage regard as empty tray unloader to use. |  |
| P05 | Tray lifter cylinder should pre-on when initial start | `cbP05` | `bP05_LoaderCylinderPreOn` | bool | `P05_LoaderCylinderPreOn` |  | ECBool | Tray lifter cylinder should pre-on when initial start | Steven 20150429 : 預先打兩下Loader汽缸 |
| P05-1 |  | `cbP05_1` | `bP05_1_UnloadCylinderPreOn` | bool | `bP05_1_UnloadCylinderPreOn` |  | ECBool | Unloader Tray lifter cylinder should pre-on when swap tray. | Steven 20240215 : 退tray前預先打兩下分離汽缸 |
| P06 | Carrier tray can use. | `cbP06` | `bP06_LoaderUseCarrierTray` | bool | `P06_LoaderUseCarrierTray` |  | ECBool | Carrier Tray Can Use. |  |
| P07 | Auto tray feed when loader no tray. | `cbP07` | `bP07NoTrayaAutoTrayFeed` | bool | `bP07NoTrayaAutoTrayFeed` |  | ECBool | Auto tray feed when loader no tray. | wei 20161121 TSMC 新增[P07]Loader No Traya Auto TrayFeed |
| P08 | Clear lot ID after tray feed. | `cbP08` | `bP08TrayFeedCleanLotID` | bool | `bP08TrayFeedCleanLotID` |  | ECBool | Clear lot ID after tray feed. | wei 20170316 Tray Feed Clean Lot ID |
| P09 | Tray end can select receive tray. | `cbP09` | `bP09TrayEndCanSelectTray` | bool | `bP09TrayEndCanSelectTray` |  | ECBool | Tray end can select receive tray. | Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定 |
| P10 | Fix tary loads new tary need propose the initial question. | `cbP10` | `bP10FixedTrayProposeTheInitialQuestion` | bool | `bP10FixedTrayProposeTheInitialQuestion` |  | ECBool | Fix tary loads new tary need propose the initial question. |  |
| P11 | Record UPH information | `cbP11` | `bP11RecordUPH` | bool | `RecordUPH` |  | ECBool | Record UPH information | Frank 20150515 |
| P13 | Enable auto tray edge push cylinder loop function. | `cbP13` | `bP13EnableAutoTrayEdgePushCylinderLoop` | bool | `bEnableAutoTrayEdgePushCylinderLoop` |  | ECBool | Enable auto tray edge push cylinder loop function. |  |
| P13-1 | Loop delay time                   (Unit : 0.1 Sec) | `edP13_1` | `iP13EdgePushCylinderLoopDelay` | int | `iP13EdgePushCylinderLoopDelay` |  | ECInteger | Loop delay time                   (Unit : 0.1 Sec) |  |
| P13-2 | On delay time                       (Unit : 0.1 Sec) | `edP13_2` | `iP13EdgePushCylinderOnDelay` | int | `iP13EdgePushCylinderOnDelay` |  | ECInteger | On delay time                       (Unit : 0.1 Sec) | JerryYang 20170531 (wei) 敲擊方式改成可以分別設定on off時間 |
| P14 | Enable auto tray receive delay function. | `cbP14` | `bP14EnableAutoTrayRecevieDelayCount` | bool | `bEnableAutoTrayRecevieDelayCount` |  | ECBool | Enable auto tray receive delay function. |  |
| P14-1 | Delay edge push cylinder count | `edP14_1` | `iP14AutoTrayRecevieDelayCount` | int | `iP14AutoTrayRecevieDelayCount` |  | ECInteger | Delay edge push cylinder count |  |
| P14-2 |  | `edP14_2` | `iP14AutoTrayRecevieLoopDelayTime` | int | `iP14AutoTrayRecevieLoopDelayTime` |  | ECInteger | Receive loop delay time                  (Unit : 0.1 Sec) |  |
| P15 | Free unload tray cylinder when open door | `cbP15` | `bUnloadTrayFree` | bool | `bUnloadTrayFree` |  | ECBool | Free unload tray cylinder when open door | jou 2011-05-27 |
| P16 | Enable hot plate edge push cylinder loop function. | `cbP16` | `bP16EnableHotplateEdgePushCylinderLoop` | bool | `bEnableHotplateEdgePushCylinderLoop` |  | ECBool | Enable hot plate edge push cylinder loop function. | jou 2011-08-09 : Hotplate也要敲敲敲 |
| P16-1 | Loop delay time                   (Unit : 0.1 Sec) | `edP16_1` | `iP16HotplateEdgePushCylinderLoopDelay` | int | `iHotplateEdgePushCylinderLoopDelay` |  | ECInteger | Loop delay time                   (Unit : 0.1 Sec) | jou 2011-08-09 : Hotplate也要敲敲敲 |
| P16-2 | On delay time                       (Unit : 0.1 Sec) | `edP16_2` | `iP16HotplateEdgePushCylinderOnDelay` | int | `iHotplateEdgePushCylinderOnDelay` |  | ECInteger | On delay time                       (Unit : 0.1 Sec) | JerryYang 20170531 (wei) 敲擊方式改成可以分別設定on off時間 |
| P17 | In arm picker must wait loader tray. (full pick up) | `cbP17` | `bP17InArmFullPickFromLoader` | bool | `DisableLoaderFull` |  | ECBool | In arm picker must wait loader tray. (full pick up) | Steven 20111026 : In Arm Full Pick from Loader Tray |
| P18 | Autotray is fail bin must manual put tray | `cbP18` | `bP18FailAutoTrayManual` | bool | `FailAutoTrayManual` |  | ECBool | Autotray is fail bin must manual put tray | jou 2012-03-16 Fail Auto Tray手動補Tray |
| P19 | Catch tray goes up then check if had catched tray | `cbP19` | `bP19CatchTrayUpThenCheck` | bool | `bCatchTrayUpThenCheck` |  | ECBool | Catch tray goes up then check if had catched tray | Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm |
| P20 | Must manual clear fix tray before initial start | `cbP20` | `bP20ManualClearFixTrayDataAfterInitialStart` | bool | `bManualClearFixTrayDataAfterInitialStart` |  | ECBool | Must manual clear fix tray before initial start | ChungHung 20130305 add for Amkor Initial Strat後不清Tray盤資料需手動清除後才可Run |
| P20-1 | Must manual clear auto tray before initial start | `chkP20_1` | `bP20_1_ManualClrAutoTrayWhenInitialStart` | bool | `bP20_1_ManualClrAutoTrayWhenInitialStart` |  | ECBool | Must manual clear auto tray before initial start | Steven 20210420 : add for TFME InitialStrat後不清Tray盤資料需手動清除後才可Run |
| P20-2 | Must manual clear loader tray before initial start | `chkP20_2` | `bP20_2_ManualClrLoadTrayWhenInitialStart` | bool | `bP20_2_ManualClrLoadTrayWhenInitialStart` |  | ECBool | Must manual clear loader tray before initial start | Steven 20230117 : add for JSCK InitialStrat後需手動清除Loader Tray後才可Run |
| P21 | Tray feed finish will notice to take out fix tray | `cbP21` | `bP21CheckFixTray` | bool | `bCheckFixTray` |  | ECBool | Tray feed finish will notice to take out fix tray | kevin 20130408 Tray feed 時偵測fix tray需取出 |
| P21-1 | Initial start check fix tray should have tray. | `chkP21_1` | `bP21_1_CheckFixTray` | bool | `bP21_1_CheckFixTray` |  | ECBool | Initial start check Fix tray should have tray. | Steven 20250321 initial start時偵測fix tray需放入 |
| P21-2 | Tray feed include loader tray. | `chkP21_2` | `bP21_2_LoaderTrayFeed` | bool | `bP21_2_LoaderTrayFeed` |  |  | Tray feed include loader tray. | Steven 20250606 : Tray Feed 包含 loader tray |
| P22 | When initial start, firsit tray need alarm | `cbP22` | `bP22EnableFirstTrayNeedAlarm` | bool | `bEnableFirstTrayNeedAlarm` |  | ECBool | When initial start firsit need alarm | ChungHung 20140521 add for ATK |
| P23-1 |  | `edP23_1` | `iOCRByNewTrayIntrvalTray` | int | `iOCRByNewTrayIntrvalTray` |  | ECInteger | OCR By New Tray Intrval Tray |  |
| P23-2 |  | `edP23_2` | `iOCRMaxInspDevices` | int | `iOCRMaxInspDevices` |  | ECInteger | Maximum decvices |  |
| P24 | Skip event happen need remove Empty and Color tray | `cbP24` | `bP24SkipEventNeedRemoveEmptyAndColorTray` | bool | `bSkipEventNeedRemoveEmptyAndColorTray` |  | ECBool | Skip event happen need remove Empty and Color tray | Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查 |
| P24 | Skip event happen need remove Empty and Color tray | — | `bP24_Active` | bool | — |  | — | Skip event happen need remove Empty and Color tray | JerryYang 20160425 add for 矽格北興 -- P24打勾 |
| P24 | Skip event happen need remove Empty and Color tray | — | `bP24_Enable` | bool | — |  | — | Skip event happen need remove Empty and Color tray | JerryYang 20160425 add for 矽格北興 -- P24 Enable |
| P24-2 | Skip event happen need remove Color tray (for IDT) | `cbP24_2` | `bP24SkipEventNeedRemoveColorTrayForIDT` | bool | `bP24SkipEventNeedRemoveColorTrayForIDT` |  | ECBool | Skip event happen need remove Color tray (for IDT) | Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤) |
| P24-3 | Two tray  must be manually removed for genernal | `cbP24_3` | `bP24TwoTrayMustManuallyRemovedForGenernal` | bool | `bP24TwoTrayMustManuallyRemovedForGenernal` |  | ECBool | Two tray  must be manually removed for genernal | Sam 20220817 : 一般P24功能連兩盤手動移除功能設開關 |
| P25 | Empty or Color tray no supple Auto 1 2 3, no load one tray | `cbP25` | `bP25EmptyColorNoSuppleAutoNoLoadEmpty` | bool | `bEmptyColorNoSuppleAutoNoLoadEmpty` |  | ECBool | Empty or Color tray no supple Auto 1 2 3, no load one tray | kevin 20151102  empty or color TRAY 不補AUTO 123 TRAY就不須載入一個空TRAY。 |
| P26 | OCR check lot | `cbP26` | `bP26_OCRCheckLot` | bool | `bP26_OCRCheckLot` |  | ECBool | OCR check lot | wei 20151117 OCR Lot check |
| P27 | Auto sorting bin tray by out arm when clean out | `cbP27` | `bP27AutoSortingBinTrayByOutArmwhenCleanOut` | bool | `bAutoSortingBinTrayByOutArmwhenCleanOut` |  | ECBool | Auto sorting bin tray by out arm when clean out | JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out |
| P28 | Auto 1 only can set bin 1 | `cbP28` | `bP28Auto1OnlyBin1` | bool | `bP28Auto1OnlyBin1` |  | ECBool | Auto 1 only can set bin 1 | Alick 20160729 add for SCC |
| P29 | Always check loader is full. Checking interval (sec): | `cbP29` | `bP29LoaderCheckIsFull` | bool | `bP29LoaderCheckIsFull` |  | ECBool | Always check loader is full. Checking interval (sec): | Steven 20160818 : CheckLoader滿盤 |
| P29 | Always check loader is full. Checking interval (sec): | `edP29` | `dP29LoaderCheckIsFullInterval` | double | `dP29LoaderCheckIsFullInterval` |  | ECBool | Always check loader is full. Checking interval (sec): | Steven 20160818 : CheckLoader滿盤 |
| P30 |  | `cbP30` | `bP30FixTryCheckRemainingAmount` | bool | `bP30FixTryCheckRemainingAmount` |  | ECBool | Fix Tray be Left Over IC Continue                            pieces | Ifor 20160829 : Check Fix Try 到達設定剩餘IC數量Alarm不停機 |
| P30 |  | `edP30` | `iP30FixCheckRemainingAmountInterval` | int | `iP30FixCheckRemainingAmountInterval` |  | ECBool | Fix Tray be Left Over IC Continue                            pieces | Ifor 20160829 : Fix Try 剩餘可放IC數量 |
| P31 | Loader tray last one feed continue run | `cbP31` | `bP31LoaderTryLastOneFeedContinueRun` | bool | `bP31LoaderTryLastOneFeedContinueRun` |  | ECBool | Loader tray last one feed continue run | Ifor 20160829 : Loader Try 最後一盤入料Alarm不停機 |
| P32 | Empty/Color Tray Pre Alarm | `cbP32` | `bP32EmptyColorTrayPreAlarm` | bool | `bP32EmptyColorTrayPreAlarm` |  | ECBool | Empty/Color Tray Pre Alarm | Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能 |
| P33 | Unloader Stock Full Pre Alarm | `cbP33` | `bP33AutoTrayPreAlarm` | bool | `bP33AutoTrayPreAlarm` |  | ECBool | Unloader Stock Full Pre Alarm | Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能 |
| P34 | Cleanout change initial start | `cbP34` | `bP34CleanOutChangeInitialMode` | bool | `bP34CleanOutChangeInitialMode` |  | ECBool | Cleanout change initial start | kevin 20170417 (wei) : CleanOut change initial Mode |
| P35 | Tray Arm home safe pos | `cbP35` | `bP35TrayArm` | bool | `bP35TrayArm` |  | ECBool | Tray Arm home safe pos | kevin 20171006 (wei) tray arm home需遮住home sensor |
| P36 | Load && Unload tray buffer tray trace no setup same | `cbP36` | `bP36BufferTrayNoSame` | bool | `bP36BufferTrayNoSame` |  | ECBool | Load && Unload tray buffer tray trace no setup same | kevin 20171117 (wei) Load && Unload 強制不能使用相同軌道 |
| P37 | Auto 1 2 3 Z cylinder always up. | `cbP37` | `bP37bAutoCylinderUP` | bool | `bP37bAutoCylinderUP` |  | ECBool | Auto 1 2 3 Z cylinder always up. | kevin 20180726 (wei) Auto 123 氣缸常態在上 |
| P38 | Use empty full tray put to color | `cbP38` | `bP38UseEmptyFullPutColor` | bool | `bP38UseEmptyFullPutColor` |  | ECBool | Use empty full tray put to color | wei 20170504 Use Empty Full Put Color |
| P39 |  | — | `bP39ClearUnloaderTrayWhenInitial` | bool | — |  | — | Loader tray-end, skip, clean-out event happen place to empty | Steven 20200826 : Murata希望在initial start的時候不要清除unloader tray,不然tray end的時候會導致疊料 |
| P39 |  | `cbP39` | `bP39LoaderHasSkipPlaceToEmpty` | bool | `bP39LoaderHasSkipPlaceToEmpty` |  | ECBool | Loader tray-end, skip, clean-out event happen place to empty | KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader |
| P40 | Tray Y speed by machine | — | `bP40LdUldUseEmptyAndColorTray` | bool | — |  | — | Tray Y speed by machine | Ifor 20201218 add: Load & Unload Use Empty And Color Tray |
| P40 | Tray Y speed by machine | `cbP40` | `bP40TrayYSpeedByMachine` | bool | `bP40TrayYSpeedByMachine` |  | ECBool | Tray Y speed by machine | Sam 20201221 : Tray y step motor by machine |
| P41 | Unload tray disable edit | `cbP41` | `bP41UnloadTrayDisableEdit` | bool | `bP41UnloadTrayDisableEdit` |  | ECBool | Unload tray disable edit | jou 20240403 : Unload Tray Disable Edit |
| P42 | Alarm when exiting Tray is complete | `cbP42` | `bP42AlarmWhenExitTrayComplete` | bool | `bP42AlarmWhenExitTrayComplete` |  | ECBool | Alarm when exiting Tray is complete | Ifor 20210428 add:Auto 退Tray 完成時報警停機 |
| P44 |  | `cbP44` | `bP44LockLoaderTrayToNone` | bool | `bP44LockLoaderTrayToNone` |  | ECBool | Lock 'Loader tray mode' to 'None'(Move out loader tray by tray arm) | JerryYang 20220331 : 矽品中山要求AUTO SKIP後可由TRAY ARM搬TRAY |
| P45 | Loader Empty tray No In Side | `cbP45` | `bP45LastLoaderNoInSide` | bool | `bP45LastLoaderNoInSide` |  | ECBool | Loader Empty tray No In Side | kevin 20221008 load 空盤 不TRAY |
| P46 | Loader tray mode save by handler. | `cbP46` | `bP46_LoadTrayModeByHandler` | bool | `bP46_LoadTrayModeByHandler` |  | ECBool | Loader tray mode save by handler. | Steven 20221117 : Loader Tray Mode設定跟著機台變 |
| P47 |  | — | `bP47_UseTrayThickAdjustZHeight` | bool | — |  | — |  | Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度 |
| P48 | Enable Auto1~3 edge push and Fixer cylinder loop | `cbP48` | `bP48UnloaderCylinderLoop` | bool | `bP48UnloaderCylinderLoop` | -- | -- | Edge push and Fixer cylinder loop | JimmyChiu 20230512 Unloader cylinder loop |
| P48 | Enable Auto1~3 edge push and Fixer cylinder loop | `edP48DelayTime` | `iP48UnloaderCylinderLoopDelay` | int | `iP48UnloaderCylinderLoopDelay` | -- | -- | Edge push and Fixer cylinder loop | JimmyChiu 20230512 Unloader cylinder loop |
| P48 | Enable Auto1~3 edge push and Fixer cylinder loop | `edP48Times` | `iP48UnloaderCylinderLoopTimes` | int | `iP48UnloaderCylinderLoopTimes` | -- | -- | Edge push and Fixer cylinder loop | JimmyChiu 20230512 Unloader cylinder loop |
| P49 | Use Local Tray Speed | `cbP49` | `bP49UseLocalTraySpeed` | bool | `bP49UseLocalTraySpeed` |  | ECBool | Use Local Tray Speed | Ifor 20200825 add: Use Local Tray Speed |
| P50 | Disabled auto track sensor detect | `cbP50` | `bP50DisabledAutoTrackSensorDetect` | bool | `bP50DisabledAutoTrackSensorDetect` |  | ECBool | Disabled auto track sensor detect | Sam 20230221 : 矽格中興國桂要求要能關閉 |
| P51 | Tray Arm wait unload tray finish, then put tray to track | `cbP51` | `bP51TrayArmPutwaitUnloadOK` | bool | `bP51TrayArmPutwaitUnloadOK` |  | ECBool | Tray Arm wait Unload tray  finish,put on tray | kevin 20230331 add TRAY arm 放 auto 123 等待tray上升避免輸 送帶滑進去 夾tray. |
| P52 | Empty/Color Last Tray Check | `cbP52` | `bP52EmptyColorLastTrayCheck` | bool | `bP51EmptyColorLastTrayCheck` |  | ECBool | Empty/Color Last Tray Check | KenHsieh 20230919 : Empty & Color Last Tray Check |
| P53 |  | `cbP53` | `bP53_ForcedScanBinCodeOfUnloader` | bool | `bP53_ForcedScanBinCodeOfUnloader` |  | ECBool | Forced scan bin code label before takeout the tray from unloader. | JerryYang 20240111 : add P53 function |
| P54 | Unloader tary check has error bin IC | `cbP54` | `P54UnloaderTaryCheckHasErrorBinIC` | bool | `P54UnloaderTaryCheckHasErrorBinIC` |  | ECBool | Unloader tary check has error bin IC | Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin |
| P55 |  | `cbP55` | `bP55LdUldUseEmptyAndColorTray` | bool | `bP40LdUldUseEmptyAndColorTray` |  | ECBool | Load && Unload Use Empty And Color Tray ( 4 Auto Tray Lane Only) | Ifor 20201218 add: Load & Unload Use Empty And Color Tray |
| P56 | Tray arm wait at color track | `chkP56` | `bP56TrayArmWaitAtColorTrack` | bool | `bP56TrayArmWaitAtColorTrack` |  | ECBool | Tray arm wait at color track | Steven 20240516 : Tray Arm等待位置改到Color |
| P57 | Loader auto clean out by input count | `cbP57` | `bP57LoaderAutoCleanOutByInputCT` | bool | `bP57LoaderAutoCleanOutByInputCT` |  | ECBool | Load stack check 3 Sensor | Sam 20250605 : Loader Count AutoCleanOut |
| P58 | Run mode After FT | `rgP58RunModeAfterFT` | `iP58RunModeAfterFT` | int | `iP58RunModeAfterFT` |  | ECBool | Bin Matrix for Sort | JimmyChiu 20250905 : FT模式結束後切換模式 |
| P59 | Unloader IC Floatting Alarm after exit | `cbP59` | `bP59UnloaderICFloattingAlarmAfterExit` | bool | `bP57UnloaderICFloattingAlarmAfterExit` |  | ECBool | Use Tray Tap | Sam 20250415 : Unloader 偵測到置偏 IC 退出後再報警 |
| P60 | Read Clip Code From Unloader(Auto1-3'#12289'Fix1-3) | `cbP60` | `bP60ReadClipCodeFromUnloader` | bool | `bP60ReadClipCodeFromUnloader` |  | ECBool | Read Clip Code From Unloader(Auto1-3、Fix1-3) | Jimmychiu 20250818 : Read Clip Code From Unloader(Auto1-3、Fix1-3) |
| P61 |  | — | `bP61UseTrayTap` | bool | — |  | — |  | Ztex 2024.11.12 Add P53 Use Tray Tap |
| P62 | First Tray Check On Unloader | `cbP62_Auto1` | `bP62Auto1` | bool | `bP62Auto1` |  | ECBool | First Tray Check On Unloader | Jimmychiu 20251205 : First Tray Check On Unloader |
| P62 | First Tray Check On Unloader | `cbP62_Auto2` | `bP62Auto2` | bool | `bP62Auto2` |  | ECBool | First Tray Check On Unloader | Jimmychiu 20251205 : First Tray Check On Unloader |
| P62 | First Tray Check On Unloader | `cbP62_Auto3` | `bP62Auto3` | bool | `bP62Auto3` |  | ECBool | First Tray Check On Unloader | Jimmychiu 20251205 : First Tray Check On Unloader |
| P62 | First Tray Check On Unloader | `cbP62` | `bP62FirstTrayCheckOnUnloader` | bool | `bP62FirstTrayCheckOnUnloader` |  | ECBool | First Tray Check On Unloader | Jimmychiu 20251205 : First Tray Check On Unloader |
| P62-1 | Function always enabled every time lot start | `cbP62_1` | `bP62AlwaysEnabledAtLotStart` | bool | `bP62AlwaysEnabledAtLotStart` |  |  | Function always enabled every time lot start | Jimmychiu 20251205 : First Tray Check On Unloader |
| P63 |  | `cbP63` | `bP63MachineStopAtIntervalTime` | ? | `bP63MachineStopAtIntervalTime` |  |  |  |  |
| P63 |  | `edP63` | `iP63IntervalTime` | ? | `iP63IntervalTime` |  |  |  |  |
| P65 | Enable ARM QA Mode | `cbP65` | `bP65EnableArmQAMode` | bool | `bP65EnableArmQAMode` |  |  |  | Ifor 20260407 : [P65] Enable ARM QA Mode |
| P65 | Enable ARM QA Mode | `edtP65` | `iP65ArmQAModeValue` | int | `iP65ArmQAModeValue` |  |  |  | Ifor 20260407 : [P65] ARM QA Mode Value |

---


## 未綁定 UI 元件的欄位

_無。所有 `P##` 開頭的欄位均已在主表中列出。_
