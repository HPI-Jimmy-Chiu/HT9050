> 保存來源：`.claude/skills/ht9045-motor-control/references/panasonic-ethercat-a6bn/errors-and-recovery.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# A6BN 的警報：Err 碼總表、603Fh、清除、警告，與 Steven 的復歸規則

> 頁碼＝PDF 頁碼（SX-DSV03736 R2.1）。§2、§3 的表**照規格書欄位抄**（A6BN p.271-274），沒有增刪欄；
> 非 EtherCAT 警報的原因與處置，本規格書只列表，細節在 SX-DSV03735 §7（A6BN p.275）——本規格書未載明的就寫「規格書未載明」。

## 1. 警報資訊從哪裡讀

| 來源 | 內容 | 出處 |
|---|---|---|
| 6041h bit3（fault）、bit7（warning） | PDS 進 Fault／有警告 | A6BN p.90 |
| 603Fh Error code | FFxxh，xx＝**主**警報號（16 進位）；讀不到子號；同時有警報與警告時顯示警報；無則 0000h | A6BN p.46、p.297 |
| 1001h Error register | bit4＝AL status 定義的警報、bit7＝其他警報；警告不顯示 | A6BN p.46、p.60 |
| Emergency message | 8 byte：603Fh、1001h、子號、0…；只在警報時送 | A6BN p.45-47 |
| 10F3h Diagnosis history | 最近 14 筆（只記「History」欄＝Yes 的警報；不記警告） | A6BN p.82-83、p.273 |
| ESC AL Status Code（0134h） | 只有 EtherCAT 相關警報有碼，其他是 0000h | A6BN p.271-273 |
| ERR 燈 | 只反映 Err80、81、85 | A6BN p.53 |
| 7 段顯示器／PANATERM | 保留**第一個**偵測到的 Err 號直到 fault reset；不可清除的警報 reset 後仍保留 | A6BN p.275、p.298 |
| 4F37h:01-04 | 主號 0-127 各一個位元，可同時看多個警報；4310h 寫主號後讀 4F37h:10 得子號 | A6BN p.263-265 |
| 4F34h、4F37h:11／12 | 目前的警告 | A6BN p.263、p.265 |
| 4F33h | 馬達不動的原因碼 | A6BN p.262 |

注意：
- 603Fh 寫入的時機跟 Emergency message 一樣，**比 6041h bit3 晚**；看到 fault 位元立刻讀 603Fh 可能還是舊值（A6BN p.297）。
- 移植樹在 ALM 位元 0→1 時用 SDO 讀一次 603Fh（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.cpp:2803`）；名稱表只套 SGDX（同檔 :2816），A6BN 的碼會只顯示數字。若要加 Panasonic 名稱表，資料就是本檔 §2、§3。
- 推論：因為 603Fh 比 bit3 晚寫，在 A6BN 上「一看到 ALM 就讀一次」可能讀到 0000h 或前一個碼；需要上機確認是否要延遲或重讀（違反「不准用計時輪詢判 1203 狀態」的 EastSun 規則前要先問）。

### 603Fh 換算（依 A6BN p.297 的公式：低 8 位＝主號的 16 進位）

| 603Fh | 主號 | 例 |
|---|---|---|
| FF0Ch | 12 | Err12.0 過電壓（規格書原例，p.46） |
| FF10h | 16 | Err16.1 扭力飽和（規格書原例，p.47） |
| FF1Bh | 27 | Err27.4／27.6／27.7（換算） |
| FF50h | 80 | Err80.x（換算） |
| FF51h | 81 | Err81.x（換算；但 Err81.7 例外＝**A000h**，p.46、p.297） |
| FF55h | 85 | Err85.0 或 85.1（規格書原例，p.46）；85.2、85.3 同主號 |
| FF58h | 88 | Err88.x（換算） |
| FFA0h-FFA9h、FFC3h、FFD2h、FFD3h | 警告 | p.297；p.46 另多列 ACh |

## 2. EtherCAT 通訊相關警報（A6BN p.271；細節 p.275-296）

欄位照規格書：Clearable、Emergency stop（*1）、History（*2）、ERR 燈、AL Status Code；後三欄（偵測時 ESM、偵測後 ESM、處置）出自各錯誤的細節頁。

