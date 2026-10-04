---
name: ht9045-io-control
description: HT9045 Handler IO 控制層模式。適用於氣缸、感測器、開關、真空吸取器或底層 IO 操作。涵蓋 TMyCylinder、TMySensor、TMySwitch、TMySucker、TMyKitSuck、TLaneIO 及 myio 模組介面。用於 IO 修改、IO 問題除錯、新增氣缸/感測器/開關定義或理解 IO 控制流程。亦涵蓋 IO 資料庫設定：IO_Table.csv / Sensor_xxxx.DB 欄位定義、ISABase 對應值（eMotionNet 0 / eISABase 1 / ePCI1735U 2 / ePCI1203 3 / ePLCbase 4）、**IO 表 Enable 技巧**（以 Enable=0 停用訊號取代改程式加廠牌判斷、保留欄位供日後擴充、ISABase=4 會強制覆寫 Enable 的陷阱、全欄留空的防護鏈、停用前須查有無「檢查 Enable 就擋機」的防呆函式）、ePLCbase 的 Port 欄 16 進位解析與 InType 強制為 1。亦涵蓋 **IO 畫面（fiosetview）的 GUI thread 阻塞風險**：ShowModal 無 Close 鈕（僅 CC_Greatek 有）、50ms Timer1 直接做阻塞式 Modbus/TCP 讀 ADAM-6024（讀失敗就地整套重連、2000ms x3 逾時、失敗訊息走 MNetLog 不是 EventLog 故卡住卻零紀錄）、FormClose 丟例外會讓 VCL CloseModal 把 ModalResult 歸 0 導致視窗關不掉（「IO 畫面卡住退不出」）—— 2026-09-15 客戶錄影已證實本案屬後者（分頁可切、標題列無「沒有回應」、LED 持續重繪＝GUI thread 與 timer 均正常），且 AppException 的 process 級去重 static 會讓重複例外完全不進 EventLog。觸發關鍵字：IO 畫面卡住, IO 畫面退不出, fiosetview, Tfiosetview, FormClose, ShowModal, CloseModal, ADAM6024, ADAMTCP_Read6KAI, Open_ADAM_6024, EP_Install, labPA, IO 表, IO_Table.csv, Sensor DB, ISABase, ePLCbase, Enable=0, 停用感測器, 保留擴充, HexStrToInt, bPLCIO, 安全 PLC IO 設定。 另含 Exit（關閉程式）時的停機／關站（V906 移植樹 W906_Main_CloseProgramOp、ShutdownSequence、1203 輸出清零與讀回、未停清單、S121／S163／Q44）→ references/exit-shutdown.md。Use when：按 Exit 關不掉輸出、關站停機、SIM 建置接真卡、1203 頁打開的 DO 沒關、未停、必須先停下才能關閉、Ctrl-C 停機。關鍵字：Exit, 關閉程式, 關站, 停機, FormClose, sbCloseProgramClick, W906_Main_CloseProgramOp, ShutdownSequence, W906_ProdCloseShutdown, W906_ServeQuitDue, W906_ConsoleCtrl, Program Close, notStopped, 未停, SwHeaterRelay, SwMotorRelay, Pci1203RouteCanWriteBit, kCmdDoSetByte, Acm_DaqDoSetByteEx, 清零, S121, S163, R38, R39, R40, Q39, Q44。
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-io-control，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 IO 控制層

氣缸、感測器、開關及真空吸取模組的 Handler IO 控制模式。

## 相關技能與代理

- **技能**: `ht9045-safe-plc-reer` - 安全 PLC（Schneider / ReeR Semi S2）架構、暫存器對應、安全門/急停 IO 表建立
- **技能**: `bcb_build` - BCB6 編譯
- **技能**: `pre-release-check` - 程式碼風險掃描
- **AGENTS.md**: `D:\HT9045\AGENTS.md` - 專案概覽
- **AGENTS.md**: `D:\AGENTS.md` - 工作區慣例（Big5 編碼、僅支援 C++98）

## 快速參考

