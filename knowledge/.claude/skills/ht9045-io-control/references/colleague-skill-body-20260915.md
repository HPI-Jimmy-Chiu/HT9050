# repo 版 SKILL.md 正文（合併前原樣保留）

> 📥 20261001 與 RogerYang 版合併時，主 SKILL.md 以 RogerYang 版為底；repo 版中「同一內容不同寫法」的段落以 RogerYang 寫法為準，
> 為避免遺失任何字句，repo 版 SKILL.md 正文整份原樣保存在此。


<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-io-control，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 IO 控制層

氣缸、感測器、開關及真空吸取模組的 Handler IO 控制模式。

## 相關技能與代理

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

```cpp
enum {
    eMotionNet  = 0,  // ADLINK MotionNet（預設）
    eISABase    = 1,  // 傳統 ISA 卡
    ePCI1735U   = 2,  // Advantech PCI-1735U
    ePCI1203    = 3,  // Advantech PCI-1203
    ePLCbase    = 4   // PLC Modbus TCP
    // 20260927 更正（ST01-E）：原表把 3／4 寫反；golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\MachineType.h:781-782、移植樹 :841-842、V899 :751-752 都是 ePCI1203=3、ePLCbase=4
};
```

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
┌─────┐     感測器到位       ┌──────┐    無延遲     ┌─────────┐
│  1  │ ─────────────────▶ │ 100  │ ───────────▶ │ return  │
│起始 │                     │延遲前│               │  true   │
└─────┘                     └──────┘              └─────────┘
   │                          │ 有延遲
   │ 感測器未到位              ▼
   │                       ┌──────┐    延遲結束   ┌─────────┐
   ▼                       │ 101  │ ───────────▶ │ return  │
┌─────┐                    │等延遲│               │  true   │
│ 2   │ (重試)             └──────┘               └─────────┘
└─────┘
```

- **case 100**: 感測器確認到位，無延遲則直接完成；有延遲則啟動計時器
- **case 101**: 等待延遲計時器結束

### TMySucker::Suck() 任務流程

真空吸取使用 `OnTask` 狀態機：

**Dummy Run 模式** (`LastSet.iRealDummy==DUMMY`)：
```
OnTask 流程（Dummy）：
┌─────┐   設定計時器    ┌─────┐   計時器結 束    ┌──────┐   延遲結束    ┌─────────┐
│  1  │ ─────────────▶ │  2  │ ─────────────▶ │  50  │ ───────────▶ │ return  │
│起始 │                 │等待 │                 │延遲中 │              │  true   │
└─────┘                └─────┘                 └──────┘               └─────────┘
```

**一般模式**：
```
OnTask 流程（一般）：
┌─────┐     真空感測器確認       ┌──────┐    無延遲     ┌─────────┐
│  1  │ ─────────────────────▶ │ 100  │ ───────────▶ │ return  │
│起始 │                         │記錄  │               │  true   │
└─────┘                         └──────┘              └─────────┘
   │                              │ 有延遲
   │ 逾時                         ▼
   ▼                           ┌──────┐   延遲結束    ┌─────────┐
┌──────┐                       │ 101  │ ───────────▶ │ return  │
│ 200  │ → return false        │等延遲│               │  true   │
│錯誤  │                       └──────┘               └─────────┘
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
┌─────┐   設定計時器    ┌─────┐   計時器結 束    ┌──────┐   延遲結束    ┌─────────┐
│  1  │ ─────────────▶ │  2  │ ─────────────▶ │  50  │ ───────────▶ │ return  │
│起始 │                 │等待 │                 │延遲中│               │  true   │
└─────┘                └─────┘                 └──────┘               └─────────┘
```

**一般模式**：
```
OffTask 流程（一般）：
┌─────┐     真空感測器確認放開       ┌──────┐
│  1  │ ─────────────────────────▶ │ 100  │
│起始 │                             │記錄  │
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

## 詳細參考

類別成員細節與配置模式請參閱：
- [io-classes.md](references/io-classes.md) - 完整類別成員參考
- [motionnet-api.md](references/motionnet-api.md) - MotionNet API 參考（SYN-TEK / 先達）
- [motionnet-mn200-api.md](references/motionnet-mn200-api.md) - MotionNet API 參考（ICP-DAS / 泓格 PISO-MN200）
- [ethercat-pci1203-api.md](references/ethercat-pci1203-api.md) - EtherCAT PCI-1203 API 參考
- [io-alias-map-doc.md](references/io-alias-map-doc.md) - IO 畫面 Alias 對照文件生成（iosetview.dfm → 標註 HTML；含四層座標校正機制 PAGE_ADJ / PAGE_NO_ADJ / COMP_ADJ 與微調工作流程）
- [exit-shutdown.md](references/exit-shutdown.md) - V906 移植樹按 Exit 時的停機（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\MainClose.cpp`）：現在的三步流程、SIM 建置接真卡為什麼關不掉 1203 頁打開的 DO、出貨建置漏的 54 個輸出、固定「未停」的項目、1203 命令面清零的做法與陷阱（先過 `Pci1203RouteCanWriteBit`、ring 0／驅動器站、只能 tick 執行緒）、S121／S163／R38～R41／Q39 與待 Steven 選的 Q44。⚠ 上面「IO 基底類型」表的 `ePLCbase`／`ePCI1203` 值寫反了，正確值見該檔檔頭

## 工具腳本

- [gen_io_alias_doc.py](scripts/gen_io_alias_doc.py) - 解析 iosetview.dfm，在 `D:\HT9045\IMG\IO\` 截圖上標註全部 TMyLedLane/TBtnPanelLane 的 Alias，輸出自含式 HTML 至 `D:\docs\manual\`。執行：`py gen_io_alias_doc.py`。腳本內建各頁人工校正值（2026-07-03 已全頁校正），修改校正值時須與現值累加。
