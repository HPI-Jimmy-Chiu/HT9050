# HT9050 溫控器對照表：DTM 站號/CH ↔ `eTempControll`

來源：`HP-9050開發機資料-20260717.xlsx` 分頁 `04_溫控器站號` ＋ `03_加熱與感溫`。
程式端對照：`MachineType.h` `enum eTempControll`（V906 與 V912 **數值完全相同**，
只差 V912 是 Big5、V906 是 UTF-8）。

---

## 1. 硬體給的：3 站 24 通道

全部台達、全部 Ethernet。

| 站號 | 型號 | CH | 溫區名稱 | 控制對象 | Sensor | 備註 |
|:----:|------|:--:|----------|:--------:|:------:|------|
| 1 | DTME08 | 1 | Hotplate 1 | SSR | PT100 | |
| 1 | DTME08 | 2 | Hotplate 2 | SSR | PT100 | |
| 1 | DTME08 | 3 | In Shuttle 1 | SSR | PT100 | |
| 1 | DTME08 | 4 | In Shuttle 2 | SSR | PT100 | |
| 1 | DTME08 | 5 | DUT 1 | SSR | PT100 | |
| 1 | DTME08 | 6 | DUT 2 | SSR | PT100 | |
| 1 | DTME08 | 7 | DUT 3 | SSR | PT100 | |
| 1 | DTME08 | 8 | DUT 4 | SSR | PT100 | |
| 2 | DTMN08 | 1 | Chamber | SCR | PT100 | 葉片式加熱棒 |
| 2 | DTMN08 | 2 | Hot Air 1 | SCR | K-Type | 熱風槍 |
| 2 | DTMN08 | 3 | Hot Air 2 | SCR | K-Type | 熱風槍 |
| 2 | DTMN08 | 4–8 | （空） | | | 保留 |
| 3 | DTME08 | 1 | SLK-1 | SSR | PT100 | Direct Heater 1 |
| 3 | DTME08 | 2 | SLK-2 | SSR | PT100 | Direct Heater 2 |
| 3 | DTME08 | 3 | SLK-3 | SSR | PT100 | Direct Heater 3 |
| 3 | DTME08 | 4 | SLK-4 | SSR | PT100 | Direct Heater 4 |
| 3 | DTME08 | 5 | SLK-5 | SSR | PT100 | Direct Heater 5 |
| 3 | DTME08 | 6 | SLK-6 | SSR | PT100 | Direct Heater 6 |
| 3 | DTME08 | 7 | SLK-7 | SSR | PT100 | Direct Heater 7 |
| 3 | DTME08 | 8 | SLK-8 | SSR | PT100 | Direct Heater 8 |

加熱元件規格（分頁 `03_加熱與感溫`）：

| 溫區 | 加熱元件 | 品號 | 電壓/功率 | 過溫保險絲 |
|------|----------|------|-----------|-----------|
| Hotplate 1/2 | 雲母加熱片（11×Ø6.5 孔 170×365mm） | P030030661 | 220V/1600W | THERMIK S06.200.05.100 |
| In Shuttle 1/2 | 同上 | P030030661 | 220V/1600W | THERMIK S06.200.05.100 |
| DUT 1–4 | 加熱棒 HEATER 6×180L | P030030121 | 230V/250W | THERMIK S06.200.05.100 |
| Chamber | 葉片式加熱棒 MAHU2 | P030030052 | 200V/1000W | 無（表上空白） |
| Hot Air 1/2 | 熱風槍（自帶 K-Type） | P030030117 | 220V/430W | 溫度開關 UP62 100℃ |
| SLK-1–8 | 加熱片 | （表上未填品號） | 220V/250W | （表上未填） |

PT100 感溫線一律 `E040110044`（MHTSPT100-3.2Φ-60L…），Chamber 用 `P030040018`（HTSPT425M47F1）。

---

## 2. 對照表：DTM 通道 → `eTempControll`

`iCh` 是 DTM 的**全域通道索引**（0-based），程式裡就是這樣拆站號的
（`fDTME08.cpp` `GetStationAndNumber`，golden `EJ1N/fDTME08.cpp:266-270`）：

```cpp
iStation = iCh / 8;   // 0-based；Modbus 起始位址 = iStation * 0x1000（GetStationCode）
iNumber  = iCh % 8;   // 0-based
```