| 模組 | 類別 | 陣列 | 最大數量 | 標頭檔 |
|------|------|------|----------|--------|
| 氣缸 | `TMyCylinder` | `Cylinder[]` | `MaxCylinderItem` | `mycylin.h` |
| 感測器 | `TMySensor` | `Sen[]` | `MAX_SENSOR_ITEM` | `mysensor.h` |
| 開關 | `TMySwitch` | `SW[]` | `MAX_SWITCH_ITEM` | `myswitch.h` |
| 真空吸嘴 | `TMySucker` | （內嵌於 TMyKitSuck） | - | `mykitsuck.h` |
| 吸取組件 | `TMyKitSuck` | 全域: InArmSuck, FTestSuck 等 | `_MAX_SUCK_ROW_ITEM`x`_MAX_SUCK_COL_ITEM` 矩陣 | `mykitsuck.h` |
| 通道 IO | `TLaneIO` | `MyLaneIO`（單例） | - | `MyLaneIo.h` |
| 基本 IO | （函式） | - | - | `myio.h` |

## IO 基底類型 (`ISABase`)

> 定義位置：`MachineType.h` `enum eIOType`

```cpp
enum eIOType {
    eMotionNet  = 0,  // ADLINK MotionNet（預設，IO 表 ISABase 欄留空時亦為 0）
    eISABase    = 1,  // 傳統 ISA 卡
    ePCI1735U   = 2,  // Advantech PCI-1735U
    ePCI1203    = 3,  // Advantech PCI-1203（EtherCAT）
    ePLCbase    = 4   // 安全 PLC Modbus TCP（Schneider / ReeR）
};
```

⚠️ `ePCI1203=3` / `ePLCbase=4`，**兩者容易記反**。IO 表 `ISABase` 欄填錯會把安全 PLC 訊號導到 EtherCAT 卡（或反之），且不會有明確錯誤訊息。

## IO 表 Enable 技巧（停用與擴充保留）

**適用情境**：某訊號在特定機型／廠牌沒有實體 IO，但程式碼已有讀取點。
與其改程式加廠牌判斷（`IsSafePLCIOType_Schneider()` 之類），**優先在 IO 表把該筆停用** —— 零程式改動、零編譯風險、零回歸；日後實體 IO 接上時只要填回欄位即可啟用，且可**逐台機器獨立決定**。

實例：ReeR(SemiS2) 安全 PLC 無 `SnAllEMG` / `SnAllSafeDoor` 匯總實體 IO，Schneider 有。以 IO 表停用取代 8 處程式判斷。

### 正確填法：整列欄位全部留空，只填 `Enable=0`

`IO_Table.csv`（15 欄）：
```
Sensor,SnAllEMG,,,,,,,,0,,,,,
Sensor,SnAllSafeDoor,,,,,,,,0,,,,,
```
`Sensor_xxxx.DB`（8 欄）：`SensorName` 之外全部留空，`Enable` 填 `0`。

**`ISABase` 欄一定要留空**，理由見下。

### ⚠️ 陷阱：`ISABase=ePLCbase(4)` 會強制覆寫 `Enable=1`

`cinitial.cpp`（CSV 路徑與 .DB 路徑各一處）：

```cpp
if(IsSafePLCIOInstall() && Sen[i].ISABase==ePLCbase)
{
    bPLCIO[Sen[i].Port][Sen[i].Bit]=true;
    Sen[i].Type   = 1;      // InType 欄失效，一律不反相
    Sen[i].Enable = 1;      // ← Enable 欄失效，一律啟用
}
```

所以 **`ISABase=4` + `Enable=0` 無法停用**，而且後果比沒停用更糟：
- `Port` 欄若沒填對 → `bPLCIO[-1][-1]` 或 `bPLCIO[0][0]` 被寫入（越界／污染）
- `IsOff()` 會走到 `MyLaneIo.cpp` 範圍檢查失敗的 `return true` 分支 → **恆為「正常」的靜默失效**

### 為什麼「全部留空」安全（防護鏈）

