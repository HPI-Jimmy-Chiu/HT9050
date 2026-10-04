# A6BN 物件字典：6000h 驅動 profile、1000h 通訊、3000h 參數、4000h／5000h 廠商物件

> 頁碼＝PDF 頁碼（SX-DSV03736 R2.1）。欄位照規格書 §9 物件表（A6BN p.301-337）；說明欄的頁碼是該物件的詳細說明頁。

## 0. 怎麼讀這張表

- 物件區段（A6BN p.57）：1000h-1FFFh CoE 通訊、3000h-3FFFh 伺服參數、4000h-4FFFh 使用者特定、5000h-5FFFh（規格書 §9-4 有 5350h-5352h）、6000h-6FFFh 驅動 profile。2000h-2FFFh 保留。
- **Attribute**（變更何時生效）（A6BN p.301）：
  A＝隨時生效；B＝馬達運轉或命令傳送中禁止改（會短暫不穩）；C＝重送控制電源或 PANATERM 設腳位後；R＝重送控制電源後；
  P＝Init→PreOP 時；S＝PreOP→SafeOP 時；H＝位置資訊確定後；X＝唯讀或不支援，不能改。
- **PDO**：No＝只能 SDO；RxPDO／TxPDO＝可映射（A6BN p.301）。
- EEPROM＝Yes 的物件可用 1010h 存（A6BN p.81）；EEPROM＝No 的在 PANATERM 顯示成唯讀（A6BN p.18、p.301）。
- 線性馬達：表中 r/min 讀成 mm/s、torque 讀成 thrust（A6BN p.14）。

## 1. 6000h 驅動 profile（A6BN p.334-337）

