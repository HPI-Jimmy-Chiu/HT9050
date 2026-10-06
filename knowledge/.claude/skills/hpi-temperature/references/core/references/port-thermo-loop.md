> 保存來源：`.claude/skills/ht9045-temperature/references/port-thermo-loop.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 移植樹的溫度程式逐檔（bthermo／uHeaterThread／cTemperFrom／EJ1N）

> P＝移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`，G＝golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`。
> 行號以 HEAD `c13d34b4`（`v906/steven-cbridge-review6`，20261001）為準。四份都照 golden **V906**（906_0625_Steven，那棵樹不在硬碟上）翻，不是 V912。
> 來源：20261001 唯讀查證（ST01-E2 派的工程師），抽查過 HeaterSimTick 與 HeaterInsOpt 兩條。

## 1. P\bthermo.cpp（5443 行）＋ bthermo.h；golden G\bthermo.cpp（4998 行）

- **狀態**：忠實翻譯，20 個 `#if 0`（清單在檔頭 `:55-103`）；標頭 15 個宣告，沒擋。
- **函式**：
  - 補償換算：`ConvertTempOffset :282`、`GetFactSetTemp :445`、`ConvertGetTempOffset :559`、`GetConvertTemp :836`；只有 5／3／2 點（`iTempMode` 8／4／2），沒有 V912 的 6 點。
  - `bGetHeaterUsed :944`；Modbus ASCII 小工具 `:1111-1192`；讀值平滑緩衝 `fBuffer[71][30]`＋`ReadAverageBuffer`（`:1194-1259`）。
  - `DoThermo :1262`、`DoThermoReal :1336`、`DoATC60Temperature :3688`、`DoSetSVOfOmronEJ1N :3848`、`DOUN150ReadTemp :4532`、
    `bGet16HeaterUsedTo4Heater :4795`、`DoSetSVOfDTME08 :5302`、`CheckLBTemp :5405`。`DoTemptureControl :4357` 註解掉（golden 也是）。
- **狀態機 `iThermoTask`**：
  - case 1（初始化）→ case 100（算設定值；變了就寫，沒變就讀）。
  - 依廠牌收回覆：TC401 200、KT4H 250、E5DC 255、DTK4848 2500；ATC 與 TriTemp 走 260／270。
  - 230 重開 COM 埠；300 換下一個通道（`Addr++`）。
  - 500～530 寫 KT4H 警報 1 的值與型別（500 落到 510、520 落到 530）。
  - 一拍只處理一個通道；`Com2Delay` 0.5 s（`:2764`），`MAX_RETRY` 2；通訊失敗時 `UN150CommError[]=true`、`UN150Read[]=999`（例 `:2828-2830`）。
- **廠牌判斷**：只看整台的 `TC401HeaterControl`（`:1264`、`:2663-2790`、`:3503`）。TC401／KT4H／E5DC 的送出擋住（G21～G23）；
  DTK4848 沒擋但 port 指標是 null（`cpublic.cpp:157`）；所有 `COM2->Comm2` 擋住（G15～G18）；EJ1N 設定值寫入擋住（G26b，`:3972-4353`）、DTME08 也擋（G29，`:5396-5399`）。
- **用到的資料**：`UN150Read`／`UN150ReadReal`／`UN150CommError`／`bUT150Install`／`iTempCode[]`；`Temperature` 結構（從 recipe 的 `Temperature.Data` 讀；
  `fTempOffSet[19][71]` 的列常數在 `forms\fTemp_Set.h:402-417`）。
- **呼叫端**：只有 `HeaterThreadProcess`（`uHeaterThread.cpp:392`），而那條執行緒從不啟動。其他：`ClearAllHotBuffer`（`MainTempMode.cpp:237`、`WebRecipeChange.cpp`）；
  `bGet16HeaterUsedTo4Heater`（GPIB，`Command.cpp:5537-6288`）；`iThermoTask=1` 在 `FileRW\Temperature.cpp:103` 被擋。