```
CSV 欄位全空
  → database.cpp  iLane/iIP/iPort/iBit = -1, bHasNullData=true
  → database.cpp  iEnable 被強制 0            ← 誤填 1 也會被歸零（雙保險）
  → ISABase 空 → eMotionNet(0) ≠ ePLCbase     ← 不觸發上述強制啟用
  → cinitial.cpp  Sen[].Enable 維持 false，且不執行 SetUseIP()
  → mysensor.cpp  IsOn()/IsOff() 於 Enable==false 立即 return false
                  （根本不呼叫 IOInputBit，無越界風險）
  → iosetview.cpp ScanLed 呼叫 IOInputBit(-1,-1,-1,-1,…)
  → MyLaneIo.cpp  「if(Ring<=0 && IP<=0 && Port<=0 && Bit<=0) return false;」短路
                  ✅ 不碰 bPLCIO[-1][-1]，也不跳「Maybe input wrong IO position!」彈窗
```

.DB 路徑：空欄經 `atoi("")` 得 0，同樣被 `MyLaneIo.cpp` 的 `<=0` 短路擋下，一樣安全。

### 停用後的行為

| 面向 | 結果 |
|------|------|
| `Sen[x].IsOff()` / `IsOn()` | 恆回 `false`（fail-safe，不會誤報警） |
| 邏輯判斷 `if(旗標 && Sen[x].IsOff())` | 恆不成立 |
| SYSTEM 分頁 LED | `Visible=false`，直接隱藏（`iosetview.cpp` SetCompomentIO） |
| `bPLCIO[][]` | 不被寫入 |

### 日後啟用（擴充保留）

實體 IO 接上後，回頭把欄位填齊即可，不需改程式／重編：
```
Sensor,SnAllEMG,,,,0x405,5,1,4,1,,,,,
```
（`ePLCbase` 的 `Port` 欄為 **16 進位**，慣例帶 `0x` 前綴；`Bit` 為 10 進位 0~7；`Lane`／`IP`／`InType`／`Enable` 會被程式覆寫，填任意值皆可。）

### 停用前必查：有沒有「檢查 Enable 就擋機」的程式

有些防呆函式會在 `Sen[x].Enable==false` 時跳訊息並中止啟動。停用前務必 grep 該 Sensor 名稱，確認沒有這類檢查，或確認有逃生條件。

已知案例 —— `csystem.cpp` `bCheckPLCAllSafedoorAndEMGEnable()`：
```cpp
if(Sen[SnSafeMode].Enable)
    return true;                                    // ← 逃生門
if(Sen[SnAllSafeDoor].Enable==false && Sen[SnAllEMG].Enable==false)
{ ...ShowMyMessage("…未被啟用，請檢查database"); bResult=false; }
```
呼叫端收到 `false` 會 `SystemStart=false; StopAllMotor();`。
→ 停用 `SnAllSafeDoor`／`SnAllEMG` 的前提是 **`SnSafeMode` 必須有建列且 Enable 生效**。

### 檢查清單

1. `ISABase` 欄留空（**絕不可填 4**）
2. `Lane`／`IP`／`Port`／`Bit`／`InType` 全部留空
3. `Enable` 填 `0`
4. grep 該 Sensor 名稱，確認無「檢查 Enable 就擋機」的防呆函式
5. 保留列本身即是文件——同時在機台設定文件註明停用原因與日後啟用條件

> **V906 移植樹實際怎麼讀表**：哪一列決定 `Cylinder[].Enable`、缺列時物件長什麼樣、空欄位會讓整列強制停用、非數字會安靜變 0、`AUTO_EMPTY_COLOR` 會蓋掉 Empty／Color 氣缸、Mot_Table GearRatio 壞值變 0 → [references/table-loading-port.md](references/table-loading-port.md)（S-23 A1，2026-10-04）

## MotionNet 架構說明

HT9045 支援兩種 MotionNet 供應商的 Master 卡：

| 供應商 | 型號 | API 參考 |
|--------|------|----------|
| **SYN-TEK（先達）** | PCI-L132 / PCI-M114G | [motionnet-api.md](references/motionnet-api.md) |
| **ICP-DAS（泓格）** | PISO-MN200 | [motionnet-mn200-api.md](references/motionnet-mn200-api.md) |

### 系統限制（共通）