| Index:Sub | 名稱 | 型別 | 單位 | 範圍 | 存取 | PDO | 模式 | EEPROM | 屬性 | 說明頁／重點 |
|---|---|---|---|---|---|---|---|---|---|---|
| 6007h:00 | Abort connection option code | I16 | － | 0-3 | rw | No | ALL | Yes | A | p.227 主電源切斷時的減速 |
| 603Fh:00 | Error code | U16 | － | 0-65535 | ro | TxPDO | ALL | No | X | p.46、p.297 FFxxh＝主警報號 |
| 6040h:00 | Controlword | U16 | － | 0-65535 | rw | RxPDO | ALL | No | A | p.88 |
| 6041h:00 | Statusword | U16 | － | 0-65535 | ro | TxPDO | ALL | No | X | p.90 |
| 605Ah:00 | Quick stop option code | I16 | － | -2-7 | rw | No | ALL | Yes | A | p.230 |
| 605Bh:00 | Shutdown option code | I16 | － | 0-1 | rw | No | ALL | Yes | A | p.232 |
| 605Ch:00 | Disable operation option code | I16 | － | 0-1 | rw | No | ALL | Yes | A | p.234 |
| 605Dh:00 | Halt option code | I16 | － | 1-3 | rw | No | ALL | Yes | A | p.235 |
| 605Eh:00 | Fault reaction option code | I16 | － | 0-2 | rw | No | ALL | Yes | A | p.236 只管 Err80-88 |
| 6060h:00 | Modes of operation | I8 | － | -128-127 | rw | RxPDO | ALL | Yes | A | p.94 開機預設 0 |
| 6061h:00 | Modes of operation display | I8 | － | -128-127 | ro | TxPDO | ALL | No | X | p.95 |
| 6062h:00 | Position demand value | I32 | command | ±2^31 | ro | TxPDO | pp hm ip csp | No | X | p.107（＝IPOS） |
| 6063h:00 | Position actual internal value | I32 | pulse | ±2^31 | ro | TxPDO | ALL | No | X | p.107 feedback scale 單位 |
| **6064h:00** | Position actual value | I32 | command | ±2^31 | ro | TxPDO | ALL | No | X | p.107 |
| 6065h:00 | Following error window | U32 | command | 0-4294967295 | rw | RxPDO | pp csp | Yes | A | p.112 |
| 6066h:00 | Following error time out | U16 | 1 ms | 0-65535 | rw | RxPDO | pp csp | Yes | A | p.112 |
| 6067h:00 | Position window | U32 | command | 0-4294967295 | rw | RxPDO | pp ip | Yes | A | p.111 |
| 6068h:00 | Position window time | U16 | 1 ms | 0-65535 | rw | RxPDO | pp ip | Yes | A | p.111 |
| 6069h:00 | Velocity sensor actual value | I32 | － | ±2^31 | ro | TxPDO | ALL | No | X | p.108 不支援，永遠 0 |
| 606Bh:00 | Velocity demand value | I32 | command/s | ±2^31 | ro | TxPDO | pv csv | No | X | |
| **606Ch:00** | Velocity actual value | I32 | command/s | ±2^31 | ro | TxPDO | ALL | No | X | p.108（＝FSPD） |
| **6071h:00** | Target torque | I16 | 0.1 % | ±32768 | rw | RxPDO | tq cst | Yes | A | p.195 超過 6072h 被限 |
| **6072h:00** | Max torque | U16 | 0.1 % | 0-65535 | rw | RxPDO | ALL | Yes | A | p.101 上限＝100×3907h／(3906h×√2) |
| 6073h:00 | Max current | U16 | 0.1 % | 0-65535 | ro | No | tq | No | X | |
| 6074h:00 | Torque demand | I16 | 0.1 % | ±32768 | ro | TxPDO | ALL | No | X | p.109 內部扭力命令 |
| 6075h:00 | Motor rated current | U32 | mA | 0-4294967295 | ro | No | ALL | No | X | |
| 6076h:00 | Motor rated torque | U32 | mN·m | 0-4294967295 | ro | No | ALL | No | X | p.109 從馬達讀出自動設定（p.105 表寫 TxPDO，p.334 寫 No） |
| **6077h:00** | Torque actual value | I16 | **0.1 %** | ±32768 | ro | TxPDO | ALL | No | X | p.109「相當於實際電流值」「只是參考值、不保證」 |
| 6078h:00 | Current actual value | I16 | 0.1 % | ±32768 | ro | TxPDO | ALL | No | X | |
| 6079h:00 | DC link circuit voltage | U32 | mV | 0-4294967295 | ro | TxPDO | ALL | No | X | |
| **607Ah:00** | Target position | I32 | command | ±2^31 | rw | RxPDO | pp csp | No | A | p.100 |
| 607Bh:01／02 | Min／Max position range limit | I32 | command | ±2^31 | rw | RxPDO | ALL | Yes | X | p.252 A6BN 內部固定 80000000h／7FFFFFFFh |
| **607Ch:00** | Home offset | I32 | command | ±2^31 | rw | RxPDO | ALL | Yes | P,H | p.253 |
| **607Dh:01／02** | Min／Max position limit | I32 | command | ±2^31 | rw | RxPDO | pp ip csp | Yes | P,H | p.102 01h≥02h＝關閉 |
| 607Eh:00 | Polarity | U8 | － | 0-255 | rw | No | ALL | Yes | P,H | p.249 只能 0 或 224 |
| **607Fh:00** | Max profile velocity | U32 | command/s | 0-4294967295 | rw | RxPDO | pp hm ip pv | Yes | B | p.100 3697h bit8＝1 時也管 tq、cst |
| **6080h:00** | Max motor speed | U32 | r/min | 0-4294967295 | rw | RxPDO | ALL | Yes | B | p.100 被 3910h 限 |
| 6081h:00 | Profile velocity | U32 | command/s | 0-4294967295 | rw | RxPDO | pp ip | Yes | A | p.100 |
| 6082h:00 | End velocity | U32 | command/s | 0-4294967295 | rw | RxPDO | pp ip | Yes | X | p.100 不支援，讀 0 |
| 6083h:00 | Profile acceleration | U32 | command/s² | 0-4294967295 | rw | RxPDO | pp pv ip | Yes | A | p.101 0 視為 1 |
| 6084h:00 | Profile deceleration | U32 | command/s² | 0-4294967295 | rw | RxPDO | pp pv ip csp csv | Yes | A | p.101 csp/csv 只在減速停止程序 |
| 6085h:00 | Quick stop deceleration | U32 | command/s² | 0-4294967295 | rw | RxPDO | pp pv hm ip csp csv | Yes | A | p.225 |
| 6086h:00 | Motion profile type | I16 | － | ±32768 | rw | RxPDO | pp pv ip | Yes | A | p.254 jerk 不支援 |
| 6087h:00 | Torque slope | U32 | 0.1 %/s | 0-4294967295 | rw | RxPDO | tq cst | Yes | A | p.204 |
| 6088h:00 | Torque profile type | I16 | － | ±32768 | rw | RxPDO | tq | Yes | A | p.204 只支援 0 linear |
| 608Fh:01／02 | Encoder increments／Motor revolutions | U32 | pulse／r | 1-4294967295 | ro | No | ALL | No | X | p.244 自動設定 |
| 6091h:01／02 | Gear ratio motor／shaft revolutions | U32 | r | 1-4294967295 | rw | No | ALL | Yes | P,H | p.244 |
| 6092h:01／02 | Feed constant feed／shaft revolutions | U32 | command／r | 1-4294967295 | rw | No | ALL | Yes | P,H | p.244 |
| **6098h:00** | Homing method | I8 | － | -128-127 | rw | RxPDO（p.143 寫 No） | hm | Yes | B | p.143 |
| **6099h:01** | Speed during search for switch | U32 | command/s | 0-4294967295 | rw | RxPDO | hm | Yes | A | p.144 |
| **6099h:02** | Speed during search for zero | U32 | command/s | 0-4294967295 | rw | RxPDO | hm | Yes | A | p.144 |
| **609Ah:00** | Homing acceleration | U32 | command/s² | 0-4294967295 | rw | RxPDO | hm | Yes | A | p.144 |
| 60A3h、60A4h | Profile jerk use／jerk | － | － | － | rw | No | pp pv ip | Yes | A | p.254 不支援 |
| 60B0h:00 | Position offset | I32 | command | ±2^31 | rw | RxPDO | csp | Yes | A | p.131 |
| 60B1h:00 | Velocity offset | I32 | command/s | ±2^31 | rw | RxPDO | pp pv hm ip csp csv | Yes | A | p.100 |
| 60B2h:00 | Torque offset | I16 | 0.1 % | ±32768 | rw | RxPDO | ALL | Yes | A | p.101 |
| **60B8h:00** | Touch probe function | U16 | － | 0-65535 | rw | RxPDO | ALL | No | A | p.218 |
| **60B9h:00** | Touch probe status | U16 | － | 0-65535 | ro | TxPDO | ALL | No | X | p.219 |
| 60BAh-60BDh:00 | Touch probe pos1 pos／neg、pos2 pos／neg | I32 | command | ±2^31 | ro | TxPDO | ALL | No | X | p.220 |
| 60C2h:01／02 | Interpolation time period value／index | U8／I8 | － | － | rw | No | ip csp csv cst | Yes | A | p.255 自動設定，勿改 |
| 60C5h:00 | Max acceleration | U32 | command/s² | 0-4294967295 | rw | RxPDO | pp hm pv ip | Yes | A | p.101 |
| 60C6h:00 | Max deceleration | U32 | command/s² | 0-4294967295 | rw | RxPDO | pp hm pv ip | Yes | A | p.101 |
| **60E0h:00** | Positive torque limit value | U16 | 0.1 % | 0-65535 | rw | RxPDO | ALL | Yes | A | p.101 **只在 3521h＝5 生效** |
| **60E1h:00** | Negative torque limit value | U16 | 0.1 % | 0-65535 | rw | RxPDO | ALL | Yes | A | p.101 同上 |
| 60E3h:01-24 | Supported homing method | I16 | － | － | ro | No | ALL | No | X | p.147 36 個 |
| 60E4h:01 | 1st additional position actual value | I32 | － | ±2^31 | ro | No | ALL | No | X | |
| 60F2h:00 | Positioning option code | U16 | － | 0-65535 | rw | RxPDO | pp | Yes | A | p.118（pp 不支援） |
| **60F4h:00** | Following error actual value | I32 | command | ±2^31 | ro | TxPDO | pp hm ip csp | No | X | p.107 |
| 60FAh:00 | Control effort | I32 | command/s | ±2^31 | ro | TxPDO | pp hm ip csp | No | X | p.108 |
| 60FCh:00 | Position demand internal value | I32 | pulse | ±2^31 | ro | TxPDO | pp hm ip csp | No | X | p.107 |
| **60FDh:00** | Digital inputs | U32 | － | 0-4294967295 | ro | TxPDO | ALL | No | X | p.239（位元見 §2） |
| 60FEh:00 | Number of entries | U8 | － | 2 | ro | **No** | ALL | No | X | p.240 |
| **60FEh:01** | Physical outputs | U32 | － | 0-4294967295 | rw | RxPDO | ALL | Yes | A | p.240（位元見 §2） |
| 60FEh:02 | Bit mask | U32 | － | 0-4294967295 | rw | RxPDO | ALL | Yes | A | p.241 |
| 60FFh:00 | Target velocity | I32 | command/s | ±2^31 | rw | RxPDO | pv csv | No | A | |
| 6403h:00 | Motor catalogue number | VS | － | － | ro | No | ALL | No | X | |
| 6502h:00 | Supported drive modes | U32 | － | 0-4294967295 | ro | TxPDO | ALL | No | X | p.93 |

