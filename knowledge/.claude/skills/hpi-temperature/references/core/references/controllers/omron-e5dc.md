> 保存來源：`.claude/skills/ht9045-temperature/references/controllers/omron-e5dc.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# Omron E5DC（E5□C 系列：E5CC／E5EC／E5AC／E5DC／E5GC）（溫控器手冊參照）

> 手冊在 Steven01 本機 `E:\HT9045W_相關料件技術文件\溫控器\OMROM溫控器\`（資料夾名拼成 OMROM），不在 repo；其他電腦沒有 E: 這顆磁碟。頁碼寫「印刷頁（PDF 第幾頁）」，PDF 頁是用 pdftotext 數 form-feed 得到的。

- golden V912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，Big5／cp950）
- 移植樹 V906＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`
- 下文只寫 `檔名:行號` 的，沒特別標就是 golden V912。

## 1. HT9045 在哪裡用

### 1.1 設定鍵與值
| 鍵 | 值 | 讀取位置（golden） |
|---|---|---|
| `[TempCtrl] HEATER_CTRL_TYPE` | `2`＝Omron E5DC（整台）；0 TC401、1 KT4H（預設）、3 No Heater、4 DTK4848 | `database.cpp:433`；常數 `cmydef.cpp:224`（`const int E5DC=2`）；顯示字 `MachineTypeUtility.cpp:22` |
| `[TempCtrl] HeaterInsOpt_<通道>` | 逐通道廠牌，`2`＝E5DC；缺鍵回退 `HEATER_CTRL_TYPE` | `HandlerSys.cpp:50-60`（`HeaterInsOpt_Read`） |
| `[TempCtrl] COM_PORT` | 溫控 COM 埠（`HSys.sTempComPort`，預設 COM2），KT4H／TC401／DTK4848／E5DC 共用 | `database.cpp:522` |