| 項目 | 限制 | 說明 |
|------|------|------|
| 每張卡片 Ring/Line 數 | **2** | Master 卡支援 2 個通訊線 |
| 每 Ring/Line 裝置數 | **64** | DeviceIP/DevNo: 0~63 |
| 每裝置 Port 數 | **4** | PortNo: 0~3 |
| 每 Port Bit 數 | **8** | Bit: 0~7 |
| 總線最大長度 | 100 公尺 | — |
| Terminator | 必須 | 最後一個 Slave 須設定 |

### 定址格式

```cpp
// TLaneIO 定址: Ring-IP-Port-Bit
MyLaneIO.IOBitOn(Ring, IP, Port, Bit, iISABase, "Alias");
//                0~1  0~63 0~3  0~7
```

### 常用 DIO Slave 模組

| 常數 | 值 | 說明 |
|------|----|------|
| `G9002_I16Q16` | 0xB2 | 16 IN / 16 OUT |
| `G9002_I32` | 0xB4 | 32 IN |
| `G9002_Q32` | 0xB0 | 32 OUT |
| `G9102_I8Q8` | 0xC4 | 8 IN / 8 OUT |

### 注意事項

1. **初始化順序**: 硬體初始化 → 開啟 MNET → 啟動 Ring → 初始化 Slave
2. **通訊錯誤**: 連續 3 次失敗會觸發錯誤旗標
3. **Terminator**: 最後一個 Slave 必須設定終端電阻

## EtherCAT PCI-1203 架構說明

> **詳細 API 請參考**: [ethercat-pci1203-api.md](references/ethercat-pci1203-api.md)

### 系統限制

| 項目 | 限制 | 說明 |
|------|------|------|
| 最大軸數 | **64** | AxisNo: 0~63 |
| 群組數 | **6** | 每組最多 8 軸插補 |
| 運動環循環 | 250μs | 最低循環時間 |
| IO 環循環 | 200μs | — |
| 最大從站數 | 65,535 | EtherCAT 標準 |

### 初始化流程

```cpp
// 1. 取得可用設備
Acm_GetAvailableDevs(DevList, MaxCount, &DevCount);

// 2. 開啟設備
Acm_DevOpen(DevNo, &DevHandle, 0);

// 3. DIO 操作
Acm_DaqDiGetByte(DevHandle, Channel, &DiData);
Acm_DaqDoSetByte(DevHandle, Channel, DoData);

// 4. 關閉設備
Acm_DevClose(&DevHandle);
```

### 與 TLaneIO 整合

當 `ISABase == ePCI1203` 時，`TLaneIO` 使用 Advantech Common Motion API：

| TLaneIO 方法 | PCI-1203 API |
|--------------|--------------|
| `IOBitOn()` | `Acm_DaqDoSetBit()` |
| `IOBitOff()` | `Acm_DaqDoSetBit()` |
| `IOInputBit()` | `Acm_DaqDiGetBit()` |
| `IOOutBitStatus()` | `Acm_DaqDoGetBit()` |

## 核心使用模式

### 氣缸 Push/Pop

```cpp
// 基本用法
Cylinder[idx].Push();  // 伸出，含感測器檢查與警報
Cylinder[idx].Pop();   // 縮回，含感測器檢查與警報

// 手動控制（無延遲/無警報）
Cylinder[idx].On();    // 直接輸出 ON
Cylinder[idx].Off();   // 直接輸出 OFF

// 狀態檢查
if (Cylinder[idx].OnSensor())  { /* 完全伸出 */ }
if (Cylinder[idx].OffSensor()) { /* 完全縮回 */ }
```

### 感測器讀取

```cpp
if (Sen[idx].Status()) { /* 已觸發 */ }
if (Sen[idx].IsOn())   { /* 同 Status() */ }
if (Sen[idx].IsOff())  { /* 未觸發 */ }
```

### 開關輸出

```cpp
SW[idx].On();          // 輸出 ON
SW[idx].Off();         // 輸出 OFF
if (SW[idx].Status()) { /* 目前為 ON */ }
```

### 真空吸取控制

