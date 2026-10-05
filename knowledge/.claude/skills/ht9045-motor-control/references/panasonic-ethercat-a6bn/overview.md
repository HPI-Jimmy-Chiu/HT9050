> 原為獨立 skill `ht9045-panasonic-ethercat-a6bn`；Steven 20261005 08:0x 要求併入 `ht9045-motor-control` 當參照（本檔＝原 SKILL.md 正文，其餘參照在同資料夾）。

# Panasonic MINAS A6BN — EtherCAT 通訊（HT9045／HT9050）

> 草稿：2026-10-04，依 Steven 要求撰寫。規格書只讀不改；本 skill 不改任何程式。
> 頁碼一律是 **PDF 頁碼**（規格書頁尾印的頁碼＋9，例如印 79＝PDF p.88）。
> 原檔：`E:\HT9045W_相關料件技術文件\EtherCat\mn_minas_a6bn_ethercat_communication_specification_pid_en.pdf`
> 文字抽取（有 `=== PAGE n ===` 標記）：`D:\AI_TempFile\panasonic-skill\a6bn_ecat_en.txt`

## 0. 先記住

1. **今天沒有任何 HT9045／HT9050 機台用 A6BN（或任何 Panasonic EtherCAT 軸）**，證據在 §2。
   這份 skill 是「將來換／加 Panasonic EtherCAT 軸時」的準備資料。
2. **A6BN 是線性馬達的龍門控制型**：規格書表列 A6BL（線性／DD 標準型）、A6BM（多功能型）、
   A6BN（龍門控制型，型號尾碼 N），「Gantry control type」只支援 Linear type（A6BN p.10、p.14、p.244）。
   規格書的 rotary 用語（torque、r/min）在線性馬達要換成 thrust、mm/s（A6BN p.14）。
3. **A6BN 不支援 pp 模式**（可以設但不保證動作），ip 也不支援；csp／hm／pv／csv／tq／cst 支援（A6BN p.10、p.11、p.93）。
   PCI-1203 的運動走 csp（研華簡報 slide 8），回原點走驅動器的 hm（`ht9050-1203-homing`）。
4. 一般旋轉型 A6 EtherCAT（例如鴻勁評估表裡的 `MADLN05BE`、`MADLN15BE`、`MCDLN35BE`）**不在這本規格書的適用範圍**；
   這本只寫 A6BL／A6BM／A6BN（A6BN p.10）。換旋轉型要另找它自己的通訊規格書。
   （規格書 1008h 的範例字串剛好是 `MADLN15BE`，A6BN p.60，只是格式示範。）
5. 規格書本身有幾處前後不一致（週期 125 µs、DC 32/64 bit、Err81.0 的週期清單…），見 §6；以「物件定義頁」為準，並上機確認。

## 1. 這顆驅動器（只列跟主站有關的）

| 項目 | 內容 | 出處 |
|---|---|---|
| 適用軟體版本 | CPU1／CPU2 Ver3.20、Manufacture software Ver1.00 | A6BN p.11 |
| 實體層 | 100BASE-TX、2 個 RJ45、線型拓樸、節點間最長 100 m | A6BN p.26、p.27 |
| Vendor ID | `0000066Fh`（1018h:01、SII 0008h） | A6BN p.36、p.61 |
| Device type 1000h | 固定 `00020192h`（低 16 位＝0192h＝402） | A6BN p.60 |
| Product code 1018h:02 | 依型號；bit31-28＝6 表示 A6BN 系列（A5BL＝5 或 D） | A6BN p.61 |
| ESC | 12 KB 位址空間（4 KB 暫存器＋8 KB process RAM）、FMMU 3、SyncManager 4 | A6BN p.27、p.32 |
| 同步模式 | DC（SYNC0）、SM2、Free Run | A6BN p.37 |
| 通訊週期 | 250 µs、500 µs、1、2、4、8、10 ms（其他值→Err81.0）；不一致處見 §6 | A6BN p.70-71 |
| PDO | 可自由映射；RxPDO／TxPDO 各最多 4 張表、各最多 32 byte | A6BN p.27、p.49-50 |
| SDO | 支援 Request／Response／SDO information／Emergency；**不支援 Complete Access** | A6BN p.27 |
| Touch probe | 2 通道，上升／下降緣 | A6BN p.27 |
| Station alias | 0-65535；預設取 SII 0004h（出廠 0），改用面板旋鈕要先寫 3741h=0 | A6BN p.15、p.55 |

