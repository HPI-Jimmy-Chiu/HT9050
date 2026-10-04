# SGDXS 物件字典（HT9050 會用到的部分）

> 來源：SIEP C710812 02H 第 15 章（p.625-695）與第 17.2 節物件總表（p.828-846）。「p.」＝02H 的 PDF 頁碼。
> 欄位：存取 RO／RW；PDO＝能否對到 PDO；EE＝「Saving to EEPROM」欄。
> ⚠ 「EE＝Yes」的意思是**這個物件可以被存**，不是寫了就存：用 SDO 寫完要 1010h:1="save" 才會留過斷電（p.177、p.635）。
> 雙軸 SGDXW 的 B 軸物件位址不在 02H（樹註解引用 SIEP C710812 05：B 軸＝A 軸＋0x800，例 603Fh→683Fh），本檔未驗證。

## 1. 通用與通訊物件

| 物件 | 名稱 | 型別 | 單位 | 存取 | PDO | EE | 值／說明 | 頁 |
|---|---|---|---|---|:-:|:-:|---|---|
| 1000h | Device Type | UDINT | – | RO | No | No | 0x00020192（Servo Drive／DS402） | p.634 |
| 1001h | Error Register | USINT | – | RO | No | No | bit0＝Generic error（0 無、1 有）；bit1～7 恆 0；也放在 emergency message byte2 | p.634、p.598 |
| 1008h | Manufacturer Device Name | STRING | – | RO | No | No | 驅動器型號 | p.634 |
| 100Ah | Manufacturer Software Version | STRING | – | RO | No | No | "xxxx.*" | p.634-635 |
| 1010h:1 | Store Parameters（Save all） | UDINT | – | RW | No | No | 寫字串 "save" 才存；簽章錯回 SDO abort；**只能在 Switch ON Disabled 寫**；存完要斷電重開才能進 Operation Enabled；讀到 bit0＝1＝支援命令存檔、存檔中讀到 0 | p.635 |
| 1011h:1 | Restore Default Parameters | UDINT | – | RW | No | No | 寫 "load" 回出廠；只能在 Switch ON Disabled；重置或斷電重開後生效 | p.635-636 |
| 1018h:1～4 | Identity | UDINT | – | RO | No | No | Vendor 0x00000539、Product 0x02200901、Revision（高 16 位主版、低 16 位次版）、Serial 恆 0 | p.636 |
| 10F1h:2 | Sync error count limit | UINT | – | RW | No | Yes | 0～15（預設 9）；超過→A.A12＋ESM 到 SAFEOP；0＝不偵測 | p.649-650 |
| 1600h～1603h | RxPDO mapping | UDINT | – | RW | No | Yes | 1600h 預設 8 筆：6040h、607Ah、60FFh、6071h、6072h、6060h、pad、60B8h | p.638-641 |
| 1A00h～1A03h | TxPDO mapping | UDINT | – | RW | No | Yes | 1A00h：6041h、6064h、6077h、60F4h、6061h、pad、60B9h、60BAh | p.642-645、p.594 |
| 1C12h／1C13h | SM PDO assignment | USINT／UINT | – | RW | No | Yes | 預設各 1 組：0x1601／0x1A01；只在 PRE-OP 可改；先寫 :0＝0 再寫 :1／:2 | p.646 |
| 1C32h／1C33h | SM sync | 多種 | ns | 多種 | No | 部分 | Free-run／DC、Sync0 週期、shift time、missed counter；見 ethercat-cia402 §3 | p.647-649 |

## 2. 廠商物件（2000h～27FFh）