| iCh | 站號 | CH | 溫區 | `eTempControll` | 值 | 依據 |
|----:|:----:|:--:|------|-----------------|---:|------|
| 0 | 1 | 1 | Hotplate 1 | `tcHotPlate1` | 0 | 名稱直對 |
| 1 | 1 | 2 | Hotplate 2 | `tcHotPlate2` | 1 | 名稱直對 |
| 2 | 1 | 3 | In Shuttle 1 | `tcShuttle1` | 2 | 名稱直對 |
| 3 | 1 | 4 | In Shuttle 2 | `tcShuttle2` | 3 | 名稱直對 |
| 4 | 1 | 5 | DUT 1 | `tcDUT1` | 29 | 名稱直對 |
| 5 | 1 | 6 | DUT 2 | `tcDUT2` | 30 | 名稱直對 |
| 6 | 1 | 7 | DUT 3 | `tcDUT3` | 31 | 名稱直對 |
| 7 | 1 | 8 | DUT 4 | `tcDUT4` | 32 | 名稱直對 |
| 8 | 2 | 1 | Chamber | `tcChamber` | 9 | 名稱直對 |
| 9 | 2 | 2 | Hot Air 1 | `tcHeatGun1` | 27 | 熱風槍＝HeatGun |
| 10 | 2 | 3 | Hot Air 2 | `tcHeatGun2` | 28 | 熱風槍＝HeatGun |
| 11–15 | 2 | 4–8 | （空） | — | — | 保留，不註冊 |
| 16 | 3 | 1 | SLK-1 | `tcAa1` | 11 | ⚠ 推導，見 §3 |
| 17 | 3 | 2 | SLK-2 | `tcAb1` | 12 | ⚠ 推導 |
| 18 | 3 | 3 | SLK-3 | `tcAc1` | 13 | ⚠ 推導 |
| 19 | 3 | 4 | SLK-4 | `tcAd1` | 14 | ⚠ 推導 |
| 20 | 3 | 5 | SLK-5 | `tcBa1` | 15 | ⚠ 推導 |
| 21 | 3 | 6 | SLK-6 | `tcBb1` | 16 | ⚠ 推導 |
| 22 | 3 | 7 | SLK-7 | `tcBc1` | 17 | ⚠ 推導 |
| 23 | 3 | 8 | SLK-8 | `tcBd1` | 18 | ⚠ 推導 |

機器可讀版本：[../data/HT9050-TempMap.json](../data/HT9050-TempMap.json)。

---

## 3. SLK-1~8 的映射是推導值，要人確認

`tcAa1 + (n-1)` 這個答案是從 `Command.cpp:568` 推的——那裡是全樹唯一一處
把「Index 的 row/col」換算成溫控代號的地方：

```cpp
iTempKit0_Arm1 = tcAa1 + (i*4 + j);   // i = index row, j = index col(0..3)
```

也就是 16 加熱器區塊是 **4 行寬、row-major**：

```
row0 →  tcAa1(11)  tcAb1(12)  tcAc1(13)  tcAd1(14)
row1 →  tcBa1(15)  tcBb1(16)  tcBc1(17)  tcBd1(18)
```

所以 **若 SLK-n 的 n 就是 2×4 index 的 row-major site 編號**，n=1..8 剛好落在
`tcAa1 + (n-1)` = 11..18。

⚠ **要確認的是 site 編號方向**：若 HT9050 的 SLK-1..8 是「先走前後排、再走列」
（也就是 SLK-1=Aa、SLK-2=Ba、SLK-3=Ab…，跟 HT9045 `iTempCode[]` 的接線順序一樣），
那對照表的後 8 列要改成 11,15,12,16,13,17,14,18。
**兩種排法都會正常開機、畫面都看起來正常，錯的只有「哪一顆 socket 在加熱」**——
所以這件事不能靠猜，要對 HT9050 的 site map／Binasgn 或直接問硬體。

`SLK` 在 HT9045 程式裡目前只有夾持相關常數（`C_SLK1_Clamp`、`SnArm1SLK`…），
**沒有任何 SLK 專屬的溫控列舉**。要不要新增 `tcSLK1..8` 是另一個決策：
新增就得動 `eTempControll`（`tcTotalCount` 目前 71），而 enum 註解自己寫著
「有增加請搜尋：溫控器要一起改」。