- **擋的理由過時**：G12／G13（`:248-249`）說兩個符號沒定義，現在都有（`DoCloseHeadterDelay` `csystem.cpp:19272`、`bHeaterDoorIsOpen` `csystem.cpp:14237`）；
  G29 說 fDTME08 沒移植，現在有 `forms\fDTME08.cpp`。

## 2. P\uHeaterThread.cpp（2082 行）＋ .h；golden G\uHeaterThread.cpp（1681 行）

- **狀態**：忠實翻譯。`THeaterThread` 是獨立類別，`Resume()` 什麼都不做（`.h:59-72`）；迴圈每 20 ms（`:403-416`）。
- **還擋著的呼叫**：`CheckATC6System`、`HeaterDoorIsOpen`、`DoHeaterOn`（`:383`、`:393`、`:397`）、`DoSwCoolingFan`（`:540`）。
  理由過時：這四個現在都在 `csystem.cpp`（`:19211`、`:14238`、`:19273`、`:24787`）。
- **`CheckHeaterOK :478`**：非常溫 recipe 要 `fHeaterOK` 與 `fHeaterStableOK` 都是 true 才回 true；InArm／OutArm 會檢查它。
- **`CheckHeater :503`**：
  - 模擬版提早回傳 `fMain->chkHeaterOk` 的值（`:505-511`；那個勾選框預設勾，`forms\fMain.cpp:63`）。
  - 出貨版：看即時 CCD 感測器、逐通道判斷（Hot／AmbientHot／Ambient）、穩定時間邏輯（`:2032-2074`）。
  - 告警：WAR15181（`:562`）；WAR15110（只記資料庫，`:573`）；讀值 999 連續 40 次 → WAR15<i+100>（`:613`）；
    超溫／低溫 WAR15xx（`:1237-1309`、`:1522-1702`、`:2009-2014`，其中三處少了 `+100`，golden 原樣）；WAR15182（`:2020`）；MES2130／MES2131（`:617`、`:1447`）。
  - 切換 `SwHeaterRelay`、`SwHeatGun`、`SwATCHeatGun`、`SwShuttleCooling`、`SwDutHeaterCoolFan`。
- **接線**：沒有任何程式建立或啟動這條執行緒。
  - 20260929 起 `W906_HeaterSimTick`（`HeaterSimTick.cpp:42`，本體在 `#ifdef SOFT_SIMULTE`）在每個 wb_serve PumpTick、MainProc 之前（`WebBridgeTags.cpp:632`）
    跑 `CheckATC6System`、`HeaterDoorIsOpen`、`CheckHeater`、`DoHeaterOn`——**不跑 `DoThermo`**。RULINGS_20260929 第 10 條。
  - 出貨版那個函式是空的 ⇒ **`fHeaterOK` 沒有任何地方會設**。
  - `EndHeaterThread`（`FileRW\MainClose.cpp:984`）什麼都不做。
- **缺的 V912 改動**：`ATC_TYPE_36` 與 `IS_ATC33()`（G `:326`、`:1507`）。

## 3. P\cTemperFrom.cpp（1433 行）＋ forms\fTemperFrom.h；golden G\cTemperFrom.cpp（2030 行）

- **已翻（一部分，「Wave A」）**：建構子 `:139`、`ShowThermo :234`、Yield 顯示 `:1162-1276`、`ShowHotName :1332`、`TempRunShowAlarmHigh／Low :1383／:1398`。
- **沒翻**（`fTemperFrom.h:72-91`）：`Timer1Timer`（golden `:1627`；100 ms，驅動 SwCCDCooling 與 RecordTemp）、`FormShow`、Panel71～73 的滑鼠事件、`Check_Tri_Temp_All_Temperature`。
- **GATE T1**（`:977-1006`）：擋掉 WAR15<Addr>（太冷）與 WAR15<Addr+100>（太熱）告警。
- **`ShowThermo` 寫什麼**：`iTempOverShowAlarmT[]`（`:796-833`）、`asGPIBTempShow[]`（GPIB 溫度回覆讀它，`Command.cpp:385` 起）、`bCCDOverTemp`。
- **接線**：沒有 `fTemperFrom` 實例（`fTemperFrom.h:375`），`ShowThermo` 只有測試呼叫。被擋的使用端：`cSetUp.cpp:1115`／`:1221`、`uYieldMonitoring.cpp:3130`、
  `csystem.cpp:15889`（G02，熱轉常溫的冷卻風扇）、`PowerSavingMode.cpp:848`。
