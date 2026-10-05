# 各種軸卡／IO 卡的開卡流程與錯誤確認對照（HT9045／HT9050）

> **給誰看**：查馬達或 IO 問題的人（Steven、St01／St02、Jimmy、EastSun）與 Claude session。Steven 20261005 23:2x 的要求：「底層是 HTMotor，再衍生不同的軸卡；IO 卡也類似」「馬達跟 IO 的問題，先確認用的是哪一種技術」「MotionNet 也有自己的開卡流程」「把各種卡的開卡與錯誤確認整理成一份 skill/reference 參照」。
> **這份是總覽**：只做「用哪種技術 → 怎麼開卡 → 怎麼知道壞了 → 怎麼恢復 → 細節去哪一份看」的對照。API 細節不重寫，每節最後都附連結。
> **基準**：
> - golden＝`D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618`（BCB6，Big5）。
> - 移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`，GitLab `origin/main` `db3a636c`（2026-10-05 23:15）。
> - 機台表＝`D:\HT9045\machines\HT9050\snapshot\machine_params\D_HT9045_system\`（HT9050 快照，main 上）與 `D:\HT9045\system\`（HT9045 實驗機現用檔）。
> **寫法**（Steven 規則）：流程寫函式名、Task、case 名稱，不寫程式行號。「檔名 函式名」就是出處。
> **相對連結的基準**：本檔建議放在 `D:\HT9045\.claude\skills\ht9045-motor-control\references\card-init-and-health.md`，連結都以這個位置計算。

---

## 0. 架構先講一次

- **馬達**：全域 `MOT[]`（`TTrayMotor` 繼承 `TMyMotor`）各自持有一個 `HTMotor* Motor`。`cinitial.cpp InitialMotorParameter` 依表格建立實際的卡類別（`TMyEtherCatMotor`、`TMyMN200Motor`、`TMySMCMotor`、`TMySYNTEKMotor`、`TMyGALILMotor`），接著對每個啟用的軸呼叫 `Motor->InitMotor()`。類別樹與成員見 [motor-classes.md](motor-classes.md)。
- **IO**：`Sen[]`、`SW[]`、`Cylinder[]` 與吸嘴物件每一點都有自己的 `ISABase`：
  - 0（MotionNet）、3（PCI1203）、4（安全 PLC）走 `MyLaneIO`（`TLaneIO`）。
  - 1（ISA）、2（PCI-1735U）走 `myio.cpp` 的 port I/O。
  - `TLaneIO` 裡面再分一次：`ISABase==ePCI1203` 呼叫 `Acm_Daq*Ex`；`IO_CARD_TYPE` 是 1／2 呼叫泓格 `mn_*`；其他呼叫先達 `_mnet_*`。見 [io-classes.md](../../ht9045-io-control/references/io-classes.md)。
- **開卡沒有統一的入口**，每種技術在不同地方開（§2）。所以「機台不動」時，第一件事是確認那一軸、那一點用的是哪一種技術（§1）。

---

## 1. 怎麼判斷一個軸／一個 IO 點用哪一種技術

### 1.1 先看 `D:\HT9045\system\Gerneral.ini` 的卡別鍵

| 鍵（段落） | 值的意思 | 影響什麼 |
|---|---|---|
| `[System] IO_CARD_TYPE` | 0＝先達 PCI-L112／L122（`MotionnetIO_L112`）；1＝泓格 PISO-MN200（`MotionnetIO_MN200`）；2＝MN200 新 IO 排列（`NewIO_MN200`）；3＝`PCI_P64C64`（HT7080B）；**4＝`PCI1203_IO`（只有移植樹有，golden 沒有這個值）** | 讀哪一種馬達表（§1.2）、MotionNet 開哪一家的卡、IO 走哪一個 API。移植樹只有 2／3／4 會讀 IO_Table 並綁定（[table-loading-port.md](../../ht9045-io-control/references/table-loading-port.md)） |
| `[System] MOTION_CARD_TYPE` | 0＝`MotionCard_SYN`（先達）；1＝`MotionCard_Contec`（SMC） | 0 會讓 `OpenPCI132Card` 跑先達開卡、`TfMain::Timer1Timer` 跑 `CheckPCI_L112State`。**HT9050 快照是 0**（陷阱見 [ht9050-1203-runtime-traps.md](ht9050-1203-runtime-traps.md) §1、§5） |
| `[System] INDEX_MOTION_CARD` | 0＝Index 四軸（MTestY1／Z1／Z2／Y2）用 Galil | 0 時這四軸一律建成 `TMyGALILMotor`，**不看表的 CardModel**；`TfMain::FormShow` 最後呼叫 `Open_GaliCard` |
| `[System] MOTIONNET_SPEED` | MotionNet 通訊速度 | MN200 開卡會用 4 種速度掃一遍，模組的速度跟這個值不一致就報 WAR1696 |
| `[System] TTL_CARD_TYPE` | >0＝TTL 介面板走 RS-232 | `myio.cpp` 的 port I/O 全部跳過 |
| `[System] SHUTTLE_SENSOR_TYPE`、`[System] VacuUnitType` | 6／7＝EtherCAT NU-EC1 感測器；VacuUnitType 1＝EtherCAT 真空模組 | golden 的 `INSTALL_ETHETCAT()` 只看這兩個，決定要不要開 PCIE-1203（§4.1） |
| `[System] SafePlcIO` | 1＝安全 PLC（Modbus TCP 172.16.8.120:502） | `InitHontechHardware` 最後 `InitPLCIO` |
| `[TrayY] LoaderUnload_StepMotor`、`[TrayY] COM PORT`、`[System] ControlPanelMode`、`[Vibration] VibrationCommunication` | Tray 步進馬達／控制面板／震動馬達共用的 RS-232 | `InitialHandler` 最後 `dmTrayMotor->RS232Init` |

鍵的讀取位置：golden `database.cpp`（`HSys` 讀 General ini 的那一段）、畫面 `HandlerSys.cpp`（`rgMotionCard`／`rgIOCard`／`rgTTLCard`）。全表見 `ht9045-general-ini` skill 的 `references/database-mapping.md`。

### 1.2 馬達：讀哪一份表、哪一欄決定類別（golden `cinitial.cpp InitialMotorParameter`）

依序判斷：
1. `IO_CARD_TYPE` 是 2 或 3（移植樹再加 4）→ 讀 `D:\HT9045\system\Mot_Table.csv`（`HSys.LoadMotData`），列名 `M%02d`，**`CardModel` 欄決定類別**。
2. 否則 `MOTION_CARD_TYPE==0` → 讀 `system\motor.db`，全部建成 `TMySYNTEKMotor`（位址＝Lane×1000＋IP×10＋Port）。
3. 否則 → 讀 `system\motor_SMC.db`（有 XY 變距時讀 `motor_SMC_XYPitch.db`），由 DB 的 `CardModel` 欄選 SMC／MN200（MC88X1 那一支已註解掉）。
4. 不論哪一條：`INDEX_MOTION_CARD==0` 時 MTestY1／Z1／Z2／Y2 改建 `TMyGALILMotor`（Galil 軸 0～3）。

Mot_Table.csv 的 `CardModel` → 類別與位址欄的意思：

| CardModel | 建立的類別 | 位址怎麼算 | BoardID／Port／IP 的意思 |
|---|---|---|---|
| `PCI1203` | `TMyEtherCatMotor` | BoardID×100＋Port | BoardID＝EtherCAT 環上的站號，Port＝站內軸號（雙軸驅動器的 A／B 軸＝0／1）。開軸用 `Acm_AxOpenbyID(dev, BoardID, Port)` |
| `MN200` | `TMyMN200Motor` | BoardID×100＋Port | BoardID＝MotionNet line，Port＝裝置號。表上的 Acc／Dec 單位是秒（大於 1 會除以 100） |
| `SYNTEK` | `TMySYNTEKMotor` | BoardID×1000＋IP×100＋Port | BoardID＝ring，IP＝slave，Port＝軸。⚠ 解碼與 motor.db 那條路不一致，見 §7.3 |
| `MC88X1` | **沒有建立物件**（golden 那一行已註解） | — | 見 §4.8 |
| `SMC` 或其他任何字 | `TMySMCMotor`（預設那一支） | BoardID×10＋Port | BoardID＝SMC 卡號（裝置名 SMC0xx），Port＝軸（類別內＋1） |
| 表上沒有那一列 | golden：`TMySMCMotor(-1)`，Enable=false | — | 指標不是 NULL，呼叫會打到不存在的 SMC（移植樹行為見 [table-loading-port.md](../../ht9045-io-control/references/table-loading-port.md)） |

### 1.3 IO：`IO_Table.csv` 的 `ISABase` 欄（golden `MachineType.h enum eIOType`）

| ISABase | 意思 | 走哪一條 | Lane／IP／Port／Bit 的意思 |
|---|---|---|---|
| 0 或空白 | `eMotionNet` | `TLaneIO`；廠牌看 `IO_CARD_TYPE`（1／2＝MN200 `mn_*`，其他＝先達 `_mnet_*`） | Lane＝ring／line，IP＝slave（0～63），Port＝0～3，Bit＝0～7 |
| 1 | `eISABase` | `myio.cpp IOInputBit(port,bit)` | Port 是 16 進位的 I/O 位址，Lane／IP 不用 |
| 2 | `ePCI1735U` | 同上 | 同上 |
| 3 | `ePCI1203` | `TLaneIO` → `Acm_DaqDoSetBitEx`／`Acm_DaqDiGetBitEx(dev, Ring, IP, Port)` | Lane＝環（HT9050 的 IO 在 ring 1），IP＝站號，**Port＝站內通道＝byte×8＋bit**（例 SnMotorPower Port 30＝3×8＋6，Bit 欄 6）。golden 呼叫時不帶 Bit 欄 |
| 4 | `ePLCbase` | 安全 PLC 的輸入快取 `bPLCInData` | Port 是 16 進位暫存器（例 0x405），Bit 0～7。⚠ 這一列的 Enable 與 InType 會被程式強制改成 1（[ht9045-io-control SKILL.md](../../ht9045-io-control/SKILL.md)「IO 表 Enable 技巧」） |

### 1.4 每種技術一列範例

| 技術 | 範例列 | 出處 |
|---|---|---|
| PCIE-1203 馬達 | `M00,MInArmX,-999999,999999,0,0,0,0,1,0,200000,50000,1000,10000,100,90,1,1,100,1,0,10000,PCI1203,100000,100000,2,,0,0`（BoardID 0、Port 0＝站 0 的 A 軸） | HT9050 快照 `Mot_Table.csv` |
| MN200 馬達 | `M02,MInArmPitch,2,18,,0,2.5,0,50,1,100,15000,100,50,-999999,999999,1,0,1,0,0,10000,MN200,0.05,0.05,0,,0,1`（line 2、裝置 18） | `D:\HT9045\system\Mot_Table.csv`（HT9045 實驗機；欄位順序跟 HT9050 的表不同，以表頭為準） |
| SMC 馬達 | `M00,MInArmX,0,0,,1,0.2363,...,SMC,90,90,2,,1,1`（卡 0、軸 0） | 同上 |
| Galil（Index） | `M14,MTestZ1,...,14,0,14,...,PCI1203,...`：表寫 PCI1203，但 `INDEX_MOTION_CARD=0`，golden 仍建 `TMyGALILMotor`；移植樹在 HT9050 上把 Galil 命令轉送 1203 | HT9050 快照 `Mot_Table.csv`、`Gerneral.ini` |
| SYN-TEK 馬達 | repo 裡沒有 SYNTEK 列的範例（舊機台走 `motor.db`） | — |
| MC88X1 | 沒有範例（golden 906 沒編進來） | — |
| PCIE-1203 IO | `Sensor,SnMotorPower,1,1,2,30,6,1,3,1`（ring 1、站 2、通道 30） | HT9050 快照 `IO_Table.csv` |
| MotionNet IO | `Sensor,SnMotorPower,0,2,1,0,1,1,0,1`（ring 0、IP 1、Port 0、Bit 1） | `D:\HT9045\docs\manual\IO_Table.csv` |
| 安全 PLC IO | `Sensor,SnAllEMG,,,,0x405,5,1,4,1` | [ht9045-io-control SKILL.md](../../ht9045-io-control/SKILL.md) |

⚠ `D:\HT9045\system\` 那台現用的 `Gerneral.ini` 是 `IO_CARD_TYPE=1`、`MOTION_CARD_TYPE=1`，照 §1.2，golden 在那台實際讀的是 `motor_SMC.db`，不是 `Mot_Table.csv`。上表的 MN200／SMC 兩列取自同一台的 CSV 版本（HAL 文件拿它當 HT9050 的範本），只用來示範欄位。
HT9050 快照的實際組成：Mot_Table 有 19 列 PCI1203 是 Enable 1，MN200 16 列與 SMC 13 列全是 Enable 0；IO_Table 有 295 列 ISABase 3 是 Enable 1，ISABase 0 的 699 列全是 Enable 0。`Gerneral.ini`：`IO_CARD_TYPE=4`、`MOTION_CARD_TYPE=0`、`INDEX_MOTION_CARD=0`、`VacuUnitType=1`、`SHUTTLE_SENSOR_TYPE=0`、`SafePlcIO=0`、`LoaderUnload_StepMotor=0`。

---

## 2. 開機時誰先開、誰後開

### 2.1 golden 906（`main.cpp TfMain::FormShow`）

1. GPIB 型號讀不到（`bHandlerModel==false`）→ 跳框後結束程式，任何卡都不開。
2. `InitialGaliDelayCount`，接著 `Open_ADAM_6024`（EP 類比模組，見 `ht9045-adam6024` skill）。
3. `cinitial.cpp InitialHandler`：
   - `InitialClass` → `InitNTPort`（ISA／1735U 的 port driver）→ `COM2->RS232Init`（Index 扭力等序列埠）。
   - `InitHontechHardware`，依序：
     - `InitSucker` → `InitialSwitch` → `IsSuckerHasIC_NewIO_MN200` → `InitialSensor`：讀 IO 表、綁定物件，這時還沒開卡。
     - **`OpenPCI132Card(true)`**（`Motor/myMN200motor.cpp`）：
       - MN200 開卡（`IO_CARD_TYPE` 1／2）；
       - 先達開卡（`IO_CARD_TYPE==0 || MOTION_CARD_TYPE==0`）；
       - 最後在 `INSTALL_ETHETCAT()` 成立時開 PCIE-1203（`OpenEtherCatMastCard`）。
     - **`InitialMotorParameter`**：建各軸類別，SMC 與先達 slave 在類別建構子裡開卡；每個啟用軸跑 `InitMotor`。
     - `InitCylinder`。
     - `SafePlcIO` 是 1 時 `InitPLCIO("172.16.8.120",502)`。
   - 後段：`LoadMachineRecord`、`SetWorkParameter`、`SetMotorSpeed`。
   - 有步進馬達／控制面板／震動馬達時 **`dmTrayMotor->RS232Init`**。
4. `DoReadLastData`、`ReadLastSetIni` 等讀檔。
5. **`SW[SwServerON].On()` → `SystemInitialOK=true` → `INDEX_MOTION_CARD==0` 時 `Open_GaliCard()`**。Galil 是最後才開的，在其他卡全部開完之後。
6. 之後由 `TfMain::Timer1Timer` 每一拍做卡片健康檢查（§3 第 5 欄）。馬達電源由 `csystem.cpp CheckMotorPowerShutDown` 送：`DoMotorPowerOn`、`SwServerON`，延遲 `SERVER_MOTOR_POWER_ON_DELAY` 後由 `CountMotorPowerDelay` 放開煞車。

### 2.2 移植樹（`tools\wb_serve.cpp` 的 `main`）

1. `LoadMachineConfig` → 讀 `IO_CARD_TYPE`；是 2／3／4 才呼叫 `LoadIoData`。
2. `InitialHandler`（照 golden）：
   - `InitNTPort` 與 `dmTrayMotor->RS232Init` 被閘住（N1-G2b、N1-G2g）。
   - `InitHontechHardware` 第一件事是 `MyLaneIO.SelectVendorBackends()`：把 IO 後端從模擬換成 `TPci1203Backend`／`TMN200Backend`／`TMnetLegacyBackend`。
   - `OpenPCI132Card` 照 golden 跑，但 MotionNet 與 SMC 的廠商函式是離線樁（`Motor\vendor_offline_motionnet.cpp`、`vendor_offline_smc.cpp`，一律回「沒有卡」）。
   - `InitialMotorParameter` 跑到 1203 軸的 `InitMotor` 時，路由還沒裝好 ⇒ `Open_Axis` 直接返回、`InitMotor` 回 false，**沒有任何訊息**。
3. `W906_BootSummary` 印一行 `[BOOT]`（機種、卡別、兩張表讀了幾列、綁了幾點、1203 軸數）。接著 `TableAudit` 驗表（E-043）。
4. `Pci1203ControlEnable`：建命令面。MachineType.h 定義了 `WB_PUMP_1203_CONTROL_LIVE` 才真的送到卡，沒定義就是 DRY RUN。接著 `W906_InstallPci1203IoRoute`、`W906_InstallPci1203MotorRoute`。
5. 橋接起來後 **`Pci1203MonitorEnable`**（`EtherCAT\Pci1203Monitor.cpp TPci1203Monitor::Open`）真的開 PCIE-1203。開失敗不是致命錯誤：主控台印 `card NOT opened: <原因>`，原因放進 `pci1203.*` tag。
6. 進 tick 迴圈前 `W906_InstallPci1203GaliRoute`（Index Z 的 Galil 命令轉 1203）。每一拍依序：
   - `Pci1203AxisIniTick`：把 `D:\HT9045\config\Pci1203Axis.ini` 的卡設定寫回去；
   - `Pci1203Monitor()->Poll()`；
   - `W906_Pci1203LinkWatchTick`、`W906_Pci1203ModuleCheckTick`；
   - `W906_BrakeAxisTick` → **`W906_BootInitMotorTick`**：馬達電源 ON 滿 1 秒後，對每個 1203 軸各跑一次 golden InitMotor。
7. golden 的 `SW[SwServerON].On()` 與 `Open_GaliCard()` **沒有翻**（`FileRW\MainBoot.cpp` 的說明）。馬達電源改由移植樹 `DoSystem` 的自動上電送出。

---

## 3. 對照總表（一種技術一列）

| 技術 | 類別／API | 開卡入口與開機呼叫點（golden） | 開卡失敗報什麼 | 執行期健康檢查 | 移植樹現況 | 詳讀 |
|---|---|---|---|---|---|---|
| **PCIE-1203 EtherCAT 馬達** | `TMyEtherCatMotor`／Advantech Common Motion（`Acm_*`） | 卡：`OpenPCI132Card` 尾段的 `OpenEtherCatMastCard`，只在 `INSTALL_ETHETCAT()` 時跑。軸：`InitialMotorParameter` → `InitMotor` → `Open_Axis` | WAR16150（開卡）；WAR16120～16123（開軸、取狀態、設參數、清錯）⚠ 跟料盤 ID 告警撞碼 | `Timer1Timer` 每 200 拍 `CheckPCI_EtherCatState`：斷線 → WAR16152 並自動重開；NU-EC1 站不在 OP → WAR16151。軸警報 → WAR24MMMk | **真的接上**：監看器開卡、命令面送命令、路由認領軸、`W906_BootInitMotorTick` 做 InitMotor；斷線 10 秒 WAR16152、模組檢查 WAR16154、E-045 驅動器警報 | §4.1；[ethercat-api.md](ethercat-api.md)、[ht9050-1203-runtime-traps.md](ht9050-1203-runtime-traps.md)、[ht9050-1203-homing](../../ht9050-1203-homing/SKILL.md)、[yaskawa-ethercat/alarms-and-recovery.md](yaskawa-ethercat/alarms-and-recovery.md) |
| **PCIE-1203 IO**（ISABase 3） | `TLaneIO`／`Acm_Daq*Ex` | 跟馬達共用同一張卡；沒有另外的開卡步驟 | 沒有框，失敗只寫 MNetLog | 跟上一列同一套環檢查 | **真的接上**：`Pci1203IoRoute`（讀監看器樣本、經命令面寫；站是驅動器、沒讀回、卡沒開時拒寫） | §4.2；[ethercat-pci1203-api.md](../../ht9045-io-control/references/ethercat-pci1203-api.md)、[exit-shutdown.md](../../ht9045-io-control/references/exit-shutdown.md) |
| **MotionNet 先達 PCI-L112／L122**（IO＋馬達環） | `TLaneIO` `_mnet_*`；馬達見下面 SYN-TEK slave | `OpenPCI132Card` 的先達段，條件 `IO_CARD_TYPE==0 \|\| MOTION_CARD_TYPE==0` | WAR1690（沒有主卡）、WAR1691（某 ring 沒有模組） | 每拍 `CheckPCI_L112State`：24V、ring 狀態、slave 錯誤表；連續超過 5 次 → `ResetMNet` 跳框、`fAllMotorHome=false` | 照翻但接離線樁；健康檢查**沒有呼叫** | §4.3；[motionnet-api.md](../../ht9045-io-control/references/motionnet-api.md) |
| **MotionNet 泓格 PISO-MN200**（IO＋MN200 馬達） | `TLaneIO` `mn_*`；`TMyMN200Motor` | `OpenPCI132Card` 的 PISO 段，條件 `IO_CARD_TYPE` 1／2；軸 `InitMotor` | WAR1694（開卡或 line 錯誤，附錯誤文字）、WAR1696（模組鮑率與設定不符） | 每拍 `CheckPCI_MN200State`：24V 偵測板、line 狀態、錯誤表；連續超過 5 次 → `ResetMNet`。軸 `GetAlarm` 看 `mn_get_error_status` | 照翻但接離線樁；健康檢查沒有呼叫 | §4.4；[motionnet-mn200-api.md](../../ht9045-io-control/references/motionnet-mn200-api.md)、[mn200-api.md](mn200-api.md) |
| **先達 motion slave**（M204／M104） | `TMySYNTEKMotor`＋`Hontech_M4`（`_Hon_m4_*`，包先達 CMNet） | 類別建構子 `SYNTEKOpenCard`：`_Hon_m4_initial` 與載入 `D:\HT9045\CFG\HT9045_M204_2.cfg` | **沒有**（WAR1692／1693 已註解掉，只回 false） | `GetAlarm` 永遠回 false；只靠 ring 檢查 | 照翻、離線樁 | §4.5；[syntek-motion-slave-api.md](syntek-motion-slave-api.md)、[hontech-m4-api.md](hontech-m4-api.md) |
| **Galil DMC**（Index 四軸） | `TMyGALILMotor`、`TMyMotor::Gali_*`／DMC32 | `TfMain::FormShow` 最後 `Open_GaliCard`（`INDEX_MOTION_CARD==0`）；`InitMotor` 是空的 | WAR1636（讀不到控制器）、WAR1635（Open／Reset／SH 失敗，同時 SoftStop、`fAllMotorHome=false`）、WAR2203（命令錯，寫 log） | `GetMotorAlarmCode` 看 `Gali_MotorAlarm` → WAR24MMMk；`IsIndexMotorOutOfPower` → `LockIndexMotorAndDoHomeProcess` | DMC32 是離線樁；`Open_GaliCard` **沒有呼叫者**；HT9050 的 MTestZ1 改轉 1203 | §4.6；[galil-api.md](galil-api.md)、[galil-motor-api.md](galil-motor-api.md) |
| **CONTEC SMC** | `TMySMCMotor`／`SmcW*` | 類別建構子 `Open_SMCCard`（`SmcWInit("SMC0xx")`，每張卡一次）；軸 `InitMotor` | **沒有**（`SmcWInit` 失敗只回 false） | `ScanMotorStatus`／`GetAlarm` → WAR24MMMk | 離線樁；HT9050 的 SMC 列全是 Enable 0 | §4.7；[smc-api.md](smc-api.md)、[smc-motor-api.md](smc-motor-api.md) |
| **和椿 MC88X1** | `HTMC88X1Motor` | golden 906 沒編（不在 `HT9045.bpr`，`new` 已註解） | — | — | 沒有 | §4.8；[mc88x1-api.md](mc88x1-api.md) |
| **Tray 步進馬達**（RS-232） | `TdmTrayMotor`（不是 HTMotor） | `InitialHandler` 最後 `dmTrayMotor->RS232Init`（115200） | 跳框「Tray Step Motor : COMxx port error」 | `DoTrayStepMotor` 問版本；震動馬達「通訊timeout」框 | **沒有移植**（閘 N1-G2g） | §4.9；[tray-step-motor-api.md](tray-step-motor-api.md) |
| **安全 PLC**（ISABase 4） | `MyPLC_IO_Modbus`（Modbus TCP） | `InitHontechHardware` 最後 `InitPLCIO`（`SafePlcIO=1`） | 沒有框（斷線訊息已註解，Ken 20250428） | `PLCStatusCheck` 斷線時每 5 秒自動重連；開機 `bCheckPLCAllSafedoorAndEMGEnable` | 閘已打開（RULINGS_20260926 第 20 條） | §4.10；[ht9045-io-control SKILL.md](../../ht9045-io-control/SKILL.md) |
| **ISA／PCI-1735U**（ISABase 1／2） | `myio.cpp` port I/O | `InitialHandler` 的 `InitNTPort`、每個 port 第一次用時 `EnableNTPort` | 沒有 | 沒有 | 閘住（x64 沒有這條路） | §4.10；[io-classes.md](../../ht9045-io-control/references/io-classes.md) |
| **EtherCAT 感測／真空模組**（NU-EC1、ECAT-VC8） | `TMyEtherCAT`／`TMyNUEC1`；VC8 走 `TLaneIO` ePCI1203 的 AI 與 SDO | 跟 1203 卡共用；NU-EC1 由 CC-Link 感測器畫面建立 | WAR16151、WAR16153（數量錯，已改成只寫 log） | `CheckPCI_EtherCatState` 檢查站在不在 OP；30 ms 的 `tmrReadInputData` 讀值失敗寫 `NewRecordProcess` | VC8／VC4 有 `Pci1203Vc8`；模組檢查排除真空模組 | §4.10 |

---

## 4. 各技術細節

### 4.1 PCIE-1203 EtherCAT 馬達（`TMyEtherCatMotor`）

**golden 開卡**（`EtherCAT/MyEtherCAT.cpp OpenEtherCatMastCard`）
1. `Acm_DevClose`，再 `Acm_GetAvailableDevs`，取第一張卡。
2. `Acm_DevOpen` 最多 3 次。每次失敗先在 `Acm_DevReOpen` 迴圈裡等到回 `EC_OpenMasterDevFailed`，再 `Acm_DevClose`、等 2 秒。
3. 等 0.5 秒，`Acm_DevEnableEvent(EVT_DEV_DISCONNET | EVT_DEV_IO_DISCONNET)`。
4. 對每個 `CardType=="PCI1203"` 而且啟用的軸做 `ResetAxisOpen` 與 `InitMotor`。

呼叫的條件是 `INSTALL_ETHETCAT()`，也就是 `SHUTTLE_SENSOR_TYPE` 是 6／7，或 `VacuUnitType=1`。**golden 沒有「表上有 PCI1203 的列就開卡」這種條件**。HT9050 快照剛好 `VacuUnitType=1`，照 golden 會開卡，但那是借用真空模組的旗標（§7.1）。

**golden 軸初始化**（`Motor/myEthercatmotor.cpp TMyEtherCatMotor::InitMotor`）
1. `Open_Axis`：`Acm_AxOpenbyID(uiDevhand, BoardID, Port)`，失敗報 WAR16120。卡沒開（`uiDevhand==0`）就直接返回，**沒有訊息**。
2. 四個 EMG 感測器任一個是 off → 跳框「請解開EMG並重新啟動軟體!!」，回 false。
3. 清錯 ladder：`Acm_AxResetError`（失敗 WAR16122）；`Acm_AxGetState` 不是 READY 就再清（再失敗 WAR16123，`goto` 重來，沒有上限）；取狀態本身失敗 WAR16121，也 `goto` 重來。
4. 設定參數：PPU=1、ElReact=0、AlmEnable=1、AlmReact=0、OrgLogic（由 SensorType 決定）、Jerk=0、`SetEtherCatInType`、依 1P2P 設定脈波輸入／輸出模式、`CFG_AxMaxVel=PJogHighSpeed`、`MaxAcc=dAcc`、`MaxDec=dDec`。任一項失敗都報 WAR16122。
5. 再清一次錯，`SetServoOn(true)`，`SetCommand(0)`、`SetPosition(0)`。

**移植樹**
- 開卡由 `TPci1203Monitor::Open` 負責：
  - `Acm_GetAvailableDevs`，再 `Acm_DevOpen`。冷開機回 0x83000002（從站還沒就緒）時每秒重試，最多 `WB_1203_OPEN_WAIT_SEC` 秒（預設 90）。
  - 回 0x8300002B（SubDevice ID 衝突）時照樣開卡，但大字警告「讀值可能歸到錯的站」。
  - 掃描站（最多 4 秒），讀 DI／DO 通道數。
  - **只對 ring 0** `Acm_MasStartRing`（ring 1 是 IO 環，刻意不啟動）。
  - 用 ID 開軸。
  - 生產端已經開了卡時走附掛模式，不重開。
- 引擎的 `TMyEtherCatMotor` 不開軸，改向路由「認領」（`EcatMotorRoute()->bind`）。Direction=1 的列會被拒絕。
- 真正的 golden InitMotor 由 `WebMotorAccessLive.cpp W906_BootInitMotorTick` 執行：
  - 條件：卡開好、軸已認領、有新樣本、馬達電源 ON 滿 1 秒。
  - 對 18 個 HT9050 軸各跑一次（`WebMotorAccess.cpp MotorAccessBootInitMotor` → `InitMotor1203`，清錯最多 3 輪），每一步寫進 op log 的 BRAKE 行。
  - MTestZ1 不在名單裡：它走 Galil 路由，伺服 ON 在 uhome case 5 送 "SH"。

**開卡或初始化失敗怎麼發現**
- golden：WAR16150（參數是錯誤碼）、WAR16120～16123。⚠ 這四個碼在 `AlarmCodeList.txt` 是料盤 ID 與三小時檢驗的文字，畫面會顯示錯的說明（§7.3）。
- 移植樹：
  - 開卡失敗只印在主控台（`card NOT opened: …`），原因放在 `pci1203.*` tag，1203 頁看得到，**不跳框**。
  - golden 的 WAR1612x 框全部在 `#if HAVE_PCI1203` 裡，沒有編進來。被拒的命令只寫 op log 的 `ROUTE` 行。
  - 開機驗表 `TableAudit` 有兩條 ERROR：T4（兩個啟用軸的 BoardID＋Port 相同）、T9（HT9050 上啟用的軸 CardModel 不是 PCI1203）。

