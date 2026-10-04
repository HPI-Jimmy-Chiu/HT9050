---
name: ht9050-yaskawa-ethercat
description: >-
  安川 Σ-X 系列 SGDXS 伺服驅動器（EtherCAT／CoE／CiA402）手冊知識庫，給 HT9050（研華 PCI-1203 EtherCAT 主站、
  Acm_* API）用：ESM 狀態與 DC 同步、預設 PDO、6040h／6041h 狀態機與位元、6060h 支援的模式、
  回原點 6098h（24／28）與 6041h bit12／13、touch probe、物件字典與使用者單位 2700h～2704h
  （6077h 扭力換算一定要看 2704h）、A.xxx 警報表與「能不能 Reset」、603Fh 錯誤碼、警告 A.9xx、
  HWBB、煞車 /BK 時序、Pn 參數、絕對編碼器重置（Fn008／2710h）、SGDXS／SGMXJ／SGMXA 型號解碼。
  每一條都標手冊頁碼（SIEP C710812 02H、型錄 KAEP03、周邊 12K）；手冊沒寫的標「手冊未載明」。
  Use when：讀寫安川驅動器物件或 Pn 參數、解讀 603Fh／A.xxx、判斷警報清不清得掉、決定清警報後要不要重歸零、
  6077h／60E0h 扭力單位換算、回原點方法與完成旗標、伺服 ON 與放煞車順序、HWBB／安全門、A.810／A.CC0、
  A.A12（EtherCAT 同步）、看懂驅動器或馬達型號、要找某個安川物件的型別／單位／能不能 PDO。
  關鍵字：SGDXS, SGDXW, Σ-X, Sigma-X, SERVOPACK, 安川, Yaskawa, EtherCAT, CoE, CiA402, DS402, ESM, PDO,
  1600h, 1A00h, 1C12h, DC, Sync0, 10F1h, 6040h, 6041h, 603Fh, 6060h, 6061h, 6064h, 6072h, 6076h, 6077h,
  60E0h, 60E1h, 2700h, 2701h, 2702h, 2703h, 2704h, 1010h, 2710h, 6098h, 6099h, 609Ah, 607Ch, 607Dh, 6502h,
  60B8h, 60B9h, touch probe, 60FDh, 60FEh, A.xxx, A.A12, A.A10, A.810, A.830, A.CC0, A.d00, A.d04, A.Eb1,
  A.9A0, fault reset, 警報清除, 斷電重開, HWBB, /BK, 煞車, brake, Pn000, Pn001, Pn002, Pn402, Pn403,
  Pn404, Pn506, Pn50A, Pn511, Fn008, SGMXJ, SGMXA, 型號解碼。
---

# 安川 Σ-X SGDXS（EtherCAT）— HT9050 用手冊知識

> 草稿，20261004，St01 子代理依手冊整理；**只寫手冊說的事，機台實測與裁決一律連到別的 skill**。
> 頁碼一律是 PDF 頁碼（02H 的 PDF 頁碼＝印刷頁碼）。引用格式：「(02H p.612)」「(KAEP03 p.41)」「(12K p.401)」。

## 0. 來源

| 簡稱 | 文件 | 原檔 |
|---|---|---|
| 02H | SIEP C710812 02H《Σ-X-Series SGDXS SERVOPACK with EtherCAT Communications References Product Manual》878 頁 | `E:\HT9045W_相關料件技術文件\EtherCat\安川馬達\siepc71081202h_7_0.pdf` |
| KAEP03 | YAI-KAEPC71081203 Σ-X 型錄（馬達家族、型號、規格） | `E:\HT9045W_相關料件技術文件\EtherCat\安川馬達\YAI-KAEPC71081203.pdf` |
| 12K | SIEP C710812 12K 周邊裝置／線材 | `E:\HT9045W_相關料件技術文件\EtherCat\安川馬達\siepc71081212k_10_0.pdf` |

文字擷取（每頁一個 `=== PAGE n ===`）：`D:\AI_TempFile\yaskawa-skill\*.txt`，用 grep 找，不要整份讀。
⚠ **雙軸 SGDXW 的手冊（SIEP C710812 05）不在這三份裡**。樹裡的註解引用它（例：B 軸物件＝A 軸＋0x800，
`EtherCAT\Pci1203Monitor.cpp` 讀 603Fh 那行），本 skill 沒有驗證那份手冊。樹註解的頁碼跟 02H 頁碼不同版，
引用時以「章節號」對照，不要混用頁碼。

