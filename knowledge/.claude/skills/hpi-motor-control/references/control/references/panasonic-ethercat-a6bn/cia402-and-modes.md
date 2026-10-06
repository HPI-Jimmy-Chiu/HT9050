> 保存來源：`.claude/skills/ht9045-motor-control/references/panasonic-ethercat-a6bn/cia402-and-modes.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# A6BN 的 CiA402：狀態機、Controlword／Statusword、模式、回原點、Touch probe、停止選項

> 頁碼＝PDF 頁碼（SX-DSV03736 R2.1）。「推論」＝規格書沒直接寫。

## 1. PDS 狀態機（FSA）

### 1a. 狀態與 Statusword（6041h）樣式（A6BN p.90）

| 6041h（x＝不管） | PDS 狀態 | 意義 |
|---|---|---|
| `xxxx xxxx x0xx 0000b` | Not ready to switch on | 初始化未完成 |
| `xxxx xxxx x1xx 0000b` | Switch on disabled | 初始化完成 |
| `xxxx xxxx x01x 0001b` | Ready to switch on | 主電源 OFF |
| `xxxx xxxx x01x 0011b` | Switched on | 伺服 OFF／伺服 ready |
| `xxxx xxxx x01x 0111b` | Operation enabled | 伺服 ON |
| `xxxx xxxx x00x 0111b` | Quick stop active | 立即停止 |
| `xxxx xxxx x0xx 1111b` | Fault reaction active | 偵測到警報、減速中 |
| `xxxx xxxx x0xx 1000b` | Fault | 警報狀態 |

### 1b. 轉換與事件（A6BN p.86-87）

| # | 轉換 | 觸發 | 動作 |
|---|---|---|---|
| 0 | 上電 → Not ready to switch on | 自動 | 自我診斷、初始化 |
| 1 | → Switch on disabled | 初始化完成自動 | 通訊建立 |
| 2 | → Ready to switch on | 非 STO 時收到 Shutdown | — |
| 3 | → Switched on | **主電源 ON** 時收到 Switch on | — |
| 4 | → Operation enabled | 收到 Enable operation | 驅動功能啟用，**所有 set point 清除** |
| 5 | Operation enabled → Switched on | Disable operation | 驅動功能停用 |
| 6 | Switched on → Ready to switch on | Shutdown；或偵測到主電源 OFF | — |
| 7、9、10、12 | → Switch on disabled | Disable voltage、Quick stop、ESM 回 Init、STO、主電源 OFF 等（細節依編號） | 驅動功能停用 |
| 8 | Operation enabled → Ready to switch on | Shutdown | 驅動功能停用 |
| 11 | → Quick stop active | Quick stop；或 6007h=3 時主電源 OFF | 開始 quick stop |
| 13 | → Fault reaction active | 偵測到錯誤；6007h=1 時主電源 OFF；退避動作觸發 | 執行 fault reaction／退避動作 |
| 14 | → Fault | 減速完成自動 | 驅動功能停用 |
| 15 | Fault → Switch on disabled | 錯誤原因排除後收到 Fault reset | 沒有錯誤因素才重設 |
| 16 | Quick stop active → Operation enabled | quick stop option code 為 5、6、7 時收到 Enable operation | 驅動功能啟用 |

重點（A6BN p.86-87）：
- 主電源 OFF 時不會 servo ready，**進不了 Switched on**。
- STO 狀態下 PDS 一律是 Switch on disabled。
- 用磁極位置估測（magnet pole estimation）時，估測完成前不會進 Operation enabled。
- 進 Operation enabled 後 **100 ms 以上**才送運轉命令（A6BN p.86；csp、hm、cst 各節也重複寫，p.134、p.148、p.214）。
- 每一步都要先在 6041h 確認轉換完成再送下一步（A6BN p.17、p.87）。

### 1c. 開機到伺服 ON 的範例（A6BN p.17）

1. 6040h=0006h（Shutdown），等 6041h 從 xx40h 變 xx21h。
2. 6040h=0007h（Switch on），等 xx21h 變 xx23h。
3. 6040h=000Fh（Enable operation），等 xx23h 變 xx27h → 伺服 ON。
4. 伺服 OFF：6040h=0007h（Disable operation），等 xx27h 變 xx23h。
- 主電路有電壓時 6041h bit4（voltage enabled）＝1，所以實際看到的值會多 0010h（A6BN p.17 註 *1）。

## 2. Controlword（6040h）（A6BN p.88-89）

| bit | 名稱 | 說明 |
|---|---|---|
| 0 | so switch on | PDS 命令 |
| 1 | ev enable voltage | PDS 命令 |
| 2 | qs quick stop | **0 才是 quick stop**（反邏輯） |
| 3 | eo enable operation | PDS 命令 |
| 4-6 | oms | 依模式：hm 的 bit4＝start homing；csp／csv／cst 不用 |
| 7 | fr fault reset | 上升緣 |
| 8 | h halt | 1＝依 605Dh 暫停；回 0 繼續；**hm 模式回 0 不會重新開始** |
| 9 | oms | 依模式（pp change on set-point；其他不用） |
| 10 | r | 保留 |
| 11 | dc disable correction | 1＝關掉參考其他軸的補正（mass ratio、velocity FF gain、thrust FF gain、other axis vibration suppression）；A6BN 龍門型用 |
| 12-15 | r | 保留 |

PDS 命令組合（bit7、3、2、1、0；－＝不管）（A6BN p.88）：

| 命令 | bit7 | bit3 | bit2 | bit1 | bit0 | 轉換 |
|---|---|---|---|---|---|---|
| Shutdown | 0 | － | 1 | 1 | 0 | 2、6、8 |
| Switch on | 0 | 0 | 1 | 1 | 1 | 3 |
| Switch on＋Enable operation | 0 | 1 | 1 | 1 | 1 | 3＋4 |
| Enable operation | 0 | 1 | 1 | 1 | 1 | 4、16 |
| Disable voltage | 0 | － | － | 0 | － | 7、9、10、12 |
| Quick stop | 0 | － | 0 | 1 | － | 7、10、11 |
| Disable operation | 0 | 0 | 1 | 1 | 1 | 5 |
| Fault reset | 上升緣 | － | － | － | － | 15 |

安全注意（A6BN p.88）：6040h 一定要走 PDO 並開 PDO watchdog；SDO 判斷不了通訊中斷，馬達可能一直通電。

## 3. Statusword（6041h）（A6BN p.90-92）

| bit | 名稱 | 說明 |
|---|---|---|
| 0 | rtso ready to switch on | 見 1a |
| 1 | so switched on | |
| 2 | oe operation enabled | |
| 3 | f fault | |
| 4 | ve voltage enabled | 1＝主電路電壓已加到 PDS |
| 5 | qs quick stop | **0＝正在回應 quick stop**（反邏輯） |
| 6 | sod switch on disabled | |
| 7 | w warning | 1＝有警告；警告不改 PDS、不停機 |
| 8 | r | 固定 0 |
| 9 | rm remote | ESM 到 PreOP 以上變 1（6040h 可被處理） |
| 10 | oms | 見下表 |
| 11 | ila internal limit active | 內部限制作用中（下面列原因） |
| 12、13 | oms | 見下表 |
| 14、15 | r | 固定 0 |

依模式的 bit（A6BN p.91）：

| 模式 | bit13 | bit12 | bit10 |
|---|---|---|---|
| pp | following error | set-point acknowledge | target reached |
| pv | max slippage error（不支援） | speed | target reached |
| tq | － | － | target reached |
| hm | homing error | homing attained | target reached |
| ip | － | ip mode active | target reached |
| csp | following error | drive follows command value | － |
| csv | － | drive follows command value | － |
| cst | － | drive follows command value | － |

bit11（internal limit active）變 1 的原因（A6BN p.92、p.103、p.136）：
- 位置控制（pp、csp）：緊急停止、扭力限制、POT／NOT、軟體極限。hm：緊急停止、扭力限制。
- 速度控制：緊急停止、扭力限制、POT／NOT、速度限制。扭力控制：緊急停止、扭力限制（3703h 可改判斷條件）、POT／NOT、速度限制。
- 扭力限制取下列最小值：6071h＋60B2h（只在 tq／cst）、6072h、3013h、3522h（3521h＝2 或 4、扭力控制以外）。扭力限制為 0 時伺服 OFF 也會是 1。
- 在軟體極限區外時一直是 1，直到回到正常範圍（A6BN p.103）。
- 3722h bit5＝1 且 6080h＝0 時馬達不動，但 bit11 **不會**變 1（A6BN p.136）。
- 注意：A6BN 的 6041h bit14 是保留位（固定 0），沒有「torque limit active」位元（A6BN p.92）；要看扭力限制中可看 bit11 或 4F22h bit5 TLC（A6BN p.260）。

位置控制的 bit10／bit13（A6BN p.110-112）：
- bit10 target reached：伺服 ON、命令產生完成、|6062h-6064h| 在 6067h（Position window）內持續 6068h（Position window time）→ 1；halt 時表示軸已停。
- bit13 following error：60F4h 超過 6065h（Following error window）持續 6066h（Following error time out）→ 1（pp、csp）。

csp 的 bit12（drive follows command value）＝1 的條件（A6BN p.133）：Operation enabled、沒有在減速（halt、POT/NOT、quick stop、shutdown、disable operation、fault、軟體極限；3787h bit13 決定 POT/NOT 算不算）、不在 halt、移動方向沒碰到 POT／NOT、沒有扭力限制（3724h bit11＝0 時才算）、沒有超出 607Dh、不在磁極估測中。

## 4. 運轉模式（6502h／6060h／6061h）

| 6060h 值 | 模式 | A6BN 支援 | 6502h bit |
|---|---|---|---|
| 0 | No mode change／no mode assigned（開機預設） | 是 | － |
| 1 | pp Profile position | 規格表寫 Yes，但**龍門型不支援**（可設不保證動作） | 0 |
| 2 | vl Velocity | 否 | 1 |
| 3 | pv Profile velocity | 是 | 2 |
| 4 | tq Torque profile | 是 | 3 |
| 6 | hm Homing | 是 | 5 |
| 7 | ip Interpolated position | 否（不要設 7） | 6 |
| 8 | csp Cyclic synchronous position | 是 | 7 |
| 9 | csv Cyclic synchronous velocity | 是 | 8 |
| 10 | cst Cyclic synchronous torque | 是 | 9 |

出處：A6BN p.93-95、p.138。6502h 的位元圖（A6BN p.93）＝bit0、2、3、5、7、8、9 為 1，換算 03ADh（推算）；規格書註明回應可能隨軟體版本不同。

切模式注意（A6BN p.94、p.96）：
- 6060h 開機是 0，**上電後一定要設**；6060h 與 6061h 都是 0 時進 Operation enabled → Err88.1。
- 設過非 0 之後再寫 0，視為「不改」，保留上一個模式。
- 設不支援的模式：用 SDO 寫會回 abort（超出範圍）（A6BN p.94）；6060h 被設成不支援的模式 → Err88.1（A6BN p.96、p.295）。
- 切換約 **2 ms**，期間 6061h 與跟模式有關的 TxPDO 值不定。
- **A6BN 不支援運轉中切模式**；要先停。運轉中（含回原點、減速停止中）切換不保證，可能 Err27.4。
- 例外：3698h bit8＝1 時，可從 csp 直接切到 hm 並以 6040h bit4＝1 起動回原點（只支援從 csp 切過來）（A6BN p.142）。
- 6060h 存 EEPROM（EEPROM 欄 Yes，A6BN p.94、p.334）。

## 5. csp（主站產生軌跡）（A6BN p.128-137）

- 目標位置＝607Ah＋60B0h（Position offset），視為絕對位置（A6BN p.134）。
- 60C2h 就是 607Ah／60B0h 的更新週期，等於 1C32h:02；主站必須照這個週期更新（A6BN p.134）。
- **伺服 OFF 時，主站要讓 607Ah＋60B0h 一直跟著 6064h**；不跟的話，外力推動過機構，下次伺服 ON 會猛衝回舊命令位置，很危險。從其他模式切進 csp 也一樣（A6BN p.134）。
  → 這就是 Steven「伺服 ON 時把 encoder 回寫成 command」的規格書依據（`D:\HT9045\.claude\skills\ht9045-motor-control\references\index-torque-autoheight.md` §5）。
- 通訊錯誤造成 607Ah 沒收到時，驅動器推估目標並校正（A6BN p.135）。
- 3722h bit5＝1：用 6080h（Max motor speed）把每週期命令變化量飽和，避免 Err27.4（A6BN p.136-137）。
- 6084h（Profile deceleration）在 csp／csv 只在減速停止程序中有效（A6BN p.101）。

其他模式：pv（p.176-185）、csv（p.186-192）、tq（p.200-207）、cst（p.208-214）。cst 的目標扭力＝6071h＋60B2h，上限在 3521h＝5 時是 60E0h／60E1h，否則是 3013h（A6BN p.214）。

## 6. 回原點（hm）

### 6a. 物件（A6BN p.140-147、p.333）

| 物件 | 內容 |
|---|---|
| 6040h bit4 | start homing，0→1 起動；起動時鎖住當下的 method、速度、加減速；回原點中再觸發會被忽略（A6BN p.142） |
| 6098h Homing method | I8；hm 中不能改；設不支援的值起動 → homing error（6041h bit13）（A6BN p.143） |
| 6099h:01 | 找開關速度（command/s）（A6BN p.144） |
| 6099h:02 | 找零點速度；原點是開關邊緣時要盡量小（A6BN p.144） |
| 6099h 上限 | 取「Max profile velocity、6080h、2147483647」最小者（規格書此處把 607Fh 誤寫成 60F7h）（A6BN p.144） |
| 609Ah Homing acceleration | 加速與減速共用；最後停在原點時用 servo lock，不用這個值；0 視為 1（A6BN p.144） |
| 607Ch Home offset | 完成時 6062h＝6064h＝607Ch；回原點中改它，下一次才生效（A6BN p.148、p.253） |
| 5350h／5351h／5352h | 撞機械端（method -1~-4）的扭力上限（0.1 %）、判定時間（ms）、判定速度（command/s）（A6BN p.144、p.333） |
| 60E3h Supported homing method | 36 個：1-14、17-30、33、34、35、37、-1、-2、-3、-4（A6BN p.147） |
| 3722h bit6／3793h | 回原點折返速度限制（減少完成時的異音）（A6BN p.150） |
| 3722h bit7 | 用 Z 相回原點時建議設 1：Z 相找過頭碰到禁止輸入時報 Err94.3（A6BN p.149） |

### 6b. 6098h 方法清單（A6BN p.143）與接線前提（A6BN p.152-167）

| method | 內容 | 必須配好的輸入（缺了 → homing error） |
|---|---|---|
| 1／2 | 負／正極限開關＋Index pulse | 1：NOT；2：POT |
| 3、4／5、6 | 正／負向 Home switch＋Index pulse（方向依起動時開關狀態） | HOME |
| 7-10 | Home switch＋Index pulse，正向起動（7、8 起動時在開關上 → 先往負方向；9、10 → 正方向） | HOME、POT |
| 11-14 | Home switch＋Index pulse，負向起動（11、12 起動時在開關上 → 先往正方向；13、14 → 負方向） | HOME、NOT |
| 17／18 | 同 1／2 但不用 Index pulse，原點＝極限開關變化點 | 17：NOT 接 SI7；18：POT 接 SI6 |
| 19-22 | 同 3-6 但不用 Index pulse | HOME 接 SI5 |
| **23-26**（含 **24**） | 同 7-10 但不用 Index pulse，原點＝Home switch 變化點 | **HOME 接 SI5、POT 要配** |
| **27-30**（含 **28**） | 同 11-14 但不用 Index pulse | **HOME 接 SI5、NOT 要配** |
| 33／34 | 只用 Index pulse，負／正方向 | － |
| 35／37 | 目前位置＝原點（設定座標）；伺服 OFF 也能做；新設計用 37 | 停止命令 100 ms 後才執行；伺服 OFF 或運轉中不保證 |
| -1／-2 | 撞機械端（正／負） | 用 5350h-5352h；判定前可能先 Err24.0（調 5350h、3014h）；回原點中不偵測 Err16.1 |
| -3／-4 | 撞機械端後反向找第一個 Index pulse | 同上 |

- 原點用開關邊緣（HOME、POT、NOT）時，這三個要分別配到 **SI5、SI6、SI7**（latch 修正腳）；配錯 → homing error（A6BN p.148、p.151）。
- 推論：移植樹在 PCI-1203 上送 `Acm_AxHome(124|128)`＝method 24／28（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EtherCAT\Pci1203MotorRoute.cpp:638`）。A6BN 支援；起動時壓在 Home switch 上，24 依「同 7、8」先往負方向、28 依「同 11、12」先往正方向（A6BN p.156-157、p.162-163）——也就是會先退開，不像 HT9050 的 SW3D-680 一直往前找（`ht9050-1203-homing` §3a）。上機確認。

