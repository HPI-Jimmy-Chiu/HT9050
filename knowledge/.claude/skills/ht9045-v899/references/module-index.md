# V3.33.899.0 子目錄模組索引

## 子目錄清單 (34 個)

| 目錄 | 檔案數 | 用途 |
|------|--------|------|
| `ARMS/` | 6 | 機械臂 UI 與 Log |
| `ATC/` | 15 | ATC 系統主控 (ATCInterface, ATC_Handler_Side, ATC_WinWay) |
| `Motor/` | 34 | 馬達驅動 (Hontech, MN200, SMC, Syntek, GALIL, EtherCAT) |
| `TempCtrl/` | 24 | 溫控 (DT4848, KT4H, TMC401, UT100, WT404, TriTemp) |
| `Automation/` | 33 | 自動化流程 (AGV, AMR, MES, HANA_ART, SCK) |
| `SECSGEM/` | 16 | SECS/GEM 通訊 (uHGemClass, uHGemHT9045) |
| `CanBus/` | 6 | CAN Bus (DNM100, NUDN1) |
| `EtherCAT/` | 10 | EtherCAT (AdvMot API, MyEtherCAT, MyNUEC1) |
| `CCLink/` | 14 | CC-Link (MyCCLink, MyCCLinkBoard) |
| `Public/` | 23 | 共用工具 (cJSON, MD5, HTEdit, MemoryAlarm, KeyPro) |
| `Interface/` | 3 | 測試介面 (InterfaceSYS, TesterTCP) |
| `MyPLC/` | 4 | PLC Modbus TCP |
| `Monitor/` | 5 | 系統監控 UI |
| `BarCode/` | 9 | 條碼讀取 |
| `Mes/` | 3 | MES 介面 (VAT) |
| `AutoClean/` | 7 | 自動清潔 |
| `AutoAlignment/` | 11 | 自動對位 (CCD Vision) |
| `HThreadCtrl/` | 2 | 執行緒控制 |
| `PMAlarm/` | 12 | 預防維護報警 |
| `ProductionInfo/` | 12 | 生產資訊/FTP/ESD/PAT |
| `SortingBinTray/` | 2 | 分 Bin 邏輯 |
| `LoadCCD/` | 3 | CCD Map 載入 |
| `BinDisplay/` | 7 | Bin 顯示 UI |
| `RotateKit/` | 9 | IC 旋轉模組 |
| `VacuumUnit/` | 5 | 真空單元 |
| `EJ1N/` | 18 | Omron EJ1N 溫控器通訊 |
| `GroundMan/` | 3 | 接地監測 |
| `OmronLaser/` | 7 | Laser 感測器 |
| `KYECFTP/` | 3 | KYEC FTP 客戶端 |
| `MR/` | 7 | Magazine/RFID/Cassette |
| `ASE_K Socket/` | 6 | ASE-K Socket 特殊介面 |
| `AutoTeach/` | — | 自動教導 (runtime 資料) |
| `search-division/` | 6 | 除法安全掃描結果 |

## 根目錄核心檔案分類

### 定義檔 (Definitions)
- `MachineDefine.h` — 全域 include 彙整
- `MachineType.h` — 機台型號、客戶代碼、編譯旗標
- `cprod.h/cpp` — 生產參數結構
- `cpublic.h/cpp` — 公用函式
- `cmydef.h/cpp` — 自定義常數與巨集

### 系統核心
- `ckernel.cpp/h` — 主控排程
- `cinitial.cpp/h` — 系統初始化
- `main.cpp/h` — 主 UI
- `uruncontrol.cpp/h` — 執行緒控制
- `HT9045.cpp` — Application 入口

### InArm 系統
- `ainarm9045.cpp/h` — InArm 主分派
- `ainarm9045_*.cpp/h` — 各 site 配置子檔 (27 種)
- `ainarm_SearchPickPlate.cpp/h` — HotPlate 取料搜尋
- `ainarm_SearchPlacePlate.cpp/h` — HotPlate 放料搜尋
- `ainarm2.cpp/h` — InArm 輔助

### OutArm 系統
- `aoutarm9045.cpp/h` — OutArm 主分派
- `aoutarm9045_*.cpp/h` — 各 site 配置子檔

### 測試模組
- `atester.cpp/h` — Index 測試主控
- `atester_32Site.cpp/h` — 32-site 測試
- `aTester_Front.cpp/h` — 前臂測試
- `aTester_Rear.cpp/h` — 後臂測試

### Shuttle
- `uShuttleThread.cpp/h` — Shuttle 執行緒
- `ShuttleMove.cpp/h` — Shuttle 移動 UI

### Tray 系統
- `acatchtray.cpp/h` — CatchTray 狀態機
- `mytray.cpp/h` — Tray 基本操作
- `cTrayMapping.cpp/h` — Tray Mapping

### 硬體控制
- `myio.cpp/h` — IO 底層
- `mysensor.cpp/h` — 感測器
- `myswitch.cpp/h` — 開關
- `mycylin.cpp/h` — 氣缸
- `mykitsuck.cpp/h` — 真空吸嘴
- `MyLaneIo.cpp/h` — Lane IO
