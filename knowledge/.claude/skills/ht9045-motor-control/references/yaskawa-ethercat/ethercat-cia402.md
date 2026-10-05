# EtherCAT 與 CiA402：SGDXS 這一端的行為

> 來源：SIEP C710812 02H（以下「p.」都是 02H 的 PDF 頁碼）。主站端（PCI-1203／Acm_*）怎麼做不在安川手冊裡，
> 看 [ht9050-1203-homing](../../../ht9050-1203-homing/SKILL.md) 與 [ethercat-api.md](../ethercat-api.md)。
> 標「依圖判讀」的是從手冊圖示讀出來的，不是手冊文字。

## 1. 通訊規格與身分

| 項目 | 內容 | 頁 |
|---|---|---|
| 標準 | IEC 61158 Type 12、IEC 61800-7 CiA402 drive profile | p.79 |
| 實體層 | 100BASE-TX；CN6A（RJ45）＝ETHERCAT IN、CN6B＝ETHERCAT OUT；CAT5、4 對遮蔽雙絞；AUTO MDIX | p.79 |
| 站間線長 | 50 m 以下 | p.166 |
| DC 週期 | 62.5 μs～4 ms，以 62.5 μs 為單位；Free-run 與 DC 可切換 | p.79 |
| Mailbox | Emergency、SDO request／response | p.79 |
| 站別識別 | ID selector S1／S2（16 位置）；可用 Explicit Device Identification（不必照接線順序） | p.79、p.592 |
| 開機辨識 | 主站用 Auto Increment Addressing 掃描，比對 Identity 與主站設定；一般要照主站設定的順序接 | p.592 |
| ESI 檔 | `Yaskawa_SGDXS-xxxxA0x.xml`，用最新版 | p.589 |
| 1000h | 0x00020192：附加資訊 0002（Servo Drive）、profile 0192（DS402） | p.634 |
| 1018h | Vendor ID 0x00000539、Product code 0x02200901、Revision＝主版／次版、Serial number 永遠 0 | p.636 |
| 1008h／100Ah／6403h／F9F0h | 驅動器型號字串、軟體版本、馬達型號字串、驅動器序號 | p.634、p.694、p.695 |

資料型別（SINT／INT／DINT／USINT／UINT／UDINT／STRING）與單位記號（Pos.／Vel.／Acc.／Trq. unit、inc）見 p.588；
26 位元編碼器每轉 67108864 inc（p.588）。

## 2. ESM（EtherCAT State Machine）

| 狀態 | Mailbox | PDO | 進入時主站做的事 | 頁 |
|---|:-:|:-:|---|---|
| INIT | ✗ | ✗ | — | p.590 |
| INIT→PRE-OP | | | 設 DL address、mailbox 的 SM、初始化 DC、寫 AL control | p.590 |
| PRE-OPERATIONAL | ✓ | ✗ | — | p.590 |
| PREOP→SAFEOP | | | 設 PDO 的 SM／FMMU；**用 SDO 設 PDO mapping 與 SM PDO assignment**；從站檢查 SM 與 DC 設定 | p.590 |
| SAFE-OPERATIONAL | ✓ | 只有輸入有效（輸出還無效） | — | p.590 |
| SAFEOP→OP | | | 主站送有效輸出、要求 OP | p.590 |
| OPERATIONAL | ✓ | ✓ | — | p.590 |

- SM0＝收 mailbox（128 bytes，0x1000）、SM1＝送 mailbox（128 bytes，0x1080）、SM2＝RxPDO（0～64 bytes，0x1100）、
  SM3＝TxPDO（0～64 bytes，0x1400）；FMMU0＝RxPDO、FMMU1＝TxPDO、FMMU2＝mailbox status（p.591）。
- **PDO mapping（1600h～1603h、1A00h～1A03h）與 1C12h／1C13h 只能在 PRE-OP 寫**（p.593、p.637、p.646）。
- 警報 **A.A10、A.A12、A.EA2 發生後 ESM 自動掉到 SAFEOP**（p.702）。
- CiA402 的 Ready to Switch ON→Switched ON 需要「主迴路電源 ON＋ESM 在 OP＋沒有被操作器／SigmaWin+ 佔用」（p.600 註 *3）。
- LED：RUN 滅＝INIT、閃＝PRE-OP、單閃＝SAFE-OP、亮＝OP；ERR 單閃＝同步錯誤自動進 SAFE-OP、雙閃＝SM watchdog timeout、
  恆亮＝PDI watchdog timeout；L/A A／B 分別是 CN6A／CN6B 的連線（p.862-863）。

