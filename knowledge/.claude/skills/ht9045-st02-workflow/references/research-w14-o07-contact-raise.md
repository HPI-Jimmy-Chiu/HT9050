# 調查：W14／O07——Tester 逾時 Reset 清料時抬高接觸，golden 有沒有寫 Contact.Data（St02-M 20260927，唯讀調查）

> Steven 的傾向（W14）：「testif 的變數是使用中的，testif_file 才是從檔案讀出來的；生產到一半變更變數通常沒有存檔…應該只是行為改變而不寫檔」＝只改記憶體、不寫配方檔。
> 待裁決（已交 ST01-M 統整）：「只改記憶體」要改的是 `DeviceForm_File`（檔案的記憶體副本）而不是 `DeviceForm`（見下）；D2（重複逾時不重備份）、D4（Reset 失敗時立刻還原）、JSCC `Contact.ini` 旗標的處理。
> 行號：912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`；906＝906_0625_Steven `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`；V906＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`。

## 結論

**golden 會寫配方檔**：抬高時寫 Contact.Data、還原時再寫一次。Steven 的傾向裡有個陷阱：V906 如果只改使用中的 `DeviceForm`，操作員下一次按 Start 時 `SetWorkParameter` 會把檔案映像結構蓋回去，抬高就不會發生。所以記憶體裡要改的是 **`DeviceForm_File`**（檔案內容的記憶體副本），不是寫磁碟。
另：原本以為的 `csystem.cpp:3106-3117` 是 **V906** 的行號；golden 的還原在 912 `csystem.cpp:15792-15804`、906 `:15020-15032`。

## 1. golden（912／906，內容相同只差行號；912 在 atester.cpp:3826-3835 多一段無關的 P65）

| 位置 | 912 | 906_0625_Steven |
|---|---|---|
| 抬高：`ProcessTesterTimeOut`（安全項 177） | atester.cpp:3860-3872 | atester.cpp:3830-3842 |
| 抬高：`TfLotInfo::btnManualI49Click`（項 167） | uLotInfo.cpp:16184-16196 | uLotInfo.cpp:15909-15921 |
| 還原：`DoCleanOutFinishCheck`，只在 `bResetModeAndCleanOut` 時 | csystem.cpp:15792-15804 | csystem.cpp:15020-15032 |

- **抬高**：只在 `DeviceForm.ContactMode` 是 Direct 或 Drop 時；設 `bRestModeBackupParm=true`，原值**從檔案讀**（`ReadIniData` 讀 `DataPath+GetLastOpenFN()+"\\Contact.Data"`）；然後寫檔 3 次：`[Mode] Contact=DirectContactMode`、`[Test Arm1]`／`[Test Arm2] Contact`＝原值＋`fI49_ChangeAboveSocket`；再 `fContact->ReadFile()`、`fMain->Reset(...)`。
- **還原**：把 3 個原值寫回檔案，再 `fContact->ReadFile()`。
- **`TfContact::ReadFile` 做什麼**（912 cContact.cpp:365-726；906 :363-）：只填 `DeviceForm_File`；高度夾到最多 1.0（:422-423）；模式變 Direct 時 `IndexDrop[0/1]=0`（:512-516）；自己也會寫檔：一定寫 :661（`fSocketInitialICCheckPositionOffset`），有時 :526、:569、:581；有 `CosFunction.bContactHeightSaveToContactIni`（JSCC）時高度改從 `system\Contact.ini` 讀（:386-418）⇒ 那種機台 golden 實際上只改模式、不改高度。
- **使用中的副本怎麼更新**：`DeviceForm` 只由 `DoDeviceConvert`（cUnitConvert.cpp:62-75）更新，經 `SetWorkParameter`（cinitial.cpp:13505-13509）；每次 Start（`ScanSystemSensor`，ckernel.cpp:365-371）與開頁時呼叫。⇒ 抬高在操作員**下一次 Start** 生效；還原後 `DeviceForm` 也要到再下一次 Start 才回來。
- **golden 永遠不還原的路徑**：還原需要 `bResetModeAndCleanOut=true`，只有 `TfMain::Reset` 的 `bResetModeIncludeCleanOut` 分支會設（main.cpp:7906-7929）；`Reset` 也可能提早 return（:7665-7686、:7823）。這些情況檔案已經被改寫 ⇒ Contact.Data 永遠停在抬高後的值。
- **先例**：同一段 golden 已經有「只改檔案映像結構、不存檔」的做法（`TestIF_File.bSCKART_LotDeviceCheck=false`，:3857）。

## 2. V906 現況（整條 I49 路徑都是死的）

- atester.cpp `ProcessTesterTimeOut` 從 :3957；I49 區塊 :4193-4244：O05 gate 在 :4199-4201；O06 gate 在 :4202-4207（`#else ret=0`，清料根本不會開始）；O07（抬高與寫檔）`#if 0` 在 :4221-4234；O08（`ReadFile`）gate 在 :4235-4237；`fMain->Reset`（:4239）呼叫的是 forms/fMain.cpp:403 的空殼 ⇒ `bResetModeAndCleanOut` 從來不會被設。
- csystem.cpp:3106-3118：還原區塊是活的但走不到；`GetLastOpenFN`、`DataPath`、`WriteIniData` 在檔內是什麼都不做的替身（:2578-2584）；`ReadFile` 也是替身（:2762）。
- 變數在 cmydef.cpp:5278-5281；`btnManualI49Click` 沒移植（forms/fLotInfo.h:1046 有記）；I49 設定在 cConfiguration.cpp:3524-3525（`fI49_ChangeAboveSocket` 0-50）。