---

## 4. 為什麼不能直接沿用 `iTempCode[]`

`cmydef.cpp:111`：

```cpp
int iTempCode[INDEX_HEAT_COUNT] =          // INDEX_HEAT_COUNT = 32 (MachineType.h:14)
{
    tcAa1, tcBa1, tcAb1, tcBb1, tcAc1, tcBc1, tcAd1, tcBd1,   // 站1 CH1-8
    tcAa2, tcBa2, tcAb2, tcBb2, tcAc2, tcBc2, tcAd2, tcBd2,   // 站2 CH1-8
    tcAe1, tcBe1, tcAf1, tcBf1, tcAg1, tcBg1, tcAh1, tcBh1,   // 站3 CH1-8
    tcAe2, tcBe2, tcAf2, tcBf2, tcAg2, tcBg2, tcAh2, tcBh2,   // 站4 CH1-8
};
```

三個結構性差異：

| | HT9045（`iTempCode`） | HT9050（本表） |
|---|---|---|
| 涵蓋範圍 | **只有 Index 的 32 組加熱器** | Hotplate／Shuttle／DUT／Chamber／Hot Air ＋ 8 組 Index 直熱 |
| 通道數 | 32（4 站滿載） | 24（3 站，站2 只用 3 CH） |
| 排列 | 前後排交錯 Aa1,Ba1,Ab1,Bb1… | 站1/2 依功能，站3 連續 SLK-1..8 |

HT9045 上 Hotplate／Shuttle／DUT／Chamber／熱風槍**不走 DTM**——它們在
`DoThermo()`（`bthermo.cpp:1338` 起）那條序列（UN150／Com）迴圈裡，
`Addr` 從 `tcHotPlate1` 往上跑；DTM 只負責 `DoSetSVOfDTME08()`
（`bthermo.cpp:5302`）那條 `for(i=0; i<INDEX_HEAT_COUNT; i++)` 的 Index 加熱器迴圈。

**HT9050 把兩邊都搬到 DTM 上了**，所以需要的不是「改 `iTempCode` 的內容」，
而是一張新的、含站號的通道表，並且要決定 `DoThermo()` 那條序列路徑在 HT9050 要不要停用。

---

## 5. 程式端現況（動手前先確認）

| 元件 | 位置 | 狀態 |
|------|------|------|
| `enum eTempControll` | `MachineType.h:638-655`（V912）／同名於 V906 | ✅ 兩棵樹數值一致 |
| `enum eHeaterType` | `MachineType.h:777-783` | `eht16HeaterDTME08=5`、`eht32HeaterDTME08=6`；**沒有 HT9050 的 24 通道型別** |
| `USE_16_HEATER` | `Gerneral.ini:19`，目前 `=2`（`eht16HeaterEJ1N`） | 換 HT9050 要改這個值 |
| `iTempCode[]` | `cmydef.cpp:111` | 32 筆、Index-only，見 §4 |
| `DoSetSVOfDTME08()` | `bthermo.cpp:5302` | 已翻譯，但寫 SV 的兩行在 `#if 0`（GATE W7-UI G29，`frmDTME08` 未移植） |
| `uDTME08Control` | `EJ1N/uDTME08Control.{h,cpp}` | 已翻譯；`GetMaxStationNumber()=4`、`GetChannelNumberPerStation()=8`（HT9050 只用 3 站，容量夠） |
| `GetStationCode()` | `EJ1N/uDTME08Control.cpp:127` | `station * 0x1000`，Modbus 起始位址 |
| `TfrmDTME08` UI | `forms/fDTME08.{h,cpp}` | 大量 `#if 0`；`GetStationAndNumber` 本身也是 GATE (E-18) |
| JSON 通道表 | `JsonBridge/gen/thermo_channels.gen.cpp` | 由 `tools/gen_tempchan.py` 從 enum 產生，71 通道；**目前沒有站號/CH 欄位** |

⚠ `DoSetSVOfDTME08()` 目前**寫不出 SV**：`frmDTME08->SetSettingSV()` 與
`GetPalGroup(i)->GetPV()` 兩行都在 `#if 0` 裡。要讓 HT9050 真的加熱，
這個 gate 得先解，或改走不經 UI 的路徑。