## 3. 同步模式（Free-run／DC）

- 由 ESC 暫存器 0x980／0x981 切換：**Free-Run＝0x980 寫 0x0000**（本地週期跟通訊週期無關）、**DC＝0x0300**（跟主站的 Sync0 同步）（p.595）。
- 1C32h（SM2 輸出）／1C33h（SM3 輸入）同步參數（p.596、p.647-649）：

| 子索引 | 1C32h | 1C33h |
|---|---|---|
| :1 Synchronization type | RO；0＝Free-Run 無 PDO、1＝Free-Run 有 PDO、2＝DC Sync0 | 同 |
| :2 Cycle time | RO；Sync0 週期 ns，62500×n（n＝1～64），主站經 ESC 設定 | 同 |
| :3 Shift time | RW，62500～Sync0 週期（預設 62500）ns，存 EEPROM | RW，0～(Sync0 週期−62500)（預設 0） |
| :4 Supported types | 0x0025 | 0x0025 |
| :5／:6 | 最小週期 62500 ns／Calc and copy time 62500 ns | 同 |
| :11 SM event missed counter | 收 PDO 失敗的次數（每失敗一次 +1） | 同 |
| :32 Sync Error | 0＝無、1＝同步錯誤 | 同 |

- **10F1h:2 Sync error count limit**（UINT、0～15、預設 9、存 EEPROM）：DC 模式下 Sync0 到了但輸出資料沒更新 → 內部計數 +3；
  正常更新 → −1；超過上限 → **A.A12（EtherCAT Output Data Synchronization Error）並把 ESM 改成 SAFEOP**。
  計數在 SAFEOP→OP 時歸零；DC 關閉或上限設 0 時不偵測。預設 9＝連續掉 3 個 frame 才報警（p.649-650）。
- 發生 A.A12 時先看 1C32h:11 估計失敗頻率再決定上限（p.649）。
- Interpolated Position 模式的 60C2h：DC Sync0 模式下自動等於 Sync0 週期；Free-run 要手動設（p.681）。

## 4. PDO

- 每個 mapping 最多 16 筆、64 bytes（p.594）。設定程序：1C12h／1C13h:0 寫 0 → 寫 mapping 各筆 → 寫筆數 → 寫 assignment → 打開（p.593-594）。
- mapping entry 格式：bit31-16 物件 index、bit15-8 子索引、bit7-0 長度（bit）（p.637）。
- **預設 mapping（p.594、p.638）**：

| 組 | RxPDO（主站→驅動器） | TxPDO（驅動器→主站） |
|---|---|---|
| 1st（1600h／1A00h）位置／速度／扭力／扭力限制／touch probe | 6040h、607Ah、60FFh、6071h、6072h、6060h、padding 8 bit、60B8h | 6041h、6064h、6077h、60F4h、6061h、padding 8 bit、60B9h、60BAh |
| 2nd（1601h／1A01h）CSP | 6040h、607Ah | 6041h、6064h |
| 3rd（1602h／1A02h）CSV | 6040h、60FFh | 6041h、6064h |
| 4th（1603h／1A03h）CST | 6040h、6071h | 6041h、6064h、6077h |

- **預設 assignment：1C12h:0＝1、1C12h:1＝0x1601；1C13h:0＝1、1C13h:1＝0x1A01**（即預設只用 2nd 組，p.646）。
- 試運轉步驟的註：「操作已對到 PDO 的物件；**用 SDO 改的話值不會寫入**」（p.320）。
- ⚠ PCI-1203 實際下的是哪組 mapping、有沒有把 6077h／6072h／60E0h 放進 PDO：安川手冊未載明，看主站設定。
- Emergency message（mailbox，8 bytes）：byte0-1＝FF00h、byte2＝1001h Error Register、byte3 保留、**byte4-5＝驅動器警報／警告碼**、byte6-7 保留（p.598）。
  通訊不穩時可能送不出來（p.702）。

## 5. CiA402 狀態機（6040h／6041h）

### 5.1 狀態轉移（依 p.600 圖判讀＋圖註）