| 物件 | 名稱 | 型別 | 單位 | 存取 | PDO | EE | 值／說明 | 頁 |
|---|---|---|---|---|:-:|:-:|---|---|
| 2000h～26FFh | SERVOPACK 參數 Pn000～Pn6FF | 依參數 | 依參數 | RW | – | – | 2□□□h＝Pn□□□（例 2100h＝Pn100）；用 EtherCAT 寫完要 1010h 存 | p.651、p.177 |
| 2700h | User Parameter Configuration | UDINT | – | RW | No | No | 預設 0。改了 2701h～2704h 或「重啟生效」的 Pn 又不想斷電時：切到 Switch ON Disabled → 設值 → 寫 1；執行完自動回 0 | p.651 |
| 2701h:1／:2 | Position User Unit 分子／分母 | UDINT | – | RW | No | Yes | 1～1073741824（預設 64／1）；1 [Pos. unit]＝:1／:2 [inc]；**比值 0.001～64000**，超出→**A.040** | p.651、p.214 |
| 2702h:1／:2 | Velocity User Unit | UDINT | – | RW | No | Yes | 1～1073741823（預設 64／1）；1 [Vel. unit]＝:1／:2 [inc/s]；比值 1/256～33554432，超出→A.A20 | p.652、p.220 |
| 2703h:1／:2 | Acceleration User Unit | UDINT | – | RW | No | Yes | 1～1073741823（預設 64／1）；1 [Acc. unit]＝:1／:2 ×10⁴ [inc/s²]；比值 1/256～1048576，超出→A.A20 | p.652、p.220 |
| **2704h:1／:2** | **Torque User Unit** | UDINT | – | RW | **No** | Yes | 1～1073741823（**預設 1／10**）；**1 [Trq. unit]＝2704h:1／2704h:2 [%]**（額定扭力的百分比）；**比值 1/256～1**，超出→**A.A20** | p.652、p.221、p.588 |
| 2710h:1～3 | SERVOPACK Adjusting Command | STRING／USINT | – | RW／RO | No | No | :1 寫命令、:2 狀態（0／1 完成、2／3 錯誤、**255 執行中**）、:3 回覆；見 §6 | p.653-655 |
| 2770h | Sensing Data Monitor | 多種 | 多種 | RO | 部分 | No | :3 主迴路 DC 電壓 [V]（PDO）、:4 Un009 累積負載率 [%]、:17 Un14E 距過載餘裕 [0.01 %]（PDO）、:24 Un13F 距過電壓餘裕 [V] 等 | p.660 |
| 2771h | Sensing Data Monitor | 多種 | 多種 | RO | 部分 | No | :4 Un17A 編碼器電源電壓 [0.1 V]、:5 Un17B 編碼器電池電壓 [0.1 V]、:16 Un178 編碼器內建電池剩餘 [%] 等 | p.661 |
| 2772h | Operation Status Monitor | 多種 | 多種 | RO | No | No | 風扇／電容／突波電路／DB 電路剩餘壽命 [0.01 %]、瞬時功率、耗電 | p.661、p.841 |
| 2776h | Controlword_VenderS | UINT | – | RW | Yes | No | bit0 EXT trace、bit1 preset position forced stop、bit12-13 增益切換（0～3＝gain 1～4） | p.662-663 |
| 27E4h | Absolute Encoder Origin Offset | DINT | – | RW | No | Yes | method 35／37 回原點時自動存；上電時與 607Ch 一起加到零點 | p.843、p.615、p.676 |

（2702h:0、2703h:0 等 Number of entries＝2，略。2770h:0 寫 21 筆但列到 :26，手冊原樣，p.660。）

## 3. CiA402 物件（6000h～）