### 6c. Statusword 組合（hm）（A6BN p.146）

| bit13 homing error | bit12 homing attained | bit10 target reached | 意義 |
|---|---|---|---|
| 0 | 0 | 0 | 回原點中 |
| 0 | 0 | 1 | 中斷或還沒開始 |
| 0 | 1 | 0 | 回原點完成，但還沒到目標位置 |
| 0 | 1 | 1 | **回原點成功完成** |
| 1 | 0 | 0 | 偵測到錯誤，仍在動 |
| 1 | 0 | 1 | 偵測到錯誤，已停 |

bit12 變 0 的時機（A6BN p.146）：控制電源 ON、ESM Init→PreOP、開始回原點（method 35／37 也會短暫約 2 ms 變 0）、PANATERM 操作結束（3799h bit0＝1 時）、Err27.4 發生。**絕對式 feedback scale 模式 bit12 永遠是 1**。

### 6d. 回原點流程與錯誤

- 增量式 scale 模式：上電後要先回原點才能定位（A6BN p.139、p.148）。
- 完成時：6062h＝6064h＝607Ch、6063h＝60FCh＝0；之前的 touch probe 值要重抓（A6BN p.148、p.253）。
- 偵測到原點到完成之間，6064h 與 60F4h 不定（A6BN p.148）。
- **偵測到原點到完成之間被主站 halt 等中斷 → Err27.7（位置資訊初始化錯誤）**（A6BN p.149），這個警報不可 clear（A6BN p.272）。
- 折返速度超過 3513h → Err26.0；超過 3615h → Err26.1（A6BN p.150）。
- Homing error 的條件（A6BN p.151）：絕對式模式下起動、不在 Operation enabled 起動（35、37 除外）、6099h:01／02 為 0（例外：35/37 的 01h；33/34/35/37 的 02h）、正負極限同時偵測到、穿越極限／Home 開關、開關安裝關係不對、HOME／POT／NOT 沒配到 SI5／SI6／SI7、method -1~-4 在撞到機械端前先碰到極限。3504h＝0 時「兩邊極限都 ON」改報 Err38.0。