（±2^31 表示 -2147483648 到 2147483647；±32768 表示 -32768 到 32767。）

**6077h 扭力讀值要特別小心**（給 Index Z 扭力那條線）：
- A6BN：0.1 %、I16、TxPDO；「相當於實際電流值」「只是參考值、不保證實際值」（A6BN p.109）。另有 6074h（內部扭力命令，0.1 %）、6078h（電流實際值，0.1 %）（A6BN p.109、p.334）。
- 安川 Σ-X 的 6077h 是扭力命令值、單位由 2704h 決定（`D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md` §2）。兩邊名字一樣，意義與單位來源不同，不能直接共用換算。
- 線性馬達下「torque」讀成「thrust（N）」（A6BN p.14）；6076h 的單位仍寫 mN·m（A6BN p.109），線性型實際對應規格書未載明。

## 2. 60FDh／60FEh 位元（A6BN p.238-242）

60FDh Digital inputs（ro、TxPDO）：

| bit | 訊號 |
|---|---|
| 0 | NOT 負方向極限 |
| 1 | POT 正方向極限 |
| 2 | HOME 原點近接 |
| 3 | 不支援 |
| 16 | 保留 |
| 17 | VI-CLR 速度積分清除中 |
| 18 | RET 退避動作輸入 |
| 19 | SI-MON1／EXT1 |
| 20 | SI-MON2／EXT2 |
| 21 | SI-MON3 |
| 22 | SI-MON4 |
| 23 | SI-MON5／E-STOP |
| 24 | INP 定位完成 |
| 25 | RET-STAT 退避狀態 |
| 4-15、26-31 | 保留／不支援 |