| 物件 | 名稱 | 型別 | 單位 | 存取 | PDO | EE | 範圍（預設）／說明 | 頁 |
|---|---|---|---|---|:-:|:-:|---|---|
| 603Fh | Error Code | UINT | – | RO | Yes | No | 最後一次警報／警告碼（警報與警告都放這裡） | p.665、p.702、p.741 |
| 6040h | Controlword | UINT | – | RW | Yes | No | 0～0xFFFF（0）；位元見 ethercat-cia402 §5.2 | p.665 |
| 6041h | Statusword | UINT | – | RO | Yes | No | 位元見 ethercat-cia402 §5.3 | p.667 |
| 605Ah | Quick Stop Option Code | INT | – | RW | No | Yes | 0～4（2） | p.670 |
| 605Bh | Shutdown Option Code | INT | – | RW | No | Yes | 0～1（0） | p.670 |
| 605Ch | Disable Operation Option Code | INT | – | RW | No | Yes | 0～1（1） | p.671 |
| 605Dh | Halt Option Code | INT | – | RW | No | Yes | −3～3（1） | p.671 |
| 605Eh | Fault Reaction Option Code | INT | – | RW | No | Yes | 0～0（0）＝伺服 OFF | p.672 |
| 6060h | Modes of Operation | SINT | – | RW | Yes | **Yes** | 0～10（0） | p.672 |
| 6061h | Modes of Operation Display | SINT | – | RO | Yes | No | 同 6060h 編碼 | p.672 |
| 6062h | Position Demand Value | DINT | Pos. unit | RO | Yes | No | 目前位置命令 | p.678 |
| 6063h | Position Actual Internal Value | DINT | inc | RO | Yes | No | 回授位置（編碼器脈波） | p.678 |
| 6064h | Position Actual Value | DINT | Pos. unit | RO | Yes | No | 回授位置 | p.678 |
| 6065h | Following Error Window | UDINT | Pos. unit | RW | No | Yes | 0～1073741823（5242880）；超過且持續 6066h → 6041h bit13 | p.678 |
| 6066h | Following Error Time Out | UINT | ms | RW | No | Yes | 0～65535（0） | p.679 |
| 6067h | Position Window | UDINT | Pos. unit | RW | No | Yes | 0～1073741823（30）；到位＋6068h 後 bit10＝1 | p.679 |
| 6068h | Position Window Time | UINT | ms | RW | No | Yes | 0～65535（0） | p.679 |
| 606Bh | Velocity Demand Value | DINT | Vel. unit | RO | Yes | No | 速度迴路輸入 | p.684 |
| 606Ch | Velocity Actual Value | DINT | Vel. unit | RO | Yes | No | 馬達速度 | p.684 |
| 606Dh／606Eh | Velocity Window／Time | UINT | Vel. unit／ms | RW | No | Yes | 20000／0 | p.684 |
| 6071h | Target Torque | INT | Trq. unit | RW | Yes | No | −32768～32767（0） | p.686 |
| 6072h | Max Torque | UINT | Trq. unit | RW | Yes | No | 0～65535（馬達最大扭力）；**上電時自動以「0.1 % 額定扭力」為單位設成馬達最大扭力** | p.688 |
| 6074h | Torque Demand Value | INT | Trq. unit | RO | Yes | No | 目前輸出的扭力命令 | p.686 |
| 6076h | Motor Rated Torque | UDINT | mN·m（線性 mN） | RO | No | No | 額定扭力 | p.686 |
| **6077h** | **Torque Actual Value** | INT | **Trq. unit** | RO | **Yes** | No | **驅動器上＝扭力命令輸出值**（不是量測值） | p.686 |
| 6078h | Current Actual Value | INT | 1/1000 額定電流 | RO | Yes | No | 電流 | p.687 |
| 607Ah | Target Position | DINT | Pos. unit | RW | Yes | No | ±2³¹（0）；PP 依 bit6 絕對／相對，CSP 恆絕對 | p.674 |
| 607Bh:1／:2 | Position Range Limit | DINT | Pos. unit | RW | Yes | Yes | 旋轉座標用（0／0） | p.680 |
| 607Ch | Home Offset | DINT | Pos. unit | RW | No | Yes | −536870912～536870911（0） | p.676 |
| 607Dh:1／:2 | Software Position Limit Min／Max | DINT | Pos. unit | RW | No | Yes | −1073741823～1073741823（0／0）；回原點完成或接絕對編碼器後生效；Min ≥ Max＝關閉 | p.674 |
| 607Fh | Max Profile Velocity | UDINT | Vel. unit | RW | Yes | Yes | 0～4294967295（2147483647） | p.675 |
| 6081h | Profile Velocity | UDINT | Vel. unit | RW | Yes | Yes | （0） | p.675 |
| 6082h | End Velocity | UDINT | Vel. unit | RW | Yes | No | （0） | p.685 |
| 6083h／6084h | Profile Acc／Dec | UDINT | Acc. unit | RW | Yes | Yes | （1000） | p.675 |
| 6085h | Quick Stop Deceleration | UDINT | Acc. unit | RW | Yes | Yes | （1000）；605Ah＝2 時用 | p.675 |
| 6086h | Motion Profile Type | INT | – | RW | Yes | Yes | （0）；2＝S 曲線 | p.605 |
| 6087h | Torque Slope | UDINT | Trq. unit/s | RW | Yes | Yes | （1000） | p.686 |
| 6098h | Homing Method | SINT | – | RW | Yes | **No** | 0～37（**37**） | p.676 |
| 6099h:1／:2 | Homing Speeds | UDINT | Vel. unit | RW | Yes | Yes | （500000／100000） | p.677 |
| 609Ah | Homing Acceleration | UDINT | Acc. unit | RW | Yes | Yes | （1000） | p.677 |
| 60A4h:1 | Profile jerk1 | UDINT | % | RW | No | Yes | 0～50（25） | p.605 |
| 60B0h | Position Offset | DINT | Pos. unit | RW | Yes | No | 寫入時加到目標位置 | p.679 |
| 60B1h／60B2h | Velocity／Torque Offset | DINT／INT | Vel.／Trq. unit | RW | Yes | No | 前饋或偏移 | p.683 |
| 60B8h | Touch probe function | UINT | – | RW | Yes | No | 見 ethercat-cia402 §8 | p.689 |
| 60B9h | Touch probe status | UINT | – | RO | Yes | No | 同上 | p.690 |
| 60BAh～60BDh | Touch probe 1／2 正／負緣 | DINT | Pos. unit | RO | Yes | No | 鎖存位置 | p.690-691 |
| 60C0h／60C1h／60C2h | Interpolation 子模式／資料／週期 | 多種 | – | RW | 部分 | No | 60C2h 插補時間＝:1 ×10^:2 s，最多 4 ms | p.681-682 |
| **60E0h** | **Positive Torque Limit Value** | UINT | **Trq. unit** | RW | Yes | Yes | 0～65535（**8000**） | p.688 |
| **60E1h** | **Negative Torque Limit Value** | UINT | **Trq. unit** | RW | Yes | Yes | 0～65535（**8000**） | p.688 |
| 60E4h:1 | External encoder position | DINT | inc | RO | Yes | Yes | 外部編碼器 | p.680 |
| 60F2h | Position option code | UINT | – | RW | Yes | No | 旋轉座標移動方式（bit6-7） | p.680 |
| 60F4h | Following Error Actual Value | DINT | Pos. unit | RO | Yes | No | 目前位置偏差 | p.679 |
| 60FCh | Position Demand Internal Value | DINT | inc | RO | Yes | No | 軌跡產生器輸出 | p.678 |
| 60FDh | Digital Inputs | UDINT | – | RO | Yes | No | bit0 N-OT、bit1 P-OT、bit2 /Home、bit16-22 SI0-6、bit24-25 HWBB1-2 | p.692 |
| 60FEh:1／:2 | Digital Outputs／Bit mask | UDINT | – | RW | Yes／No | Yes | bit0 /BK、bit17-19 SO1-3；mask 預設 0x000C0000 | p.692-693 |
| 60FFh | Target Velocity | DINT | Vel. unit | RW | Yes | No | （0） | p.685 |
| 6403h | Motor Catalogue Number | STRING | – | RO | No | No | 接上的馬達型號 | p.694 |
| 6502h | Supported Drive Modes | UDINT | – | RO | No | No | 0x03ED | p.672-673 |
| F9F0h | Manufacturer Serial Number | STRING | – | RO | No | No | 驅動器序號 | p.695 |