## 2. HT9045／HT9050 現在有沒有用 A6BN？——沒有

| 證據 | 位置 |
|---|---|
| HT9050 的 EtherCAT ring 0 實測清單：14 站全是安川 SGDXS／SGDXW 與 SW3D-680 步進驅動器 | 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.h:584-590`；軸數 6×SGDXW(2 軸)+3×SGDXS+5×SW3D-680＝20（同檔 :1672） |
| 硬體表與機台馬達表：伺服全是安川 SGDX／SGMX，步進另列 | `D:\HT9045\.claude\skills\ht9050-hw\references\motors-9050.md` §2 |
| HT9050 機台馬達表 `Enable=1` 的 19 列 CardModel 全是 `PCI1203`（表沒有廠牌欄） | `D:\HT9045\machines\HT9050\Mot_Table.csv` |
| HT9045 的 `D:\HT9045\system\Mot_Table.csv`：CardModel 只有 SMC 24 列、MN200 20 列，**沒有 PCI1203** | 同檔第 23 欄 |
| 驅動器類型判斷只認 SGDX 名稱、CoE profile 402、名稱含 SERVOPACK；沒有 Panasonic 分支 | `Pci1203MotorRoute.cpp:859-878` `Pci1203MotorRouteDriveKind()` |
| 移植樹、golden V912（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy`）、golden 0618 的 EtherCAT 程式都找不到 `A6BN`／`MINAS`／`066F`／`MADLN` | 唯讀 grep，20261004 |