| Err | 名稱（規格書原文） | Clearable | Emergency stop | History | ERR 燈 | AL Status Code | 偵測時 ESM | 偵測後 ESM | 原因／處置（摘要） | 細節頁 |
|---|---|---|---|---|---|---|---|---|---|---|
| 80.0 | ESM unauthorized request error protection | Yes | Yes | Yes | Blinking | 0011h | 全部 | 非 OP 維持；OP→SafeOP | 不合法的狀態轉換請求；檢查主站 | p.275 |
| 80.1 | ESM undefined request error protection | Yes | Yes | Yes | Blinking | 0012h | 全部 | 同上 | 未定義的請求碼 | p.276 |
| 80.2 | Bootstrap requests error protection | Yes | No | Yes | Blinking | 0013h | Init→Bootstrap | Init | 請求 Bootstrap | p.277 |
| 80.3 | Incomplete PLL error protection | Yes | No | Yes | Single flash | 002Dh | PreOP→SafeOP（37B0h bit8＝1 時也含 SafeOP→OP） | PreOP | 同步 1 s 內沒完成；查 DC 設定、延遲／drift 補償、SM2 送出時序、配線、雜訊、加大 3742h；解不了就重送控制電源 | p.278 |
| 80.4 | PDO watchdog error protection | Yes | Yes | Yes | Double flash | 001Bh | SafeOP、OP（實際只在 OP，因只檢查 SM2 watchdog） | SafeOP | RxPDO 在 0400h／0420h 設的時間內沒來；查主站送出是否中斷、加大 watchdog、配線、雜訊 | p.279 |
| 80.6 | PLL error protection | Yes | Yes | Yes | Single flash | 0032h | SafeOP、OP | SafeOP | 運轉中同步脫鎖；同 80.3；解不了就重送控制電源 | p.280 |
| 80.7 | Synchronization signal error protection | Yes | Yes | Yes | Single flash | 002Ch | SafeOP、OP（37B0h bit7＝1 時只 OP） | SafeOP | SYNC0／IRQ 連續漏掉超過 3742h bit0-3；同 80.3 | p.281-282 |
| 81.0 | Synchronization cycle error protection | Yes | No | Yes | Blinking | 0035h | PreOP→SafeOP | PreOP | 週期不支援，或 ESC 09A0h 與 1C32h:02 不一致 | p.283 |
| 81.1 | Mailbox error protection | Yes | No | Yes | Blinking | 0016h | Init→PreOP、PreOP、SafeOP、OP、Init→Bootstrap、Bootstrap | Init | SM0/1 設定錯；照 ESI 設 | p.284 |
| 81.4 | PDO watchdog error protection | Yes | No | Yes | Blinking | 001Fh | PreOP→SafeOP | PreOP | watchdog 時間 < 2×週期（Free Run < 2 ms） | p.285 |
| 81.5 | DC error protection | Yes | No | Yes | Blinking | 0030h | PreOP→SafeOP | PreOP | 0981h bit2-0 不是 000b／011b | p.286 |
| 81.6 | SM event mode error protection | Yes | No | Yes | Blinking | 0028h | PreOP→SafeOP | PreOP | 1C32h:01／1C33h:01 值不合法或不一致 | p.287 |
| 81.7 | SyncManager2/3 error protection | Yes | No | Yes | Blinking | 001Dh／001Eh | PreOP→SafeOP、SafeOP、OP | PreOP | SM2（001Dh）／SM3（001Eh）位址、長度、控制暫存器錯；照 ESI 設 | p.288 |
| 85.0 | TxPDO assignment error protection | Yes | No | Yes | Blinking | 0024h | PreOP→SafeOP | PreOP | TxPDO 超過 32 byte | p.289 |
| 85.1 | RxPDO assignment error protection | Yes | No | Yes | Blinking | 0025h | PreOP→SafeOP | PreOP | RxPDO 超過 32 byte | p.290 |
| 85.2 | Lost link error protection | Yes | Yes | Yes | Double flash | 0000h | PreOP、SafeOP、OP、Bootstrap | Init | port 斷線超過 3743h（出廠 0＝不偵測）；查配線與主站 | p.291 |
| 85.3 | SII EEPROM error protection | **No** | No | Yes | Flickering | 0051h | 全部 | Init | SII 與物件的 VendorID／Product code／Revision 不符、讀寫 SII 錯、0502h bit11-14；沒接主站也發生 → 驅動器可能故障，換機 | p.292 |
| 88.0 | Main power undervoltage protection (AC insulation detection 2) | Yes | Yes | No | OFF | 0000h | PreOP、SafeOP、OP | 維持 | 6007h＝1 時主電源 OFF（Operation enabled／Quick stop active）或主電源 OFF 時收到 Switch on；查電源、接線 | p.294 |
| 88.1 | Control mode setting error protection | Yes | Yes | Yes | OFF | 0000h | 全部 | 維持 | 6060h、6061h 都 0 時進 Operation enabled，或設了不支援的模式；也查 Pr6.47 bit0、bit3 | p.295 |
| 88.2 | ESM requirements during operation error protection | Yes | Yes | Yes | OFF | 0000h | Init、PreOP、SafeOP、OP | 照主站的請求轉 | Operation enabled／Quick stop active 時收到 ESM 轉換；3799h bit0＝1 且 PANATERM 伺服 ON（D2 警告）時收到 ESM 轉換 | p.293 |
| 88.3 | Improper operation error protection | **No** | Yes | Yes | OFF | 0000h | PreOP、SafeOP、OP（電子齒輪那項：Init→PreOP） | 維持（電子齒輪那項：照主站請求） | touch probe 選 EXT1/EXT2 卻沒配輸入；軟體極限啟用時座標 wraparound；電子齒輪超出 8000 倍到 1/1000 或運算溢位 → 修設定後**重送控制電源** | p.296 |