## 7. Touch probe（60B8h-60BDh）（A6BN p.215-224）

- 觸發源：EXT1（要配 SI5）、EXT2（要配 SI6）或 Z 相；latch 的是 feedback 位置（A6BN p.215-216）。
  SI5／SI6 的設定值：3404h＝00202020h（EXT1 a 接點）／00A0A0A0h（b 接點）；3405h＝00212121h（EXT2 a 接點）／00A1A1A1h（b 接點）（A6BN p.216）。
- 觸發訊號 ON、OFF 寬度都要 **2 ms 以上**；外部輸入有取樣差，觸發附近速度要放慢（A6BN p.215）。
- 選 EXT1／EXT2 但沒配輸入 → Err88.3；絕對式 scale 選 Z 相 → Err88.3；Z 相不要選下降緣（A6BN p.215、p.296）。
- ESM 回 Init 或切到 hm 時 touch probe 自動關閉（60B9h 清 0）（A6BN p.215）。
- 推論／待確認：回原點用 Home 開關邊緣要 HOME 接 SI5（A6BN p.148），touch probe EXT1 也要接 SI5（A6BN p.215）；兩個能不能同時用，規格書未載明。

60B8h（Touch probe function）位元（A6BN p.218）：

| bit | probe 1 | bit | probe 2 |
|---|---|---|---|
| 0 | 1＝啟用 | 8 | 1＝啟用 |
| 1 | 0＝只抓第一次、1＝連續 | 9 | 同左 |
| 2 | 0＝EXT1、1＝Z 相 | 10 | 0＝EXT2、1＝Z 相 |
| 4 | 1＝上升緣取樣 | 12 | 1＝上升緣取樣 |
| 5 | 1＝下降緣取樣 | 13 | 1＝下降緣取樣 |
| 3、6、7 | 保留／不支援 | 11、14、15 | 保留／不支援（15 廠商用） |