## 4. 2704h 與扭力換算（Index Z1 會用）

1. 定義：1 [Trq. unit]＝2704h:1／2704h:2 [%]（p.588、p.652）。單位是**額定扭力的百分比**——額定扭力本身用 6076h 讀（mN·m，p.686）。
2. 預設 1／10 ⇒ 1 count＝0.1 %；比值只能在 1/256～1 之間（p.652、p.221）⇒ 1 count 最粗 1 %、最細 1/256 %。
3. 用到 Trq. unit 的物件：6071h、6072h、6074h、**6077h**、60B2h、**60E0h／60E1h**、6087h（[Trq. unit/s]）（p.686、p.688、p.683）。
   換了 2704h，這些物件的數字意義一起變。
4. 換算（推導）：扭力 % ＝ 6077h × 2704h:1 ÷ 2704h:2；扭力 N·m ＝ 該 % ÷ 100 × 6076h ÷ 1000。
5. 2704h 本身 PDO＝No（p.652），要用 SDO 讀；改了要 2700h=1 或斷電重開才生效，要 1010h 才留得住（p.651、p.177）。
6. ⚠ 6072h 的說明寫「上電時以 0.1 % 額定扭力為單位」自動設定（p.688），同表又說單位是 Trq. unit。2704h 不是預設值時開機那個數字怎麼解釋，手冊未載明——要用到時上機讀。
7. 6077h 是**命令值**（送進電流迴路的值），量到的電流是 6078h（p.686-687）。正負號跟驅動器座標走，Pn000 只改實體轉向（p.186）。
   國際牌→安川的比例與正負號規則（Steven Q87）見 [index-torque-autoheight.md](../../ht9045-motor-control/references/index-torque-autoheight.md) §3。