## 1. 這顆驅動器在 HT9050 的位置

- SGDXS＝單軸 Σ-X 驅動器，EtherCAT 介面型號第 5～6 碼是 `A0`（KAEP03 p.45）；遵循 IEC 61158 Type 12 與
  IEC 61800-7 CiA402 drive profile，100BASE-TX，CN6A＝IN、CN6B＝OUT（02H p.79）。站間線長 ≤ 50 m（02H p.166）。
- 身分：1000h＝0x00020192（伺服、DS402）、1018h Vendor ID＝0x00000539、Product code＝0x02200901（02H p.634、p.636）。
- 主站是研華 PCI-1203（Acm_* API）：**伺服 ON、模式切換、PDO、回原點命令都是卡片代送**；HT9050 的軸、站號、
  哪些是 SGDXW／SGDXS 看 [motors-9050.md](../ht9050-hw/references/motors-9050.md)。
- **回原點是驅動器自己做（method 24／28）**，卡片只轉命令——細節與機台實測全在
  [ht9050-1203-homing](../ht9050-1203-homing/SKILL.md)，本 skill 只補手冊面。

## 2. 物件速查（HT9045 樹最常碰的）

| 物件 | 名稱 | 型別／單位 | PDO | 存 EEPROM | 重點 | 頁 |
|---|---|---|:-:|:-:|---|---|
| 6040h | Controlword | UINT | Yes | No | bit0～3 狀態命令；**bit7 0→1＝清警報／警告**；bit8 Halt；bit11／12 開 Pn404／Pn405 | 02H p.665-667 |
| 6041h | Statusword | UINT RO | Yes | No | bit3 Fault、bit7 Warning、bit10 Target reached、**回原點時 bit12 attained／bit13 error**、bit14 扭力限制中 | 02H p.667-669 |
| 603Fh | Error Code | UINT RO | Yes | No | 最後一個警報／警告碼 | 02H p.665 |
| 6060h／6061h | Modes of Operation／Display | SINT | Yes | Yes／No | 1 PP、3 PV、4 TQ、6 Homing、7 IP、8 CSP、9 CSV、10 CST | 02H p.672 |
| 6502h | Supported Drive Modes | UDINT RO | No | No | 0x03ED（無 vl 模式；bit5 hm＝1） | 02H p.672-673 |
| 6064h | Position Actual Value | DINT RO [Pos. unit] | Yes | No | 回授位置（6063h 是 inc） | 02H p.678 |
| 6077h | Torque Actual Value | INT RO [Trq. unit] | Yes | No | **驅動器上＝扭力命令輸出值**，不是量測值 | 02H p.686 |
| 6076h | Motor Rated Torque | UDINT RO [mN·m] | No | No | 額定扭力 | 02H p.686 |
| 6072h | Max Torque | UINT [Trq. unit] | Yes | No | 開機自動設成馬達最大扭力 | 02H p.688 |
| 60E0h／60E1h | Pos／Neg Torque Limit | UINT [Trq. unit] | Yes | Yes | 預設 8000；與 6072h、Pn402～405 取最小者生效 | 02H p.688、p.620 |
| 2704h:1／:2 | Torque User Unit | UDINT | No | Yes | **1 Trq. unit＝2704h:1／2704h:2 %**，預設 1／10＝0.1 % | 02H p.652、p.588 |
| 2701h～2703h | Pos／Vel／Acc User Unit | UDINT | No | Yes | 電子齒輪與單位（預設 64／1） | 02H p.651-652 |
| 2700h | User Parameter Configuration | UDINT | No | No | Switch ON Disabled 時寫 1＝新單位與「重啟生效」參數立即生效 | 02H p.651 |
| 1010h:1 | Store Parameters | UDINT | No | No | 寫 "save" 才會存 EEPROM；只能在 Switch ON Disabled 寫；存完要斷電重開才能 Operation Enabled | 02H p.635、p.177 |
| 6098h | Homing Method | SINT | Yes | **No** | 0～37，預設 **37**（目前位置當原點）；24／28＝/Home 開關 | 02H p.613-615、p.676 |
| 6099h:1／:2、609Ah | Homing Speeds／Acc | UDINT | Yes | Yes | 預設 500000／100000 [Vel. unit]、1000 [Acc. unit] | 02H p.677 |
| 607Ch | Home Offset | DINT | No | Yes | 原點偏移 | 02H p.676 |
| 2710h | SERVOPACK Adjusting Command | — | No | No | 絕對編碼器重置 1008h、多圈上限 1013h | 02H p.653-655 |
| 60FDh | Digital Inputs | UDINT RO | Yes | No | bit0 N-OT、bit1 P-OT、bit2 /Home、bit24／25 HWBB1／2 | 02H p.692 |