- 外部輸入時上升＋下降緣可同時開（A6BN p.218）。
- 起動：bit0／bit8 從 0→1 時才讀入其他設定；改設定要先回 0 再設 1（A6BN p.221）。

60B9h（status）：bit0／8 運作中；bit1／9 上升緣已存；bit2／10 下降緣已存（A6BN p.219）。3697h bit13＝1 時，每次 latch 讓 bit1/2/9/10 反轉輸出（連續模式用）（A6BN p.215、p.223）。
60BAh／60BBh＝probe 1 上升／下降緣位置；60BCh／60BDh＝probe 2（command 單位、TxPDO）（A6BN p.220）。
延遲修正：3709h（25 ns 單位，-2000~2000），3724h bit5＝1 時上升用 3709h、下降用 3792h（A6BN p.224）。

## 8. 停止與錯誤反應的選項碼（A6BN p.225-237）

減速優先順序（高→低）（A6BN p.226）：伺服側減速（警報時）＞STO 減速＞伺服側減速（伺服 OFF、主電源 OFF）＞伺服側減速（驅動禁止）＞Fault 減速＞退避動作＞其他 CoE 側減速（quick stop、shutdown、disable operation）＞極限類減速（POT／NOT、軟體極限）＞Halt 減速＞一般減速。高優先的來了就切過去；低優先的來了維持先接受的那個。