```cpp
// 於 TMyKitSuck 內
Suck[row][col].Suck();     // 真空 ON + 破壞 OFF（阻塞式）
Suck[row][col].Destroy();  // 真空 OFF + 破壞 ON（阻塞式）
Suck[row][col].On();       // 快速 ON（不等待）
Suck[row][col].Off();      // 快速 OFF（不等待）
Suck[row][col].Normal();   // 全部關閉
```

### 底層 IO (TLaneIO)

```cpp
// MotionNet/PCI1203（Ring-IP-Port-Bit 定址）
MyLaneIO.IOBitOn(Ring, IP, Port, Bit, iISABase, "Alias");
MyLaneIO.IOBitOff(Ring, IP, Port, Bit, iISABase, "Alias");
bool val = MyLaneIO.IOInputBit(Ring, IP, Port, Bit, iISABase, "Alias");
bool out = MyLaneIO.IOOutBitStatus(Ring, IP, Port, Bit, iISABase, "Alias");
```

### 傳統 TTL IO (myio.cpp)

```cpp
// ISA/PCI port-bit 定址
IOBitOn(port, bit);
IOBitOff(port, bit);
bool val = IOInputBit(port, bit);
bool out = IOOutBitStatus(port, bit);
```

## 安全門檢查

所有 IO 函式執行前皆會呼叫 `IdleCheckSafeDoorByCylinder()`。若安全門開啟，IO 操作將被阻擋並提前返回。

## Kit IC 追蹤

`TMyKitSuck` 管理 IC 狀態矩陣 `Item[row][col]`：

| 數值 | 常數 | 意義 |
|------|------|------|
| 0 | `NULL_IC` | 空位 |
| 1 | `HAS_IC` | 有 IC |
| 2 | `HAS_NULL_IC` | 佔位符 |
| 3 | `HAS_HOT_IC` | 熱 IC |
| 100+ | `TEST_PASS+bin` | 測試結果 |

## Switch-Case 任務流程

### TMyCylinder::Push() / Pop() 任務流程

氣缸 Push/Pop 使用 `Task` 狀態機處理延遲：

```
Task 流程：
┌─────┐     感測器到位      ┌──────┐    無延遲     ┌─────────┐
│  1  │ ─────────────────▶ │ 100  │ ───────────▶ │ return  │
│起始 │                    │延遲前│              │  true   │
└─────┘                    └──────┘              └─────────┘
   │                          │ 有延遲
   │ 感測器未到位              ▼
   │                       ┌──────┐    延遲結束   ┌─────────┐
   ▼                       │ 101  │ ───────────▶ │ return  │
┌─────┐                    │等延遲│              │  true   │
│ 2   │ (重試)             └──────┘              └─────────┘
└─────┘
```

- **case 100**: 感測器確認到位，無延遲則直接完成；有延遲則啟動計時器
- **case 101**: 等待延遲計時器結束

### TMySucker::Suck() 任務流程

真空吸取使用 `OnTask` 狀態機：

**Dummy Run 模式** (`LastSet.iRealDummy==DUMMY`)：
```
OnTask 流程（Dummy）：
┌─────┐   設定計時器   ┌─────┐   計時器結束   ┌──────┐   延遲結束   ┌─────────┐
│  1  │ ─────────────▶ │  2  │ ─────────────▶ │  50  │ ───────────▶ │ return  │
│起始 │                │等待 │                │延遲中│              │  true   │
└─────┘                └─────┘                └──────┘              └─────────┘
```

**一般模式**：
```
OnTask 流程（一般）：
┌─────┐     真空感測器確認     ┌──────┐    無延遲     ┌─────────┐
│  1  │ ─────────────────────▶ │ 100  │ ───────────▶ │ return  │
│起始 │                        │記錄  │              │  true   │
└─────┘                        └──────┘              └─────────┘
   │                              │ 有延遲
   │ 逾時                         ▼
   ▼                           ┌──────┐   延遲結束   ┌─────────┐
┌──────┐                       │ 101  │ ───────────▶ │ return  │
│ 200  │ → return false        │等延遲│              │  true   │
│錯誤  │                       └──────┘              └─────────┘
└──────┘
```