**執行期健康檢查**
- golden：
  - `TfMain::Timer1Timer` 每 200 拍（註解寫約 6 秒）跑 `CheckPCI_EtherCatState`，只在 `INSTALL_ETHETCAT()` 時跑：
    - `Acm_DevCheckEvent` 報斷線 → WAR16152（Ring0／Ring1／Ring Error），接著 `OpenEtherCatMastCard` 重開卡、重新 InitMotor；
    - NU-EC1 站不在 OP → WAR16151，同樣重開。
  - 軸警報走 `ScanAllMotorStatus`／`GetMotorAlarmCode` → WAR24MMMk（§5）。
- 移植樹：
  - `W906_Pci1203LinkWatchTick`（`EtherCAT\Pci1203LinkWatch.h`）：曾經 OP 的站不再是 OP、監看器自己停用、卡沒開、或掃不到任何站，**連續 10 秒** → WAR16152。照 golden 停機，**不會自動重開卡**。
  - `W906_Pci1203ModuleCheckTick`（`EtherCAT\Pci1203ModuleCheck.h`）：第一次連線把模組清單寫進 `D:\HT9045\system\Pci1203Modules.ini`；之後每次連線 10 秒內比對（缺少、沒進 OP、站號變了、多出來）→ WAR16154。WAR16154 是新碼，AlarmCodeList 裡還沒有。
  - E-045：`EcatAlarmScan.cpp W906_EcAlarmScanRow`，只限 HT9050。驅動器出現 ALM 或 ERROR_STOP → 走 golden 的升警報，同一拍做一次 `ResetState`；清不掉就鎖住，要求整機斷電重開。細節見 [ht9050-1203-runtime-traps.md](ht9050-1203-runtime-traps.md) §3。

