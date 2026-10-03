# golden TTimer 全表（工具產生，不要手改）

> 產生：`python tools/timer_census.py`（AI(W906-TIMER-TABLE) 20261001）。golden＝`HT9011UC_Code_V3.33.906.0_20260618`。
> 分類與欄位意思見工具檔頭；計畫書見 `docs/TIMER_TABLE_PLAN.md`。完整欄位（含每一個 Enabled／Interval 寫入點）在同名 `.tsv`。

**124 支 TTimer、60 張表單、本體合計 15581 行**；分類 ?＝2、A＝72、B＝19、C＝30、D＝1。

| 表單 | Timer | 類 | Enabled | Interval | 開機就建 | 本體 | 行數 | fShow 檢查 | 重入旗標 | 沒放回的 return | 移植樹 TfX::Handler（活／全） | 名字出現的檔數 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| TfMain | Timer1 | A | False | 30 | yes | `main.cpp:2696-3838` | 1143 |  | bRunTimer1 | 2763,2928 | 0／6 | 68 |
| TfMain | Timer10 | C | True | 100 | yes | `main.cpp:34156-34200` | 45 |  |  |  | 1／4 | 4 |
| TfMain | Timer2 | B | False | 1000 | yes | `main.cpp:20848-21682` | 835 | 20860 | bTimer2Run |  | 0／9 | 34 |
| TfMain | Timer3 | A | False | 1000 | yes | `main.cpp:25106-25742` | 637 |  |  |  | 0／6 | 6 |
| TfMain | Timer4 | A | False | 1000 | yes | `main.cpp:28469-28502` | 34 |  | bTimerRunning |  | 0／0 | 2 |
| TfMain | Timer5 | C | True | 1000 | yes | `main.cpp:31210-31297` | 88 |  | bTimerRunning |  | 0／0 | 0 |
| TfMain | Timer6 | A | False | 1000 | yes | `main.cpp:31299-31548` | 250 |  | bTimerRunning |  | 0／0 | 0 |
| TfMain | Timer7 | C | True | 1000 | yes | `main.cpp:31550-31638` | 89 |  | bTimerRunning |  | 0／0 | 0 |
| TfMain | Timer8 | C | True | 1000 | yes | `main.cpp:32076-32168` | 93 |  | bTimerRunning | 32098 | 0／0 | 0 |
| TfMain | Timer9 | A | False | 100 | yes | `main.cpp:32678-32691` | 14 |  |  |  | 0／0 | 0 |
| TfMain | TimerDLL | A | False | 100 | yes | `main.cpp:33273-33431` | 159 |  | bTimerRunning |  | 0／0 | 0 |
| TfMain | TimerESD | C | True | 1000 | yes | `main.cpp:30833-31095` | 263 |  | bRun |  | 0／0 | 0 |
| TfMain | TimerScanKey | A | False | 30 | yes | `main.cpp:31994-32025` | 32 |  | bRunTimer1 |  | 0／0 | 0 |
| TfMain | TimerTemperatureStorageMinute | C | True | 1000 | yes | `main.cpp:31097-31102` | 6 |  |  |  | 0／0 | 0 |
| TACTForm | ACTConnectTimer | C | True | 50 | yes | `AutoTemperature.cpp:1459-1483` | 25 |  |  |  | 0／0 | 1 |
| TACTForm | TimerACT | A | False | 50 | yes | `AutoTemperature.cpp:887-1273` | 387 |  | bTimerRun |  | 0／0 | 2 |
| TATCInterfaceForm | ATCWatchTimer | A | False | 200 | yes | `ATC/ATCInterface.cpp:412-818` | 407 |  |  |  | 1／2 | 3 |
| TATCInterfaceForm | TimerATC | A | False | 5000 | yes | `ATC/ATCInterface.cpp:1769-1849` | 81 |  | bTimerRun |  | 1／2 | 2 |
| TATCInterfaceForm | TimerChillerStop | A | False | 5000 | yes | `ATC/ATCInterface.cpp:1421-1477` | 57 |  |  |  | 1／2 | 2 |
| TATC_InterfaceForm | CommFlagTimer | C | True | 1000 | yes | `ATC/ATC_Handler_Side.cpp:2803-2845` | 43 |  | bRunTimer |  | 0／0 | 1 |
| TATC_InterfaceForm | Timer | C | True | 10 | yes | `ATC/ATC_Handler_Side.cpp:1692-1708` | 17 |  | bRunTimer |  | 0／0 | 1 |
| TCCDInterfaceForm | Timer1 | A | False | 200 | yes | `（找不到本體）` | 0 |  |  |  | 0／0 | 0 |
| TCCDInterfaceForm | Timer2 | C | True | 200 | yes | `（找不到本體）` | 0 |  |  |  | 0／0 | 0 |
| TCOM2 | TimerHPCard | A | False | 100 | yes | `rs232.cpp:4316-4670` | 355 |  |  |  | 0／0 | 1 |
| TFSECS | TimerSecsAlarm | A | False | 1000 | yes | `SECSGEM/UsecegemMainFrom.cpp:1023-1103` | 81 |  | bRun |  | 0／0 | 0 |
| TFSECS | TimerTooling | A | False | 100 | yes | `SECSGEM/UsecegemMainFrom.cpp:724-731` | 8 |  |  |  | 0／0 | 0 |
| TFTool | Timer1 | B | True | 50 | yes | `tools.cpp:43-199` | 157 | 45 |  |  | 0／0 | 68 |
| TFormBarcodeReader | TimerKeyIn | A | False | 50 | yes | `BarcodeReader.cpp:182-402` | 221 |  |  |  | 1／1 | 2 |
| TFormHS | TimerAutoBackup | A | False | 1000 | yes | `HS_Function.cpp:84-511` | 428 |  | bTimerRunning |  | 0／4 | 3 |
| TFormHS | TimerRTMMsg | A | False | 100 | yes | `HS_Function.cpp:4163-4183` | 21 |  | bTimerRunning |  | 0／0 | 2 |
| THGem | Timer1 | A | False | 300 | yes | `SECSGEM/uHGemEquipment.cpp:5176-5527` | 352 |  | bTimerRunning |  | 7／7 | 68 |
| TLoadCCD | Timer1 | C | True | 1000 | yes | `LoadCCD/LoadCCDMap.cpp:255-262` | 8 |  |  |  | 0／0 | 68 |
| TMyMessageBox | Timer1 | B | True | 10 | yes | `mymessbox.cpp:538-757` | 220 | 540 |  |  | 0／3 | 68 |
| TTrayEditForm | Timer1 | B | True | 100 | yes | `uTrayEditForm.cpp:499-520` | 22 | 502 |  |  | 0／0 | 68 |
| TZteach | AutoTimer | B | True | 30 | yes | `AutoTeach/InOutArmZteach.cpp:4552-4646` | 95 | 4555 |  |  | 0／0 | 0 |
| TdmTrayMotor | tmrTrayStepMotor | A | False | 20 | yes | `Motor/TrayStepMotor.cpp:102-107` | 6 |  |  |  | 0／0 | 0 |
| TfAGV | Timer1 | C | True | 1000 | yes | `（找不到本體）` | 0 |  |  |  | 0／0 | 0 |
| TfAGV | Timer2 | C | True | 10 | yes | `Automation/AGV.cpp:1290-1307` | 18 |  | bIn |  | 0／5 | 34 |
| TfAutoAlignment | Timer1 | C | True | 1000 | yes | `AutoAlignment/AutoAlignment.cpp:370-381` | 12 |  |  |  | 0／0 | 68 |
| TfAutoAlignment | tmrCCDConnect | A | False | 1000 | yes | `AutoAlignment/cAutoAlignment.cpp:790-805` | 16 |  | bTimerRun |  | 0／0 | 0 |
| TfAutoAlignment | tmrCCDInitial | A | False | 100 | yes | `AutoAlignment/cAutoAlignment.cpp:775-787` | 13 |  | bTimerRun |  | 0／0 | 0 |
| TfAutoAlignment | tmrProcessGetData | A | False | 1 | yes | `AutoAlignment/cAutoAlignment.cpp:807-822` | 16 |  | bTimerRun |  | 0／0 | 0 |
| TfAutomation | tmrOLP | A | False | 1 | yes | `Automation/automation.cpp:435-644` | 210 |  | bRun |  | 2／2 | 2 |
| TfBarCode | TimerBarcodeChangeFile | A | False | 1000 | yes | `BarCode/BarCode.cpp:6195-6212` | 18 |  |  |  | 0／0 | 0 |
| TfBarCode | TimerBottom8CCDInitial | A | False | 100 | yes | `BarCode/BarCode.cpp:9539-9938` | 400 |  | bTimerRun |  | 0／1 | 2 |
| TfBarCode | TimerBotton8CCDConnect | A | False | 1000 | yes | `BarCode/BarCode.cpp:9192-9537` | 346 |  | bTimerRun |  | 0／1 | 2 |
| TfBarCode | TimerCCDInitial | A | False | 100 | yes | `BarCode/BarCode.cpp:5545-5839` | 295 |  | bTimerRun |  | 0／0 | 1 |
| TfBarCode | TimerDownCCDConnect | A | False | 1000 | yes | `BarCode/BarCode.cpp:2904-3126` | 223 |  | bTimerRun |  | 0／0 | 1 |
| TfBarCode | TimerProcess2DData | A | False | 1 | yes | `BarCode/BarCode.cpp:3476-3876` | 401 |  | bTimerRun |  | 0／0 | 0 |
| TfBarCode | tmr1 | C | True | 100 | yes | `BarCode/BarCode.cpp:2474-2497` | 24 |  |  |  | 0／0 | 11 |
| TfCCLink | Timer1 | B | True | 1000 | yes | `CCLink/MyCCLinkSensor.cpp:782-1452` | 671 | 788 |  |  | 0／0 | 68 |
| TfCCLink | tmrCanBusSearch | C | True | 1000 | yes | `CCLink/MyCCLinkSensor.cpp:2351-2455` | 105 |  |  |  | 0／0 | 0 |
| TfConfiguration | Timer1 | B | True | 1000 | yes | `cConfiguration.cpp:5919-5944` | 26 | 5921 |  |  | 3／4 | 68 |
| TfContact | OTDTimer | A | False | 100 | yes | `cContact.cpp:15220-15282` | 63 |  |  |  | 0／0 | 1 |
| TfContact | TimerEP | A | False | 1000 | yes | `cContact.cpp:16886-16948` | 63 |  |  |  | 0／2 | 3 |
| TfContact | timerContact | A | False | 50 | yes | `cContact.cpp:21355-21366` | 12 |  |  |  | 1／1 | 2 |
| TfDefrostNote | Timer1 | C | True | 500 | yes | `cDefrostNote.cpp:27-46` | 20 |  |  |  | 0／0 | 68 |
| TfDynamicTemp | Timer1 | C | True | 100 | yes | `DynamicTemp.cpp:330-337` | 8 |  |  |  | 1／1 | 68 |
| TfFixAICCD | TimerDownFixAICCDConnect | A | False | 1000 | yes | `FixAICCD.cpp:375-509` | 135 |  | bTimerRun |  | 0／0 | 0 |
| TfFixAICCD | tmrLightControl | C | True | 100 | yes | `FixAICCD.cpp:1310-1324` | 15 |  |  |  | 0／0 | 0 |
| TfFixAICCD | tmrProcessFixAICCDData | A | False | 1 | yes | `FixAICCD.cpp:155-373` | 219 |  | bTimerRun |  | 0／0 | 0 |
| TfGroundMan | Timer1 | C | True | 30 | yes | `GroundMan/GroundMan.cpp:551-575` | 25 |  | bRunTimer1 |  | 1／1 | 68 |
| TfHome | Timer1 | B | True | 10 | yes | `uhome.cpp:4885-4889` | 5 | 4887 |  |  | 0／0 | 68 |
| TfLaserSensor | Timer1 | C | True | 1 | yes | `OmronLaser/LaserSensor.cpp:777-1075` | 299 |  |  |  | 1／1 | 68 |
| TfLaserSensor | TimerInArm | C | True | 1 | yes | `OmronLaser/LaserSensor.cpp:1322-1578` | 257 |  |  |  | 1／1 | 3 |
| TfLotInfo | ATCTransferFileTime | C | True | 1000 | yes | `uLotInfo.cpp:16599-16612` | 14 |  |  |  | 0／0 | 1 |
| TfLotInfo | LotKeyInTime | A | False | 50 | yes | `uLotInfo.cpp:11543-11660` | 118 |  |  |  | 1／1 | 2 |
| TfLotInfo | NetATCTime | A | False | 200 | yes | `uLotInfo.cpp:8558-9350` | 793 |  |  |  | 0／0 | 3 |
| TfLotInfo | Timer1 | A | False | 100 | yes | `uLotInfo.cpp:5255-5342` | 88 |  |  |  | 0／0 | 68 |
| TfLotInfo | Timer2 | C | True | 1000 | yes | `uLotInfo.cpp:6934-7191` | 258 |  |  |  | 1／1 | 34 |
| TfLotInfo | Timer3 | A | False | 1000 | yes | `uLotInfo.cpp:8476-8485` | 10 |  |  |  | 1／1 | 6 |
| TfLotInfo | Timer4 | C | True | 1000 | yes | `uLotInfo.cpp:16487-16493` | 7 |  |  |  | 1／1 | 2 |
| TfLotInfo | TimerERMS | A | False | 200 | yes | `uLotInfo.cpp:10035-10211` | 177 |  | bRun |  | 0／0 | 1 |
| TfLotInfo | tmrChamberBoost | A | False | 50 | yes | `uLotInfo.cpp:11664-11737` | 74 |  | bRun |  | 0／0 | 2 |
| TfLtcSensor | DellTest | A | False | 100 | yes | `LtcSensor.cpp:826-835` | 10 |  |  |  | 0／0 | 0 |
| TfLtcSensor | TimerLtcSensor | A | False | 100 | yes | `LtcSensor.cpp:130-171` | 42 |  |  |  | 0／0 | 0 |
| TfMonitor | MonitorTimer | A | False | 100 | yes | `Monitor/MonitorInterface.cpp:143-327` | 185 |  |  |  | 0／0 | 3 |
| TfMotorTest | Timer1 | B | True | 5 | yes | `uMotorTest.cpp:912-983` | 72 | 914 |  |  | 0／0 | 68 |
| TfMotorTest | Timer2 | A | False | 10 | yes | `uMotorTest.cpp:2031-2037` | 7 |  |  |  | 0／0 | 34 |
| TfNote | Timer1 | B | True | 10 | yes | `note.cpp:3143-3525` | 383 | 3145 |  |  | 0／2 | 68 |
| TfNote | TimerFTP | C | True | 100 | yes | `note.cpp:5567-5622` | 56 |  |  |  | 0／0 | 1 |
| TfNote | timerKeyence | A | False | 300 | yes | `note.cpp:6470-6528` | 59 |  | bTimerIn |  | 0／0 | 0 |
| TfNote | tmrKeyIn | B | False | 500 | yes | `note.cpp:6443-6458` | 16 | 6445 |  |  | 0／0 | 1 |
| TfOCR | Timer1 | A | False | 1 | yes | `OCR.cpp:1467-1495` | 29 |  | bEnter |  | 0／0 | 68 |
| TfOCR | TimerOcrChangeFile | A | False | 100 | yes | `OCR.cpp:1043-1061` | 19 |  |  |  | 0／0 | 0 |
| TfOCR | TimerOcrOn | A | False | 1000 | yes | `OCR.cpp:916-982` | 67 |  |  |  | 0／0 | 0 |
| TfObserver | Timer1 | B | True | 1000 | yes | `cObserver.cpp:708-753` | 46 | 710 |  |  | 1／1 | 68 |
| TfOffSet | Timer1 | A | False | 100 | yes | `cOffSet.cpp:3280-3339` | 60 |  | bRun |  | 0／1 | 68 |
| TfOffSet | TimerSetupTeach | A | False | 1000 | yes | `cOffSet.cpp:3100-3205` | 106 |  |  |  | 1／1 | 3 |
| TfOmron | Timer2 | B | False | 1000 | yes | `EJ1N/OmronEJ1N.cpp:1927-1981` | 55 | 1931 |  |  | 0／0 | 34 |
| TfPrecaution | tm_CheckEditEmpty | B | False | 300 | yes | `Precaution.cpp:45-74` | 30 | 48 |  |  | 1／1 | 2 |
| TfProductionInfo | tm_IPSCControl | A | False | 500 | yes | `ProductionInfo/ProductionInfo.cpp:3792-3910` | 119 |  | bIsRunning |  | 0／0 | 0 |
| TfProductionInfo | tm_PI | A | False | 500 | yes | `ProductionInfo/ProductionInfo.cpp:942-1019` | 78 |  | bIsRunning |  | 0／0 | 0 |
| TfRFID | Timer1 | ? | True | 1000 | no | `MR/RFID.cpp:1532-1538` | 7 |  |  |  | 0／0 | 68 |
| TfSCKART | TimerSCKARTFlow | A | False | 1000 | yes | `Automation/SCK_ART.cpp:1403-1519` | 117 |  |  |  | 0／0 | 1 |
| TfSCKART | TimerTSV | A | False | 100 | yes | `Automation/SCK_ART.cpp:4074-4101` | 28 |  | bTimerRun |  | 0／0 | 1 |
| TfServerFrm | ProcTimer | C | True | 100 | yes | `Automation/fRENESAS_ServerFrm.cpp:37-160` | 124 |  |  |  | 0／0 | 0 |
| TfServerFrm | ShowTimer | C | True | 1000 | yes | `Automation/fRENESAS_ServerFrm.cpp:177-180` | 4 |  |  |  | 0／0 | 0 |
| TfShowBinSelect | TimerAutoCleanCount | A | False | 1000 | yes | `cShowBinSelect.cpp:2168-2214` | 47 |  |  |  | 1／2 | 4 |
| TfShowBinSet | Timer1 | B | True | 100 | yes | `cShowBinSet.cpp:226-232` | 7 | 228 |  |  | 0／0 | 68 |
| TfSmartDiagnostic | SmartDiagnosticTimer | A | False | 1000 | yes | `SmartDiagnostic.cpp:729-783` | 55 |  |  |  | 1／1 | 3 |
| TfSocketCommunication | Timer1 | D | False | 1000 | yes | `（找不到本體）` | 0 |  |  |  | 0／0 | 0 |
| TfSortCT | Timer1 | A | False | 1000 | yes | `cSortCT.cpp:823-838` | 16 |  |  |  | 0／2 | 68 |
| TfSortCT | Timer2 | C | True | 5000 | yes | `cSortCT.cpp:840-855` | 16 |  |  |  | 0／0 | 34 |
| TfTeach | Timer1 | B | True | 30 | yes | `uteach.cpp:1359-1411` | 53 | 1362 | bTimerOn |  | 0／0 | 68 |
| TfTeach | tmr_PitchLoop | A | False | 15 | yes | `uteach.cpp:5929-5932` | 4 |  |  |  | 0／0 | 1 |
| TfTemp_Set | tmr_ATC_Deforst | A | False | 200 | yes | `uTemp_Set.cpp:6816-6838` | 23 |  |  |  | 1／1 | 2 |
| TfTemperFrom | Timer1 | B | True | 100 | yes | `cTemperFrom.cpp:1617-1682` | 66 | 1619 |  |  | 2／4 | 68 |
| TfTesterTCP | TimerProcessTCPData | A | False | 1 | yes | `Interface/TesterTCP.cpp:349-552` | 204 |  |  |  | 0／2 | 9 |
| TfTesterTCP | TimerTCPIPConnect | A | False | 1000 | yes | `Interface/TesterTCP.cpp:185-239` | 55 |  | bTimerRun |  | 0／2 | 5 |
| TfTowerLight | Timer1 | B | True | 10 | yes | `cTowerLight.cpp:148-153` | 6 | 150 |  |  | 1／1 | 68 |
| TfTrayMapping | TimerCCDTrayInitial | A | False | 100 | yes | `cTrayMapping.cpp:973-1094` | 122 |  | bTimerRun |  | 0／0 | 1 |
| TfTrayMapping | TimerDownCCDTrayConnect | A | False | 1000 | yes | `cTrayMapping.cpp:782-966` | 185 |  | bTimerRun |  | 0／0 | 2 |
| TfTrayMapping | TimerProcessTrayData | A | False | 1 | yes | `cTrayMapping.cpp:1264-1708` | 445 |  | bTimerRun |  | 0／0 | 1 |
| TfTrayMapping | tmrNFC | A | False | 100 | yes | `cTrayMapping.cpp:7111-7239` | 129 |  | bRunTimer |  | 0／0 | 1 |
| TfVacuumUnit | tmr1 | B | True | 1000 | yes | `VacuumUnit/VacuumUnit.cpp:255-302` | 48 | 257 |  |  | 1／1 | 11 |
| TfYieldMonitoring | Timer1 | C | True | 100 | yes | `uYieldMonitoring.cpp:3616-3626` | 11 |  |  |  | 0／0 | 68 |
| Tfiosetview | Timer1 | A | False | 50 | yes | `iosetview.cpp:93-214` | 122 |  |  |  | 0／0 | 68 |
| Tfiosetview | Timer2 | A | False | 1000 | yes | `iosetview.cpp:1275-1279` | 5 |  |  |  | 0／0 | 34 |
| TframeProdInfo | ShowTimer | ? | True | 100 | no | `Automation/ufrmProdInfo.cpp:18-29` | 12 |  |  |  | 0／0 | 0 |
| TfrmDTME08 | TimerUpdate | A | False | 100 | yes | `EJ1N/fDTME08.cpp:158-176` | 19 |  |  |  | 0／1 | 2 |
| TfrmDefrost | tmrClose | A | False | 1000 | no | `TempCtrl/TriMachineDeforst.cpp:68-97` | 30 |  |  |  | 0／0 | 0 |
| TfrmDefrost | tmrGetTemperatureStatus | A | False | 1000 | no | `TempCtrl/TriMachineDeforst.cpp:26-42` | 17 |  |  |  | 0／0 | 0 |
| TfrmDefrost | tmrMessageFlash | A | False | 1000 | no | `TempCtrl/TriMachineDeforst.cpp:99-130` | 32 |  |  |  | 0／0 | 0 |