（規格書目錄把 Err80.0 叫「Inaccurate ESM demand error protection」、表格叫「ESM unauthorized request error protection」，同一個（A6BN p.271、p.275）。）

## 3. 非 EtherCAT 警報（A6BN p.272-273，照抄）

全部 ERR 燈＝OFF、AL Status Code＝0000h（A6BN p.272-273），下表省略這兩欄。

| Err | 名稱（規格書原文） | Clearable | Emergency stop | History |
|---|---|---|---|---|
| 11.0 | Control power supply undervoltage protection | Yes | No | No |
| 12.0 | Over-voltage protection | Yes | No | Yes |
| 13.0 | Main power supply undervoltage protection (between P to N) | Yes | Yes | No |
| 13.1 | Main power supply undervoltage protection (AC interception detection) | Yes | Yes | No |
| 14.0 | Over-current protection | No | No | Yes |
| 14.1 | IPM error protection | No | No | Yes |
| 15.0 | Over-heat protection | No | Yes | Yes |
| 15.1 | Encoder over-heat protection | No | Yes | Yes |
| 16.0 | Over-load protection | Yes *3) | No | Yes |
| 16.1 | Torque saturation error protection | Yes | No | Yes |
| 18.0 | Over-regeneration load protection | No | Yes | Yes |
| 18.1 | Regenerative transistor error protection | No | No | Yes |
| 24.0 | Position deviation excess protection | Yes | Yes | Yes |
| 24.1 | Speed deviation excess protection | Yes | Yes | Yes |
| 26.0 | Over-speed protection | Yes | Yes | Yes |
| 26.1 | 2nd over-speed protection | Yes | No | Yes |
| 27.4 | Position command error protection | Yes | Yes | Yes |
| 27.6 | Operation command contention protection | Yes | No | Yes |
| 27.7 | Position information initialization error protection | No | No | Yes |
| 28.0 | Pulse regeneration limit protection | Yes | Yes | Yes |
| 29.1 | Counter overflow protection 1 | No | No | Yes |
| 29.2 | Counter overflow protection 2 | No | No | Yes |
| 33.0 | Duplicated input allocation error 1 protection | No | No | Yes |
| 33.1 | Duplicated input allocation error 2 protection | No | No | Yes |
| 33.2 | Input function number error 1 protection | No | No | Yes |
| 33.3 | Input function number error 2 protection | No | No | Yes |
| 33.4 | Output function number error 1 protection | No | No | Yes |
| 33.5 | Output function number error 2 protection | No | No | Yes |
| 33.8 | Latch input allocation error protection | No | No | Yes |
| 34.0 | Software limit protection | Yes | No | Yes |
| 36.0-1 | EEPROM parameter error protection | No | No | No |
| 37.0-2 | EEPROM check code error protection | No | No | No |
| 38.0 | Over-travel inhibit input protection 1 | Yes | No | No |
| 38.1 | Over-travel inhibit input protection 2 | Yes | No | No |
| 38.2 | Over-travel inhibit input protection 3 | No | No | Yes |
| 50.0 | Feedback scale connection error protection | No | No | Yes |
| 50.1 | Feedback scale communication error protection | No | No | Yes |
| 50.2 | Feedback scale communication data error protection | No | No | Yes |
| 51.0-51.5 | Feedback scale status error protection 0-5（各一列） | No | No | Yes |
| 55.0 | A-phase connection error protection | No | No | Yes |
| 55.1 | B-phase connection error protection | No | No | Yes |
| 55.2 | Z-phase connection error protection | No | No | Yes |
| 55.3 | CS signal logic error protection | No | No | Yes |
| 55.4 | AB phase open phase error protection | No | No | Yes |
| 60.0 | Motor setup error protection | No | No | No |
| 60.1 | Motor combination error 1 protection | No | No | No |
| 60.2 | Motor combination error 2 protection | No | No | No |
| 60.3 | Linear motor automatic setup error protection | Yes | No | Yes |
| 61.0 | Magnet pole position estimation error 1 protection | Yes | No | Yes |
| 61.1 | Magnet pole position estimation error 2 protection | Yes | No | Yes |
| 61.2 | Magnet pole position estimation error 3 protection | No | No | No |
| 70.0 | U-phase current detector error protection | No | No | Yes |
| 70.1 | W-phase current detector error protection | No | No | Yes |
| 72.0 | Thermal error protection | No | No | Yes |
| 84.3 | Synchronous establishment initialization error protection | No | No | Yes |
| 87.0 | Forced alarm input protection | Yes | Yes | No |
| 87.1 | Retracting operation completion (I/O) *6) | *4) | Yes *5) | Yes |
| 87.2 | Retracting operation completion (communication) *6) | *4) | Yes *5) | Yes |
| 87.3 | Retracting operation error *6) | *4) | Yes *5) | Yes |
| 90.6 | Reference axis instruction error protection | Yes *6) | Yes | Yes |
| 92.1 | Feedback scale data recovery error protection | No | No | Yes |
| 93.3 | Feedback scale connection error protection | No | No | Yes |
| 93.4 | Function setting error protection | Yes | No | Yes |
| 93.5 | Parameter setting error protection 4 | No | No | Yes |
| 93.8 | Parameter setting error protection 6 | No | No | Yes |
| 93.9 | Table setting error protection | Yes | No | No |
| 94.3 | Home position return error protection 2 | Yes | No | Yes |
| 96.2-8 | Control unit error protection 1 to 7 | No | No | Yes |
| 98.2 | Communication hardware error protection 2 | No | No | Yes |
| 98.3 | Communication hardware error protection 3 | No | No | Yes |
| Other | Other error protection | - | - | - |