**HTMotor 提供的介面**
- `ScanMotorStatus(Led)`：有 ALM、LMT、ORG。`iInposLed` 在 1203 上永遠是 false。
- `MotionDone()`：ERROR_STOP 永遠不算 READY。
- `GetAlarm()`、`ResetState()`（清錯）、`ResetAxisOpen()`、`SetServoOn()`。

**怎麼恢復**
- golden：自動重開卡並重新 InitMotor。⚠ 重開會把座標歸 0，但不清 `fAllMotorHome`（§7.3）。
- 移植樹：1203 頁「重新掃描」（`kCmdCardRescan`）、Reset Error（`kCmdAxResetError`）、伺服 ON（`kCmdAxSvOn`）。
  - ID 衝突：改站號，再把 **EtherCAT 站斷電重開**（重開 PC 沒有用）。
  - 安川驅動器斷電後出現 A.A12，要 Reset Error。
  - E-045 鎖住：整機斷電重開，再 HOME。

### 4.2 PCIE-1203 IO（ISABase 3）

- golden：
  - `TLaneIO::IOBitOn`／`IOInputBit` 呼叫 `Acm_DaqDoSetBitEx`／`Acm_DaqDiGetBitEx(uiDevhand, Ring, IP, Port)`。
  - `uiDevhand` 只有 `OpenEtherCatMastCard` 會給值，卡沒開時每次讀寫都失敗，**只寫 MNetLog**（主畫面的 MNet 記錄，不是 EventLog）。
  - `CheckPortRangeErr` 只檢查 eMotionNet 的點，1203 的點不檢查。
  - VC8 真空模組走同一個 handle：AI 讀 `Acm_DaqDiGetByteEx`，門檻讀寫 SDO 0x8000＋Port×0x10、子索引 0x13。
