> 保存來源：`.claude/skills/ht9045-temperature/references/controllers/rkc-srz.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# RKC SRZ（溫控器手冊參照）

> 手冊放在 Steven01 本機 `E:\HT9045W_相關料件技術文件\溫控器\RKC_SRZ_溫控器\`（不在 repo；其他電腦沒有這顆 E 槽）。
> **HT9045 程式沒有用到 SRZ**（§4）。本檔只是讓人知道這包資料是什麼，將來真的要接再回來看手冊。
> 頁碼寫「印刷頁（PDF 頁）」，因為 `SRZ_Menual.pdf` 的印刷頁是「章-頁」格式。整理日期 20261001，用 `pdftotext` 抽字；資料夾裡的 .exe 一律沒有執行。

## 1. SRZ 是什麼

- RKC INSTRUMENT 的 **Module Type Controller SRZ**：裝在 DIN 軌上的模組式溫控器。主要模組有 **Z-TIO**（溫度控制模組，2 通道或 4 通道）與 **Z-DIO**（數位 I/O 模組）（`SRZ_Menual.pdf` 1.2～1.3 節，p.1-3～1-7）。
- 模組用側面的 joint connector 互接，通訊線只要接到其中一個模組，接在一起的模組都能通訊（`SRZ.pdf`）。Z-TIO（主機通訊型 A／B）最多接 16 個；同一條通訊線上的 SRZ 模組（含其他功能模組）最多 31 個（p.10-11（PDF 319）；`SRZ.pdf` 2.5 節）。
- 電源 24 V DC，4 通道型最大 140 mA、2 通道型最大 80 mA（`SRZ.pdf`）。
- 一個模組多通道、站號一個模組一個——這點跟程式裡的 TC401（一台 4 通道，站號＝序號÷4＋1）類似，但協定與暫存器完全不同。

## 2. 手冊清單

| 檔案（絕對路徑） | 語言 | 版本／日期 | 內容 | 先看哪幾頁 |
|---|---|---|---|---|
| `E:\HT9045W_相關料件技術文件\溫控器\RKC_SRZ_溫控器\SRZ_Menual.pdf` | English | IMS01T04-E1，Copyright 2006；346 頁 | SRZ 完整 Instruction Manual（Z-TIO＋Z-DIO） | 目錄 PDF 4-14；5 章通訊前設定 p.5-2～5-5（PDF 52-55）；7 章 Modbus p.7-1～（PDF 89～）；資料表 p.7-22～7-48（PDF 110～）；規格 p.10-11（PDF 319） |
| `E:\HT9045W_相關料件技術文件\溫控器\RKC_SRZ_溫控器\RKC_SRZ Inst_c1中文操作手冊_20081209.pdf` | 中文 | IMS01T04-C1（封面），Copyright 2006；344 頁 | 上面那本的中文版 | 目錄的章節與頁碼跟 E1 相同（5-1、7-1 MODBUS、7-22、7-23、10-1）。**內文抽不出字**（pdftotext：Unknown character collection 'Adobe-Japan1'），Read 工具也無法看圖 ⇒ 本次只核對目錄，內容以英文版為準 |
| `E:\HT9045W_相關料件技術文件\溫控器\RKC_SRZ_溫控器\SRZ.pdf` | English | Z-TIO Instruction Manual IMS01T01-E4，Copyright 2006；2 頁（A3 單張） | Z-TIO 安裝、互接、配線、端子 | 通訊配線它叫你看 IMS01T02-E（Host Communication Quick Operation Manual），**那本不在資料夾裡** |
| `E:\HT9045W_相關料件技術文件\溫控器\RKC_SRZ_溫控器\SRZIni.ini` | — | 587 bytes，2011-12-29 | `[COMPAR]` ADDRESS=0、COMPORT=3、SPEED=3、DATABIT=0、PARITY=0、STOPBIT=0、DELAY=0、LOADER=1、ID_LO=0；`[GRAPH]` PV／SV／MV 顏色與 4 組上下限（PV 0～400、MV -10～110）、LOGFILE=0；`[OTHER]` LOG=0 | 推測是下面 WinUCI 工具的設定檔（LOADER=1 看起來是走 loader 通訊）；沒有執行工具確認 |
| `...\RKC_SRZ_溫控器\WinUciSRZje1201.exe` | — | 1,064,960 bytes，2008-09-03 | 推測是 RKC 的 SRZ PC 設定軟體（WinUCI-SRZ） | **不要執行** |
| `...\RKC_SRZ_溫控器\comk_driver_a.exe`、`comk_driver_b.exe` | — | 163,410／3,168,642 bytes，2011-11-04 | 推測是 COM-K（RKC 的 USB loader 轉換器，手冊 p.10-11 提到）驅動程式 | **不要執行** |

## 3. 通訊摘要（`SRZ_Menual.pdf`）

- 主機通訊：RS-485 兩線、半雙工 multi-drop、start-stop 同步；終端電阻外接（p.10-11（PDF 319））。
- 協定二選一：**RKC communication**（ANSI X3.28-1976 subcategory 2.5, B1）或 **Modbus-RTU**；用模組右側 DIP 開關第 6 位選（OFF＝RKC、ON＝Modbus），**出廠＝RKC communication**；第 7、8 位一定要 OFF；同一條線上所有模組 DIP 1～8 要設一樣，改了要重開電或切 RUN/STOP 才生效（p.5-3（PDF 53））。
- 速度：DIP 1、2 四種組合對應 4800／9600／19200／38400 bps，**出廠 19200**（p.5-3）。
- 資料格式：DIP 3～5；7 bit 的設定在 Modbus 下無效（Modbus 只能 8 bit），**出廠 8 bit 無同位、stop 1**（p.5-3）。
- 站號：模組正面旋轉開關 0～F，出廠 0。Modbus 位址＝設定值＋1（Z-TIO 1～16）；Z-DIO＝設定值＋17（17～32）。RKC communication 則是 Z-TIO 0～15、Z-DIO 16～31（p.5-2（PDF 52））。
- Modbus 只有 **RTU 模式**：8 bit 二進位、CRC-16、訊息內字元間隔要小於 24 bit 時間（p.7-3（PDF 91））。功能碼 03H 讀、06H 寫單筆、08H loopback、10H 寫多筆（p.7-3）。錯誤回應＝功能碼＋80H，錯誤碼 1 功能碼錯、2 位址錯、3 筆數超過、4 自我診斷錯（p.7-4（PDF 92））。
- 回應時間上限：03H 50 ms、06H 30 ms、08H 30 ms、10H 100 ms（p.5-5（PDF 55））。
- 資料：0000H～FFFFH（FFFFH＝-1），有小數點的值去掉小數點當整數（p.7-12（PDF 100））。Engineering setting 資料在 RUN 中是唯讀，要先切 STOP 才能改（p.7-22（PDF 110））。上電時是 STOP，RUN/STOP 開關切到 RUN 才開始控制（p.5-4（PDF 54））。
- 常用暫存器（Z-TIO，p.7-23～7-31（PDF 111-119））：

| 項目 | 位址 | 屬性 |
|---|---|---|
| Measured value (PV) | CH1～CH4＝0000H～0003H | RO |
| Comprehensive event state | CH1＝0004H 起 | RO |
| RUN/STOP transfer | 006DH（每模組一個） | R/W，0＝STOP |
| Event 1 set value (EV1) | CH1＝0076H 起 | R/W |
| Set value (SV) | CH1＝008EH、CH2＝008FH（依序） | R/W |
| Input type | CH1＝0176H 起 | R/W（Engineering） |
| Decimal point position | CH1＝017EH 起 | R/W（Engineering） |

- Loader 通訊（給 PC 工具用）：USB 轉換器 COM-K、38400 bps、8 bit、無同位、stop 1、ANSI X3.28、一對一（p.10-11）。

## 4. HT9045 程式沒有用到

證據（20261001 grep）：
- golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（*.cpp／*.h／*.dfm）：`grep -i rkc` 只命中名稱裡剛好有這三個字母的識別字——`SwRKCleanOut`（12）、`SnRKCoverOpen`（9）、`SnRKCleanOut`（9）、`DoTeachMarkCalibration`（6）、`SwRKCoverOpen`（5）、`ledSnRKCleanOut`、`btnSwRKCleanOut`、`iATCBackupErrKCode`、`NetworkConnect`、`ATC_NetworkConnect`、`epn_SnRKCleanOut`、`bScanReworkConsFail`。`SRZ`／`Z-TIO`／`ZTIO`／`WinUci`：0 筆。
- 移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`：同樣只有上面那類 IO 名稱（另有 `build\` 下產生的 IOSetView ID、`MarkCfgDue_`）；`SRZ`／`Z-TIO`：0 筆。
- `D:\HT9045\web\page\`、`D:\HT9045\system\`、`D:\HT9045\.claude\skills\`：`\bRKC\b|\bSRZ\b|Z-TIO` 0 筆。
- 廠牌值裡沒有 RKC：`HEATER_CTRL_TYPE` 只有 0 TC401、1 KT4H、2 E5DC、3 No Heater、4 DTK4848（golden `cmydef.cpp:222-226`）；`USE_16_HEATER` 0～6 只有 KT4H／EJ1N／DTME08（`MachineType.h:717-723`）；沒編譯的舊架構 `TempCtrl\MyTemptureSet.h:4-8` 也只有 UT100／KT4H／DT4848／WT404／TMC401。
- 同資料夾的 `溫控器改善報告.pdf`（2012）與 `溫控器選用差異表20181025.pptx`（2018）都沒提到 RKC。

結論：SRZ 不是 HT9045 的溫控器，程式裡沒有任何 RKC 通訊碼。檔案日期集中在 2008～2011（`SRZ.pdf`／`SRZ_Menual.pdf` 2011-11-02、`SRZIni.ini` 2011-12-29），推測是 2011 年前後評估過，但資料夾裡沒有評估結果的文件。

將來真的要接的話：要走 Modbus-RTU（DIP 6 改 ON、DIP 3～5 選 8 bit），新寫一組 RTU 讀寫函式（CRC 可沿用 golden `cpublic.cpp:399-420` 的 `CRC_Check`），並注意出廠是 STOP、19200、RKC 協定，跟 HT9045 溫控 COM 埠的 9600 8N1 不同。

<!-- preserved-content:end -->