規格書註解（A6BN p.273-274）：
- *1) Emergency stop＝3510h（Sequence at alarm）設 4-7 時，這些警報會緊急停止運轉（細節見 SX-DSV03735）。
- *2) History＝Yes 的警報會記進 10F3h:06-13（Diagnosis message 1-14）。
- *3) Err16.0 發生後約 10 s 才變成可清除；收到清除命令後，等到可清除狀態才開始清。
- *4) Err87.1-87.3 能不能清看 3668h bit0-2（規格書寫成「3668h8」）：bit0＝87.1、bit1＝87.2、bit2＝87.3，0＝不可清、1＝可清。
- *5) 雖然屬性是「緊急停止」，但退避動作條件成立時，不管 Pr5.10（Sequence at alarm）都走退避動作，完成時才出警報；之後照緊急停止警報處理（含警報時的防落下功能）。
- *6) 清 Err90.6 時一定要初始化位置資訊。（原表 87.1-87.3 名稱後也標了 *6)，但 *6) 的內容是講 Err90.6，照抄不解釋。）
- Err27.7 的發生條件之一：回原點在「偵測到原點到完成」之間被主站 halt 等取消（A6BN p.149）。
- Err94.3：3722h bit7＝1 時，Z 相回原點移動異常、碰到禁止輸入（A6BN p.149）。
- Err38.0：3504h＝2 時任何一個 POT／NOT ON；3504h＝0 且回原點時兩邊極限同時 ON 也報這個（A6BN p.151、p.237）。