- 移植樹：
  - 要 `IO_CARD_TYPE=4` 才會讀表（`TableAudit` T1：HT9050 不是 4 就報 ERROR）。
  - `SelectVendorBackends` → `TPci1203Backend` → `EtherCAT\Pci1203IoRoute.cpp`：
    - 讀：直接用監看器的 DI／DO 樣本；
    - 寫：經命令面 `kCmdDoSetBit`／`kCmdDoSetByte`；
    - 卡沒開、那個 byte 沒讀回、或那一站是驅動器（CiA 402 或名稱有 SERVOPACK）時拒寫，原因留在紀錄。
  - ⚠ golden 的輸出快取 `OutPortData[4][64][4]` 用 1203 的站號與通道當索引會越界。這是 `TMySwitch::Status`／`TMyCylinder::GetOutBit` 的問題，路由本身不碰這個快取（`Pci1203IoRoute.h` 檔頭）。
- 健康與恢復跟 §4.1 同一套（連線監看、模組檢查）。按 Exit 時清輸出見 [exit-shutdown.md](../../ht9045-io-control/references/exit-shutdown.md)。

### 4.3 MotionNet 先達 PCI-L112／L122（`OpenPCI132Card` 的先達段）

**開卡**（條件 `IO_CARD_TYPE==0 || MOTION_CARD_TYPE==0`）
1. 第一次（`bfirst`）：`_l112_open`，不行再試 `_l122_open`。兩張都沒有 → **WAR1690**「No SYN-TEK Master Card」，結束開卡。
2. ring 0、ring 1 各做一次：
   - 第一次：`_mnet_set_ring_config(ring, MOTIONNET_SPEED)` → `_mnet_reset_ring` → 等 20 ms → `_mnet_get_ring_active_table` → 存成 `D:\HT9045\system\deviceInfo.cfg`。
   - 不是第一次：讀並清 slave 錯誤表，載入上次存的 active table。