完整清單（含 605Ah～605Eh、607Dh、60B8h～60BBh、60FEh…）→ [references/object-dictionary.md](references/object-dictionary.md)。

## 3. 重置／恢復規則摘要（手冊 × Steven 常設規則）

| 狀況 | 手冊怎麼說 | HT9050 怎麼做（Steven） |
|---|---|---|
| 警報「Alarm Reset Possibility＝Yes」 | 排除原因後用 Fault Reset（6040h bit7 0→1）清（02H p.702、p.734、p.665） | 清一次；清掉的軸**重新歸零**（Q88、Q98） |
| 警報「＝No」 | 不能清（02H p.702） | **全機斷電重開後歸零**（Q88、Q98） |
| A.810／A.820 | Fault Reset 清不掉，要做絕對編碼器重置（02H p.222） | 屬「No」；重置後多圈資料變 −2～+2 圈（02H p.222），之後一定要重新歸零 |
| A.A10／A.EA2 | 表上 Yes，但處置寫「斷電重開、重新建立通訊」（02H p.721、p.732） | ⚠ 待 Steven 確認歸哪一類（見 alarms 檔 §8） |
| A.A10／A.A12／A.EA2 發生 | ESM 掉到 SAFEOP（02H p.702）；Ready to Switch ON→Switched ON 要 ESM 在 OP（02H p.600） | 主站要先回 OP 才能再伺服 ON |
| 主迴路斷電／HWBB | Operation Enabled 自動回 Switch ON Disabled（02H p.600 *4） | 斷電＝重新歸零（Q88）；電回來不可自動續做 |
| 伺服 ON 與煞車 | 送 Enable Operation 後至少等 50 ms＋煞車放開延遲再下命令（02H p.206） | **先伺服 ON 再放煞車**（Q89） |
| 驅動器 Fault | 605Eh 只有 0＝伺服 OFF（02H p.672） | IO／馬達裝置 ERROR＝機台不可動；**停止永遠不擋**（Q96） |