## 4. 10F3h 警報履歷（A6BN p.82-83）

- 最多 14 筆，10F3h:01 固定 0Eh；10F3h:02＝最新一筆所在的 sub（無履歷時 0）。
- 清除：寫 **00h** 到 10F3h:03；寫其他值回 abort 06090030h。清完 10F3h:05 bit5＝1，直到下一次警報。
- 每筆（OS 型）：Diag code（＝603Fh 值）、Flags（固定 0002h）、Text ID（高 8 位＝主號、低 8 位＝子號）、Time stamp（不支援，固定 0）。
- 只記警報、不記警告；有些警報不記（見「History」欄）。上電時從驅動器 EEPROM 讀回。
- 不能 PDO；每個 sub 用 SDO 讀，不保證同時性。
- 履歷比 603Fh 好用：**Text ID 有子號**（603Fh 沒有），例如分得出 Err27.4 和 27.7。

## 5. 清除警報與警告（A6BN p.298-299）

三種方法任一（EtherCAT 相關警報的復歸）：
1. **通訊**：主站先把 AL Control（0120h）bit4（Error Ind Ack）寫 1，再把 6040h bit7 從 0→1（Fault reset）。完成後 PDS 從 Fault → **Switch on disabled**。
2. PANATERM 清除。完成後同樣 → Switch on disabled。
3. 外部警報清除輸入 A-CLR 從 OFF→ON。完成後同樣 → Switch on disabled。

注意（A6BN p.298）：
- AL Status 的通知和警報之間有時間差，不同步。
- 7 段顯示器保留第一個 Err 號直到 fault reset；**不可清除的警報 reset 後仍保留**。
- 多個警報同時發生時，要全部原因都排除才清得掉。
- A-CLR 是 ON 的狀態下，從 PANATERM 或 EtherCAT 清都清不掉；要先把 A-CLR 關掉。
- **PDS 在 Fault reaction active（還在減速）時清不掉**。
- 清完是 Switch on disabled，要重新 Shutdown → Switch on → Enable operation（cia402-and-modes.md §1c）才會伺服 ON。

警告（A6BN p.299）：
- 3627h 設成 latch 的警告，原因排除後也不會自己消失；用 fault reset（6040h bit7 0→1）、PANATERM、或 A-CLR OFF→ON 清。Fault reaction active 時一樣清不掉。
- A-CLR 是 ON 時不會出警告。