3. 某 ring 一個模組都沒有 → **WAR1691**「No SYN-TEK DIO Module」（附 Ring 編號），結束開卡。
4. `_mnet_start_ring`。逐一用 `_mnet_get_slave_type` 掃 IP，把模組種類填進 `MyLaneIO.iUseMNetIP[ring][ip]`（Motion、32OUT、32IN、16IN16OUT、8AI、NONE），每條 line 寫一行 MNetLog 摘要。

**執行期**（`TfMain::Timer1Timer` 每一拍 `CheckPCI_L112State`；回 2 時這一拍 Timer1 剩下的工作全部跳過）
- L122 才有 24V 偵測：`_l122_lio_input_read`，bit1＝偵測板有接線，bit0＝24V 正常。連續 2 次斷電 → `ResetMNet` 跳框「24V MNet電源異常」，`fAllMotorHome=false`。
- `_mnet_get_ring_status` 兩條 ring 用 0x78 遮罩或狀態是 0 時，依位元分類：IO 模組錯誤（從錯誤表找出哪一個 IP）、軸控模組錯誤、主控端設定錯誤、主控端操作錯誤；兩條 ring 一起錯就判斷成「24V開關被關閉」。
- 連續超過 5 次 → `ResetMNet` 跳框、`fAllMotorHome=false`；還沒到 5 次只寫 MNetLog。

**IO 類別提供的介面**
- `TLaneIO::CheckPortRangeErr` 回傳碼（`GetIOErrStr`）：1＝Bit 不在 0～7；2＝Ring／IP／Port 超出範圍；3＝那個 IP 不是 IO 模組（看 `iUseMNetIP`）；4／5＝16IN16OUT 模組的輸出／輸入 port 用錯。
- 位址錯 → MNetLog，並跳框「Maybe input wrong IO position!」（IO 畫面開著時，輸入點不跳框）。
- 廠商函式回錯 → 只寫 MNetLog。

**怎麼恢復**（`ResetMNet`）
- ring 恢復正常後的下一次檢查呼叫 `ResetMNet(false)`：
  1. `OpenPCI132Card(true)` 重開卡；
  2. `SwMotorRelay` 關 1 秒再開，等 0.5 秒（9046AU 時間乘 3）；
  3. Bin 顯示器重新初始化；
  4. `fLtcSensor->SetLtcSensor(0／1)`。
- 之後操作員要重新 HOME。

**移植樹**
- `OpenPCI132Card` 照翻，但 `_l112_open` 是離線樁，而且不寫輸出參數，`existcard` 是沒有初始值的區域變數。在 HT9050（`MOTION_CARD_TYPE=0`）上結果可能是 WAR1690／WAR1691，也可能把亂值寫進 `iUseMNetIP`（[ht9050-1203-runtime-traps.md](ht9050-1203-runtime-traps.md) §5）。
- `CheckPCI_L112State` 沒有任何呼叫者（`MainTimerSegments.cpp` 檔頭）。

### 4.4 MotionNet 泓格 PISO-MN200（`OpenPCI132Card` 的 PISO 段＋`TMyMN200Motor`）

**開卡**（條件 `IO_CARD_TYPE` 是 1 或 2）
1. `mn_open_all(&NumLine)` 失敗 → **WAR1694**（附 `GetMN200_Error_Code` 的文字），結束開卡。
2. 每條 line 用 4 種鮑率各試一次：`mn_stop_line` → `mn_set_comm_speed` → `mn_reset` → `mn_start_line` → 讀並清錯誤表 → 用 `mn_get_dev_info` 掃 64 個 IP。
   - 掃到的模組種類填進 `iUseMNetIP` 與 `myLine[].RaudRate`／`Type`。
   - 馬達模組另外設 `CUST_REPLACE_SPEED_PAR`、`CUST_FIXED_MAX_SPEED=100K`。
3. 用 `MOTIONNET_SPEED` 掃到的模組比 4 種鮑率合計掃到的少 → 顯示 MNet 樹，報 **WAR1696**。文字列出「Line:x_IP:y_Rate:z」，代表那顆模組的 DIP 鮑率跟設定不一樣。
4. 每條 line 改回 `MOTIONNET_SPEED`：reset → start → 對 64 個 IP 各做一次 `mn_get_cmdcounter`（避免上次沒 stop line 就斷電，找不到第一個 ID）→ 讀並清錯誤表。任一步錯誤報 WAR1694。

**軸初始化**（`TMyMN200Motor::InitMotor`）
1. `mn_set_motion_cfg`：EL_PROC、ALM_PROC 設成 SUDDEN_STOP，SD_ENA 關閉，ORG_LOGIC 依 SensorType。
2. `SetMN200InType`，`SetCommand(0)`、`SetPosition(0)`。
3. 依 1P2P 設定 ENC_MODE 與 PULSE_MODE，最後 `SetServoAlarm`。
4. 任一步失敗 → 跳框（板號、port、項目）並寫 MNetLog。

**執行期**（每拍 `CheckPCI_MN200State`）
- 24V 偵測（`mn200_get_di`）：
  - bit3＝新式脈波型：狀態維持不變超過 `i24V_PULSE_COUNT` 拍（約 2.1 秒）→ `ResetMNet`，再對 DO bit0 送一個 IO Clear 脈波；
  - bit1＝舊式：連續 2 次 24V 斷電 → `ResetMNet`；
  - 兩種都不是 → 跳框「24V偵測板未安裝或者接線脫落!」。
