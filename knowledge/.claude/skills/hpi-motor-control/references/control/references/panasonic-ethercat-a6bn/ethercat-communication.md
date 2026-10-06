> 保存來源：`.claude/skills/ht9045-motor-control/references/panasonic-ethercat-a6bn/ethercat-communication.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# A6BN EtherCAT 通訊層：ESM、SM／FMMU、同步、PDO、SDO、通訊錯誤

> 頁碼＝PDF 頁碼（規格書 SX-DSV03736 R2.1）。研華資料引自 `E:\HT9045W_相關料件技術文件\EtherCat\PCI-1203_EtherCAT機制簡介_20220211.pptx`（下稱「研華簡報」）。
> 「推論」＝規格書沒直接寫、是把規格書和研華資料／移植樹拼起來得到的，要上機確認。

## 1. 主站／從站與設定檔

- 主站用 Panasonic 提供的 ESI（XML）產生 ENI，再依 ENI 建網路（A6BN p.15、p.26）。
- ESI 有兩種：不含 OD（小檔）、含 OD（大檔）（A6BN p.16）。
- SII 是接在 ESC 上的 EEPROM，A6BN 是 16 Kbit；內含 ESC 初始化資料、mailbox 大小、PDO 映射等（A6BN p.26、p.34）。
- 規格書只保證線型拓樸；其他接法要另外問 Panasonic（A6BN p.26、p.27）。
- A6BN 不能跟 RTEX 機種（A4N、A5N、A6N）混接；可以跟 A5BL 同一條 EtherCAT（A6BN p.26）。
- 規格書提醒：雜訊造成從站沒收到 frame 時，要由主站檢查並重送（A6BN p.14）。

**研華 PCI-1203 這一側**（研華簡報）：
- 初始化與連線全由卡片韌體自動做；**開卡條件是所有從站都在 OP**，否則 `Dev_Open failed`；開不了卡通常是「從站 EEPROM 讀不到正確資訊」或「EEPROM 正確但 PDO 數量／長度不對」（slide 7）。
- 要求從站 ESI 的 PDO 可動態配置：`RxPdo Fixed="0"`、`TxPdo Fixed="0"`（slide 12）。A6BN 支援 Free PDO Mapping（A6BN p.27），但 Panasonic 的 ESI 是否標 `Fixed="0"`：規格書未載明（要拿到 ESI 檔才知道）。

## 2. ESM（EtherCAT State Machine）

| ESM 狀態 | SDO（mailbox） | TxPDO（從→主） | RxPDO（主→從） | 出處 |
|---|---|---|---|---|
| Init | 不可 | 不可 | 不可 | A6BN p.30 |
| Pre-Operational（PreOP） | 可 | 不可 | 不可 | A6BN p.30 |
| Safe-Operational（SafeOP） | 可 | 可 | 不可 | A6BN p.30 |
| Operational（OP） | 可 | 可 | 可 | A6BN p.30 |
| Bootstrap | — | — | — | A6BN p.30 |

- ESC 暫存器任何狀態都能存取（A6BN p.30）。
- 連續切狀態時，要確認上一個轉換完成再送下一個（A6BN p.30）。
- 從 OP 往下切的過程中，如果命令更新、SYNC0、SM2 事件先停了，可能出通訊錯誤（A6BN p.30）。
- 不合法的轉換（例如 Init→SafeOP、Init→OP、PreOP→OP、任何→Bootstrap 等）→ Err80.0；未定義的請求碼 → Err80.1；請求 Bootstrap → Err80.2（A6BN p.275-277）。

**ESM 和 PDS（CiA402 電源狀態機）的關係**（A6BN p.31）：
- 從 PreOP／SafeOP／OP 收到切回 Init 的命令 → PDS 變 Switch on disabled。
- PDS 在 Operation enabled（含 Quick stop active）時收到任何 ESM 轉換命令 → **Err88.2**，PDS 進 Fault（A6BN p.31、p.293）。
- 要進 Operation enabled 應在 ESM＝OP 時做（A6BN p.31 註 *4）。
- 非 EtherCAT 原因的 Fault 不改 ESM；EtherCAT 相關錯誤時 ESM 依 8-2 節各錯誤的規定變化（A6BN p.31 註 *3）。
- 例：OP→PreOP 時若 PDS 在 Operation enabled，先 Err88.2，再依 605Eh 減速；減速期間 ESM 仍維持 OP，減速越慢、轉到 PreOP 越久（主站逾時要留餘裕）（A6BN p.31 註 *5）。

**啟動時間**：控制電源 ON 後初始化約 2-3 s（可用 3618h 延長）；DC／SM2 同步完成最多 1 s（A6BN p.38、p.39）。3618h 單位 100 ms，初始化約 1.5 s＋設定值×0.1 s（A6BN p.53）。

**建議的啟動步驟**（A6BN p.15）：清 ESC 暫存器、檢查 VendorID／ProductCode、設 station alias、設 mailbox 用的 SM／FMMU → Init→PreOP；確認 PreOP 後設 DC、PDO 用的 SM／FMMU → PreOP→SafeOP；確認 SafeOP 後 → OP。
規格書的 DC 例子：`1C32h:01=2`、`1C32h:02=2000000`（2 ms）、`1C33h:01=2`、`1C33h:03=0`（A6BN p.15）。

## 3. ESC、SyncManager、FMMU

- ESC IP core：ET1810／ET1811／ET1812（Altera FPGA 用）（A6BN p.25、p.32）。
- 位址空間 12 KB：0000h-0FFFh 暫存器、之後 8 KB 是 process data RAM（A6BN p.32）。
- ESC 資訊暫存器初值：FMMUs supported＝03h、SyncManagers supported＝04h、RAM size＝08h（A6BN p.32）。
- FMMU 暫存器 0600h-062Fh，3 組，每組 16 byte（logical start、length、physical start、type、activate…）（A6BN p.33）。
- Watchdog 暫存器：0400h divider、0410h PDI、0420h process data、0440h status（A6BN p.33）。
- DC：0981h activation、0990h start time、09A0h SYNC0 cycle time（A6BN p.33）。

**SyncManager 用途固定**（1C00h，唯讀）（A6BN p.62）：

| SM | 用途 | 1C00h 值 | SII 預設位址／大小 |
|---|---|---|---|
| SM0 | Mailbox 接收（主→從） | 1 | 1000h／0100h（256 byte） |
| SM1 | Mailbox 傳送（從→主） | 2 | 1200h／0100h（256 byte） |
| SM2 | RxPDO（process data output） | 3 | 規格書未載明 |
| SM3 | TxPDO（process data input） | 4 | 規格書未載明 |

SM0／SM1 的 SII 值出自 A6BN p.36（Standard／Bootstrap mailbox 相同）。
SM 設定錯誤會報：Err81.1（SM0/1：位址、重疊、奇數位址、長度小於 32 byte、控制暫存器 0804h／080Ch 碼不對）（A6BN p.284）；Err81.7（SM2/3：位址、重疊、長度跟 PDO 大小不同、控制暫存器）（A6BN p.288）。

## 4. 同步模式

| 模式 | 觸發 | 特性 | 出處 |
|---|---|---|---|
| DC（SYNC0） | 所有從站共用 System Time，SYNC0 事件觸發伺服處理 | 精度高；主站要做傳播延遲補償與週期性 drift 補償 | A6BN p.37、p.38 |
| SM2 | 收到 RxPDO 的 SM2 事件 | 沒有延遲補償、精度低；主站送出時序要固定（專用硬體），抖動大會同步不了或報警 | A6BN p.37、p.39 |
| Free Run | 從站本地計時器 | 簡單、即時性不足；不要用短於 250 µs 的週期送 PDO | A6BN p.37、p.40 |

**1C32h（SM2 同步）／1C33h（SM3 同步）**（A6BN p.70-79）：

| 物件 | 內容 |
|---|---|
| 1C32h:01 Sync mode | 00h Free Run、01h SM2、02h DC SYNC0；03h 不支援（A6BN p.70） |
| 1C33h:01 Sync mode | 00h Free Run、02h DC SYNC0、**22h SM2**；01h、03h 不支援（A6BN p.72） |
| 自動切換 | PreOP→SafeOP 時依 ESC 0981h（DC activation）自動改 1C32h:01／1C33h:01：DC 開著就變 02h；DC 關著維持原值（但 02h 會變 00h）（A6BN p.70、p.72） |
| 1C32h:02 Cycle time | 可設 250000、500000、1000000、2000000、4000000、8000000、10000000 ns；其他值 → Err81.0（A6BN p.70-71） |
| 1C32h:05 Minimum cycle time | 250000 ns（A6BN p.71） |
| 1C32h:06 Calc and copy time | 25000 ns（僅供參考）（A6BN p.71） |
| 1C33h:03 Shift time（input shift） | 0-3875000 ns，125000 ns 一階，且小於週期；通常設 0（A6BN p.72、p.75、p.80） |
| 1C33h:06 Calc and copy time | 45000 ns（僅供參考）（A6BN p.73） |
| 1C32h:03 Shift time | 不支援（A6BN p.70） |

- Err81.5：ESC 0981h bit2-0 只能是 000b 或 011b（A6BN p.286）。
- Err81.6：1C32h:01、1C33h:01 必須是上表的合法值，而且兩個要設一樣（A6BN p.287）。
- 60C2h（Interpolation time period）隨週期自動設定，不要改；表列 250 µs→25/-5、500 µs→5/-4、1 ms→1/-3、2 ms→2/-3、4 ms→4/-3（A6BN p.255）。
- 不一致：規格表（p.27）與 DC／SM2 表（p.74、p.76）還列 125 µs；Err81.0 說明（p.283）只列到 4 ms。見 SKILL.md §6。

**PCI-1203 實際用的週期**：移植樹只讀不設（`Acm_MasGetComCyclicTime`，`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203Monitor.cpp:702`），值要在機台上唯讀量；必須落在 A6BN 接受的集合內。

## 5. PDO

### 5a. 物件與限制
- RxPDO 映射表 1600h-1603h、TxPDO 映射表 1A00h-1A03h，每張 sub 01h-20h（最多 32 項）（A6BN p.49、p.58-59、p.64-65）。
- 指派：1C12h（SM2／RxPDO）、1C13h（SM3／TxPDO），各最多 4 張表（A6BN p.50、p.63）。
- 每個方向最多 32 byte；超過 → Err85.0（TxPDO）／Err85.1（RxPDO）（A6BN p.27、p.289-290）。
- 映射項格式：bit31-16 index、bit15-8 subindex、bit7-0 bit 長度（A6BN p.64）。
- 不要重複映射同一個物件（行為不保證）（A6BN p.64-65）。
- 被 PDO 更新的物件不要再用 SDO 寫；SDO 寫的值會被 PDO 蓋掉（A6BN p.41、p.48）。

### 5b. 修改規則（A6BN p.63-65、p.68）
1. 只能在 **PreOP** 改，而且要先把 sub 00h 寫 0，才能改 01h 以後；其他狀態寫 → Abort `06010003h`。
2. 改完把 sub 00h 寫成實際項數。
3. **PreOP→SafeOP 時才生效**（TxPDO 在 SafeOP 生效、RxPDO 在 OP 生效，A6BN p.68）。
4. 想下次開機不用重設：寫 `1010h:01 = 65766173h`（"save"）存 EEPROM（A6BN p.68、p.81）。
- PANATERM 物件編輯器改：要 ESM=Init，不用先寫 0（A6BN p.69）。

### 5c. 出廠預設映射（ESI 定義，A6BN p.66-67）

| 組 | RxPDO 內容 | TxPDO 內容 |
|---|---|---|
| 1（1600h／1A00h）位置控制＋touch probe | 6040h、6060h、607Ah、60B8h | 603Fh、6041h、6061h、6064h、60B9h、60BAh、60F4h、60FDh |
| 2（1601h／1A01h）位置／速度／扭力＋touch probe | 6040h、6060h、6071h、607Ah、6080h、60B8h、60FFh | 603Fh、6041h、6061h、6064h、606Ch、6077h、60B9h、60BAh、60FDh |
| 3（1602h／1A02h）位置／速度＋touch probe＋扭力上限 | 6040h、6060h、6072h、607Ah、60B8h、60FFh | 同組 2 |
| 4（1603h／1A03h）位置／速度／扭力＋touch probe＋扭力上限 | 6040h、6060h、6071h、6072h、607Ah、6080h、60B8h、60FFh | 同組 2 |

- 1C12h／1C13h 出廠指派哪一張表：規格書未載明（只說「通常一張就夠、不用改」，A6BN p.50；出廠值看 Standard specification SX-DSV03740）。

### 5d. 研華 PCI-1203 的預設 PDO（研華簡報 slide 12-14）與 A6BN 的落差（推論）

| 方向 | 研華預設 | A6BN 對照 |
|---|---|---|
| RxPDO（1600h） | 6040h:00（16）、6060h:00（8）、607Ah:00（32）、**60FEh:00（32）** | A6BN 的 60FEh:00h 是 U8「Number of entries」、PDO＝No；可映射的是 60FEh:01h Physical outputs、02h Bit mask（A6BN p.240、p.337） |
| TxPDO（1A00h） | 6041h:00（16）、6061h:00（8）、6064h:00（32）、60FDh:00（32） | 全部 TxPDO 可映射（A6BN p.334、p.337） |

- 推論：如果 1203 真的照字面把 `60FE:00/32bit` 寫進 1600h，A6BN 可能拒收（abort）或 PDO 長度不符（Err81.7／Err85.1），結果是到不了 OP、卡片開不起來。HT9050 的安川、SW3D 為什麼沒事、1203 實際寫了什麼，要在機台上用 SDO 唯讀 1600h／1A00h 看。**待 EastSun／研華確認**。
- 兩邊大小：研華預設 Rx 11 byte、Tx 11 byte，都在 A6BN 的 32 byte 內（推算）。
- 研華預設 TxPDO 沒有 603Fh；移植樹是在 ALM 位元上升時用 SDO 讀一次 603Fh（`Pci1203Monitor.cpp:2803`）。

## 6. Mailbox／SDO

- Mailbox type：只支援 03h CoE；EoE、FoE、SoE、VoE 不支援（A6BN p.42）。SII 0x001C「Mailbox protocol」值為 000Ch（A6BN p.36；規格書沒有解釋各位元）。
- SDO 支援 Request、Response、SDO Information、Emergency；不支援 Complete Access（A6BN p.27、p.42）。
- SDO 更新時間不固定（A6BN p.41）；寫 EEPROM 期間不接受其他 SDO（A6BN p.81）。
- **逾時**（A6BN p.43）：
  - Mailbox request 100 ms：主站送出後 WKC 有更新才算從站收到，沒更新就重試，超過 100 ms 主站側逾時。
  - Mailbox response 10 s：從站產生回應的最長時間。
  - 主站連續送同一個 mailbox counter → 從站暫停 SDO 接收，要 Init→PreOP 才恢復。
- **Abort code**（A6BN p.44；標「Not supported」的表示這台不會回）：

| Abort code | 意義 |
|---|---|
| 05040001h | Client/Server command specifier 不合法或未知 |
| 06010000h | 不支援的存取 |
| 06010002h | 寫唯讀物件 |
| 06010003h | Subindex 不能寫，寫入時 SI0 必須為 0（PDO 映射改錯狀態也回這個） |
| 06020000h | 物件不存在 |
| 06060000h | 硬體錯誤導致存取失敗 |
| 06070010h | 資料型別不符／長度不符 |
| 06090011h | Subindex 不存在 |
| 06090030h | 數值超出範圍（只限寫入） |
| 06090031h | 寫入值太大 |
| 06090032h | 寫入值太小 |
| 06090036h | 最大值小於最小值 |
| 08000020h | 資料無法傳給或存進應用程式 |
| 08000022h | 因目前裝置狀態無法傳送或儲存 |
| 08000023h | OD 動態產生失敗或沒有 OD |
| 05030000h、05040000h、05040005h、06010001h、06040041h、06040042h、06040043h、06040047h、06070012h、06070013h、08000000h、08000021h | 規格書標 Not supported |

- 1010h 寫錯（寫 1010h:00 或寫的值不是 65766173h）會回 abort（A6BN p.81）。

## 7. Emergency message

- 驅動器發生**警報**時經 mailbox 主動送出；只有警告不送（A6BN p.45）。
- Init 期間最多暫存 8 筆，到 PreOP 以上才送；超過 8 筆從最舊的丟（A6BN p.45）。
- 開關：10F3h:05 bit0（預設 1＝送）（A6BN p.45、p.83）。
- 8 byte 格式：byte0-1＝603Fh 值、byte2＝1001h 值、byte3＝警報子號、byte4-7＝00h（A6BN p.46-47）。例：Err16.1 → `FF10h, 80h, 01h, 00h…`；fault reset 清掉 → 全 0（A6BN p.47）。
- Err81.7 例外：error code＝A000h、error register＝10h，data 帶出是哪一項 SM2/3 設定錯（A6BN p.47）。
- 短時間內反覆發生／清除，可能只收到最後狀態那一筆；mailbox 通訊中發生／清除，通知會延遲（A6BN p.45）。

## 8. 前面板燈號（A6BN p.52-53）

| 燈 | 狀態 → 意義 |
|---|---|
| RUN（綠） | OFF＝Init；Flickering＝Bootstrap；Blinking＝PreOP；Single flash＝SafeOP；ON＝OP |
| ERR（紅） | OFF＝無 AL status 定義的警報；Blinking＝通訊設定錯；Single flash＝同步事件錯；Double flash＝application watchdog 逾時；Flickering＝初始化錯；ON＝PDI 錯 |
| L/A IN、L/A OUT（綠） | OFF＝沒連線；Flickering＝連線且有資料；ON＝連線無資料 |

- ERR 燈只反映 Err80.0-7、Err81.0-7、Err85.0-7（A6BN p.53）。
- Link 建立慢：改 3722h bit11（LINK 建立模式），或相鄰驅動器的 3618h 設不同值（A6BN p.53）。

## 9. Station alias（節點位址）

三種方式（A6BN p.54-56）：
1. 3741h＝1（**出廠預設**）：開機把 SII 0004h（出廠 0）放到 ESC 0012h。
2. 3741h＝0：面板旋鈕（低 8 位）＋3740h（高 8 位）放到 ESC 0012h；兩者都 0 時 alias＝0。
3. Explicit Device ID：主站設 AL Control（0120h）bit5，驅動器把旋鈕＋3740h 的值放在 AL Status Code（0134h）；期間若有 AL status 型警報，回的是警報碼。

- 3740h／3741h 只在控制電源 ON 時生效（A6BN p.55）。
- 用旋鈕設 alias 的步驟：上電、把 3741h=0 寫進 EEPROM、斷電、設旋鈕、再上電（A6BN p.15）。旋鈕單獨只能 0-255（A6BN p.15）。
- SII 0004h 跟 0007h（checksum）要一起改，其他 SII 位址不要動（A6BN p.35）。
- 推論：多台 A6BN 照出廠值上線，alias 都是 0；PCI-1203 的 SDK 是用站號定址（`Pci1203Monitor.h:575-595` 記錄了 HT9050 兩台 SW3D 撞號後「看不到」的現象），所以接線前就要排好號。

## 10. 存 EEPROM（1010h）

- `1010h:01 = 65766173h`（"save"）：把 EEPROM 欄＝Yes 的物件全部存起來；完成後讀回 00000001h（不分成敗）（A6BN p.81）。
- 最長約 10 s，**期間不能關控制電源**；Err11.0（控制電源欠壓）時無法存；EEPROM 寫入次數有限（A6BN p.81）。
- 3000h 參數屬性 C、R 的要重送控制電源才生效（A6BN p.81）。
- PANATERM 物件編輯器改的值，A6BN 有些是立即反映（跟 A5BL 不同），各物件看自己的屬性（A6BN p.18、p.69）。

## 11. 通訊錯誤怎麼處理（摘要；完整表在 errors-and-recovery.md）

| 情況 | 驅動器行為 | 出處 |
|---|---|---|
| RxPDO 停了（SM2 watchdog 逾時） | Err80.4，ESM 退到 SafeOP，ERR 燈 double flash；A6BN 只檢查 SM2 watchdog，所以只在 OP 偵測 | A6BN p.279 |
| Watchdog 時間設太短（< 2×週期；Free Run < 2 ms） | Err81.4，PreOP→SafeOP 時擋下 | A6BN p.285 |
| 同步做不起來（1 s 內 PLL 鎖不上） | Err80.3，停在 PreOP | A6BN p.278 |
| 同步中途脫鎖 | Err80.6，退到 SafeOP | A6BN p.280 |
| SYNC0／IRQ 連續漏掉超過 3742h bit0-3 次數 | Err80.7，退到 SafeOP；3742h bit0-3＝0 時關閉偵測 | A6BN p.281-282 |
| 某個 port 斷線超過 3743h | Err85.2，退到 Init；**3743h 出廠 0＝不偵測**；只有偵測到斷線的那台會報，後面的站要靠 PDO watchdog | A6BN p.291 |
| Err80-88 的減速方式 | 依 605Eh（0＝照 3510h、1＝6084h、2＝6085h），停下後進 Fault | A6BN p.236 |
| csp 中途掉一兩個週期 | 驅動器推估目標位置並校正 | A6BN p.135 |
| set brake 輸出 | 通訊中斷時一律 set brake＝1（煞車作動） | A6BN p.241 |

- 6040h（Controlword）與 60FEh 的煞車控制，規格書都要求**用 PDO 並開 PDO watchdog**：SDO 無法判斷通訊中斷，馬達可能一直通電、煞車可能不作動（A6BN p.88、p.240）。
- AL Status Code 與 ESM 每次偵測到新的 EtherCAT 錯誤都更新；7 段顯示器、PANATERM、abort message 保留第一個偵測到的 Err 號，直到清除（A6BN p.275）。

<!-- preserved-content:end -->