| 從 | 命令／條件 | 到 |
|---|---|---|
| 上電 | 自動 | Not ready to switch on → Switch on disabled |
| Switch on disabled | Shutdown | Ready to switch on |
| Ready to switch on | Switch on（且主迴路 ON、ESM 在 OP，註 *3） | Switched on |
| Switched on | Enable operation | **Operation enabled（伺服 ON）** |
| Operation enabled | Disable operation | Switched on |
| Operation enabled／Switched on | Shutdown | Ready to switch on |
| Ready to switch on | Disable voltage 或 Quick stop | Switch on disabled |
| Switched on | Disable voltage 或 Quick stop（或主電源 OFF、或 jog 操作，註 *2） | Switch on disabled |
| Operation enabled | Disable voltage（或主電源 OFF、或 HWBB 輸入，註 *4） | Switch on disabled |
| Operation enabled | Quick stop | Quick stop active |
| Quick stop active | Disable voltage（或主電源 OFF、HWBB、馬達已停，註 *1） | Switch on disabled |
| 任何狀態 | 驅動器錯誤（EtherCAT down error） | Fault reaction active → Fault |
| Fault | **Fault reset**（6040h bit7 0→1） | Switch on disabled |

圖上分區：(A) 控制電源 ON、主迴路 OFF、伺服 OFF；(B) 主迴路 ON 但馬達未通電（Switched on）；(C) 馬達通電（Operation enabled、Quick stop active、Fault reaction active）（p.600）。

### 5.2 Controlword 6040h（UINT、RW、PDO、預設 0、不存 EEPROM，p.665）

| 命令 | bit7 | bit3 | bit2 | bit1 | bit0 | 對應最小值（推導） |
|---|:-:|:-:|:-:|:-:|:-:|---|
| Shutdown | 0 | – | 1 | 1 | 0 | 0x0006 |
| Switch ON | 0 | 0 | 1 | 1 | 1 | 0x0007 |
| Switch ON＋Enable operation | 0 | 1 | 1 | 1 | 1 | 0x000F |
| Disable voltage | 0 | – | – | 0 | – | 0x0000 |
| Quick stop | 0 | – | 0 | 1 | – | 0x0002 |
| Disable operation | 0 | 0 | 1 | 1 | 1 | 0x0007 |
| Enable operation | 0 | 1 | 1 | 1 | 1 | 0x000F |
| Fault reset | 0→1 | – | – | – | – | bit7 上升緣 |

（命令表 p.601、p.665-666；「最小值」欄是把「–」當 0 推出來的，不是手冊原文。）

其他位元（p.665-667）：bit4～6、bit9＝依模式；**bit7＝警報／警告 Reset（0→1）**；bit8＝Halt（依 605Dh 停）；
**bit11＝開 Torque Limit Parameter 2404h（Pn404）、bit12＝開 2405h（Pn405）**；bit10、13～15 保留。

| 模式 | bit4 | bit5 | bit6 | bit8 | bit9 |
|---|---|---|---|---|---|
| Profile Position | New set point（0→1） | Change set immediately | 0＝絕對、1＝相對 | Halt | Change of set point |
| Homing | **1＝開始／繼續回原點** | 保留 | 保留 | 1＝依 605Dh 停（0 才讓 bit4 生效） | 保留 |
| CSP／CSV／CST | 保留 | 保留 | 保留 | Halt | 保留 |
| Interpolated Position | Enable interpolation | 保留 | 保留 | Halt | 保留 |
| PV／PT | 保留 | 保留 | 保留 | Halt | 保留 |

PP 的 bit9／5／4 組合：0／0／0→1＝目前定位完成後才開始下一筆；X／1／0→1＝立刻開始；1／0／0→1＝用目前速度走到目前目標再接下一筆（p.666）。

### 5.3 Statusword 6041h（UINT、RO、PDO，p.667-670）

| bit | 意義 | bit | 意義 |
|---|---|---|---|
| 0 | Ready to switch ON | 8 | 保留 |
| 1 | Switched ON | 9 | Remote：6040h 正在被處理 |
| 2 | Operation enabled | 10 | 依模式（多為 Target reached） |
| 3 | **Fault** | 11 | Internal limit active：軟體極限截了目標、N-OT／P-OT 動作、或 IP／CSP 插補速度超限 |
| 4 | Voltage enabled（主電源 ON） | 12、13 | 依模式 |
| 5 | Quick stop | 14 | **Torque limit active**（0＝未限、1＝限制中） |
| 6 | Switch ON disabled | 15 | Safety active（安全功能動作中） |
| 7 | **Warning** | | |

狀態判讀（p.668 表；遮罩寫法為推導）：