PCI-1203 這一側：移植樹清警報用 `Acm_AxResetError`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Control.cpp:1723-1724`）。它有沒有替 A6BN 做「AL Control bit4＋6040h bit7」兩步，**規格書與樹都看不出來**，接 A6BN 前要上機確認（尤其 Err80／81／85 這類 AL Status 型警報）。

## 6. 對到 Steven 的規則（機台規則優先；規格書是依據與補充）

Steven 原話與裁決在 `D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md` §5（1003 Q88／Q89）；HT9050 機台現況（POWERDROP、HOME 停止、M-a／M-b 待改）在 `D:\HT9045\.claude\skills\ht9050-1203-homing\SKILL.md` §3f、§6。

| 狀況 | Steven 的規則 | A6BN 規格書的依據／補充 |
|---|---|---|
| 控制電源斷電 | 斷電後必須歸零 | 控制電源 ON 會重設位置資訊、homing attained 歸 0（p.146、p.243）。 |
| 只有主電源（L1-L3）掉、控制電源還在 | 斷電後必須歸零（HOME 途中掉電：停止 HOME、全部 HomeFlag 清掉、電回來不可自動續做） | 規格書的「位置資訊重設時機」沒有列主電源 OFF（p.243）；主電源 OFF 會依 6007h／3507h-3509h 減速並可能 Err88.0／Err13.x（p.227-229、p.294）。規格書沒說要重新歸零，**照 Steven 的規則做**。 |
| EtherCAT 斷線後重新建立（ESM 回 Init 再到 PreOP） | （規則沒特別講） | **要重新歸零**：Init→PreOP 會重設位置資訊、bit12 歸 0、增量式軟體極限失效（p.102、p.146、p.243）。Err85.2、81.1、85.3 都會讓 ESM 回 Init（p.284、p.291、p.292）。 |
| 警報可以清（Clearable＝Yes） | 清一次，清掉的軸要重新歸零 | 用 §5 的方法；Fault reaction active 中先等減速完。Err27.4 本身就會重設位置資訊（p.243）。Err16.0 要等約 10 s（p.273）。Err90.6 清時一定要初始化位置資訊（p.274）。 |
| 警報清不掉（Clearable＝No） | 停止 HOME，提示「請斷電重開後再歸零」 | 不可清除的 fault reset 後仍保留（p.298）。Err88.3：先修設定（輸入指派、軟體極限、電子齒輪）再重送控制電源（p.296）。Err33.x、36.x、37.x、60.x 是參數／EEPROM／馬達設定類，斷電前要先修設定，否則重開還是一樣（推論）。Err85.3 沒接主站也發生 → 換驅動器（p.292）。 |
| 一直沒斷電、已歸零的軸 | 不用再歸零；伺服 ON 時把 encoder 回寫成 command | csp 規格書要求：伺服 OFF 時主站讓 607Ah＋60B0h 一直跟 6064h，否則伺服 ON 會衝回舊命令位置（p.134）。 |
| 放煞車 | 先伺服 ON（本輪 SVON 完成）才放煞車 | 若用驅動器的 set brake（60FEh:01 bit0）：0＝放開、1＝作動；必須 PDO＋watchdog；通訊中斷自動 set brake＝1（p.240-241）。進 Operation enabled 後 100 ms 以上才給命令（p.86）。BRK-OFF 的時序在 SX-DSV03735 §9-2，本規格書未載明。HT9050 的煞車是機台 IO 輸出（`Sw*Breaker`），不是驅動器的 set brake（`ht9050-hw\references\motors-9050.md` §4）。 |
| IO／馬達裝置 ERROR | 機台不准動；停止永遠不擋 | Fault 時驅動功能停用（p.87 event 14）；Halt（6040h bit8）、Quick stop（bit2＝0）、Disable operation 隨時可下（p.88）。**代價要知道**：回原點在偵測到原點後、完成前被 halt 取消 → Err27.7，不可清，要整機斷電重開（p.149、p.272）。 |
| 絕對式 feedback scale 的軸 | 規則是「斷電後必須歸零」 | 絕對式模式下**不需要也不能**用 hm 回原點：起動就 homing error（p.151、p.251）；bit12 永遠 1（p.146）。「歸零」在這種軸上要定義成什麼（例如只確認 607Ch 與位置合理），**要 Steven 決定**。 |

## 7. 待確認／規格書未載明（接 A6BN 前要問清楚）

1. Panasonic A6BN 的 ESI 檔 `RxPdo/TxPdo Fixed` 是否為 0（研華要求動態 PDO）——規格書未載明，要拿 ESI。
2. 研華預設把 `60FE:00/32bit` 放進 RxPDO，A6BN 的 60FEh:00 不能映射——1203 實際寫什麼、A6BN 會不會因此到不了 OP（研華簡報 slide 14；A6BN p.240、p.337）。
3. `Acm_AxResetError` 是否做 AL Control bit4（Error Ind Ack）＋6040h bit7（A6BN p.298）。
4. PCI-1203 ring 實際週期是否在 A6BN 的 250 µs-10 ms（且 Err81.0 頁只列到 4 ms）內（A6BN p.70、p.283）。
5. 1C12h／1C13h 出廠指派哪一張映射表：規格書未載明（看 SX-DSV03740）。
6. 3506h、3510h 各值的停止方式：本規格書未載明（SX-DSV03735 §6-3-2）。
7. 4DA0h（警報附帶資訊）在 A6BN 到底支不支援：說明頁有（p.257），物件表說不支援（p.328）。
8. 回原點用 HOME 開關邊緣要 HOME 接 SI5（p.148），touch probe EXT1 也要 SI5（p.215）——兩者能否並存：規格書未載明。
9. 線性馬達下 6076h（mN·m）、6077h（0.1 %）對應的是推力（N）還是什麼基準：規格書只給用語替換規則（p.14），未逐一說明。
10. 龍門功能（4309h、430Ah、4D5Ch、6040h bit11、Err90.6）需要主站送哪些資料：本規格書只列物件，細節在 SX-DSV03735。

<!-- preserved-content:end -->