HT9045 golden **有**用 Panasonic，但那是 Index Z 的 **MINAS A4／A5 走 RS-232**（讀扭力、寫扭力上限；移動走 Galil），
不是 EtherCAT：`INDEX_DRIVER_TYPE`／`Panasonic_DRIVER`（移植樹 `cmydef.h:93-95`、`rs232.cpp:219-226`），
細節見 `D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md` §1
與兄弟草稿 `D:\AI_TempFile\skill-drafts\ht9045-panasonic-rs232\`。
另有 `E:\HT9045W_相關料件技術文件\EtherCat\EtherCat評估.xlsx`：HT-3309 的 EtherCAT 導入成本評估，
研華方案用 PCI-1203＋`MADLN05BE`／`MADLN15BE`／`MCDLN35BE`（旋轉型 A6 EtherCAT），只是評估，不是 HT9045／HT9050。

## 3. 如果掛到 PCI-1203 底下（推論，未上機驗證）

| 會發生的事 | 依據 |
|---|---|
| `Pci1203MotorRouteDriveKind()` 會回 1（DS402、驅動器回原點），因為 1000h 報 402 | A6BN p.60；`Pci1203MotorRoute.cpp:864-871` |
| 全機／單軸 HOME 送 `Acm_AxHome(124／128)`＝method 24／28；A6BN 支援這兩個（60E3h 清單） | A6BN p.147；`Pci1203MotorRoute.cpp:638` |
| 但 method 24 要 HOME 接 SI5 且 POT 有配；method 28 要 HOME 接 SI5 且 NOT 有配，否則 homing error | A6BN p.148、p.162、p.163 |
| 回完的位置＝607Ch（home offset），不是固定 0；樹的 HOME-READYDONE 用「cmd==0 才算成功」只在 607Ch=0 時成立 | A6BN p.148、p.253；`ht9050-1203-homing` §3c |
| 6098h／6099h／609Ah 在 A6BN 有存 EEPROM（安川 6098h 不存） | A6BN p.143-144、p.335；`Pci1203Gear.h:542-545` |
| 603Fh 讀得到（FFxxh＝主警報號），但面板不會顯示名稱：名稱表只給 SGDX | A6BN p.297；`Pci1203Monitor.cpp:2803`、`:2816` |
| 60E0h／60E1h 扭力上限在 A6BN **只有 3521h=5 才生效**；樹的 Index Z 扭力上限就是寫 60E0h／60E1h | A6BN p.101、p.214；`Pci1203Control.cpp:3350-3352` |
| 6077h 也是 0.1 %，但 A6BN 說它「相當於實際電流值、只是參考值」；安川的 6077h 是扭力命令值，換算不能直接套 | A6BN p.109；`index-torque-autoheight.md` §2 |
| 研華預設 PDO 把 `60FE` sub `00` 32 bit 放進 RxPDO；A6BN 的 60FEh:00h 是 U8「Number of entries」不能映射，可映射的是 60FEh:01h／02h → 可能開卡失敗，要問研華／EastSun | 研華簡報 slide 14；A6BN p.240、p.337 |
| 出廠 station alias 都是 0（SII），多台 A6BN 一上線就同號，跟 HT9050 SW3D 撞號是同一類問題 | A6BN p.15、p.55；`Pci1203Monitor.h:575-595` |
| 伺服 ON（Operation enabled）時換 ESM 狀態（例如重掃／重開 ring）→ Err88.2，PDS 進 Fault | A6BN p.31、p.293 |
| 研華：所有從站都要到 OP 卡片才開得起來，否則 `Dev_Open failed` | 研華簡報 slide 7 |

研華簡報：`E:\HT9045W_相關料件技術文件\EtherCat\PCI-1203_EtherCAT機制簡介_20220211.pptx`。

## 4. 最常用的物件（速查；完整表在 object-dictionary.md）

| Index | 名稱 | 型別／單位 | 存取／PDO | 重點 | 頁 |
|---|---|---|---|---|---|
| 6040h | Controlword | U16 | rw／RxPDO | 要走 PDO 並開 PDO watchdog；bit7 fault reset；bit8 halt；bit11 disable correction（A6BN） | p.88-89 |
| 6041h | Statusword | U16 | ro／TxPDO | bit3 fault、bit7 warning、bit11 internal limit active、bit12/13 依模式 | p.90-92 |
| 6060h | Modes of operation | I8 | rw／RxPDO | 開機預設 0，一定要設；0 狀態下 servo ON→Err88.1 | p.94、p.96 |
| 6061h | Modes of operation display | I8 | ro／TxPDO | 切模式約 2 ms 才生效 | p.95-96 |
| 603Fh | Error code | U16 | ro／TxPDO | FFxxh，xx＝主警報號（16 進位）；讀不到子號；Err81.7 例外＝A000h | p.46、p.297 |
| 607Ah | Target position | I32 command | rw／RxPDO | csp 每個週期更新 | p.100、p.134 |
| 6064h | Position actual value | I32 command | ro／TxPDO | 伺服 OFF 時主站要讓命令跟著它 | p.107、p.134 |
| 606Ch | Velocity actual value | I32 command/s | ro／TxPDO | | p.108 |
| 6077h | Torque actual value | I16 0.1 % | ro／TxPDO | 相當於實際電流；參考值、不保證 | p.109 |
| 6072h | Max torque | U16 0.1 % | rw／RxPDO | 上限由 3907h／3906h 算出 | p.101 |
| 60E0h／60E1h | Positive／Negative torque limit | U16 0.1 % | rw／RxPDO | 只在 3521h=5 生效 | p.101 |
| 607Ch | Home offset | I32 command | rw／RxPDO | 回原點完成後 6062h=6064h=607Ch | p.148、p.253 |
| 6098h | Homing method | I8 | rw | 支援 1-14、17-30、33、34、35、37、-1~-4 | p.143、p.147 |
| 6099h:01／02 | Homing speeds | U32 command/s | rw／RxPDO | 找開關／找零點速度；0→homing error | p.144、p.151 |
| 609Ah | Homing acceleration | U32 command/s² | rw／RxPDO | 加減速共用 | p.144 |
| 60B8h／60B9h | Touch probe function／status | U16 | rw RxPDO／ro TxPDO | EXT1=SI5、EXT2=SI6 或 Z 相 | p.215-219 |
| 60FDh | Digital inputs | U32 | ro／TxPDO | bit0 NOT、bit1 POT、bit2 HOME | p.239 |
| 60FEh:01 | Physical outputs | U32 | rw／RxPDO | bit0 set brake（1＝煞車作動）；要 PDO＋watchdog | p.240 |
| 605Eh | Fault reaction option code | I16 | rw | 只管 Err80-88 的減速；其他警報照 3510h | p.236 |
| 1C32h:01／02 | Sync mode／Cycle time | U16／U32 ns | rw | 0 Free Run、1 SM2、2 DC SYNC0 | p.70 |
| 1010h:01 | Save all parameters | U32 | rw | 寫 `65766173h`（"save"）存 EEPROM，最長約 10 s | p.81 |
| 4F33h | Cause of motor no work | I32 | ro | 馬達不動的原因碼，除錯先看 | p.262 |

## 5. 重置／復歸規則摘要（規格書 × Steven 的機台規則）

Steven 的規則（1003 Q88／Q89，`index-torque-autoheight.md` §5；以及 IO／馬達 ERROR 不准動、停止永遠不擋）對到 A6BN：

| Steven 規則 | A6BN 規格書怎麼說 |
|---|---|
| 斷電後必須歸零 | 增量式 feedback scale：控制電源 ON、ESM Init→PreOP 都會重設位置資訊，6041h bit12（homing attained）歸 0（p.146、p.243）；軟體極限在 Init→PreOP 後失效，要重新回原點（p.102）。**EtherCAT 重新建立通訊也算，一樣要重新歸零。** 絕對式 scale 不需要也不准回原點（啟動就 homing error，p.151、p.251）——規則要另外決定，見 errors-and-recovery.md §6。 |
| 警報可 clear → clear 後該軸重新歸零 | 「Clearable」欄＝Yes 的才清得掉（p.271-273）；清法：AL Control bit4（Error Ind Ack）=1，再送 6040h bit7 0→1，PDS 回 Switch on disabled（p.298）。Fault reaction active 期間清不掉（p.298）。Err27.4 會重設位置資訊（p.243），一定要重歸零。 |
| 警報清不掉 → 整機斷電重開再歸零 | 「Clearable」＝No 的，fault reset 後仍保留（p.298）；例：Err85.3 SII EEPROM、Err88.3、Err27.7、Err14.x 等（p.271-273）。Err88.3（電子齒輪）規格書直接寫「重新送控制電源」（p.296）。 |
| 先伺服 ON 才放煞車 | 用 60FEh:01 bit0 控煞車時，0＝不作動（放開）、1＝作動；一定要走 PDO＋PDO watchdog（p.240）；通訊中斷時一律 set brake=1（p.241）。Operation enabled 後 100 ms 以上才送運轉命令（p.86）。BRK-OFF 時序在 SX-DSV03735 §9-2，本規格書未載明。 |
| IO／馬達裝置 ERROR → 機台不准動；停止永遠不擋 | 驅動器進 Fault 後 drive function disabled（p.87 event 14）；Halt（6040h bit8）、Quick stop（bit2=0）隨時可下（p.88）。注意：回原點在「偵測到原點到完成」之間被 halt 中斷 → **Err27.7，不可 clear**（p.149、p.272），等於要整機斷電重開——停止照樣要放行，但要知道代價。 |

## 6. 規格書內部不一致（用之前先知道）

| 項目 | 一處寫 | 另一處寫 |
|---|---|---|
| 最短週期 | 規格表 125 µs（p.27）、DC／SM2 表列 125 µs（p.74、p.76） | A6BN 欄 250 µs 起（p.20）；1C32h:05＝250000、可設值 250 µs-10 ms（p.70-71） |
| Err81.0 合法週期 | 250 µs-4 ms（p.283） | 250 µs-10 ms（p.70-71） |
| DC 位元數 | 「DC 32bit」（p.27） | 64 bits（p.20、p.38） |
| 60C2h 自動值 | 只列 250 µs-4 ms（p.255） | 週期可到 10 ms（p.70） |
| 6098h PDO | 「No」（p.143） | 「RxPDO」（p.335） |
| 4DA0h Alarm accessory information | 有完整說明（p.257-258） | 物件表標「A6BN 不支援」（p.328） |
| 603Fh 警告號清單 | A0h-A9h、ACh、C3h、D2h、D3h（p.46） | A0h-A9h、C3h、D2h、D3h（p.297） |

## 7. 要查什麼，讀哪一份

| 要做的事 | 讀 |
|---|---|
| ESM、SM／FMMU、同步模式與週期、預設 PDO、PDO 改法、SDO／mailbox、Emergency、通訊錯誤 | [ethercat-communication.md](ethercat-communication.md) |
| Controlword／Statusword、PDS 狀態機、6060h 模式、回原點 method、touch probe、quick stop／halt／fault reaction | [cia402-and-modes.md](cia402-and-modes.md) |
| 6000h 物件完整表（型別、單位、存取、PDO、EEPROM、屬性）、3000h 參數對應、4000h 監看物件 | [object-dictionary.md](object-dictionary.md) |
| Err 碼總表（Clearable／Emergency stop／History／ERR 燈／AL Status Code）、603Fh、清警報、警告、對應 Steven 規則 | [errors-and-recovery.md](errors-and-recovery.md) |

## 8. 相關 skill

- `D:\HT9045\.claude\skills\ht9045-motor-control\SKILL.md`：馬達類別；`references\ethercat-api.md`（TMyEtherCatMotor／AdvMotApi）、
  `references\ht9050-1203-runtime-traps.md`（golden 流程跑在 1203 上的陷阱）、`references\index-torque-autoheight.md`（扭力 6077h 換算、Steven 規則原話）。
- `D:\HT9045\.claude\skills\ht9050-1203-homing\SKILL.md`：PCI-1203 上的回原點是驅動器做的；§5「新增驅動器型號的檢查清單」換 A6BN 時要逐條做。
- `D:\HT9045\.claude\skills\ht9050-hw\references\motors-9050.md`：HT9050 每一軸的驅動器型號。
- 兄弟草稿 `D:\AI_TempFile\skill-drafts\ht9045-panasonic-rs232\`：golden 的 MINAS A4／A5 RS-232（Index Z 扭力）。
- `ht9045-motor-control/references/yaskawa-ethercat/`：安川 Σ-X SGDXS（HT9050 實際在用的驅動器）的 EtherCAT／CiA402、物件字典、警報與重置；兩邊的 6077h 意義不同（安川＝扭力命令值×2704h，A6BN＝相當於實際電流的參考值），本 skill 不重複。
- `ht9045-motor-control/references/panasonic-rs232/`：HT9045 Index Z 用的 Panasonic MINAS A4／A5 是走 RS232（不是 EtherCAT），讀扭力等驅動器內部資料看那一份。

## 9. 改這塊的規矩

- 機台端不自己動馬達；接 A6BN 前先在 1203 監看頁唯讀量（1000h、1018h、6502h、60E3h、3741h、603Fh），動馬達要 EastSun 在旁邊（`ht9050-1203-homing` §7）。
- 規格書沒寫的不要猜；寫「規格書未載明」並列進 errors-and-recovery.md §7 的待確認清單。