- **case 1/2**: 啟動真空，開始偵測
- **case 100**: 記錄真空建立時間，無延遲則完成；有延遲則進入 101
- **case 101**: 等待 OnDelayTime 延遲
- **case 200**: 逾時錯誤狀態

### TMySucker::Destroy() 任務流程

真空破壞使用 `OffTask` 狀態機：

**Dummy Run 模式**：
```
OffTask 流程（Dummy）：
┌─────┐   設定計時器   ┌─────┐   計時器結束   ┌──────┐   延遲結束   ┌─────────┐
│  1  │ ─────────────▶ │  2  │ ─────────────▶ │  50  │ ───────────▶ │ return  │
│起始 │                │等待 │                │延遲中│              │  true   │
└─────┘                └─────┘                └──────┘              └─────────┘
```

**一般模式**：
```
OffTask 流程（一般）：
┌─────┐     真空感測器確認放開     ┌──────┐
│  1  │ ─────────────────────────▶ │ 100  │
│起始 │                            │記錄  │
└─────┘                            └──────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    │                 │                 │
                    ▼ 無延遲          ▼ 有延遲          ▼ 需重試
               ┌─────────┐       ┌──────┐          ┌──────┐
               │ return  │       │ 101  │          │ 300  │
               │  true   │       │等延遲│          │再破壞│
               └─────────┘       └──────┘          └──────┘
                                    │                  │
                                    ▼                  │
                               ┌─────────┐             │
                               │ return  │◀────────────┘
                               │  true   │  (DestroyAgainCount 次後)
                               └─────────┘
```

- **case 1/2**: 關閉真空，開啟破壞
- **case 100**: 記錄破壞時間；判斷是否需延遲或重試
- **case 101**: 等待 OffDelayTime 延遲
- **case 300**: DestroyAgain 重試破壞（針對難放開的 IC）

### TLaneIO::GetIOErrStr() 錯誤碼對照

```cpp
switch(iErr) {
    case 0: return "SUCCESS";
    case 1: return "Bit not between 0 to 7";         // Bit 範圍錯誤
    case 2: return "Ring, IP, or Port not in range"; // 位址範圍錯誤
    case 3: return "UseMNetIP Err";                  // MotionNet IP 錯誤
    case 4: return "16IN/OUT - output port error";   // 16IN/OUT 輸出錯誤
    case 5: return "16IN/OUT - input port error";    // 16IN/OUT 輸入錯誤
}
```

## ⚠ IO 畫面（`fiosetview`）的 GUI thread 阻塞風險

IO check and verify 畫面（`iosetview.cpp` / `Tfiosetview`）有三個結構性風險，客訴症狀是**「IO 畫面卡住退不出」**（2026-09-14 偉測 HHT-25）。

### 1. 它是 modal，而且多數客戶沒有 Close 按鈕

- `fiosetview->ShowModal()`（`main.cpp` L28855、`uteach.cpp` L2948）
- `btnClose->Visible=true` **只在 `CUSTOMER_CODE==CC_Greatek`**（`iosetview.cpp` L480-483）；其他客戶只能按標題列的系統 X（`BorderIcons=[biSystemMenu]`）
- modal 期間 VCL 會 disable 其他視窗 → **主畫面的 `sbStateRecord2` 等按鈕全部按不到**（出事時取證的第一個障礙）

### 2. 50ms Timer1 直接做阻塞式 Modbus/TCP（最容易卡死的一條）

`Tfiosetview::Timer1Timer`（L155）在 `EP_Install==3 || EP_Install==5` 時，每個 tick 呼叫：

| 行 | 呼叫 | 條件 |
|---|---|---|
| L205 / L215 | `ADAM_ReadPA(&dReadV, 0)` | 一定跑 |
| L221 | `ADAM_ReadVoltage(1)` | `USE_CKD_FCM_CleanAir` |
| L224 | `ADAM_ReadPA(&dReadV, 2)` | `INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL(0)` 或 `DOUBLE_EP_MULTI` ← **0 也會進來**，所以一般機台每 tick 打 **2 次** TCP |