| 狀態 | 位元樣式（bit7..0） | 遮罩判斷 |
|---|---|---|
| Not ready to switch ON | x0xx 0000 | (sw & 0x4F)==0x00 |
| Switch ON disabled | x1xx 0000 | (sw & 0x4F)==0x40 |
| Ready to switch ON | x01x 0001 | (sw & 0x6F)==0x21 |
| Switched ON | x01x 0011 | (sw & 0x6F)==0x23 |
| Operation enabled | x01x 0111 | (sw & 0x6F)==0x27 |
| Quick stop active | x00x 0111 | (sw & 0x6F)==0x07 |
| Fault reaction active | x0xx 1111 | (sw & 0x4F)==0x0F |
| Fault | x0xx 1000 | (sw & 0x4F)==0x08 |

例：0x0633＝Switched ON＋主電源 ON（bit4）＋Remote（bit9）＋Target reached（bit10）（樹 `EtherCAT\Pci1203Control.cpp` 註解裡的實測值，用上表解）。

依模式的 bit10／12／13（p.668-670）：

| 模式 | bit10 | bit12 | bit13 |
|---|---|---|---|
| Profile Position | Target reached（Halt＝1 時代表已停） | Set-point acknowledge | Following error |
| **Homing** | 見 §7.4 | Homing attained | Homing error |
| CSP／CSV／CST | Target reached（CST 恆 0） | 1＝目標值被採用、0＝被忽略 | Following error（CSV／CST 恆 0） |
| Interpolated Position | Target reached | IP mode active | 保留 |
| Profile Velocity | Target reached | 1＝速度低於 Pn502 | 保留 |
| Profile Torque | Target reached | 保留 | 保留 |

### 5.4 停止選項碼（p.670-672；全部 INT、RW、不能 PDO、存 EEPROM）

| 物件 | 範圍（預設） | 值 |
|---|---|---|
| 605Ah Quick Stop | 0～4（**2**） | 0＝直接 disable（去 Switch ON Disabled）；1＝用一般減速停再去 Switch ON Disabled；2＝用 6085h quick stop 減速停再去；3＝用扭力極限減速停再去。值 4 手冊未說明。扭力模式一律照 0 |
| 605Bh Shutdown | 0～1（0） | Operation Enabled→Ready to Switch ON 時：0＝disable；1＝減速停（註：原文寫「moves to Switch ON Disabled」） |
| 605Ch Disable Operation | 0～1（**1**） | Operation Enabled→Switched ON 時：0＝disable；1＝減速停 |
| 605Dh Halt | −3～3（**1**） | 1／−1＝一般減速；2／−2＝6085h；3／−3＝扭力極限；負值停後在「停止判定速度」以下零位鎖定；0 保留；停完留在 Operation Enabled |
| 605Eh Fault Reaction | 0～0（0） | 0＝disable（伺服 OFF） |

「一般減速」用的物件：PP／IP／CSP／CSV 用 6084h，Homing 用 **609Ah**（p.670-671）。

### 5.5 伺服 ON 前後的時序（手冊）

- 控制電源 ON 後 ALM 輸出最多 10 秒；等 ALM OFF（警報清除）才上主迴路（p.135）。
- 控制電源先上或與主迴路同時上；關機先關主迴路再關控制電源；關掉後至少等 3 秒再上控制電源（p.136）。
- **送 Servo ON（Enable Operation）後至少等 50 ms＋煞車放開延遲時間，才從主站下命令**（p.206 註 *2）。延遲表見 parameters 檔 §3。
- 伺服 ON 送不進去時看 /S-RDY：主電源 ON、無 HWBB、無警報、無 FSTP、（無極性感測器的馬達）極性偵測完成（p.135、p.242）。
- 試運轉（PP 模式）流程：接 CN1／CN6A → 主站設站號與 PDO → 上電 → 進 OP → 6060h=1 → 用 Controlword 讓 Statusword 到 Operation enabled → 設目標與速度並觸發（p.320）。

## 6. 運轉模式 6060h／6061h

- 6060h（SINT、RW、PDO、0～10、預設 0、**存 EEPROM**）：0＝無模式／不變、1＝Profile Position、2＝保留、3＝Profile Velocity、
  4＝Torque Profile、6＝Homing、7＝Interpolated Position、8＝Cyclic Sync Position、9＝Cyclic Sync Velocity、10＝Cyclic Sync Torque；其他保留（p.672）。
- 6061h（SINT、RO、PDO）回報目前模式，值同 6060h（p.672）。
- 6502h＝0x03ED：bit0 pp、bit2 pv、bit3 tq、**bit5 hm**、bit6 ip、bit7 csp、bit8 csv、bit9 cst 支援；**bit1 vl（Velocity mode）不支援**（p.672-673）。
  （p.603 的條列把 4 號寫成「Torque Profile Velocity Mode」，對照 p.672 是 Torque Profile Mode。）