| 物件 | 值 | 位置／速度模式（pp、csp、ip、csv、pv） | hm | tq、cst |
|---|---|---|---|---|
| 605Ah Quick stop option code（A6BN p.230） | 0 | 照 3506h 停 → Switch on disabled | 同左 | 同左 |
| | 1 | 6084h 停 → Switch on disabled | 609Ah | 6087h |
| | 2 | 6085h 停 → Switch on disabled | 6085h | 6087h |
| | 3 | 60C6h 停 → Switch on disabled | 60C6h | 0 扭力 |
| | 5／6／7 | 同 1／2／3，但停在 **Quick stop active** | 同左 | 5、6 用 6087h；7 用 0 扭力 |
| | -1、-2 | 廠商用 | | |
| 605Bh Shutdown option code（A6BN p.232） | 0／1 | Shutdown：照 3506h／6084h 停 → Ready to switch on；Disable voltage：→ Switch on disabled | 1＝609Ah | 1＝6087h |
| 605Ch Disable operation option code（A6BN p.234） | 0／1 | 照 3506h／6084h 停 → Switched on | 1＝609Ah | 1＝6087h |
| 605Dh Halt option code（A6BN p.235） | 1／2／3 | 6084h／6085h／6072h＋60C6h 停，維持 Operation enabled | 609Ah／6085h／6072h＋60C6h | 1、2＝6087h；3＝0 扭力 |
| 605Eh Fault reaction option code（A6BN p.236） | 0／1／2 | **只對 Err80-88**：照 3510h／6084h／6085h 停 → Fault；其他警報一律照 3510h | 609Ah 取代 6084h | 1、2＝6087h |
| 6007h Abort connection option code（A6BN p.227） | 0-3 | **主電源被切斷時**的減速：0 不動作、1 Fault（依 605Eh）、2 Disable voltage（依 605Bh）、3 Quick stop（依 605Ah） | | |