這台機台 `D:\HT9045\system\Gerneral.ini`：`:302 COM_PORT=COM12`、`:311 HEATER_CTRL_TYPE=2`、`:19 USE_16_HEATER=2`（Index 區走 EJ1N，見 `omron-ej1n.md`）。71 通道與廠牌下拉的細節見 `D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §2～§3、`references\golden-v912-channels.md` §1。

### 1.2 COM 埠與通訊參數
- `rs232.cpp:266-283`（`flag[1] && USE_NEW_TEMPCTRL_FUNCTION==false`）：`Comm2` 開 9600 bps、8 data、Parity None、1 stop。`USE_NEW_TEMPCTRL_FUNCTION` 在 `database.cpp:340` 被強制成 false，所以一定走這段。
- 埠檢查 `rs232.cpp:165-174`（整台 No Heater 時不開）。
- ⇒ **E5DC 面板必須改成 9600／8／1／None**，出廠是 9600／7／2／Even（§3.1）。

### 1.3 站號規則
- 站號＝通道序號 `Addr`＋1，`%02d`（兩位**十進位**），`cpublic.cpp:467`（讀）、`:484`（寫）。例：HotPlate1＝01、Chamber＝10、CCD＝11；71 通道最大到 71。
- 程式沒有「指定站號」的設定；E5DC 面板的 `u-no` 要照這個公式設。

### 1.4 程式位置（golden V912）
| 作用 | 位置 | 內容 |
|---|---|---|
| 讀 PV＋Status | `cpublic.cpp:463-477` `E5DCReadTemp(Addr)` | `STX` `%02d` `00` `0` `0101` `C0` `0000` `00` `0002` `ETX` BCC `\r\n` |
| 寫 SV | `cpublic.cpp:480-488` `E5DCWriteTemp(Addr, Temp)` | `STX` `%02d` `00` `0` `0102` `C1` `0003` `00` `0001` `0000%04X` `ETX` BCC `\r\n` |
| BCC | `EJ1N\TextProcess.cpp:359-371` `SetBCC` | 從站號 XOR 到最後一字，再 XOR `ETX`（0x03） |
| 何時寫／讀 | `bthermo.cpp:2492`、`:2528-2531`、`:2562-2565`、`:2572`、`:2591-2594` | 設定溫度變了（`Temp!=OldTemp[Addr]`）才寫 `E5DCWriteTemp(Addr, Temp*10)`，否則讀；送完等回覆 0.5 s，下一步 `Task=255` |
| 換通道 | `bthermo.cpp:3209-3217` case 300 | `WriteCommand=-1`、`Addr++`、`g_iHeaterTypeIdx_SendCmd=Addr` |
| 收資料 | `rs232.cpp:747-750` | 這一筆是 E5DC 就把整包 `sprintf` 成 `Com2Buffer` |
| 解析回覆 | `bthermo.cpp:2803-2972`（case 255） | 見下表 |
| Configuration 頁手動 | `cConfiguration.cpp:5627-5629`（寫）、`:5647-5649`（讀） | 同一組函式 |

回覆解析（`Com2Buffer` 是 AnsiString，1 起算）：

| 位置 | 欄位 | 程式 |
|---|---|---|
| 1 | STX | — |
| 2-3 | 站號 | — |
| 4-5 | 子位址 `00` | — |
| 6-7 | End code | `:2820` 要 `00` |
| 8-11 | MRC/SRC | `:2850` 不是 `0101` 就 `Task=300`（不解析） |
| 12-15 | Response code | `:2821` 要 `0000` |
| 16-23 | PV（8 位 hex） | `:2881-2882` 只取 `SubString(20,4)`（後 4 位）÷10 |
| 24-31 | Status（8 位 hex） | `:2879-2880` 轉成 32 字元 0/1 字串 |
| 32／33 | ETX／BCC | BCC 沒檢查（`:2818` 被註解） |

- `:2822` 另有 `Length()<30` 長度保護（JerryYang 20240417）。
- `:2884`：Status 第 26 字（bit 6 Input error）或第 10 字（bit 22 Setup area 1）是 1 → 讀值設 999（KenHsieh 20230927）。
- 失敗重試：`MAX_RETRY=2`（`:1246`）；用完就 `StopComm`（`:2834`、`:2958`），`CommunCTErr>5` 設 `UN150CommError`、讀值 999，case 230 再 `StartComm`。
- Index 區：`USE_16_HEATER`＝2／3 時 Aa1～Bd2（3 時連 Ae1～Bh2）不走這個 COM 埠（`bthermo.cpp:1331-1367`）。

### 1.5 移植樹 V906 現況
- `cpublic.cpp:581-649` 整段 `#if 0`（`:623` `E5DCReadTemp`、`:640` `E5DCWriteTemp`；原因是 `COM2->Comm2` 沒翻）。
- `bthermo.cpp:2685-2694` 寫入（G23a）、`:2747-2756` 讀取（G23b）都 `#if 0`；case 255 解析 `:3042-3233` 還在（`:3065` 長度保護、`:3133` 取 PV）。
- 廠牌判斷只看 `TC401HeaterControl`（＝`HEATER_CTRL_TYPE`），沒有逐通道。
- ⇒ **移植樹目前不會送任何命令給 E5DC。**

## 2. 手冊清單