60FEh:01 Physical outputs（rw、RxPDO；02h 是同位置的 bit mask，mask＝0 的輸出當 0 處理，set brake 例外）：

| bit | 訊號 | 0 | 1 |
|---|---|---|---|
| 0 | set brake | 煞車不作動 | **煞車作動** |
| 16 | EX-OUT1 | 輸出電晶體 OFF | ON |
| 19 | vel-loop torque limit | 速度環不限扭力 | 用 4312h 限扭力 |
| 20 | vel-loop integral clear | 不清 | 速度積分一直為 0 |

- 用 set brake 控煞車：**必須用 PDO 且開 PDO watchdog**；要先在 3410h-3412h 指派輸出腳（A6BN p.240）。
- 輸出狀態表（A6BN p.241）：Reset 時與「通訊中斷」（PDO 失效＝ESM 離開 OP，或 SDO 失效＝回 Init）時，set brake 一律＝1（煞車作動）。
- EX-OUT1 在通訊中斷時保持或歸 0，看 3724h bit0（0＝保持、1＝歸 0）（A6BN p.242）。
- 要用 60FEh 就要映射到 RxPDO（A6BN p.242）。

## 3. 1000h 通訊區（A6BN p.58-83、p.301-305）

| Index:Sub | 名稱 | 型別 | 存取 | 重點 | 頁 |
|---|---|---|---|---|---|
| 1000h:00 | Device type | U32 | ro | 固定 00020192h | p.60 |
| 1001h:00 | Error register | U8 | ro | bit4＝AL status 定義的警報（Err80.0-4、80.6-7、81.0-7、85.0-1、85.3）；bit7＝其他（Err85.2、88.0-3 與非通訊警報）；警告不顯示 | p.46、p.60 |
| 1008h:00 | Manufacturer device name | VS（18 byte） | ro | 型號，16 字元補空白 | p.60 |
| 1009h:00 | Manufacturer hardware version | VS | ro | | p.60 |
| 100Ah:00 | Manufacturer software version | VS | ro | software version 3 | p.60 |
| 1010h:01 | Save all parameters | U32 | rw | 寫 65766173h；完成讀回 1 | p.81 |
| 1018h:01-04 | Vendor ID／Product code／Revision／Serial | U32 | ro | Vendor 0000066Fh；serial 若是 A000-Z999 型，bit15-0＝FFFFh，改看 4D15h | p.61 |
| 10F3h:01-13 | Diagnosis history | － | － | 最多 14 筆；見 errors-and-recovery.md §4 | p.82-83 |
| 1600h-1603h | Receive PDO mapping 1-4 | U32 | rw | 屬性 S；PreOP 才能改 | p.64、p.302 |
| 1A00h-1A03h | Transmit PDO mapping 1-4 | U32 | rw | 同上 | p.65、p.303 |
| 1C00h:01-04 | SM communication type | U8 | ro | 1、2、3、4 固定 | p.62 |
| 1C12h:00-04 | SM2 PDO assign | U8／U16 | rw | 1600h-1603h；屬性 S | p.63 |
| 1C13h:00-04 | SM3 PDO assign | U8／U16 | rw | 1A00h-1A03h；屬性 S | p.63 |
| 1C32h | SM2 synchronization | － | － | 見 ethercat-communication.md §4 | p.70-71 |
| 1C33h | SM3 synchronization | － | － | 同上 | p.72-73 |
| 3744h:00 | Software version（CPU1／CPU2） | I32 | ro | bit27-16＝version1、bit11-0＝version2（規格書例：1.23 與 4.56 → 01230456h） | p.61 |