- **缺的 V912 改動**：`IS_ATC33`、`bATC32UseTJMode`（G `:1275-1288`）。

## 4. P\EJ1N\

- **已翻**：
  - `TextProcess`：17 個函式翻了 16 個（`HexStrToInt` 擋），在 `ht9045_globals`，bthermo 少了它連不起來。
  - `MyOmronPanel`（畫圖部分擋）、`uModbusCommand`、`uSocketServerClient`。
  - `uDTME08Control`：Modbus/TCP，4 站 × 8 通道；站碼＝站 × 0x1000、slave ID 1。暫存器：SV 0x000、AT 0x250、PV 0x268、狀態 0x288、感測器型別 0x028、週期 0x0F8、輸出 2 0x0D0。
    設定檔：移植樹 GATE (1) 讀 `D:\HT9045\system\DTME08_Control.ini [SocketSetting]`（`uDTME08Control.h:135-141`），golden 讀 exe 資料夾（G `EJ1N\uDTME08Control.h:54`）；⛔ 20261001：`system\` 底下沒有這個檔（Steven01 只有 `D:\HT9045\EXE\DTME08_Control.ini`），移植樹會用預設 `127.0.0.1:59999`。DTME08＝台達 DTM（手冊對照：`references\controllers\delta-dtm.md`）。`ReceiveInitial` 是回 false 的空殼（golden 也標未完成）。移植樹檔頭說 DTME08 是「Panasonic」、golden `EJ1N\fDTME08.cpp:163` 註解說「Omron」，兩個都錯。
- **沒翻**：`OmronEJ1N.cpp`（`TfOmron`）、`OmronThermo.cpp`（手冊對照：`references\controllers\omron-ej1n.md`）。golden 的 EJ1N 走 `COM_PORT_OMRON`，38400、7 bit、偶同位、2 stop；CompoWay/F（STX…ETX＋BCC）；
  單元位址 `%02X`＝Addr+1；8 個單元 × 4 通道；由 `TOmronProcessThread` → `Timer1Timer` 輪詢（G `EJ1N\OmronEJ1N.cpp:45-55`、`:333-338`、`:376`）。
- `P\forms\fDTME08.cpp` 只是外殼：55 個成員只有 22 個是活的、建構子是空的、`myPalGroup` 沒人寫。
- **接線**：執行時沒有人建立 `uDTME08Control` 或 `fOmron`；`fOmron` 的呼叫擋住（`csystem.cpp:10697`）。

## 5. 網頁看得到什麼

- 主畫面：`temp.sv`、`temp.soak`、`temp.mode` 是活的（`WebBridgeTags.cpp:1010-1012`，`ht9045_wire_main.js:33-41` 顯示）。
- 一律 null：`temp.pv` 與 `zone.hotplate`／`shuttle`／`index`／`heatgun.*`（`kUnloadedTags`，`WebBridgeTags.cpp:221-231`）。
- 沒人用：`JsonBridge\StageThermo.cpp:171` 產生 213 個 `temp.zone.<ch>.pv／comm／inst`，`tools\wb_serve.cpp:2888` 每拍會發（⛔ 20261002 更正：原寫「只有測試呼叫」），但 live＝false 一律 null，沒有頁面讀。各機型畫面、格子名稱與實體位址：references/main-screen-display.md。
- `D:\HT9045\web\page\Status.TemperFrom.html`：11 個溫度格都是「---」（沒有來源），只有 OCR 是活的；`Main.HeaterView.html` 是靜態。
- `HW.OmronEJ1N.html`（route "omron"，`WebPageTable.cpp:83`）：表單的靜態複製，0 個鍵接線；frmDTME08 沒有網頁（`WebPageTable.cpp:153`）。

<!-- preserved-content:end -->