路徑前綴都是 `E:\HT9045W_相關料件技術文件\溫控器\OMROM溫控器\`。

| 檔名 | 語言 | 版次／日期 | 內容 | 先讀哪幾頁 |
|---|---|---|---|---|
| `E5□C Digital Temperature Controllers COMMUNICATIONS MANUAL (New).pdf` | 英 | H175-E1-10（2014-08），186 頁 | CompoWay/F、Modbus、免程式通訊 | 1-2～1-8（PDF 20-26）規格與參數；2-2～2-21（PDF 28-47）frame、服務、operation command、response code；3-2～3-10（PDF 50-58）變數表；3-23～3-25（PDF 71-73）Status |
| `E5□C Digital Temperature Controllers Users MANUAL.pdf` | 英 | H174-E1-08（2014-08），392 頁 | 安裝、端子、參數 | 1-2-2 型號（PDF 30-34）；2-28～2-30 E5DC 端子（PDF 62-64）；4-12 輸入種類表（PDF 120；步驟在 4-11，PDF 119）；6-21 `cmwt`（PDF 253）；6-66 `alfa`（PDF 298）；A-5 規格（PDF 349）；A-19 Adjustment level 預設值（PDF 363） |
| `E5CC E5ECE5AC E5DCE5CB 通訊手冊.pdf` | 中 | H181-CN1-04，165 頁 | H175 中文版 | 文字層的中文字擷取不出來（字型沒有對應碼），本文以英文版為準 |
| `E5CC E5ECE5AC E5DCE5CB 用戶手冊.pdf` | 中 | H180-CN1-04，356 頁 | H174 中文版 | 同上 |
| `E5CC E5ECE5AC E5DCE5CB 用戶手冊_參數流.pdf` | 中 | 2 頁（檔案日期 2023-06-01） | 面板層級／參數流程圖（`l.adj`、`cmwt`、`in-t`、`dp`、`sl-h`…） | 面板設定時對照 |
| `E5CC E5ECE5AC E5DCE5CB系列溫度控制器 .pdf` | 中 | 32 頁 | E5CC／E5EC 簡介，含輸入種類表 | 參考 |
| `E5CC E5ECE5AC E5DCE5CB系列溫度控制器 中文型錄 (New).pdf` | 中 | Cat. No. E5□C/E5CB-TW5-07（2014-08），274 頁 | 型錄 | PDF 56：E5DC 型號一覽 |
| `E5CC-800 E5EC-800 (Simple Type ) Digital Temperature Controllers USER MANUAL (New).pdf` | 英 | H211-E1-02，100 頁 | `-8@@` Simple Type | p.15-16（PDF 17-18）：`-800` 無選項、`-802` RS-485＋HB/HS |
| `AA-1409150015周廷瑋報價(E5DC).pdf` | 中 | 2014-09-15 | E5DC-QX2ASM-802、E5DC-CX2ASM-800、E5DC-CX2ASM-815 各 1 台 | — |
| `AA-141202001周廷瑋總報價(E5DC).pdf` | 中 | 2014-12-02 | E5DC-QX2ASM-802、-QX2AUM-802、-CX2ASM-815、-CX2AUM-815、Y92F-53 量產報價 | — |
| `AA-141127001周廷瑋總報價(E5GC).pdf` | 中 | 2014-11-27 | E5GC-QX1A6M-015、-CX1A6M-015（E5GC，不是 E5DC） | — |
| `Omron 鴻勁科技溫控器曲線調整.pptx` | 中 | 檔案日期 2015-03-04，4 張 | **內容是 E5DC**：升溫比 Panasonic 慢 2～3 分；建議進「進階層級」把 α 從 0.65 調成 0.4 或 0.3 再做 AT | 第 2 張 |

型號拆解（H174 1-2-2，PDF 30-34；E5DC 端子 PDF 62-64）：`E5DC-①②③④⑤M-⑦`，① RX 繼電器／QX SSR 電壓／CX 電流輸出；② 輔助輸出 0 或 2；③ A＝100～240 VAC、D＝24 VAC/DC；④ S＝主體＋端子座、U＝只有主體；M＝萬用輸入；⑦ 000 無、002 RS-485＋CT1、015 RS-485、016 Event 1、017 Event 1＋CT1。報價單的 `-802／-815／-800` 不在 H174 表上；H211 p.15-16 說 `-8@@` 是 Simple Type（只寫了 E5CC／E5EC）。照標準型類推，`-802`＝RS-485＋CT、`-815`＝RS-485、`-800`＝無通訊（推論，沒查到 E5DC 的原文）⇒ **`-800` 沒有 RS-485，不能接 HT9045 的溫控埠。**

## 3. 通訊重點（手冊原文整理）

### 3.1 協定與出廠值
- 支援 **CompoWay/F** 與 **Modbus-RTU**（H175 1-2，PDF 20）。通訊介面：RS-485 2 線半雙工多點、起止同步、ASCII；鮑率 9.6／19.2／38.4／57.6 kbps；資料長度 7／8 bit；停止位元 1／2；同位 none／even／odd；錯誤偵測 CompoWay/F 用 BCC、Modbus 用 CRC-16；沒有流量控制，也沒有 retry；接收緩衝 217 bytes；回送等待（send data wait）0～99 ms，預設 20 ms。
- 出廠值（H175 1-7，PDF 25）：`psel`＝cwf（CompoWay/F）、`u-no`＝1、`bps`＝9.6、`len`＝7、`sbit`＝2、`prty`＝even、`sdwt`＝20。
- 改了通訊參數要重置控制器才生效（H175 1-8，PDF 26）。通訊位址 C3 0010～0014：站號、鮑率（3＝9.6、4＝19.2、5＝38.4、6＝57.6）、資料長度、停止位元、同位（0 none、1 even、2 odd）（H175 3-10，PDF 58）。
- 收到回覆後至少等 2 ms 才能送下一個命令（H175 1-3，PDF 21）。記憶體錯誤或開機初始化中（還沒讀到 PV）不接受命令、也不回覆（H175 2-7，PDF 33）。

### 3.2 站號與配線
- `u-no` 範圍 0～99，預設 1，不可重複（H175 1-6、1-8，PDF 24、26）。frame 裡的站號是 BCD 00～99，或 `XX`（廣播，不回覆）（H175 2-2，PDF 28）。
- 1:N 最多 32 台（含 host）、總長 500 m、遮蔽雙絞線 AWG24～18（H175 1-4，PDF 22）。
- E5DC 的 RS-485 端子：4＝A(−)、3＝B(+)。傳輸線兩端都要裝終端電阻，用 120 Ω（1/2 W），合成阻值至少 54 Ω（H175 1-5，PDF 23）。E5CC／EC／AC 是 14＝A(−)、13＝B(+)（H175 1-4，PDF 22）。

### 3.3 Frame（H175 2-1～2-2，PDF 28-33）
- Command：`STX(02H)` + 站號 2 + 子位址 `00` + SID `0` + command text + `ETX(03H)` + BCC 1。
- BCC＝從站號到 ETX（含）逐 byte XOR。例：`00` `00` `0` `0503` ETX → BCC 35H（2-3，PDF 29）。
- Response：`STX` + 站號 + `00` + end code 2 + text + `ETX` + BCC。frame 沒收到 ETX＋BCC 就不回覆。
- End code（2-3，PDF 29）：00 正常、0F FINS command error、10 parity、11 framing、12 overrun、13 BCC、14 format、16 sub-address、18 frame length。
- Command text＝MRC SRC＋資料；回覆是 MRC SRC＋MRES SRES（response code）＋資料，無法執行時只回 MRC/SRC＋response code（2-6，PDF 32）。
- 變數類型（2-7，PDF 33）：C0／80＝setup area 0 唯讀；C1／81＝setup area 0 可讀寫；C3／83＝setup area 1 可讀寫（沒有 C2）。C*＝double word（8 位 hex），8*＝word（4 位）。
- 數值：8 位 hex，負數用 2 的補數，拿掉小數點再轉 hex（105.0 → 1050 → H'0000041A）（2-4，PDF 30）。

服務（MRC/SRC，2-7，PDF 33）：

| MRC SRC | 服務 | 備註 |
|---|---|---|
| 01 01 | Read Variable Area | bit position 固定 `00`；double word 一次 1～25 個（2-8，PDF 34） |
| 01 02 | Write Variable Area | double word 一次 1～24 個（2-9，PDF 35） |
| 01 04 | Composite Read | double word 最多 20 項（2-11，PDF 37） |
| 01 13 | Composite Write | double word 最多 12 項（2-12，PDF 38） |
| 05 03 | Read Controller Attributes | 回 10 字元型號＋緩衝 `00D9`（2-13，PDF 39） |
| 06 01 | Read Controller Status | 00 運轉中／01 沒在控制＋相關資訊（2-14，PDF 40） |
| 08 01 | Echoback Test | 0～200 bytes（2-15，PDF 41） |
| 30 05 | Operation Command | 見 §3.6 |

### 3.4 PV／SV 與相關位址
| 類型 位址 | 參數 | 出處 |
|---|---|---|
| C0 0000 | PV | 3-2（PDF 50） |
| C0 0001 | Status（§3.5） | 同上 |
| C0 0002 | Internal Set Point | 同上 |
| C0 000E | Decimal Point Monitor | 同上 |
| C1 0003 | **Set Point**（Operation level，範圍 SP 下限～SP 上限） | 3-3（PDF 51） |
| C1 0004／0005／0006 | Alarm Value 1／上限 1／下限 1（−1999～9999）；0007～ Alarm 2、000A～ Alarm 3 | 同上 |
| C3 0000 | Input Type：0 Pt100 −200～850、**1 Pt100 −199.9～500.0**、5 K −200～1300、**6 K −20.0～500.0**…（0～30） | 3-8（PDF 56） |
| C3 0005／0006 | SP 上限／下限 | 3-8～3-9（PDF 56-57） |
| C3 000D～000F | Alarm 1～3 Type（0 OFF、1 上下限、2 上限、3 下限、8 絕對值上限…） | 3-9（PDF 57） |
| C3 0028 | α（2-PID 常數 0.00～1.00） | 3-12（PDF 60） |
| C3 004C／004D | Protocol Setting／Send Data Wait | 3-13（PDF 61） |

- 輸入種類出廠值 5（K −200～1300 ℃，整數解析度）（H174 4-12，PDF 120；E5DC 端子頁 PDF 62 也寫出廠是 K 熱電偶）。
- 溫度輸入的小數位數由感測器種類決定，`dp` 只管類比輸入（H174 4-49，PDF 157；5-2，PDF 163）。
- Alarm 1 Type 出廠值 2（上限警報）（H174 4-8，PDF 116）。

### 3.5 Status（C0 0001）位元（H175 3-23～3-25，PDF 71-73）
| bit | 0／1 | bit | 0／1 |
|---|---|---|---|
| 0 | Heater overcurrent（CT1） | 16-19 | Event input 1～4 |
| 1 | Heater current hold（CT1） | 20 | Write mode：Backup／RAM write |
| 2 | A/D converter error | 21 | 非揮發記憶體：RAM＝NV／RAM≠NV |
| 3 | HS alarm（CT1） | 22 | **Setup area：0／1** |
| 4 | RSP input error | 23 | AT：取消／執行中 |
| 6 | **Input error** | 24 | RUN／STOP（0＝Run） |
| 7 | Potentiometer input error | 25 | Communications writing：OFF／ON |
| 8／9 | 控制輸出（加熱／冷卻） | 26 | Auto／Manual |
| 10／11 | HB alarm（CT1／CT2） | 27 | Program start |
| 12-14 | Alarm 1～3 | 28／29／31 | CT2 overcurrent／hold／HS |
| 15 | Program end output | 5、30 | spare |

### 3.6 Operation Command（30 05 + 命令碼 2 + 相關資訊 2）（H175 2-16～2-20，PDF 42-46）
| 碼 | 內容 | 相關資訊 |
|---|---|---|
| 00 | Communications Writing | 00 OFF／01 ON |
| 01 | RUN／STOP | 00 Run／01 Stop |
| 02 | Multi-SP | 00～07 |
| 03 | AT | 00 取消／01 100% AT／02 40% AT |
| 04 | Write Mode | 00 Backup／01 RAM write |
| 05 | Save RAM Data | 00 |
| 06 | Software Reset | 00 |
| 07 | Move to Setup Area 1 | 00 |
| 08 | Move to Protect Level | 00 |
| 09 | Auto／Manual | 00 自動／01 手動 |
| 0B | Parameter Initialization（只能在 setup area 1） | 00 |
| 0C | Alarm Latch Cancel | 00～05、0F 全部 |
| 0D | SP Mode | 00 Local／01 Remote |
| 0E | Invert Direct/Reverse | 00／01 |
| 11 | Program Start | 00 Reset／01 Start |

- Setup area 0＝控制中（可讀 PV、寫 SP、切 RUN/STOP）；setup area 1＝**控制停止**（才能寫 initial setting）。開機是 area 0；回 area 0 要重開電或 Software Reset（2-20，PDF 46）。移到 initial setting level 時輸出會關掉（H175 p.8 Safe Use）。

### 3.7 寫入條件、EEPROM 與 RAM write mode
- **只有 Communications Writing（`cmwt`）＝ON 才能寫參數**（H175 1-2，PDF 20）。`cmwt` 在 Adjustment level，出廠 OFF（H174 6-21，PDF 253；A-19，PDF 363）。不送 operation command 的話，就要在面板設 ON。
- 寫入回 2203 的情形：`cmwt`＝OFF、在 setup area 0 寫 setup area 1 的參數、AT 執行中、非揮發記憶體錯誤（H175 2-10，PDF 36）。
- 非揮發記憶體可寫 1,000,000 次（H174 A-5，PDF 349）。手冊要求「透過通訊頻繁改值時用 RAM write mode」（H175 p.8；2-18，PDF 44）。RAM write mode 要 `cmwt`＝ON 才有效；把 `cmwt` 關掉時，operation／adjustment level 的參數會寫進非揮發記憶體（2-18）。手冊這幾段沒寫出廠的 write mode，可以讀 Status bit 20 確認。

### 3.8 Response code（H175 2-21，PDF 47）
0000 正常；0401 不支援的命令；1001 命令過長；1002 過短；1101 變數類型錯；1103 起始位址超出範圍；1104 結束位址超出範圍；1003 元素數與資料數不符；110B 回應過長；1100 參數錯（bit position 不是 00、寫入值超出範圍、operation command 碼錯）；3003 寫唯讀（C0）；2203 操作錯誤（見 §3.7）。

## 4. 與程式對照

| 項目 | 手冊 | golden V912 | 判定 |
|---|---|---|---|
| Frame、SID、子位址、BCC | §3.3 | `cpublic.cpp:467-476`、`TextProcess.cpp:359-371` | 一致 |
| 站號 | BCD 00～99 | `%02d`（Addr+1） | 一致；序號 ≥99 會變 3 位數，但 71 通道用不到 |
| 讀命令 | 0101、C0 0000 起 2 個＝PV＋Status | `0101 C0 0000 00 0002` | 一致 |
| 寫命令 | 0102、C1 0003＝Set Point | `0102 C1 0003 00 0001 0000%04X` | 一致 |
| 回覆欄位位置 | §3.3 | §1.4 表 | 一致；Status 用到的 bit 6、bit 22 位置也對 |
| 通訊參數 | 出廠 9600／7／2／Even | 9600／8／1／None（`rs232.cpp:268-273`） | **面板一定要改**，否則所有回覆都是 parity／framing 錯 |
| 小數點 | 溫度小數位數由輸入種類決定；出廠 5（K，整數） | PV ÷10（`bthermo.cpp:2882`）、SV ×10（`:2530`） | **輸入種類必須是 0.1 解析度**（1、2、3、4、6、8、10、14）。程式不寫輸入種類（EJ1N 那邊會寫，E5DC 不會）。出廠值時讀值小 10 倍；寫 125 ℃ 會送 1250，又在 K 的 SP 上限 1300 以內，會被接受成 1250 ℃ |
| `cmwt` | 要 ON 才能寫 | 從不送 `3005 00 01`，也不看 Status bit 25 | 面板要設 ON；沒設的話寫入回 2203，而程式（見下一列）分不出是 `cmwt` 的問題 |
| 寫入確認被當錯誤 | 0102 正常回覆只有 MRC/SRC＋0000，整個 frame 17 bytes | `:2822` 的 `Length()<30` 排在 `:2850` MRC/SRC 判斷前面 | **疑點（從程式推的，沒量過）**：正常的寫入確認會走錯誤路徑（`ReceiveErr=true`、`CommRetry[Addr]−1`）。case 300 把 `WriteCommand` 設回 −1，下一次讀成功會把 `CommRetry` 補回（`:2848`），平常沒事；同一通道在兩次成功讀取之間連寫 3 次以上，會走到 `StopComm`＋`CommunCTErr++`（`:2834`） |
| 回覆 BCC | 收端應驗 BCC | 不驗（`:2818` 註解掉） | 偏離；EJ1N 那邊有驗 |
| BCC 後面的 CR LF | frame 到 BCC 結束 | 多送 `\r\n`（`cpublic.cpp:476`、`:486`） | 手冊沒定義 BCC 後的多餘字元；依「重收 STX 會重新開始接收」與「沒 ETX＋BCC 不回覆」推測被忽略，現場可用 |
| PV 取幾位 | 8 位 hex、2 的補數 | 只取後 4 位（`:2881`） | 0 ℃ 以下會讀成 6000 多度；正溫度沒問題 |
| SV 負值 | 8 位 hex | `0000%04X` | 負數會印成 `0000FFFFxxxx`（12 位）→ 1003／1100；只影響負的設定溫度 |
| SV 取整 | 整數 EU | `Temp*10` 傳給 `int Temp`（`cpublic.cpp:480`）→ 無條件捨去 | 誤差 < 0.1 ℃ |
| Status 註解 | bit 3＝HS alarm（CT1） | `bthermo.cpp:2865` 寫「SSR故障」 | 只是註解用字；bit 22「設定模式」＝Setup area 1，對 |
| 接收拼包 | — | `rs232.cpp:749` 每次 OnReceive 覆蓋 `Com2Buffer` | 33 bytes 若分兩次進來，前半會被蓋掉 → 解析失敗 → retry（程式面風險，手冊沒規定） |
| EEPROM | 頻繁寫用 RAM write mode | 不送 `3005 04 01`；只在設定溫度變了才寫（`:2492`） | 正常用法不算頻繁；如果日後改成週期性寫入，要先切 RAM write mode |
| 回應時間 | send data wait 預設 20 ms；收完至少等 2 ms | 等 0.5 s（`:2572`），一次一通道 | 一致 |

移植樹 V906：E5DC 的送收都還在 `#if 0`（§1.5）。上表的疑點（長度保護、BCC、負值、拼包）都是 golden 原本的寫法；解閘時要改就算偏離 golden，要另外裁決。