## 4. 3000h 伺服參數（值得知道的）

對應規則（A6BN p.270）：參數 PrX.YY（YY＜100）→ 物件 3XYYh（十進位數字直接當十六進位位數，例 Pr4.10 → 3410h）；YY≥100 時百位與十位的「10」寫成 A（例 Pr7.100 → 37A0h）。參數細節在 SX-DSV03735，本規格書只列表（A6BN p.306-325）。

| Index | 名稱 | 範圍 | 屬性 | 為什麼要知道 | 頁 |
|---|---|---|---|---|---|
| 3001h | Control mode setup | 0-6 | R | | p.306 |
| 3004h | Inertia ratio | 0-10000 % | B | | p.306 |
| 3013h | 1st torque limit | 0-500 % | B | 3521h≠5 時 cst 的扭力上限 | p.214、p.306 |
| 3014h | Position deviation excess setup | 0-1073741824 command | A | 撞機械端回原點要配合調 | p.166、p.306 |
| 3400h-3407h | SI1-SI8 input selection | 0-16777215 | C | HOME→SI5、POT→SI6、NOT→SI7；EXT1→SI5、EXT2→SI6 | p.148、p.216、p.313 |
| 3410h-3412h | SO1-SO3 output selection | 0-16777215 | C | set brake／BRK-OFF 要指派 | p.240、p.313 |
| 3504h | Over-travel inhibit input setup | 0-2 | C | 0 依 3505h、1 依 6085h、2 報 Err38.0 | p.237、p.315 |
| 3505h | Sequence at over-travel inhibit | 0-2 | C | | p.237 |
| 3506h | Sequence at Servo-Off | 0-9 | B | | p.226 |
| 3507h | Sequence upon main power off | 0-9 | B | | p.227 |
| 3508h | L/V trip selection upon main power off | 0-3 | B | | p.227 |
| 3509h | Detection time of main power off | 20-2000 ms | C | | p.227 |
| 3510h | Sequence at alarm | 0-7 | B | Err80-88 以外警報的停止方式；4-7 時「Emergency stop」欄＝Yes 的警報會緊急停止 | p.226、p.273 |
| 3511h | Torque setup for emergency stop | 0-500 % | B | | p.237 |
| 3513h | Over-speed level setup | 0-20000 r/min | B | 折返速度超過 → Err26.0 | p.150 |
| 3521h | Selection of torque limit | 0-5 | B | **＝5 才用 60E0h／60E1h** | p.101、p.214 |
| 3522h | 2nd torque limit | 0-500 % | B | 3521h＝2 或 4 時用 | p.92 |
| 3615h | 2nd over-speed level setup | 0-20000 r/min | B | → Err26.1 | p.150 |
| 3618h | Power-up wait time | 0-100（100 ms） | R | 上電初始化約 1.5 s＋設定×0.1 s | p.53 |
| 3627h | Warning latch state setup | 0-3 | C | 警告 latch 時要 fault reset 才清 | p.299 |
| 3668h | （退避動作警報清除屬性 bit0-2） | － | － | Err87.1-87.3 能不能清（規格書寫成「3668h8」） | p.274、p.320 |
| 3697h | Function expansion setup 3 | I32 | B | bit8：607Fh 也管 tq/cst；bit13：60B9h 反轉輸出 | p.100、p.215 |
| 3698h | Function expansion setup 4 | I32 | R | bit8：csp→hm 直接起動回原點 | p.142 |
| 3703h | Output setup during torque limit | 0-1 | A | tq／cst 判斷扭力限制 | p.92 |
| 3709h／3792h | Correction time of latch delay 1／2 | ±2000（25 ns） | B | touch probe 延遲修正 | p.224 |
| 3722h | Communication function extended setup 1 | I16 | R | bit5 csp 命令飽和；bit6 回原點折返限速；bit7 Z 相回原點碰禁止輸入報警；bit11 LINK 模式 | p.53、p.131、p.149-150 |
| 3723h | Communication function extended setup 2 | I16 | B | bit14 偏差顯示方式 | p.97 |
| 3724h | Communication function extended setup 3 | I16 | C | bit0 EX-OUT1 斷線時保持／歸 0；bit5 latch 延遲分上下緣；bit7 伺服 OFF 時 60B2h 保留；bit11 csp bit12 判斷 | p.101、p.131、p.224、p.242 |
| 3740h | Station Alias setup (high) | 0-255 | R | alias 高 8 位 | p.55 |
| 3741h | Station Alias selection | 0-2 | R | 出廠 1＝用 SII；0＝旋鈕＋3740h | p.55 |
| 3742h | Maximum continuation communication error | I16 | R | bit0-3：Err80.7 門檻（0＝不偵測） | p.282 |
| 3743h | Lost link detection time | 0-32767 ms | R | **出廠 0＝不偵測 Err85.2** | p.291 |
| 3787h | Communication function extended setup 5 | I16 | C | bit13：POT／NOT 減速算不算「不跟命令」 | p.133 |
| 3793h | Homing return speed limit value | 0-20000 r/min | C | | p.150 |
| 3799h | Communication function extended setup 6 | I16 | B | bit0：通訊建立中允許 PANATERM 操作（會出 D2 警告；伺服 ON 由 PANATERM 給時 PDS 不會進 Operation enabled） | p.19 |
| 37B0h | Communication function extended setup 7 | I32 | B | bit7、bit8：Err80.7／80.3 的偵測時機 | p.278、p.281 |
| 3901h | Feedback scale resolution | 0-536870912（0.001 µm/pulse） | R | 決定 608Fh:02 | p.244、p.325 |
| 3910h | Maximum over-speed level | 0-20000（mm/s 或 r/min） | R | 6080h 的上限 | p.100、p.325 |
| 3920h | Magnet pole detection scheme selection | 0-3 | R | | p.325 |

