# 評估：HT9050 的 Index Z 扭力上限改走 1203（EtherCAT SDO），不走 RS232

> 使用者 20260925 16:3x：「IndexZ扭力原本是透過RS232來通訊，如果現況有1203，是否只要透過1203就能獲取訊息? 如果技術上可行，開始評估9050都是透過1203來獲取，你用machine type=9050來區隔功能使用」（週末排最後）。
> 本檔是評估，**沒有改程式**。量測：筆電 main `ddae528d`、golden 906（cp950 讀）、`machines/HT9050/`。

## 結論：**有條件可行**

- 技術上可行：HT9050 的 Index Z 是 1203 EtherCAT 軸，驅動器是 SERVOPACK；EastSun 的 `Pci1203Control` 已經有 SDO 寫入／讀回的基礎建設。
- 條件（都在 EastSun 的模組或機台上）：① SDO 白名單加扭力限制物件（EastSun 決定，RULINGS 第 42 條他可以直接決定）；② 確認驅動器型號與單位；③ 上機量一次。
- 目前的狀態（第 38 條照 golden）：HT9050 **出貨組態**跑 Index 自動流程會在 RS232 重試後報「Motor torque set error」停下 —— 這條路做完就解掉。

## 事實（附出處）

| 項目 | 內容 | 出處 |
|---|---|---|
| golden 寫什麼 | Panasonic A5：封包 `06 00 17 00 0D lo hi 00 00 sum` ＝ 參數寫入、群 0、編號 0x0D ＝ **Pr0.13 第一扭力限制（額定扭力 %）**；舊型（非 A5）寫 0x5E | golden `rs232.cpp` `TCOM2::WriteIndexTorqueSetting_Pana`（:1847 附近的 iWriteAndCheckMotorTorque 呼叫它） |
| 什麼時候寫 | 接觸流程 `DoTestHeadMotor` case 120／12000 寫 `Prod.iMaxPreasure`；歸零 case 320／322 寫 **300**（只在 `SW[SwReadTorue].Enable` 的機台）；寫完讀回比對，不符重試 5 次回 2 | golden `atester.cpp:6126`／`:6757`、`uhome.cpp` 320/322；移植樹 `rs232.cpp`（c55e2954） |
| 讀扭力回饋 | `ReadTorque_Panasonic` → `edTorue0/1`（主畫面扭力欄、case 12110 用） | golden `rs232.cpp:814-820` |
| HT9050 的 Index Z | `MTestZ1`＝M14，**CardModel=PCI1203**、BoardID 14、Enable=1；`MTestZ2`＝M15 SMC、**Enable=0**（單 Z） | `machines/HT9050/Mot_Table.csv:16-17` |
| 驅動器 | SERVOPACK（Yaskawa），EastSun MT-E1 以「DS402／SERVOPACK」分支 Home | 機台端 MT-E1（INBOX 第 28 列） |
| 1203 的 SDO | `Pci1203Control` 用 `Acm_DevWriteSDOData`／`Acm_DevReadSDOData`，**逐物件白名單**：2710h（Fn008 絕對值編碼器重置）、齒輪比、Pn000／Pn21D／Pn50A-B 的讀改寫；檔頭寫明「an SDO write is not a reviewable capability —— 要逐物件列」 | `EtherCAT/Pci1203Control.h:76-206`、`:317-387` |

## 對照（建議）

| golden（RS232 Panasonic） | 1203（SERVOPACK，CiA402） | 說明 |
|---|---|---|
| Pr0.13 第一扭力限制，單位 % | **0x60E0 正扭力限制＋0x60E1 負扭力限制**，單位 **0.1%** | 寫入值＝`iMaxPreasure × 10`；歸零的 300% ⇒ 3000。Yaskawa 另有 Pn402／Pn403（%），用哪一組由 EastSun 定 |
| 寫完讀回比對、不符重試 5 次回 2 | 寫 SDO → 讀回 SDO → 比對；不符重試 5 次回 2 | 保留 golden 狀態機語意（`iWriteAndCheckMotorTorque` 的回傳 1／2 不變） |
| `ReadTorque_Panasonic` 讀扭力回饋 | **0x6077 實際扭力（0.1%）**：監看器樣本或一次 SDO 讀 | 換算回 golden 顯示單位 |
| `SW[SwReadTorue]` 切 Z1／Z2 繼電器 | 不需要（每軸各自的 SDO） | HT9050 單 Z |

## 建議的實作切法（等上面三個條件）