## 5. 注意事項（E5DC 上 HT9045 前的面板設定與配線）

1. **通訊設定層**（在 operation level 按 O 鍵 3 秒進 initial setting，再按 O 鍵不到 1 秒；H175 1-7，PDF 25）：`psel`＝cwf、`u-no`＝通道序號＋1（§1.3）、`bps`＝9.6、`len`＝8、`sbit`＝1、`prty`＝none、`sdwt` 用預設 20。改完要重開電（H175 1-8）。
2. **輸入種類**（initial setting level `in-t`）：依感測器選 0.1 解析度那一檔（Pt100 → 1、K → 6）；不要用出廠值 5。建議同時把 SP 上限（`sl-h`，C3 0005）設成機台允許的最高溫，萬一單位搞錯也有保護。
3. **Adjustment level 的 `cmwt` 設 ON**，否則 HT9045 寫不進 SV，控制器會照面板上的 SP 跑。
4. 不要讓控制器停在 initial setting level：那時控制停止（setup area 1），HT9045 會顯示 999（`bthermo.cpp:2884`）。
5. 配線：RS-485 B(+)／A(−) 不可接反（E5DC 端子 3／4，以實機端子座標示為準）；傳輸線兩端接 120 Ω 1/2 W 終端電阻；遮蔽雙絞線；同一段最多 32 台（含 PC）、總長 500 m。同一條線上還有 KT4H／DTK4848 時，站號不能撞（站號都由序號推，TC401 一台吃 4 個序號，見 heater-control skill §3.3）。
6. 型號：要選有 RS-485 的（標準型 `-002`／`-015`；報價單的 `-802`／`-815`）。`-800` 沒有通訊（§2 推論）。
7. **PID／ON-OFF**：`cntl` 出廠是 `onof`（ON/OFF 控制，H174 6-44，PDF 276），`st` 出廠 `on`（6-45，PDF 277）；ON/OFF 控制時不能做 AT（6-20，PDF 252）。2015 年的 α 投影片與 2018 年的選用比較都是在 2-PID＋AT 下做的 ⇒ 建議 `cntl`＝`pid` 並做一次 AT（20261001 ST01-E3 對手冊找到；現場標準待 EastSun 確認）。程式不讀也不寫這一項。
8. 升溫太慢：照 `Omron 鴻勁科技溫控器曲線調整.pptx`，在進階功能層把 `alfa` 從 0.65 調成 0.4～0.3 後重做 AT（`alfa` 要 2-PID 且 ST＝OFF 才顯示；H174 6-66，PDF 298）。進階功能設定層要先在保護層把 `icpt` 設 0（出廠 1），再在初始設定層 `amov` 輸入 −169（H174 4-5～4-6，PDF 113-114）。
9. 有兩個 Setup Tool port 的機種（含 E5DC），兩個 port 不要同時插線；USB 轉換線插著時不要開關控制器電源（H175 p.8）。

<!-- preserved-content:end -->