- 每條 line 檢查 `mn_get_line_status`（`MN200_Line_status_OK`）與錯誤表（「MNet Ring r IP n IO Device Error」）。同一條 ring 連續超過 5 次 → `ResetMNet` 跳框、`fAllMotorHome=false`、清錯誤旗標。
- 軸：`GetAlarm` 看 `mn_get_error_status` 的 0x20 位元。

**恢復**：同 §4.3 的 `ResetMNet`。

**移植樹**：照翻、接離線樁，健康檢查沒有呼叫。x64 的 MN200 DLL 可行性見 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RECON_MN200_PISO.md`。

### 4.5 先達 motion slave（`TMySYNTEKMotor`＋`Hontech_M4`）

- **開卡在類別建構子** `SYNTEKOpenCard`：
  - 用 `_mnet_get_slave_type` 判斷模組型號。
  - M204：`_Hon_m4_initial`，再 `_Hon_m4_load_motion_file("D:\HT9045\CFG\HT9045_M204_2.cfg")`。M104 同樣做法。
  - 失敗只回 false，**沒有告警**：WAR1692「Open SYN-TEK DIO Module Fail」與 WAR1693「Load SYN-TEK Motion Module Config Fail」兩行已經註解掉。
- `InitMotor` 只呼叫 `_Hon_m4_set_feedback_src`：步進用 command，伺服用 encoder（M204 是 3，其他是 2）。golden 註解（Dell 2013）：這一行不能跟載入 cfg 放在一起，因為載入 cfg 會把 feedback 來源蓋掉。
- **`GetAlarm()` 永遠回 false**（jou 20180103）⇒ 先達軸的驅動器警報不會從 `GetAlarm` 冒出來，只剩 Led 掃描與 ring 檢查。
- `HomeFlag()` 讀 `_Hon_m4_get_io_status` 的 bit4。
- `Hontech_M4` 是鴻勁自己包的 C 函式層，底下呼叫先達 CMNet 的 `_mnet_m204_initial`／`_mnet_m4_initial`（`CMNet.h`：`G9004_M204 // SYNTEK-M204`），**不是泓格的 API**（§7.4）。
- 移植樹：照翻，接離線樁。

### 4.6 Galil DMC（Index 四軸）

- **選用**：`INDEX_MOTION_CARD==0` 時，MTestY1／Z1／Z2／Y2 建成 `TMyGALILMotor(0..3)`，不看 CardModel。Enable 強制是 true（Index 三軸機型的 Y2 例外）。`TMyGALILMotor::InitMotor` 什麼都不做。
- **開卡**（`Motor/myGALILmotor.cpp Open_GaliCard`，在 `FormShow` 最後、其他卡都開完之後）：
  1. `DMCGetControllerDesc` 失敗 → **WAR1636**「Initial Index Motion Card Fail!」。
  2. `DMCOpen` 失敗 → SoftStop、`SystemStart=false`、**WAR1635**（附 `GetGalilErrString`）、`fAllMotorHome=false`。
  3. `DMCReset` 失敗：處理同上。
  4. 送 `"SH"`（所有軸伺服 ON）失敗：寫 WAR2203「Gail Command Err」，並報 WAR1635。
  5. 都成功 → `bGali_CardInstall=true`。關程式時 `Close_GaliCard`。
- **執行期**：
  - `ckernel.cpp GetMotorAlarmCode`：`Gali_MotorAlarm` 為真而且 `MotorPowerOnDelay==0` → SoftStop、重設 Index 掃描旗標、`Gali_ScanMotStatus`，報 WAR24MMMk。
  - `csystem.cpp IsIndexMotorOutOfPower`：EMG、馬達斷電、Z1／Z2 的 `Led[iServoOn]` 是 false 任一成立 → `LockIndexMotorAndDoHomeProcess`：送 ST、Index 煞車 OFF、`StopAllMotor`、`fAllMotorHome=false`。
  - 面板按 Power Off 時 `fHome->GaliMotorServoOff`（伺服 OFF 時要先鎖住 Z 軸煞車）。