8. Pn402～Pn405（1 % 單位）與 60E0h／60E1h（Trq. unit）一起作用、**取最小者**（p.620）。

## 5. 樹裡實際碰到的物件（唯讀 grep，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT`，行號會漂）

| 物件 | 怎麼用 | 位置 |
|---|---|---|
| 1000h、1008h | 判斷是不是 DS402／名稱含 SGDX、SERVOPACK | `Pci1203Monitor.cpp`（`Acm_DevReadSDOData` 0x1000、0x1008） |
| 603Fh（B 軸 +0x800） | 監看器讀警報碼 | `Pci1203Monitor.cpp`（`0x603F + Pci1203GearAxisBase`） |
| 6076h、6077h | 額定扭力、扭力讀值（給網頁） | `Pci1203Monitor.cpp` |
| 60E0h／60E1h | SDO 寫入扭力上限並讀回 | `Pci1203Control.cpp`（`Acm_DevWriteSDOData`）、`Pci1203Monitor.cpp` |
| 2700h～2704h、1010h | 電子齒輪頁：寫 → 2700h 套用 → 1010h 存 | `Pci1203Gear.h`（三步驟註解）、`Pci1203Control.cpp` |
| 6098h、6099h、609Ah、607Ch | 讀驅動器回原點設定（寫入由 Acm_AxHome 代做） | `Pci1203Gear.h` |
| 2710h | Fn008 絕對編碼器重置（四步序列、輪詢 :2） | `Pci1203Control.cpp`（`kCmdAxAbsEncoderReset`） |
| Pn000、Pn50A／Pn50B、Pn21D | 轉向、超程輸入、編碼器解析度相容 | `Pci1203Control.cpp`（SDO 寫入，站號共用時拒絕） |

## 6. 2710h 調整命令（p.653-655）

可執行的服務（p.654）：

| 服務 | Request code | 要準備 | 時間 | 不能執行的情況 |
|---|---|:-:|---|---|
| Absolute Encoder Reset（Fn008） | 1008h | 要 | 最多 5 s | Pn00C=n.□□□1 且沒接編碼器；接增量編碼器；Pn002=n.□1□□（當增量用）；**伺服 ON 中** |
| Autotune Motor Current Detection Signal Offset | 100Eh | 不要 | 最多 5 s | 主迴路 OFF、伺服 ON、馬達轉動中 |
| Multiturn Limit Setting | 1013h | 要 | 最多 5 s | 同上前三項；**沒有發生 A.CC0 時** |
| Triggers at Preset Positions | 2025h | 要 | 最多 5 s | – |

送法（每一步都是 CCMD=01h、CSIZE=02h 寫進 2710h:1；p.654-655）：
1. CADDRESS=00002000h、CDATA=request code → :2 Status 變 1 才往下；
2. （要準備的）CADDRESS=00002001h、CDATA=0002h；
3. CADDRESS=00002001h、CDATA=0001h 執行；
4. CADDRESS=00002000h、CDATA=0000h 結束。任何一步出錯都要做第 4 步停止。
命令格式：byte2 CCMD（00h 讀、01h 寫）、byte3 CSIZE、byte4-7 CADDRESS、byte8-15 CDATA；回覆 byte0＝狀態、byte8-15＝R_DATA／ERRCODE（p.653）。

## 7. SDO abort code（p.847，摘要）

| 碼 | 意義 |
|---|---|
| 0x05040000 | SDO protocol timeout |
| 0x06010000／01／02 | 不支援的存取／讀了唯寫物件／**寫了唯讀物件** |
| 0x06020000 | 物件不存在 |
| 0x06040041／42 | 物件不能對到 PDO／PDO 長度超過 |
| 0x06070010／12／13 | 資料型別長度不符（太長／太短）——**寬度給錯就會回這個**（例：6098h 是 SINT 1 byte） |
| 0x06090011 | 子索引不存在 |
| 0x06090030／31／32 | 超出範圍／太大／太小 |
| 0x08000020／21／22 | 資料無法傳送或儲存（一般／本地控制／**目前裝置狀態不允許**，例：1010h 不在 Switch ON Disabled） |

（「例」是本檔對照 1010h、6098h 條件推出的常見原因，不是 p.847 原文。）