- 6007h 相關：3507h（主電源 OFF 時伺服側減速）、3508h bit0（0＝伺服 OFF、1＝報 Err13.1）、3509h（主電源 OFF 偵測時間，20-2000 ms；2000＝停用 AC 斷電偵測）。主電源切斷後 70 ms 開始依 6007h 減速，超過 3509h 改依 3507h（A6BN p.227-229）。P-N 間電壓下降 → Err13.0 最優先，依 3510h（A6BN p.228）。
- 6007h＝1 且主電源 OFF（Operation enabled／Quick stop active）→ Err88.0（A6BN p.294）。
- 3506h（伺服 OFF 時的停止方式）、3510h（警報時的停止方式，Err80-88 以外）詳細在 SX-DSV03735 §6-3-2，本規格書未載明數值意義（A6BN p.226）。
- 出貨時，EtherCAT 相關警報（Err80-88）的減速依 605Eh；極限輸入的減速依 6085h；要依設備調整（A6BN p.16）。
- 驅動禁止（POT／NOT）：3504h＝0 依 3505h 停；＝1 依 6085h（位置／速度）或 6087h（扭力）停；＝2 → Err38.0。3511h 緊急停止扭力（%，0＝用一般扭力限制）。36A2h（csp）解除禁止的偏差門檻，3504h≠1 時設 0（A6BN p.237）。
- POT 要裝在正方向那一側、NOT 在負方向；裝反動作不保證；要預留減速距離（A6BN p.237）。