## 3. 建議的「只改記憶體」做法（偏離 D-W14）

**抬高（取代 O07、O08）**
```
if(!bRestModeBackupParm){                        // D2：不重複備份（golden 會疊加抬高）
  bRestModeBackupParm=true; sRestModeRecipe=GetLastOpenFN();          // 新變數
  iRestModeBackContactMode=DeviceForm_File.ContactMode;
  dRestModeBackContactHeigh1/2=DeviceForm_File.IndexContact[0/1];
  dRestModeBackDrop1/2=DeviceForm_File.IndexDrop[0/1]; }             // 新：golden 的 ReadFile 會把 Drop 歸零
DeviceForm_File.ContactMode=DirectContactMode;
DeviceForm_File.IndexDrop[0]=DeviceForm_File.IndexDrop[1]=0;          // ＝cContact.cpp:512-516
if(!CosFunction.bContactHeightSaveToContactIni)                       // ＝golden 的實際結果
  DeviceForm_File.IndexContact[i]=CheckRange(backup_i+IniConfig.fI49_ChangeAboveSocket,1.0,fIndexDownPos);
RecordProcess("I49 raise, memory only");
```
不呼叫 `WriteIniData`、不呼叫 `ReadFile`；不直接動 `DeviceForm`——下一次 Start 的 `SetWorkParameter` 會把抬高值複製進 `DeviceForm` 與 `Prod.TestZ*_Test`，跟 golden 一樣。

**還原（csystem.cpp:3106-3118）**：`GetLastOpenFN()==sRestModeRecipe` 才把備份放回 `DeviceForm_File`，否則只清旗標；拿掉 `WriteIniData` 與 `ReadFile` 替身，**同一顆 commit** 刪掉 :2578-2584 的替身。
**中止／當機**：磁碟上沒變，重開或當機會從檔案讀回原值；任何一次 `ReadFile` 都會取消抬高。選配 D4（給 Steven）：`Reset` 回 false 或 `bResetModeAndCleanOut` 沒被設時立刻還原（golden 這些情況會一直抬著）。
**走得到嗎**：O06（安全檢查）、O05、`TfMain::Reset` 翻好之前整條還是死的 ⇒ ctest 直接呼叫兩個 helper（例 `W906_I49_Raise()`／`W906_I49_Restore()`）。

**ctest**：`W906_INIDATA_ROOT` 指 %TEMP%，放 `<recipe>\Contact.Data`（Mode=Drop、Arm1/Arm2＝-134.00/-134.28、Drop=2.0），I49＝2.0。抬高後 `ContactMode==Direct`、高度 -132.00/-132.28、Drop=0；`DoDeviceConvert` 後 `DeviceForm.IndexContact[0]==-13200`。還原後全部回原值、旗標清掉。檔案檢查：雜湊、大小、修改時間不變，資料夾清單不變。邊界：夾值（-0.5 → 1.0）、第二次抬高不重備份、不同配方名不還原、JSCC 旗標時只改模式。

## 4. 風險

- **抬高值會不會被存到磁碟？沒找到這種路徑**：Contact 頁開頁前重讀檔案（golden `FormShow` cContact.cpp:1174-1175；V906 網頁 `FormShowAndSnap` → `DF_ReadFile`，FileRW/DeviceForm_File.gen.inc:896）；它的存檔（:14258、gen.inc:2972-3025）寫的是頁面欄位值不是結構；`SaveAllFile`（912 csystem.cpp:23542）與 `TfBuilder::bSaveAllFillOrFile`（cBuilder.cpp:509）沒人呼叫；SECS 配方下載（SECSGEM/uHGemHT9045.cpp:1493-1647）寫的是收到的資料；`BackupSetupFile` 只複製檔案（golden 會備份到抬高後的檔）。
- **遠端設定可能讓記憶體與檔案不同步**：V906 Automation/auto9045.cpp:1066-1074、:1165-1200，以及 HTSET 350（golden Command.cpp:13419-13427；V906 :17146-17156，gate 中）會把 `DeviceForm_File` 寫進 Contact.Data。它們先指定遠端值，所以不會存到抬高值；但之後的還原會把舊值放回記憶體，而檔案保留遠端值，直到下一次 `ReadFile`。golden 在磁碟上也有同樣問題。
- **抬高可能提早被取消（安全方向，但清料會用正常高度）**：任何 `ReadFile` 都會：Contact 頁開／關（cContact.cpp:1174、:1848）、Setup 更新／存檔（cSetUp.cpp:3642、:4148、:4154）、main.cpp:28496、載入配方（main.cpp:9327、:9339）。操作員看到的不同：golden 的 Contact 頁會顯示抬高後的值，這裡顯示檔案值。
- **中途換配方**：golden 會把舊配方的值寫進新配方；配方名檢查擋掉這個。
- **還原前第二次逾時**：golden 會疊加抬高；D2 擋掉。
- **JSCC `Contact.ini` 旗標**：golden 這時只改模式；建議照做，是不是 golden bug 由 Steven 決定。
- **替身拆除順序**：在記憶體版還原落地前先拆 csystem 的替身，會讓真的寫檔回來，或（一點一點拆）寫到磁碟根目錄的 `"\\Contact.Data"`。
- 不做 `ReadFile` 本身的幾次寫檔（:526、:569、:581、:661）是無害偏離；`LastSet.bBreakSCKART=true` 照 golden 仍會存。

**動程式前**：要 Steven 確認「只改記憶體」＝改 `DeviceForm_File`（不是 `DeviceForm`），以及 D2／D4 與 JSCC 的處理。