往下是 `adam6024.cpp::ADAM_ReadVoltage` → `ADAMTCP_Read6KAI`，而 `iConnectionTimeout / iSendTimeout / iReceiveTimeout` 皆為 **2000ms**（`adam6024.cpp` L36-38）。
讀失敗時（原碼 L465-471）**就地** `Close_ADAM_6024(); Open_ADAM_6024();` 做整套重連，而 `Open_ADAM_6024` → `fCheckConnectStatus_ADAM6024` 內還有 **4 個 `ShowMyMessage`**（L2970/2978/2985/2995）。

⇒ ADAM 一有狀況，GUI thread 一次被卡數秒，X 按不動、畫面像死當。
⚠ 而且**卡住卻幾乎零紀錄**：`MyDBIProcess("Motion", "ADAMTCP_Open Fail!")` 走 **MNetLog 不是 EventLog**；`Open_ADAM_6024` 的 `iCount<=100` 之前更是完全靜默（`iCount` 初值 90，要連續失敗 11 次才會報）。

**修正（V3.33.912.1，2026-09-14）**：重連加冷卻 —— **首次失敗仍立即重連**（不延後正常復原），只有**連續**失敗才用 `TQPF_Timer` 節流成 5 秒一次；節流期間 `bADAM6420Install=false` 會讓 `ADAM_ReadVoltage` 在 L445 提前 `return 0`，不再阻塞 GUI thread。

### 3. `FormClose` 丟例外 → modal 視窗永遠關不掉

`Tfiosetview::FormClose`（L290）第一件事就是 `Close_ADAM_6024(); Open_ADAM_6024();`（關視窗卻去重連硬體）。

VCL 的坑：

```
TCustomForm::CloseModal()
  try    if CloseQuery then ... (觸發 OnClose)
  except ModalResult := 0;  HandleException;   ← 例外被吞，ModalResult 歸 0
```

⇒ **`FormClose` 只要丟例外，modal 就關不掉，而且每按一次 X 重演一次**；同時 `fShow=false` / `Timer1->Enabled=false` 等收尾全部被跳過（timer 繼續在關不掉的視窗上跑）。

**修正（V3.33.912.1）**：`FormClose` 內的硬體呼叫分段包 `try/catch`，確保狀態收尾一定執行。

### ★ 影片實證（2026-09-15 偉測 HHT-25）：卡的是「關不掉」，不是「卡住」

客戶 12 秒手機錄影把三個機制分了勝負：

| 觀察 | 判讀 |
|---|---|
| 分頁在 12 秒內切了 **Shuttle → Stack 2 → Vacuum → Shuttle**，每次都即時反應 | GUI thread 與訊息迴圈**完全正常** |
| 標題列正常藍色，**沒有**「(沒有回應)」 | Windows 認定 app 有在抽訊息 |
| LED / 分頁內容每幀正常繪製 | `Timer1Timer` 每 tick **跑得完**（`ScanLed` 有執行）|
| 游標停在右上紅 X 反覆點，視窗紋風不動 | 只有 **關閉** 這條路壞掉 |

⇒ **上面第 2 點（50ms timer 阻塞 Modbus/TCP）在本案被排除**——timer 沒有被卡住。
⇒ **頭號機制是第 3 點：`FormClose` 丟例外 → VCL `CloseModal` 把 `ModalResult` 歸 0。**
症狀特徵完全吻合：程式全活、只有 X 無效、**每按一次重演一次**。

> ⚠ 第 2 點仍是真實缺陷（修法保留），只是**不是這一案的根因**。判別法：
> 阻塞型 → 畫面遲鈍、分頁切換要等、標題列可能出現「沒有回應」；
> `CloseModal` 型 → **一切正常，唯獨關不掉**。

#### 為什麼 EventLog 看不到那個例外（重要陷阱）

`TfMain::AppException`（`main.cpp` L27641~）的去重用的是 **process 生命週期的 static**：

```cpp
static AnsiString sLastMsg="";
static int iSameCount=0;
if(sMsg==sLastMsg) { iSameCount++; if(iSameCount==3 || (iSameCount%1000)==0) RecordProcess(...); return; }
```