1. `rs232.cpp` 的 `iWriteAndCheckMotorTorque`／`ReadTorque`：`MachineTypeChoice==Type_HT9050` 時改走 1203（其他機種照 golden 走 RS232，不動）。
   ⚠ 分支判斷只用 machine type（使用者原話）；註解寫明「golden 沒有 HT9050」。
2. `Pci1203Control` 新增內部 kind（沒有 wire 名，比照 MT-E1 的 kCmdAxSetExtDrive）：`kCmdAxTorqueLimitWrite`／`kCmdAxTorqueLimitRead`，
   白名單加 0x60E0／0x60E1（或 Pn402／Pn403）；**非同步**：狀態機送命令、下一拍收結果，不在節拍執行緒上等（避免擋輪詢）。
3. 讀回不符 5 次 → 回 2（golden 的「Motor torque set error」照舊）。
4. 測試：筆電用假後端驗封包／物件值／重試次數；機台上 EastSun 在旁驗一次實際扭力（例如 iMaxPreasure=30 時 Z 下壓的電流上限）。

## 要 EastSun／使用者確認的

| # | 問題 | 預設 |
|---|---|---|
| 1 | 用 CiA402 0x60E0／0x60E1，還是 Yaskawa Pn402／Pn403？ | 0x60E0／0x60E1（標準物件，驅動器換廠牌也通） |
| 2 | HT9050 Z 軸 SERVOPACK 型號與韌體支不支援 60E0h/60E1h 的 SDO 寫入 | 上機讀一次 |
| 3 | `Prod.iMaxPreasure` 在 HT9050 配方裡的單位是否仍是「額定扭力 %」 | 是（沿用 golden） |
| 4 | SDO 寫入扭力限制是否可以在運轉中每次接觸前寫（golden 每次 case 12000 都寫） | 照 golden |

## 20260925 22:0x EastSun 回覆與介面定案（經機台端 session 轉交）

| # | EastSun 的回答 |
|---|---|
| 1 | 用 **CiA402 60E0h／60E1h**（單軸 SGDXS；雙軸 SGDXW 的 B 軸是 68E0h／68E1h，偏移用 `Pci1203GearAxisBase(stationAxis)`；安川 SIEPC71081202 15.14.2/3、SIEPC71081205 14.15.2/3） |
| 2 | **同意**加進 Pci1203Control 的 SDO 寫入白名單 |
| 3 | 驅動器支援度：排進上機量測，**只讀**（不寫） |
| 4 | 扭力回饋照機台現在的做法：監看器每輪讀 `Acm_AxGetActTorque` 為主、選到的軸單次 SDO 6077h 備援（機台 MT-E3a `b90b0ca`，Pci1203Monitor 的 torque 欄位、`pci1203.ax<N>.torque／torquePct` tag；**還沒帶回筆電**） |

值：golden Pr0.13 只有一個「第一扭力限制」（%）⇒ 60E0h 與 60E1h **寫同一個值＝golden % × 10**（歸零 300% → 3000）；驅動器取 60E0h/60E1h 與 Pn402/Pn403（預設 800%）較小者。

**分工（筆電 20260925 22:0x 同意機台端提議）**
- **機台（EastSun 的 1203 底層）**：`Pci1203Control` 新內部 kind（沒有 wire 名）`kCmdAxTorqueLimitSet` —— 參數 `axis`（1203 軸序）、`value`（0.1% 單位，UINT16）；
  一次 Execute 內：寫 60E0h 與 60E1h（B 軸 68E0h/68E1h）→ 兩個都讀回 → 比對；`result.ok`＝兩寫都成功且兩讀都等於 value；`result.value`＝60E0h 讀回值；失敗時 `result.error` 寫明是哪一步。
- **筆電（Index 流程）**：`rs232.cpp` 的 `iWriteAndCheckMotorTorque(MotorIndex, Torque)` 在 SOFT_SIMULTE 早退之後加
  `MachineTypeChoice==Type_HT9050` 分支（其他機種照 golden 走 RS232 不動）：MotorIndex 0 → MTestZ1（1 → MTestZ2，未啟用時照 golden 視為不寫）；
  每次呼叫送一次 `kCmdAxTorqueLimitSet(value=Torque×10)`，**回傳值照 golden：0＝進行中（這次沒對上，下次再試）、1＝成功、2＝連續 5 次沒對上**；
  因為生產函式庫不帶 HAVE_PCI1203，筆電經 wb_serve 安裝的掛鉤呼叫 Pci1203Control（沒有掛鉤＝回 2 並寫明原因，不假成功）；ctest 用假後端驗值（%×10）、重試次數、回傳值。
- 扭力回饋（主畫面扭力欄）等 `b90b0ca` 帶回後，HT9050 分支改讀監看器的 torquePct。