## 9. 位置資訊（A6BN p.243-253）

- 位置資訊（4F04h、6062h、6063h、6064h、60FCh）重設的時機：**控制電源 ON、ESM Init→PreOP、回原點完成**、清絕對式多圈、PANATERM 試運轉／頻率分析／Z 相搜尋／fit gain 結束、PANATERM 設腳位、**Err27.4 發生**（A6BN p.243）。
- 電子齒輪：用 608Fh、6091h、6092h（不用 Pr0.08-0.10）；有效範圍 8000 倍到 1/1000，超出或運算溢位 → Err88.3（A6BN p.244）。線性型 608Fh:01 固定 1,000,000、608Fh:02＝feedback scale 解析度（nm/p）；出廠 6092h:01 讓 1 nm/p 的 scale 是 1:1（A6BN p.244）。生效時機同上（A6BN p.244）。
- 607Eh Polarity：只能 0（不反轉）或 224（位置、速度、扭力都反轉）；也作用在 POT／NOT（A6BN p.249-250）。
- 607Bh Position range limit：A6BN 固定 80000000h／7FFFFFFFh（改不了）（A6BN p.22、p.252）。
- 607Dh 軟體極限：只在 pp、ip、csp；座標確定後才生效（絕對式：PreOP 以上；增量式：回原點完成後）；01h＜02h 才啟用，01h≥02h 關閉；啟用時座標 wraparound → Err88.3；碰到極限依 quick stop 斜率減速（A6BN p.102-103）。增量式在 Init→PreOP 後軟體極限失效，要重新回原點（A6BN p.102）。
- 絕對式 feedback scale：位置控制下不需要回原點；6064h＝((H×2^24＋L)×電子齒輪反算)＋607Ch，溢位 → Err29.1（A6BN p.251）。

<!-- preserved-content:end -->