- 動態切模式：主站換模式時要同時更新該模式的 PDO 物件；切到 PP／Homing／IP 時 bit4＝0 先在目前位置停、bit4＝1 立即開始；
  切到 PV／TQ／CSP／CSV／CST 立即生效（p.603）。
- PP 重點：607Ah 行程要 ≤ 2³¹−1；6086h＝2 用 S 曲線，jerk 在 60A4h:1（0～50 %、預設 25）；
  **改 607Ah／6081h／6083h／6084h 只能在停止或等速時改**（p.605-606）。
- CSP：目標位置一律絕對值；60B1h 速度前饋、60B2h 扭力前饋（p.611、p.683）。

## 7. 回原點（Homing）

### 7.1 相關物件（p.613、p.676-677）

| 物件 | 型別 | 存取 | PDO | 存 EEPROM | 範圍（預設） |
|---|---|---|:-:|:-:|---|
| 607Ch Home Offset | DINT | RW | No | Yes | −536870912～536870911（0）[Pos. unit] |
| 6098h Homing Method | SINT | RW | Yes | **No** | 0～37（**37**） |
| 6099h:1 Speed during search for switch | UDINT | RW | Yes | Yes | 0～4294967295（500000）[Vel. unit] |
| 6099h:2 Speed during search for zero | UDINT | RW | Yes | Yes | 0～4294967295（100000）[Vel. unit] |
| 609Ah Homing Acceleration | UDINT | RW | Yes | Yes | 0～4294967295（1000）[Acc. unit] |

輸出：6041h、60FCh（Position demand internal value）或 6062h（p.613 圖）。

### 7.2 支援的方法（p.613-615、p.676）

| 值 | 內容 |
|---|---|
| 0 | 不回原點 |
| 1 | 負向極限開關＋index pulse（N-OT 未動作時反向起跑；原點＝N-OT 解除後第一個 index） |
| 2 | 正向極限開關＋index pulse |
| 7～10 | /Home 開關＋index，正向起跑；起跑時 /Home 已動作則依所需邊緣決定方向；遠離開關時遇行進方向的極限開關會反轉 |
| 11～14 | 同 7～10，反向起跑 |
| **24** | /Home 開關、正向起跑；**同 method 8 但不看 index**，原點只取決於 /Home（或極限開關）的變化 |
| **28** | /Home 開關、反向起跑；同 method 12 但不看 index |
| 33、34 | 只用 index pulse |
| 35、37 | **目前位置當原點**；不在 Operation Enabled 也能執行（**37 是預設**）；接絕對編碼器時偏移自動存進 27E4h 與非揮發記憶體，建議 607Ch 設 0 |

手冊沒列出的值（3～6、15～23、25～27、29～32、36）：手冊未載明是否支援。index pulse＝編碼器 C 相（p.615）。
方法 1、2、7～14、24、28 標 *1：**超程警報開著時不能用限位開關回原點**（p.677；p.201 同句，指 Pn00D=n.2□□□）。

### 7.3 method 24／28 的三種起點（依 p.615 圖判讀）

- **24**：原點＝/Home 動作區的「負向側邊緣」，最後一段**正向**接近。
  - 起點在動作區負向側 → 正向走到邊緣停。
  - 起點在動作區內（/Home 已動作）→ 先反向走出動作區，再折返正向接近邊緣。
  - 起點在動作區正向側 → 正向走到 P-OT，折返反向穿過動作區，再折返正向接近邊緣。
- **28**：鏡像——原點＝動作區的「正向側邊緣」，最後一段**反向**接近；起點在動作區負向側時是走到 N-OT 再折返。
- 這跟 HT9050 實測「SGDXW 壓在原點上起跑會先退開」相符（[ht9050-1203-homing](../../../ht9050-1203-homing/SKILL.md) §3a）。

### 7.4 回原點時的 6041h（p.669）

| bit13 Homing error | bit12 Homing attained | bit10 Target reached | 意義 |
|:-:|:-:|:-:|---|
| 0 | 0 | 0 | 回原點進行中 |
| 0 | 0 | 1 | 被中斷或還沒開始 |
| 0 | 1 | 0 | 原點已定義，但動作還在進行 |
| **0** | **1** | **1** | **正常完成** |
| 1 | 0 | 0 | 回原點錯誤，速度不為 0 |
| 1 | 0 | 1 | 回原點錯誤，速度為 0 |