- **恢復**：Galil 只在開機開一次卡 ⇒ 要重新開卡就是重開程式，之後 HOME（uhome case 5 送 "SH"）。
- **移植樹**：
  - DMC32 的 7 個函式是離線樁（`Motor\vendor_offline_galil.cpp`，一律回 `DMCERROR_CONTROLLER`）。
  - `Open_GaliCard` 有定義，但**沒有呼叫者**。
  - HT9050 的 MTestZ1 由 `Pci1203GaliRoute` 轉成 1203 命令（`W906_GaliRoutedSingalHome`，見 [ht9050-1203-homing](../../ht9050-1203-homing/SKILL.md) §2b）。
  - 表上沒有的 Index 軸，開機時由 `W906_Ht9050IndexAbsentAtBoot` 設成停用。這是偏離 golden 的做法。
  - gclib 只有 64 位元版：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RECON_GALIL_gclib.md`。

### 4.7 CONTEC SMC（`TMySMCMotor`）

- **開卡在類別建構子** `Open_SMCCard`：`SmcWInit("SMC0"+卡號)`，每張卡只做一次（記在 `MCSMCCardInstall[]`）。失敗只回 false，**不報警**。症狀會延後出現：那張卡的每一軸之後都動不了。
- `InitMotor` 依序：
  1. `SetEncodeMultiple`；
  2. `SmcWSetCtrlTypeIn`：ALM、INP、LTC（安川線性馬達只有 INP、LTC）；
  3. `SmcWSetCtrlTypeOut`；
  4. 依 SensorType `SmcWSetOrgLog`；
  5. `SetSMCInType`，`SetCommand(0)`、`SetPosition(0)`；
  6. 依 1P2P `SmcWSetPulseType`；
  7. `MotOutputOn(1)`（伺服 ON 輸出）、`SetServoAlarm`、`SmcWSetInitParam`。
- 執行期：`ScanMotorStatus` 讀 `SmcWGetCtrlInOutStatus`，結果放進 Led → `GetErrorIndex` → WAR24MMMk。Latch 讀取錯誤用 `SmcWGetErrorString` 放進 `ErrorString`。
- 回原點是卡片做的（`SMCMotHome`，跟 1203 的驅動器式回原點不同，見 [ht9050-1203-homing](../../ht9050-1203-homing/SKILL.md) §1a）。
- 移植樹：`SmcW*` 是離線樁（回 -1）⇒ 每個 SMC 軸都安靜地離線。HT9050 上的 SMC 列全是 Enable 0；表上缺列的軸會建一個 `TMySMCMotor(-1)` 佔位。

### 4.8 和椿 MC88X1（`HTMC88X1Motor`）

- golden 906 **沒有編進來**：`HTMC88X1Motor.cpp` 不在 `HT9045.bpr`，`InitialMotorParameter` 裡的 `new HTMC88X1Motor` 已註解（Steven 20231218 HT7080B）。CardModel 寫 MC88X1 的列會在 CSV 那條路上留下 NULL 指標（§7.3）。
- HT7080B 用的是 `IO_CARD_TYPE=3`（`PCI_P64C64`），不在本專案範圍。移植樹沒有這個類別。[mc88x1-api.md](mc88x1-api.md) 只有廠商 API，沒有寫開卡流程。

### 4.9 Tray 步進馬達（`TdmTrayMotor`，RS-232）

- 選用：`[TrayY] LoaderUnload_StepMotor=1`，或控制面板（`ControlPanelMode`）、震動馬達（`VibrationCommunication`）任一個開著，三者共用一個序列埠，埠號是 `[TrayY] COM PORT`（預設 COM18）。
- 開埠：`InitialHandler` 最後 `dmTrayMotor->RS232Init`，115200 baud，`StartComm`。失敗 → 跳框「Tray Step Motor : COMxx port error」，並寫一筆 Exception。
- 執行期：`DoTrayStepMotor` 狀態機用 `CMD_CheckStepMotorVer` 問各軌道的韌體版本；震動馬達沒回應時跳「震動馬達通訊timeout」。
- 移植樹：**沒有移植**。`dmTrayMotor` 只是 dfm2rc 產生的版面資料，`InitialHandler` 裡的閘 N1-G2g。HT9050 不用（`LoaderUnload_StepMotor=0`）。

### 4.10 其他 IO

- **安全 PLC**（ISABase 4，`SafePlcIO=1`）：
  - `InitPLCIO` 啟動 `TPLCIOThread`。`PLCStatusCheck` 在斷線時每 5 秒自動重連，「PLC disconnect」那一框已經註解掉 ⇒ 斷線時沒有任何訊息。
  - 開機 `csystem.cpp bCheckPLCAllSafedoorAndEMGEnable` 會擋：`SnSafeMode` 沒建列時，`SnAllSafeDoor` 與 `SnAllEMG` 不能同時停用。
  - Enable／InType 被強制成 1 的陷阱見 [ht9045-io-control SKILL.md](../../ht9045-io-control/SKILL.md)。
- **ISA／PCI-1735U**（ISABase 1／2）：
  - `InitNTPort` 之後直接 port I/O，每個 port 第一次用時 `EnableNTPort`。
  - 沒有健康檢查。`TTL_CARD_TYPE>0` 時全部跳過。
  - 移植樹兩個呼叫都閘住（`cinitial.cpp InitialHandler` 的 N1-G2b、`myio.cpp` 的 `MYIO_ENABLENTPORT`）。
- **EtherCAT 感測／真空模組**：
  - NU-EC1（接 FS-N12 放大器）由 `TMyEtherCAT` 每 30 ms 讀一次（`tmrReadInputData`）。讀取錯誤寫 `NewRecordProcess("EtherCAT Read I/O error")` 一類的紀錄；`TaskClear` 會觸發 case 500 重開卡。
  - ECAT-VC8 見 §4.2。
  - 移植樹另有 `EtherCAT\Pci1203Vc8.h`；模組檢查把 VC8／VC4 排除。
- **ADAM-6024**（EP 比例閥）：`FormShow` 在 `InitialHandler` 之前呼叫 `Open_ADAM_6024`。連線與錯誤全部見 `ht9045-adam6024` skill。

---

## 5. 共通：馬達警報碼怎麼讀、IO 錯誤記在哪裡

- **馬達**（golden `note.cpp MotorIndexToJamCode`、`ShowMotorErrorMessage`；`Motor/mymotor.cpp TMyMotor::GetErrorIndex`）：
  - 碼是 `WAR24` 接三位數馬達編號，再接一位 k。例 MInArmX＝M00 → WAR24000k。
  - k 的意思：1＝扭力不足或馬達斷電；2＝扭力不足；3／4＝CW／CCW 極限；5／6＝軟體極限；7＝位置錯誤，要 HOME 後重來（CW／CCW 燈亮但沒有警報）；8＝Motor Alarm（其他情況都歸到 8）。
  - 1203 只給得出 ALM 與 LMT ⇒ 通常是 k=8，驅動器自己的錯誤碼（603Fh／A.xxx）不會帶進框（[ht9050-1203-runtime-traps.md](ht9050-1203-runtime-traps.md) §3）。
  - 報警時同時做：`StopAllMotor`、Galil 送 ST、Index 煞車 OFF、`fAllMotorHome=false`（一定要重新 HOME）。
  - 移動函式的回傳值：`MotorMove` 回 -4＝伺服 Alarm（表在 [ht9045-motor-control SKILL.md](../SKILL.md)）。
- **IO**：
  - 位址錯與廠商呼叫失敗寫 **MNetLog**（主畫面 `slMNetLog`／`mmoMNet`），**不是 EventLog**。
  - 氣缸逾時的碼一律是 31000＋氣缸編號，DB 裡寫的告警碼會被覆寫（[io-classes.md](../../ht9045-io-control/references/io-classes.md)）。

---

## 6. 馬達或 IO 出問題時先問的問題

1. **是哪一軸、哪一點？** 拿到 Alias（例 MInArmZA、SnMotorPower）與告警碼（WAR24MMMk、WAR169x、WAR1612x、WAR1615x、WAR1635／1636）。
2. **它用哪一種技術？**
   - 馬達：先看 `Gerneral.ini` 的 `IO_CARD_TYPE`、`MOTION_CARD_TYPE`、`INDEX_MOTION_CARD`，決定讀哪一份表（§1.2），再看那一列的 `CardModel`。Index 四軸先問 `INDEX_MOTION_CARD`。
   - IO：看那一列的 `ISABase`；0 再看 `IO_CARD_TYPE`（§1.3）。
3. **那一列本身對不對？** Enable 是不是 1、位址欄有沒有空的（空了會被強制成 0）、數字有沒有打錯字（打錯會安靜地變 0）、BoardID＋Port 有沒有重複、GearRatio 是不是 0。
   - 移植樹看主控台的 `[BOOT]` 行與 `TABLEAUDIT` 行（[table-loading-port.md](../../ht9045-io-control/references/table-loading-port.md)）。
4. **卡有沒有開起來？** 對照 §3 第 3、4 欄：
   - golden：找開機時的 WAR1690／1691／1694／1696／16150／1635／1636，MNetLog 的 line 摘要。
   - 移植樹：主控台 `card OPEN`／`card NOT opened`、`ID CONFLICT` 警告、1203 頁的 card 狀態。
   - ⚠ SMC 與先達 slave 開卡失敗**不會有任何告警**，只會「之後每一軸都動不了」。
5. **軸有沒有初始化、伺服有沒有 ON？**
   - golden：開機時 EMG 壓著，1203 軸就不會 InitMotor（要重開軟體）。
   - 移植樹：看 op log 的 BRAKE 行 `InitMotor (boot): …`，與 `pci1203` 樣本的 SvOn。
6. **執行中斷過嗎？**
   - MotionNet：MNetLog 有沒有 Ring／IP 錯誤、24V 偵測板訊息。
   - 1203：op log 的 `LINK` 行（斷線、恢復、WAR16152、WAR16154）、`W906 ECALARM` 行。
   - Galil：WAR2203。
7. **需要重新 HOME 嗎？** 下列情況 `fAllMotorHome` 都會被清掉：任何 WAR24MMMk、`ResetMNet`、Galil 開卡失敗、Index 斷電。回原點規則見 [ht9045-motor-home](../../ht9045-motor-home/SKILL.md)、[ht9050-1203-homing](../../ht9050-1203-homing/SKILL.md)。
8. **細節去哪裡看**：§3 最後一欄。

---

## 7. 缺口（golden 與移植樹不同、還沒移植、沒有文件）

### 7.1 golden 與移植樹不同

| 項目 | golden | 移植樹 | 證據 |
|---|---|---|---|
| 1203 誰開卡 | `OpenEtherCatMastCard`，條件只有 `INSTALL_ETHETCAT()`（EtherCAT 感測器或 `VacuUnitType=1`） | 監看器 `Pci1203MonitorEnable`；`OpenEtherCatMastCard` 沒有 SDK 時不開卡、直接回成功 | golden `EtherCAT/MyEtherCAT.cpp INSTALL_ETHETCAT`／`OpenEtherCatMastCard`；移植樹 `tools\wb_serve.cpp main`、`EtherCAT\MyEtherCAT.cpp OpenEtherCatMastCard` 的 `#else` |
| 1203 軸 InitMotor 的時機 | 開機在 `InitialMotorParameter` 裡做；EMG 壓著就不做 | 開機那一次一定失敗而且沒有訊息；改在馬達電源 ON 滿 1 秒後由 `W906_BootInitMotorTick` 做；清錯從無上限改成最多 3 輪 | `Motor\myEthercatmotor.cpp InitMotor`／`Open_Axis`、`WebMotorAccessLive.cpp W906_BootInitMotorTick` |
| 1203 斷線 | 第一次看到就報 WAR16152，並自動重開卡＋全部 InitMotor（座標歸 0） | 連續 10 秒才報 WAR16152；不自動重開，由操作員按「重新掃描」 | `CheckPCI_EtherCatState` 對照 `W906_Pci1203LinkWatchTick` 的說明 |
| 1203 開卡或命令失敗 | WAR16120～16123 跳框 | 不跳框，只有主控台、tag、op log `ROUTE` 行 | `Motor\myEthercatmotor.cpp`（`#if HAVE_PCI1203`）、[ht9050-1203-runtime-traps.md](ht9050-1203-runtime-traps.md) §3 |
| 1203 模組在位檢查 | 沒有（只有 NU-EC1 的 OP 檢查） | `Pci1203Modules.ini` 比對 → WAR16154（新碼） | `EtherCAT\Pci1203ModuleCheck.h` |
| 1203 驅動器警報 | 自動運轉只重掃 5 軸，而且要 `ServoAlarmOn` | E-045，HT9050 限定 | `EcatAlarmScan.cpp` |
| 開機驗表 | 沒有 | `TableAudit` T1～T12 | `TableAudit.h` |
| `m_Axishand` 陣列大小 | `[999]`，HT9050 的 M108（BoardID 108 ⇒ MotorID 1080）會越界 | 改成 `[2560]`（RULINGS_20260925 第 28 條） | `Motor\myEthercatmotor.h` |
| Index 軸 Enable | Galil 軸強制開啟 | HT9050 表上沒有的軸停用 | `cinitial.cpp InitialMotorParameter` 的 `W906_Ht9050IndexAbsentAtBoot` |
| 馬達電源與 Galil 開卡 | `FormShow` 最後送 `SwServerON.On()`、`Open_GaliCard()` | 兩者都沒翻（改用 `DoSystem` 自動上電；Galil 沒有呼叫者） | `FileRW\MainBoot.cpp` 的說明 |