裁決原文：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`（Q88、Q89 在
「20261003 22:5x」；Q96 在「20261004 07:4x」；Q98 在「20261004 17:0x」）。完整對照與灰色地帶 →
[references/alarms-and-recovery.md](references/alarms-and-recovery.md) §8。

## 4. 最容易踩的事

1. **6077h 的單位不是固定的**：Trq. unit＝2704h:1／2704h:2 %；預設 0.1 %，但 2704h 存 EEPROM、可能被改過，
   讀扭力前先讀 2704h（02H p.652、p.588）。換算與正負號裁決看
   [index-torque-autoheight.md](../ht9045-motor-control/references/index-torque-autoheight.md)。
2. **寫物件只進 RAM**：Pn（2000h～26FFh）與 2701h～2704h 用 SDO 寫完要 1010h:1="save" 才會留過斷電
   （02H p.177、p.635）；「重啟生效」的要 2700h=1 或斷電重開（02H p.651）。
3. **已對到 RxPDO 的物件用 SDO 寫不進去**（02H p.320 註）。PCI-1203 實際對哪些物件到 PDO，安川手冊未載明。
4. **6098h 不存 EEPROM、開機預設 37**（目前位置當原點）——主站每次 home 前都要寫（02H p.676）。
5. 回原點 24／28 開關在 /Home（預設 CN1-12，Pn511 n.X□□□）；超程中斷規則反直覺：method 28 遇 P-OT、
   method 24 遇 N-OT 才報 homing error（02H p.204）。超程警報開著（Pn00D=n.2□□□）時不能用限位開關回原點（02H p.201、p.677）。
6. 回原點完成的正式證據是 6041h bit12＝1、bit13＝0、bit10＝1（02H p.669）；樹目前不讀（見 homing skill §5-9）。
7. 煞車：/BK 由驅動器控時，警報一發生馬達立刻斷電、不管 Pn506（02H p.207）；超程時 /BK 保持放開（02H p.206）——
   垂直軸要設 Pn001=n.□□1□ 零位鎖定（02H p.199）。用 60FEh 手動放 /BK 時伺服 OFF 也不會自動煞（02H p.693）。
8. Fault Reset 不會清掉警報履歷；同一警報一小時內連發只記一次（02H p.736）。

## 5. 讀哪一份

| 要做的事 | 讀 |
|---|---|
| ESM、DC／Free-run、PDO、狀態機轉移表、6060h 模式、回原點方法、touch probe | [references/ethercat-cia402.md](references/ethercat-cia402.md) |
| 某個物件的型別／單位／存取／能否 PDO／預設值／頁碼；2704h 換算；SDO abort code | [references/object-dictionary.md](references/object-dictionary.md) |
| A.xxx 意義、停止方式、能不能 Reset、603Fh、警告 A.9xx、履歷、對應 Steven 規則 | [references/alarms-and-recovery.md](references/alarms-and-recovery.md) |
| Pn000 轉向、電子齒輪、煞車 Pn506～508、超程、扭力限制、軟體極限、絕對編碼器、HWBB | [references/parameters-and-functions.md](references/parameters-and-functions.md) |
| 驅動器／馬達型號解碼、額定扭力、煞車與編碼器選項、線材 | [references/hardware.md](references/hardware.md) |

## 6. 相關 skill（不重複寫）

- [ht9050-1203-homing](../ht9050-1203-homing/SKILL.md)：Acm_AxHome 124／128、速度怎麼進 6099h／609Ah、READYDONE、步進差異。
- [index-torque-autoheight.md](../ht9045-motor-control/references/index-torque-autoheight.md)：Index Z1 扭力 6077h×2704h、正負號（Q87）、歸零／伺服 ON／煞車規則（Q88／Q89）。
- [ht9050-1203-runtime-traps.md](../ht9045-motor-control/references/ht9050-1203-runtime-traps.md)：ERROR_STOP 不是 READY、驅動器錯誤碼沒帶進框、座標改寫被拒。
- [ethercat-api.md](../ht9045-motor-control/references/ethercat-api.md)：TMyEtherCatMotor／Acm_* 介面。
- [motors-9050.md](../ht9050-hw/references/motors-9050.md)：哪幾軸是 SGDXW／SGDXS、煞車軸、雙軸配對。
- [ht9045-panasonic-ethercat-a6bn](../ht9045-panasonic-ethercat-a6bn/SKILL.md)：Panasonic MINAS A6BN（線性馬達龍門型）EtherCAT；目前沒有機台在用，6077h 意義跟安川不同。
- [ht9045-panasonic-rs232](../ht9045-panasonic-rs232/SKILL.md)：HT9045 Index Z 的 Panasonic A4／A5 走 RS232 讀扭力（golden rs232.cpp），HT9050 改走 1203 的 6077h 前的原始做法。

## 7. 手冊沒寫、要上機或問人的

- 603Fh 的數值編碼（例：A.A12 是否讀到 0x0A12）手冊未逐字寫出；只說存「目前警報碼」（02H p.702）。上機讀一次確認。
- 警報清除後 603Fh 是否歸 0：手冊未載明。
- PCI-1203 實際配置的 PDO（是否含 6077h、6072h、60E0h）：安川手冊未載明，看主站 ENI／樹的監看器。
- 雙軸 SGDXW 的物件窗（+0x800）、兩軸共用的警報行為：本來源未涵蓋（SIEP C710812 05）。
- HT9050 馬達編碼器碼是 `U`（26 位元絕對、多圈要電池）；機台有沒有裝電池、Pn002 設成什麼：要上機看（見 hardware 檔）。