## 5. 4000h 監看／特定物件（A6BN p.256-269、p.326-332）

規格書 §9-3 標「*1) In A6BN series, it is not supported」的不列（4304h、4314h、4315h、4320h、4321h、4351h、4C00h、4D51h-4D53h、4DA0h、4F03h、4F4Bh-4F4Fh、4FC2h；A6BN p.326-332）。

| Index:Sub | 名稱 | 型別／單位 | PDO | 用途 | 頁 |
|---|---|---|---|---|---|
| 4308h | History number | U8 0-3 | No | 選 4DA0h 要看第幾筆（但 4DA0h 在 A6BN 標不支援） | p.256、p.326 |
| 4309h:01-03 | Target position of reference axis1-3 | I32 command | RxPDO | 龍門參考軸目標（csp）；細節規格書未載明 | p.326 |
| 430Ah | Target position of orthogonal axis | I32 command | RxPDO | 同上 | p.326 |
| 4310h | Alarm main no | U8 0-127 | No | 指定要讀哪個主號的子號（配 4F37h:10） | p.256 |
| 4312h | Velocity control loop torque limit | U16 0.1 % | RxPDO | 60FEh bit19 開啟時生效 | p.101 |
| 4D12h／4D15h | Motor／Drive serial number | VS | No | | p.256 |
| 4D29h | Over load factor | U16 0.1 % | TxPDO | 對額定負載比 | p.109、p.256 |
| 4D5Ch | Gantry function expansion setup | U16 | RxPDO | 龍門功能；細節規格書未載明 | p.327 |
| 4F01h | Following error actual value (after filtering) | I32 command | TxPDO | | p.106 |
| 4F04h | Position command internal value (after filtering) | I32 command | TxPDO | | p.106 |
| 4F11h | Regenerative load ratio | I32 % | TxPDO | | p.109 |
| 4F21h／4F22h／4F23h | Logical input／output／input (expansion) | U32 | TxPDO | 邏輯 IO；4F22h bit0 S-RDY、bit1 ALM、bit2 INP、bit3 BRK-OFF、bit4 ZSP、bit5 TLC、bit15 SRV-ST（0＝伺服 ON）；4F21h bit7 E-STOP、bit3 POT、bit2 NOT、bit1 A-CLR（位元是照規格書表格儲存格順序數的，用前上機確認） | p.259-260 |
| 4F25h／4F26h | Physical input／output | U32 | TxPDO | SI1-SI8、SO1-SO3 實體腳 | p.261 |
| **4F33h** | Cause of motor no work | I32 | No | 0 無；1 非 servo ready；2 沒伺服 ON；3 禁止輸入；4 扭力限制太小；7 位置命令太少；10 命令速度 ≤30 r/min；12 命令扭力 ≤5 %；13 6080h ≤30 r/min；14 其他 | p.262 |
| 4F34h | Warning flags | I32 | No | 目前警告（過載、風扇、回生、壽命、主電源 OFF、PANATERM 命令…） | p.263 |
| **4F37h:01-04、10、11、12** | Multiple alarm／warning information | I32 | No | 主號 0-127 的位元圖；4310h 設主號後讀 :10 得子號；:11、:12 警告 A0h-DFh | p.263-265 |
| 4F41h、4F48h、4F0Dh 等 | 編碼器／外部 scale 相關 | － | － | 多數在 A6BN 固定回 0（不支援） | p.106、p.266 |
| 4F49h | External scale absolute position | I32 pulse | TxPDO | feedback scale 絕對位置 | p.106 |
| 4F87h／4F88h | External scale data (Higher／Lower) | I32 pulse | TxPDO | 高／低 24 bit | p.106 |
| 4F61h | Power on cumulative time | I32（30 min） | No | | p.266 |
| 4F62h | Temperature of amplifier | I32 °C | No | | p.266 |
| 4F64h／4F65h | Inrush relay／Dynamic brake operating count | I32 | No | | p.267 |
| 4F66h-4F68h | Fan operating time／Fan life／Capacitor life | I32 | No | | p.267 |
| 4F77h | Lost link error count | U16 | No | | p.330 |
| 4F78h | Synchronization signal error count | U16 | No | SYNC／IRQ 連續漏掉次數 | p.267 |
| 4F83h／4F84h | External scale communication (data) error count | U16 | TxPDO | | p.267 |
| 4F91h-4F94h | 磁極位置估測精度、時間、最大移動量 | － | TxPDO | | p.268 |
| 4FA1h／4FA5h／4FA6h | Velocity command／internal position command／velocity error | I32 r/min | TxPDO | | p.108 |
| 4FA8h／4FA9h | Positive／Negative direction torque limit value | I32 **0.05 %** | TxPDO | 實際生效的扭力上限（注意單位是 0.05 %） | p.109 |
| 4FABh | Gain switching flag | I32 | TxPDO | | p.268 |
| 4FB1h-4FB7h | Deterioration diagnosis | I32 | No | 劣化診斷 | p.269 |
| 4FFFh | Target position echo | I32 command | TxPDO | 回傳 607Ah | p.107 |

## 6. 5000h（A6BN p.333）

| Index | 名稱 | 單位 | 存取 | PDO | 屬性 | 用途 |
|---|---|---|---|---|---|---|
| 5350h | Homing torque limit value | 0.1 % | rw | RxPDO | A | 撞機械端回原點的扭力上限 |
| 5351h | Homing detection time | ms | rw | RxPDO | A | 判定時間 |
| 5352h | Homing detection velocity value | command/s | rw | RxPDO | A | 判定速度；0＝不看速度 |