### 7.2 還沒移植（移植樹是離線樁或閘住）

- MotionNet（先達＋MN200）、SMC、Galil 的廠商函式**全部是離線樁**（`Motor\vendor_offline_motionnet.cpp`、`vendor_offline_smc.cpp`、`vendor_offline_galil.cpp`）。⇒ **移植樹現在只有 PCIE-1203 會真的動**，HT9045 舊機台（MN200＋SMC＋Galil）跑移植樹完全不能控制。
- `CheckPCI_L112State`／`CheckPCI_MN200State` 沒有呼叫者（`MainTimerSegments.cpp` 檔頭說 Timer1Timer 的 IO 卡那一段還要另外補），⇒ `ResetMNet` 的 24V 與 ring 自動恢復也不會跑。
- `CheckPCI_EtherCatState` 有翻、沒呼叫，改由 LinkWatch 取代（§7.1）。
- Tray 步進馬達、控制面板、震動馬達的 RS-232（閘 N1-G2g）；`InitNTPort`／`EnableNTPort`（N1-G2b、`myio.cpp`）；MN200 開卡時顯示 MNet 樹（`ShowMNetTree`，閘 W3 (a)(b)）。
- HT9050 上 `MOTION_CARD_TYPE=0` 讓先達開卡跑在離線樁上（`existcard` 沒有初始值）；伺服 ON 重新同步走 SYN 那一支、寫 0（[ht9050-1203-runtime-traps.md](ht9050-1203-runtime-traps.md) §1、§5）。

### 7.3 golden 本身的疑似缺陷（照翻；都沒有上機驗證）

1. **1203 的告警碼跟料盤告警撞碼**：`myEthercatmotor.cpp` 用 WAR16120～16123，但 `D:\HT9045\Error\AlarmCodeList.txt` 與移植樹 `AlarmCodeCatalog.cpp` 裡：
   - WAR16120＝「Loader Tray ID no respond」、WAR16121＝「Empty Color Tray ID no respond」（`acatchtray.cpp`、`cTrayMapping.cpp` 也用這兩個碼）；
   - WAR16122＝「Load tray need take out tray manually」（`asendic_Loader.cpp`）；
   - WAR16123＝三小時檢驗。
   ⇒ 1203 開軸或設參數失敗時，畫面會顯示料盤的說明。
2. **重開 1203 卡時座標歸 0，但不清 `fAllMotorHome`**：`OpenEtherCatMastCard` → 每軸 `InitMotor` → `SetCommand(0)`／`SetPosition(0)`，`CheckPCI_EtherCatState` 也沒有清 `fAllMotorHome`。斷線自動恢復後，機台可能用錯的座標繼續跑。移植樹不自動重開，而且路由拒絕在 DS402 上改座標，所以碰不到這個問題。
3. `CheckPCI_EtherCatState` 的 ring 0「再數一次站數確認」永遠不會跑：`iTotalDeviceRing0` 從來沒有被賦值。NU-EC1 的迴圈固定跑 3 次。
4. **SYNTEK 列的位址算法前後不一致**：CSV 那條路算 BoardID×1000＋IP×**100**＋Port，但 `TMySYNTEKMotor` 建構子用 `(Addr%1000)/10` 解出 IP ⇒ IP 不是 0 時會解錯。motor.db 那條路用 IP×10，是對的（`cinitial.cpp InitialMotorParameter` 的 SYNTEK 那一支對照 `mySYNTEKmotor.cpp` 建構子）。
5. **MC88X1 列會解參考 NULL**：CSV 那條路不建立物件，接著就寫 `MOT[i].Motor->Enable`。`TMyMotor` 建構子不設定 `Motor`，全域 `MOT[]` 的初值是 NULL。
6. `TMyMN200Motor::GetMN200ErrorMessage` 查不到錯誤文字時，寫進 MNetLog 的是空字串（組好的 `StrCh` 沒有用到）。
7. `TMySYNTEKMotor::GetAlarm` 永遠回 false；SMC 與先達 slave 開卡失敗都不報警（§4.5、§4.7）。
8. 安全 PLC 斷線沒有任何訊息（`PLCStatusCheck` 那一框已註解）。

### 7.4 沒有文件，或文件有錯

- **開卡順序、健康檢查、開卡失敗的告警碼，在現有的 skill 裡都沒有寫**。現有參考是廠商 API（`*-api.md`）與 1203 執行期陷阱；`galil-api.md` 的 `Open_GaliCard` 只是空殼範例。本檔就是補這一塊。
- `ht9045-motor-control` 的 SKILL.md「控制卡整合」表與 [hontech-m4-api.md](hontech-m4-api.md) 把 `Hontech_M4` 寫成「泓格 ICP-DAS M2X4」。實際上它包的是**先達** CMNet（`_mnet_m204_initial`、`G9004_M204 // SYNTEK-M204`），唯一的使用者是 `TMySYNTEKMotor`。
- [motor-classes.md](motor-classes.md) 的 `OpenPCI132Card` 只寫成「開啟 PCI-L132 卡片」。實際上它是 MN200、先達、PCIE-1203 三種卡的總入口。
- [exit-shutdown.md](../../ht9045-io-control/references/exit-shutdown.md) 檔頭說 SKILL.md 的 `ePCI1203`／`ePLCbase` 寫反了，但 main 上的 SKILL.md 現在已經是對的（3／4），這句警告過時了。
- 移植樹 `MyLaneIo.cpp SelectVendorBackends` 的說明寫「`INSTALL_ETHETCAT()` 不涵蓋 HT9050」。HT9050 快照是 `VacuUnitType=1`，所以照 golden 其實會開卡；正確的說法是「golden 沒有因為有 1203 的列而開卡的條件」。
- `ht9045-io-control` 的 SKILL.md 連到的 `ht9045-safe-plc-reer` skill，main 上沒有。
- 還沒有任何文件寫：MotionNet／MN200／SMC／Galil 在移植樹要真的接上時，該補什麼（SDK、x64、健康檢查的接點）。RECON 三份（`docs\RECON_MN200_PISO.md`、`RECON_GALIL_gclib.md`、`RECON_PCIE1203_CommonMotion.md`）只做了 SDK 評估。

---

## 8. 建議放的位置與指標行

- 本檔：`D:\HT9045\.claude\skills\ht9045-motor-control\references\card-init-and-health.md`。
- `ht9045-motor-control/SKILL.md` 的「詳細參考」最前面加一行：
  `- [card-init-and-health.md](references/card-init-and-health.md) - 各種軸卡／IO 卡的開卡流程與錯誤確認對照：先判斷用哪種技術（Gerneral.ini 卡別鍵、Mot_Table CardModel、IO_Table ISABase），再看開卡入口、開機順序、失敗告警碼、執行期健康檢查、恢復方式，以及 golden 與移植樹的差異（PCIE-1203／MotionNet 先達與 MN200／Galil／SMC／MC88X1／Tray 步進／安全 PLC）`
- `ht9045-io-control/SKILL.md` 的「詳細參考」加一行：
  `- [card-init-and-health.md](../ht9045-motor-control/references/card-init-and-health.md) - IO 卡（MotionNet 先達／MN200、PCIE-1203、安全 PLC、ISA／1735U）的開卡流程、24V／ring／EtherCAT 斷線檢查與 ResetMNet 恢復，跟馬達卡放在同一份對照`
- 兩支 SKILL.md 的 description 可加關鍵字：開卡, OpenPCI132Card, OpenEtherCatMastCard, CheckPCI_MN200State, CheckPCI_L112State, CheckPCI_EtherCatState, ResetMNet, Open_GaliCard, WAR1694, WAR1696, WAR16150, WAR16152, WAR16154。
- 同時修 §7.4 的兩處文件錯誤：Hontech_M4 的廠牌、exit-shutdown.md 過時的警告。