⇒ 只要**同一句例外訊息在這個 process 裡先前已經記過**，之後第 4~999 次**完全靜默**。
操作員按個十幾次 X，一筆都不會出現。
**因此「該時段沒有 Exception row」不能推論「沒有例外」** —— 要往前翻**整天**的 EventLog 找該訊息的**第一次**出現（那一次一定會記，且訊息內容會指出是哪個呼叫爆掉）。

#### 定位根因要拿的東西

1. 出事當天**整天**的 `EventLogTxt`，grep UnitName=`Exception` 的所有列（找第一次出現＋`@MsgBoxStep=` 欄位）
2. 若還是找不到 → 卡住當下做 **process dump**（工作管理員 → 右鍵 `HT9045.exe` → 建立傾印檔案），看主執行緒是否停在 `ShowModal` 迴圈、以及 `FormClose` 內哪一個呼叫拋出

### 判讀提示

- 症狀「IO 畫面退不出」而 EventLog 零紀錄 → 優先查 ADAM-6024（`172.16.8.110`，Modbus/TCP port **502**）的網路與連線數（模組上限 8 條，超過要 `ClearAllConnection`）。
- 截圖上 Index 分頁的 `V:` / `PA:` 就是 `labEPValue` / `labPA`；值異常（例如 `PA: -3`）可佐證 ADAM 讀值有問題。
- 相關：`ht9045-staterecord-analysis` **LL-24**（彈窗與 Button log 的留痕規則、GUI thread 卡住時如何取證、為什麼不能叫客戶「先按 State Record」）。

## 詳細參考

類別成員細節與配置模式請參閱：
- [io-classes.md](references/io-classes.md) - 完整類別成員參考
- [motionnet-api.md](references/motionnet-api.md) - MotionNet API 參考（SYN-TEK / 先達）
- [motionnet-mn200-api.md](references/motionnet-mn200-api.md) - MotionNet API 參考（ICP-DAS / 泓格 PISO-MN200）
- [ethercat-pci1203-api.md](references/ethercat-pci1203-api.md) - EtherCAT PCI-1203 API 參考
- [io-alias-map-doc.md](references/io-alias-map-doc.md) - IO 畫面 Alias 對照文件生成（iosetview.dfm → 標註 HTML；含四層座標校正機制 PAGE_ADJ / PAGE_NO_ADJ / COMP_ADJ 與微調工作流程）
- [exit-shutdown.md](references/exit-shutdown.md) - V906 移植樹按 Exit 時的停機（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp`）：現在的三步流程、SIM 建置接真卡為什麼關不掉 1203 頁打開的 DO、出貨建置漏的 54 個輸出、固定「未停」的項目、1203 命令面清零的做法與陷阱（先過 `Pci1203RouteCanWriteBit`、ring 0／驅動器站、只能 tick 執行緒）、S121／S163／R38～R41／Q39 與待 Steven 選的 Q44。⚠ 上面「IO 基底類型」表的 `ePLCbase`／`ePCI1203` 值寫反了，正確值見該檔檔頭
- [cylinder-sensor-layer.md](references/cylinder-sensor-layer.md) - 缺列／Enable 0 時呼叫端拿到什麼：Push／Pop 不看 Enable、感測器 IsOn 與 IsOff 都回 false、golden 開機強制開啟安全門與蓋掉表的氣缸、C3／C4 規則建議（S-23，20261004）

## 工具腳本

- [gen_io_alias_doc.py](scripts/gen_io_alias_doc.py) - 解析 iosetview.dfm，在 `D:\HT9045\IMG\IO\` 截圖上標註全部 TMyLedLane/TBtnPanelLane 的 Alias，輸出自含式 HTML 至 `D:\docs\manual\`。執行：`py gen_io_alias_doc.py`。腳本內建各頁人工校正值（2026-07-03 已全頁校正），修改校正值時須與現值累加。

---

## 合併補充：repo 既有參考（20261001）

- [references/colleague-skill-body-20260915.md](references/colleague-skill-body-20260915.md)：repo 版 SKILL.md 正文原樣保存（合併前）