### 7.5 回原點中遇超程（p.204）

- method 1、11、12、13、14、**28**、34：**P-OT 輸入**→ bit13＝1，回原點取消。
- method 2、7、8、9、10、**24**、33：**N-OT 輸入**→ bit13＝1，回原點取消。

### 7.6 原點偏移與軟體極限

- 增量編碼器：回原點完成後，零點＝原點＋607Ch。絕對編碼器：每次上電時零點＝原點＋607Ch＋27E4h（p.676）。
  若編碼器位置 X 要當 0，607Ch 設 −X（p.225）。偏移在上電或 2700h 生效後加進 6064h（p.225）。
- 607Dh 軟體極限：**回原點完成後、或接絕對編碼器時生效**；Min ≥ Max（預設 0／0）＝關閉；值要含 607Ch 偏移（p.674）。
- 回原點進行中不能用 touch probe 1（已開的會被關掉）（p.622、p.689）。
- 手冊另有 Fn003 Origin Search（試運轉工具，p.328），不是 CiA402 homing。

HT9050 的做法（卡片 124／128、速度從 PTP 參數種進 6099h／609Ah、READYDONE 判準、樹不讀 bit12／13）：
見 [ht9050-1203-homing](../../../ht9050-1203-homing/SKILL.md) §1b、§3c、§5。

## 8. Touch probe（60B8h～60BDh，p.622-623、p.689-691）

- 兩組可同時用。觸發來源：probe 1＝/Probe1（預設 CN1-10，SI4）或編碼器 C 相；probe 2＝/Probe2（預設 CN1-11，SI5）
  （p.622 寫 probe 2 只接 /Probe2；p.689 的 60B8h bit10＝1 又寫可選 C 相——兩處不一致，以實測為準）。
- 腳位與極性：Pn511＝n.□□X□（/Probe1）、n.□X□□（/Probe2）（p.622、p.804）。

60B8h Touch probe function（UINT、RW、PDO、預設 0，p.689）：

| bit | 0 | 1 |
|---|---|---|
| 0（8） | 關 probe 1（2） | 開 |
| 1（9） | Single trigger（只鎖第一次） | Continuous（每次都鎖） |
| 2（10） | 觸發源＝/Probe1（/Probe2） | 觸發源＝C 相 |
| 4（12） | 停止上升緣取樣 | 開始上升緣取樣 |
| 5（13） | 停止下降緣取樣 | 開始下降緣取樣 |
| 6 | 鎖 6064h | 鎖外部編碼器位置（要開外部編碼器監看才可用） |

括號內是 probe 2 的位元。Continuous 模式下每次開始鎖都會重讀 bit2，連續用同一觸發源時不要改 bit2（p.689）。

60B9h Touch probe status（UINT、RO、PDO，p.690）：bit0／8＝probe 1／2 已啟用；bit1／9＝有上升緣鎖存值；bit2／10＝有下降緣鎖存值；
bit6／14、bit7／15＝Continuous 模式每鎖一次就翻轉（下降緣／上升緣）。
鎖存值：60BAh／60BBh＝probe 1 上升／下降緣，60BCh／60BDh＝probe 2（DINT、RO、PDO、[Pos. unit]，p.690-691）。

## 9. 數位 I/O（60FDh／60FEh，p.692-693）

- 60FDh（UDINT、RO、PDO）：bit0 N-OT、bit1 P-OT、bit2 /Home（0＝OFF、1＝ON）；bit16～22＝SI0～SI6；bit24／25＝HWBB1／HWBB2（0＝開路、1＝閉合）。
- 60FEh:1 Physical outputs（UDINT、RW、PDO、存 EEPROM）：bit0 **/BK（0＝煞車作用、1＝放開）**、bit17～19＝SO1～SO3；
  60FEh:2 Bit mask（預設 0x000C0000＝只開 SO2、SO3）。與 250Eh／250Fh／2510h…分配的輸出做 OR，重複分配要用 mask 關掉。
- 只有 ESM 在 OP 時才有效；不在 OP 時 /BK 的物件設定被忽略（不能用物件放煞車），SO1～SO3 照 255Ch／255Dh 的通訊異常輸出設定（p.692）。
- ⚠ **用 60FEh 放 /BK 後，只要沒有警報就一直有效——伺服 OFF 了煞車仍是放開的**，手冊標「極度危險」（p.693）。
