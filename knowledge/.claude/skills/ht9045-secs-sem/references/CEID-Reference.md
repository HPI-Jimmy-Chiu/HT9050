# CEID 收集事件參照表

> 資料來源：`SECS_20260416_Steven.xlsx` + `uHGemHT9045.h` ETypeStruct enum
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`
>
> **注意**：每個 CEID 預設自帶一個與 CEID 編號相同的 Report ID，該 Report 只含一個變數：**SVID 1027**（系統時間）。

## 版本控制

| 版本 | 日期 | 更新者 | 說明 |
|------|------|--------|------|
| V1.00 | 2026-04-01 | Steven | 初版：Excel 252 + 程式碼 288 事件 |
| V1.01 | 2026-04-16 | Steven (AI) | 加入版本控制機制 |
| V1.02 | 2026-04-16 | Steven (AI) | 同步 SECS_20260416_Steven.xlsx：Excel HT9045 增至 288，與 Code 289 一致（+TotalEvent 哨兵） |

Excel 定義 **288** 個 CEID，程式碼列舉 **289** 個（含 TotalEvent 哨兵）。

---

## EventReport 函式說明

> 來源：`uHGemEquipment.cpp` — `void __fastcall THGem::EventReport(unsigned iDataID, unsigned iCeid)`

### 呼叫方式

```cpp
EventReport(1, SECS_EVENT.DoStart);   // iDataID 固定傳 1，iCeid 傳對應 CEID 編號
```

### 封包格式

| 條件 | 訊息 | 格式 |
|------|------|------|
| 一般（預設） | **S6F11** Event Report Send | `L[3]` `<U4 DataID>` `<U4 CEID>` `<Report Data>` |
| `chkAnnotatedEventReport` 勾選 | **S6F13** Annotated Event Report Send | `L[3]` `<U4 DataID>` `<U4 CEID>` `<Annotated Report Data>` |

> `Report Data` 由 `SendCeid(iCeid)` 根據已登錄的 Report ID → SVID 組成。

### 抑制條件（不發送）

| 條件 | 說明 |
|------|------|
| `IniConfig.bEnable_SECS_GEM == false` | SECS/GEM 功能未開啟，直接 return |
| `CUSTOMER_CODE == CC_TFME_CHINA && GemControlState <= 1` | TFME China 客戶在 Offline 狀態下不上報 |
| `IsEnableEvent()` 回傳 false | 該 CEID 在 Host 端被 S2F37 disable，跳過並印 log |

### 特殊行為

| CEID | 名稱 | 說明 |
|-----:|------|------|
| 24 | `DoExit` | Event 送出後**自動執行** `DoSeparate()` 並關閉 SECS/GEM 連線（`srvGem->Close()` / `clientGem->Close()`、Timer 停止、Form 關閉） |

---

## 完整 CEID 對照表

| CEID | 程式碼名稱 | 說明 | 程式碼 |
|-----:|-----------|------|:------:|
| 1 | DoStart | Press Start button without IC inside handler | ✓ |
| 2 | DoPause | Press Pause button | ✓ |
| 3 | DoOneCycle | Press One Cycle button | ✓ |
| 4 | DoCleanOut | Press Clean Out button | ✓ |
| 5 | DoClearCount | Press Clear Count button | ✓ |
| 6 | DoLotStart | Press Lot Start button | ✓ |
| 7 | DoLot | Press Lot button | ✓ |
| 8 | DoLotEnd | Press Lot End | ✓ |
| 9 | SwitchRunMode | Switching Real/Dummy Mode | ✓ |
| 10 | SwitchTesterMode | Switching Tester Online/Offline Mode | ✓ |
| 11 | SwitchProduction | Switching Production / Adjustment mode | ✓ |
| 12 | SwitchEngineer | Switching Normal/Engineering mode | ✓ |
| 13 | SwitchTemperature | Switching Ambient/Temperature | ✓ |
| 14 | SwitchStartMode | Switching Start Mode | ✓ |
| 15 | SwitchSetupFile | Switching setup file | ✓ |
| 16 | SwitchUser | Switching User Level | ✓ |
| 17 | EnterTool | Enter Tool Page | ✓ |
| 18 | EnterConfig | Enter Maintenance Page | ✓ |
| 19 | EnterOffset | Enter Offset Page | ✓ |
| 20 | EnterSpeed | Enter Speed Page | ✓ |
| 21 | EnterIO | Enter I/O Page | ✓ |
| 22 | EnterMessage | Enter Message Page | ✓ |
| 23 | EnterDebug | Enter Debug Page | ✓ |
| 24 | DoExit | Press Exit button | ✓ |
| 25 | DoHome | Press Home button | ✓ |
| 26 | GetTestResult | Get Test Result | ✓ |
| 27 | RunStatus | Change Machine State | ✓ |
| 28 | DoRetry | Press Retry button | ✓ |
| 29 | DoSkip | Press Skip button | ✓ |
| 30 | DoAlarmReset | Press Alarm Reset button | ✓ |
| 31 | DoTrayEnd | Press Tray End button | ✓ |
| 32 | DoTrayFeed | Press Tray Feed button | ✓ |
| 33 | DoReset | Press Reset button | ✓ |
| 34 | DoAutoClean | Auto Clean Start | ✓ |
| 35 | Auto1Full | Auto 1 Full | ✓ |
| 36 | Auto2Full | Auto 2 Full | ✓ |
| 37 | Auto3Full | Auto 3 Full | ✓ |
| 38 | Fix1Full | Fix 1 Full | ✓ |
| 39 | Fix2Full | Fix 2 Full | ✓ |
| 40 | Fix3Full | Fix 3 Full | ✓ |
| 41 | OneCycleFinish | One Cycle Finish | ✓ |
| 42 | CleanOutFinish | Clean Out Finish | ✓ |
| 43 | DownloadRecipe | DownLoad Recipe | ✓ |
| 44 | SiteOnOff | Site On/Off | ✓ |
| 45 | ArmOnOff | Arm Enabled/Disabled | ✓ |
| 46 | SwitchTempData | Change Temp Default and Soak Time | ✓ |
| 47 | SwitchSpeed | Change Handler Speed | ✓ |
| 48 | ChangeEC | Change EC | ✓ |
| 49 | TrayFeedFinish | Tray Feed Finish | ✓ |
| 50 | AutoCleanFinish | Auto Clean Finish | ✓ |
| 51 | SiteMappingStart | Site Mapping Start | ✓ |
| 52 | SiteMappingEnd | Site Mapping End | ✓ |
| 53 | UPHRecordStart | UPH Record Start | ✓ |
| 54 | UPHRecordEnd | UPH Record End | ✓ |
| 55 | InitialArtStart | Initial ART Start | ✓ |
| 56 | TesterFT | Change Tester Program to FT | ✓ |
| 57 | TesterRT | Change Tester Program to RT | ✓ |
| 58 | ReadyForArt | Ready for ART | ✓ |
| 59 | ArtReceiveTrayOK | ART Receive Tray OK | ✓ |
| 60 | ArtReceiveTraySTART | ART Receive Tray START | ✓ |
| 61 | ArtRTFinish | RT Finish | ✓ |
| 62 | ArtTrayFeedFinish | ART Finish | ✓ |
| 63 | ArtFTFinish | FT Finish | ✓ |
| 64 | DownLoadRecipeByFTPOK | DownLoad Recipe by FTP OK | ✓ |
| 65 | DownLoadRecipeByFTPNG | DownLoad Recipe by FTP NG | ✓ |
| 66 | LoadTrayFinish | Load Tray Finish | ✓ |
| 67 | TrayTestFinish | Tray Test Finish | ✓ |
| 68 | AutoCleanClearCount | Auto Clean Clear Count | ✓ |
| 69 | SiteMappingStop | Site Mapping Stop | ✓ |
| 70 | BarcodeReaderEnter | Barcode Reader Enter | ✓ |
| 71 | OTDLock | OTD Lock | ✓ |
| 72 | OTDUnLock | OTD UnLock | ✓ |
| 73 | MymessboxOK | MymessboxOK | ✓ |
| 74 | RemoteProgramClose | Remote Program Close | ✓ |
| 75 | ChangeTesterPrgToEQC | Change Tester Program to EQC | ✓ |
| 76 | DoStartHasIC | HasIC Press Start button | ✓ |
| 77 | ReadCurrentESDData | Read Current ESD Data | ✓ |
| 78 | JamSkipICCount | Jam Skip IC Count | ✓ |
| 79 | REVERSED79 | REVERSED79 | ✓ |
| 80 | ReadNowHandlerData | Read Now Handler Data | ✓ |
| 81 | ReadATCTemperature | Read ATC Temperature | ✓ |
| 82 | ReadATCRefTemperature | Read ATC Ref Temperature | ✓ |
| 83 | ReadNowEPPenconder | Read Now EP Penconder | ✓ |
| 84 | RunStatus_FT | Run Status FT | ✓ |
| 85 | RunStatus_RT | Run Status RT | ✓ |
| 86 | MapNoArmHasIC | Map No Device Arm Has Device | ✓ |
| 87 | MapHasICArmRetry | Map Has Device Arm Error Retry | ✓ |
| 88 | MapHasICArmSkip | Map Has Device Arm Error Skip | ✓ |
| 89 | PreAlarmMessage | Pre Alarm Message | ✓ |
| 90 | GetTestResultAndBarcode | Get Test Result and Barcode | ✓ |
| 91 | SECSOffline | SECS Offline | ✓ |
| 92 | SECSOnline | SECS Online | ✓ |
| 93 | SECSOnlineRemote | SECS Online Remote | ✓ |
| 94 | TransferBlocked | Transfer Blocked | ✓ |
| 95 | CassetteLoadComplete | Cassette Load Complete | ✓ |
| 96 | CassetteIDReadComplete | Cassette ID Read Complete | ✓ |
| 97 | ReadyToProcessComplete | Ready To Process Complete | ✓ |
| 98 | ReadyToCarrierOutLot | Ready To Carrier Out Lot | ✓ |
| 99 | CassetteOutComplete | Cassette Out Complete | ✓ |
| 100 | CassetteUnclamped | Cassette Unclamped | ✓ |
| 101 | ReadyToUnload | Ready To Unload | ✓ |
| 102 | UnloadComplete | Unload Complete | ✓ |
| 103 | ReadyToCarrierOutTray | Ready To Carrier Out Tray | ✓ |
| 104 | ReadyToCombinePass | Ready To Combine Pass | ✓ |
| 105 | ReadyToCombineFail | Ready To Combine Fail | ✓ |
| 106 | MachineNoStart | Machine No Start | ✓ |
| 107 | ReadyToCombinePassLotEnd | Ready To Combine Pass Lot End | ✓ |
| 108 | DoCSTLotStart | Cassette Lot Start | ✓ |
| 109 | DieCountFailMessageClose | Die Count Fail Message Close | ✓ |
| 110 | CleanOutTrayFeedFinish | Clean Out Tray Feed Finish | ✓ |
| 111 | MapNoICArmAutoSkip | Map No Device Arm Auto Skip | ✓ |
| 112 | MRRunModeChange | MR Run Mode Change | ✓ |
| 113 | AccessModeChange | Access Mode Change | ✓ |
| 114 | SoftwareBin | Software Bin | ✓ |
| 115 | TrayIDChange | Tray ID Change | ✓ |
| 116 | ReadyToLoadNoLot | Ready To Load No Lot | ✓ |
| 117 | ReadyToLoadNoTray | Ready To Load No Tray | ✓ |
| 118 | ReadyToLoadNoCassette | Ready To Load No Cassette | ✓ |
| 119 | ART_SRQKIND2_FTLOTSTART | SRQKIND2:  FT lot start | ✓ |
| 120 | ART_SRQKIND4_RTLOTSTART | SRQKIND4:  RT lot start | ✓ |
| 121 | ART_SRQKIND8_LOTEND | SRQKIND8:  lot end | ✓ |
| 122 | ART_SRQKIND10_FINALLOTEND | SRQKIND10:  Final lot end | ✓ |
| 123 | SafeDoorOnOff | Safe Door On Off | ✓ |
| 124 | SaveRecipe | Save Recipe | ✓ |
| 125 | EESUGOffestSelect | EESUG Offest Select | ✓ |
| 126 | EESUGOffestModify | EESUG Offest Modify | ✓ |
| 127 | Backtonormal | Back to normal | ✓ |
| 128 | TestStart | Test Start | ✓ |
| 129 | TestFinish | Test Finish | ✓ |
| 130 | MaterialReceive | Material Receive | ✓ |
| 131 | SlotMapCountOK | Slot Map Count OK | ✓ |
| 132 | CHECK_IN | CHECK IN | ✓ |
| 133 | CHECK_OUT | CHECK OUT | ✓ |
| 134 | ReadyToCombineFailLotEnd | Ready To Combine Fail Lot End | ✓ |
| 135 | ReadyToOHTLotEnd | OHT Lot End | ✓ |
| 136 | Auto1Unloadtray | Auto 1 Unload tray | ✓ |
| 137 | Auto2Unloadtray | Auto 2 Unload tray | ✓ |
| 138 | Auto3Unloadtray | Auto 3 Unload tray | ✓ |
| 139 | DoVisualSortLotStart | DoVisualSortLotStart | ✓ |
| 140 | PreLoadTray | PreLoadTray | ✓ |
| 141 | GemControlStateChange | GemControlStateChange | ✓ |
| 142 | PickerCountWasCleared | Picker Count was Cleared. | ✓ |
| 143 | UploadPickerCount | Upload Picker Count | ✓ |
| 144 | RequestPickerCount | Request Picker Count | ✓ |
| 145 | Auto4Unloadtray | Auto 4 Unload tray | ✓ |
| 146 | Auto5Unloadtray | Auto 5 Unload tray | ✓ |
| 147 | Auto6Unloadtray | Auto 6 Unload tray | ✓ |
| 148 | Auto4Full | Auto 4 Full | ✓ |
| 149 | Auto5Full | Auto 5 Full | ✓ |
| 150 | Auto6Full | Auto 6 Full | ✓ |
| 151 | Fix4Full | Fix 4 Full | ✓ |
| 152 | Fix5Full | Fix 5 Full | ✓ |
| 153 | Fix6Full | Fix 6 Full | ✓ |
| 154 | LoadNoTray | LoadNoTray | ✓ |
| 155 | LoadFullTray | LoadFullTray | ✓ |
| 156 | LoadOnlyOneTray | LoadOnlyOneTray | ✓ |
| 157 | Loader_ReadyToUnload | Loader_ReadyToUnload | ✓ |
| 158 | Loader_FinishUnload | Loader_FinishUnload | ✓ |
| 159 | Empty_PreLoadTray | Empty_PreLoadTray | ✓ |
| 160 | EmptyOnlyOneTray | EmptyOnlyOneTray | ✓ |
| 161 | EmptyNoTray | EmptyNoTray | ✓ |
| 162 | EmptyFullTray | EmptyFullTray | ✓ |
| 163 | Color_PreLoadTray | Color_PreLoadTray | ✓ |
| 164 | ColorOnlyOneTray | ColorOnlyOneTray | ✓ |
| 165 | ColorNoTray | ColorNoTray | ✓ |
| 166 | Empty_PutTrayToAuto1 | Empty_PutTrayToAuto1 | ✓ |
| 167 | Empty_PutTrayToAuto2 | Empty_PutTrayToAuto2 | ✓ |
| 168 | Empty_PutTrayToAuto3 | Empty_PutTrayToAuto3 | ✓ |
| 169 | Empty_PutTrayToAuto4 | Empty_PutTrayToAuto4 | ✓ |
| 170 | Empty_PutTrayToAuto5 | Empty_PutTrayToAuto5 | ✓ |
| 171 | Empty_PutTrayToAuto6 | Empty_PutTrayToAuto6 | ✓ |
| 172 | Empty_PutCoverToAuto1 | Empty_PutCoverToAuto1 | ✓ |
| 173 | Empty_PutCoverToAuto2 | Empty_PutCoverToAuto2 | ✓ |
| 174 | Empty_PutCoverToAuto3 | Empty_PutCoverToAuto3 | ✓ |
| 175 | Empty_PutCoverToAuto4 | Empty_PutCoverToAuto4 | ✓ |
| 176 | Empty_PutCoverToAuto5 | Empty_PutCoverToAuto5 | ✓ |
| 177 | Empty_PutCoverToAuto6 | Empty_PutCoverToAuto6 | ✓ |
| 178 | Color_PutTrayToAuto1 | Color_PutTrayToAuto1 | ✓ |
| 179 | Color_PutTrayToAuto2 | Color_PutTrayToAuto2 | ✓ |
| 180 | Color_PutTrayToAuto3 | Color_PutTrayToAuto3 | ✓ |
| 181 | Color_PutTrayToAuto4 | Color_PutTrayToAuto4 | ✓ |
| 182 | Color_PutTrayToAuto5 | Color_PutTrayToAuto5 | ✓ |
| 183 | Color_PutTrayToAuto6 | Color_PutTrayToAuto6 | ✓ |
| 184 | Color_PutCoverToAuto1 | Color_PutCoverToAuto1 | ✓ |
| 185 | Color_PutCoverToAuto2 | Color_PutCoverToAuto2 | ✓ |
| 186 | Color_PutCoverToAuto3 | Color_PutCoverToAuto3 | ✓ |
| 187 | Color_PutCoverToAuto4 | Color_PutCoverToAuto4 | ✓ |
| 188 | Color_PutCoverToAuto5 | Color_PutCoverToAuto5 | ✓ |
| 189 | Color_PutCoverToAuto6 | Color_PutCoverToAuto6 | ✓ |
| 190 | Auto1_LoadTrayFinish | Auto1_LoadTrayFinish | ✓ |
| 191 | Auto2_LoadTrayFinish | Auto2_LoadTrayFinish | ✓ |
| 192 | Auto3_LoadTrayFinish | Auto3_LoadTrayFinish | ✓ |
| 193 | Auto4_LoadTrayFinish | Auto4_LoadTrayFinish | ✓ |
| 194 | Auto5_LoadTrayFinish | Auto5_LoadTrayFinish | ✓ |
| 195 | Auto6_LoadTrayFinish | Auto6_LoadTrayFinish | ✓ |
| 196 | Auto1_ReadyToUnload | Auto1_ReadyToUnload | ✓ |
| 197 | Auto2_ReadyToUnload | Auto2_ReadyToUnload | ✓ |
| 198 | Auto3_ReadyToUnload | Auto3_ReadyToUnload | ✓ |
| 199 | Auto4_ReadyToUnload | Auto4_ReadyToUnload | ✓ |
| 200 | Auto5_ReadyToUnload | Auto5_ReadyToUnload | ✓ |
| 201 | Auto6_ReadyToUnload | Auto6_ReadyToUnload | ✓ |
| 202 | Auto1NoTray | Auto1NoTray | ✓ |
| 203 | Auto2NoTray | Auto2NoTray | ✓ |
| 204 | Auto3NoTray | Auto3NoTray | ✓ |
| 205 | Auto4NoTray | Auto4NoTray | ✓ |
| 206 | Auto5NoTray | Auto5NoTray | ✓ |
| 207 | Auto6NoTray | Auto6NoTray | ✓ |
| 208 | ColorFullTray | ColorFullTray | ✓ |
| 209 | TrayEndFinish | TrayEndFinish | ✓ |
| 210 | Empty_FinishUnload | Empty_FinishUnload | ✓ |
| 211 | Color_FinishUnload | Color_FinishUnload | ✓ |
| 212 | PowerSavingStart | Power Saving Start | ✓ |
| 213 | PowerSavingEnd | Power Saving End | ✓ |
| 214 | Reserved_03 | Reserved 03 | ✓ |
| 215 | Reserved_04 | Reserved 04 | ✓ |
| 216 | Reserved_05 | Reserved 05 | ✓ |
| 217 | LoadPortStatusChanged | Load port status changed | ✓ |
| 218 | EmptyPortStatusChanged | Empty port status changed | ✓ |
| 219 | ColorPortStatusChanged | Color port status changed | ✓ |
| 220 | Auto1PortStatusChanged | Auto1 port status changed | ✓ |
| 221 | Auto2PortStatusChanged | Auto2 port status changed | ✓ |
| 222 | Auto3PortStatusChanged | Auto3 port status changed | ✓ |
| 223 | Fix1PortStatusChanged | Fix1 port status changed | ✓ |
| 224 | Fix2PortStatusChanged | Fix2 port status changed | ✓ |
| 225 | Fix3PortStatusChanged | Fix3 port status changed | ✓ |
| 226 | Auto4PortStatusChanged | Auto4 port status changed | ✓ |
| 227 | Auto5PortStatusChanged | Auto5 port status changed | ✓ |
| 228 | Auto6PortStatusChanged | Auto6 port status changed | ✓ |
| 229 | Fix4PortStatusChanged | Fix4 port status changed | ✓ |
| 230 | Fix5PortStatusChanged | Fix5 port status changed | ✓ |
| 231 | Fix6PortStatusChanged | Fix6 port status changed | ✓ |
| 232 | Reserved_21 | Reserved 21 | ✓ |
| 233 | Reserved_22 | Reserved 22 | ✓ |
| 234 | SafetyDoorOpen | Safety Door Open | ✓ |
| 235 | SafetyDoorClosed | Safety Door Closed | ✓ |
| 236 | LoadPortBundleArrived | Load Port Bundle Arrived | ✓ |
| 237 | LoadPortBundleRead | Load Port Bundle Read | ✓ |
| 238 | RemoteStart | Remote start | ✓ |
| 239 | UnexpectedBundleIDRead | Un-expected Bundle ID Read | ✓ |
| 240 | UnexpectedUNITIDRead | Un-expected UNIT ID Read | ✓ |
| 241 | BundleCompleteProcessed | Bundle Complete Processed | ✓ |
| 242 | BundleCompleteIDRead | Bundle Complete ID Read | ✓ |
| 243 | NoCoverTray_2DID | No Cover Tray (2D ID) | ✓ |
| 244 | NoCoverTray_Normal | No Cover Tray (Normal) | ✓ |
| 245 | BundleEnd_Auto1 | Bundle End(Auto1) | ✓ |
| 246 | BundleEnd_IDREAD_Auto1 | Bundle End ID Read(Auto1) | ✓ |
| 247 | BundleEnd_Auto2 | Bundle End(Auto2) | ✓ |
| 248 | BundleEnd_IDREAD_Auto2 | Bundle End ID Read(Auto2) | ✓ |
| 249 | ProcessEnd | Process End | ✓ |
| 250 | DoStartAutoHeight | START Auto contact height | ✓ |
| 251 | BundleEnd_Auto3 | Bundle End(Auto3) | ✓ |
| 252 | BundleEnd_IDREAD_Auto3 | Bundle End ID Read(Auto3) | ✓ |
| 253 | BundleEnd_Auto4 | Bundle End(Auto4) | ✓ |
| 254 | BundleEnd_IDREAD_Auto4 | Bundle End ID Read(Auto4) | ✓ |
| 255 | BundleEnd_Auto5 | Bundle End(Auto5) | ✓ |
| 256 | BundleEnd_IDREAD_Auto5 | Bundle End ID Read(Auto5) | ✓ |
| 257 | BundleEnd_Auto6 | Bundle End(Auto6) | ✓ |
| 258 | BundleEnd_IDREAD_Auto6 | Bundle End ID Read(Auto6) | ✓ |
| 259 | BundleEnd_Fix1 | Bundle End(Fix1) | ✓ |
| 260 | BundleEnd_IDREAD_Fix1 | Bundle End  ID Read(Fix1) | ✓ |
| 261 | BundleEnd_Fix2 | Bundle End(Fix2) | ✓ |
| 262 | BundleEnd_IDREAD_Fix2 | Bundle End  ID Read(Fix2) | ✓ |
| 263 | BundleEnd_Fix3 | Bundle End(Fix3) | ✓ |
| 264 | BundleEnd_IDREAD_Fix3 | Bundle End  ID Read(Fix3) | ✓ |
| 265 | BundleEnd_Fix4 | Bundle End(Fix4) | ✓ |
| 266 | BundleEnd_IDREAD_Fix4 | Bundle End  ID Read(Fix4) | ✓ |
| 267 | BundleEnd_Fix5 | Bundle End(Fix5) | ✓ |
| 268 | BundleEnd_IDREAD_Fix5 | Bundle End  ID Read(Fix5) | ✓ |
| 269 | BundleEnd_Fix6 | Bundle End(Fix6) | ✓ |
| 270 | BundleEnd_IDREAD_Fix6 | Bundle End  ID Read(Fix6) | ✓ |
| 271 | LoaderTrayState | Loader Tray State | ✓ |
| 272 | AGVSupplement | AGVSupplement | ✓ |
| 273 | AGVLDUnLDStatus | AGVLDUnLDStatus | ✓ |
| 274 | SECSGEMConsecutiveFailure | SECS GEM consecutive failure | ✓ |
| 275 | Loader_Buffer_HasTray | Loader Buffer Has Tray | ✓ |
| 276 | Loader_Buffer_NoTray | Loader Buffer No Tray | ✓ |
| 277 | OutputPort1BinCode | Output Port 1 Bin Code | ✓ |
| 278 | OutputPort2BinCode | Output Port 2 Bin Code | ✓ |
| 279 | OutputPort3BinCode | Output Port 3 Bin Code | ✓ |
| 280 | OutputPort4BinCode | Output Port 4 Bin Code | ✓ |
| 281 | OutputPort5BinCode | Output Port 5 Bin Code | ✓ |
| 282 | OutputPort6BinCode | Output Port 6 Bin Code | ✓ |
| 283 | MaterialModeChange | Material Mode Change | ✓ |
| 284 | PortStateUpdated | Port State Updated | ✓ |
| 285 | UnloaderTrayIDReadOK | Unloader Tray ID Read OK | ✓ |
| 286 | UnloaderTrayIDReadFail | Unloader Tray ID Read Fail | ✓ |
| 287 | LoaderTrayIDReadFail | Loader Tray ID Read Fail | ✓ |
| 288 | MaximumOutputPortReport | Maximum Output Port Report | ✓ |
