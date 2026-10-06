> 保存來源：`.claude/skills/ht9045-temperature/references/controllers/index.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# HT9045 溫控器手冊索引

> 手冊放在 Steven01 本機 `E:\HT9045W_相關料件技術文件\溫控器\`（不在 repo；其他電腦沒有這顆 E 槽）。下表的「手冊資料夾」都在這底下。
> 程式位置一律指 golden V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，cp950，用 `iconv -f BIG5 -t UTF-8` 讀）。廠牌怎麼選、溫控迴圈怎麼跑、V906 移植樹現況：`D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md`。
> 整理日期 20261001（St01 ST01-E2，Steven「E:\HT9045W_相關料件技術文件\溫控器 這邊有個品牌的溫控器手冊 幫我加入到skill的參照之中」「同品牌也可以分開，類似 OMRON E5DC 與 EJ1N 就不太依樣」）。
> 一個控制器系列一份參照檔，都在本資料夾 `D:\HT9045\.claude\skills\ht9045-temperature\references\controllers\`；「無手冊」＝資料夾裡沒有這個廠牌的手冊。
> PDF 不複製進 repo；資料夾裡的 .exe（VB6 範例、RKC 工具與驅動）一律不要執行。手冊對程式找到的疑點彙總在 §4。
> 給現場的**手動設定步驟**（面板按鍵、DIP／旋轉開關、ADAM-4520、DCISoft，附手冊截圖）：RD5 入口網站 repo 9050motionview 的 `public/Docs/manual/TempController_Manual/20261001_溫控器手動設定指南_草稿.pdf`（St01 ST01-E3 整理、ST01-E2 審；草稿，待 EastSun 上機確認）。

## 1. 總表

| 廠牌／型號 | HT9045 的設定值或使用處 | 介面／協定 | 參照檔 | 手冊資料夾 |
|---|---|---|---|---|
| TC401（程式叫 TMC401；程式裡沒寫廠牌） | `[TempCtrl] HEATER_CTRL_TYPE`＝0（`cmydef.cpp:222`；顯示字 "TC401" `MachineTypeUtility.cpp:20`） | 共用溫控 COM 埠（`[TempCtrl] COM_PORT`，9600 8N1，`rs232.cpp:266-283`）；Modbus RTU 8 byte＋CRC-16：寫 fc 06、暫存器 00C8H＋通道，讀 fc 03、暫存器＝通道、1 筆（`cpublic.cpp:422-461`）；一台 4 通道，站號＝序號÷4＋1、通道＝序號%4（`bthermo.cpp:2522`、`:2556`） | 無手冊 | — |
| Panasonic KT4H | `HEATER_CTRL_TYPE`＝1（**缺鍵預設**，`database.cpp:433`）；`HeaterInsOpt_<通道>`＝1；`[System] USE_16_HEATER`＝1 eht16Heater／4 eht32HeaterKT4H 的 Index 區（`MachineType.h:718`、`:721`） | 共用溫控 COM 埠 9600 8N1；Modbus ASCII（`cpublic.cpp:179-201`），站號＝序號＋1、兩位十六進位 | `panasonic-kt4h.md` | 資料夾根目錄（`kt4h_f412e-2.pdf`、`Panasonic KT4H溫度控制器操作手冊(English).pdf`、`KT4H_inst_e.pdf`） |
| Omron E5DC | `HEATER_CTRL_TYPE`＝2（`cmydef.cpp:224`） | 共用溫控 COM 埠；CompoWay/F `STX … ETX BCC`，站號＝序號＋1、兩位十進位（`cpublic.cpp:463-488`） | `omron-e5dc.md` | `OMROM溫控器\`（E5CC／E5EC／E5AC／E5DC／E5CB 用戶手冊與通訊手冊、E5□C 英文手冊） |
| Omron EJ1N（EJ1N-TC4A 等）；EJ1G | `USE_16_HEATER`＝2 eht16HeaterEJ1N／3 eht32HeaterEJ1N（`MachineType.h:719-720`），只管 Index 區；SV 由 `DoSetSVOfOmronEJ1N`（`bthermo.cpp:1207-1211`）。EJ1G 在程式裡 0 筆 | 自己的 COM 埠 `[TempCtrl] COM_PORT_OMRON`（`database.cpp:523`，預設 COM7），38400、7 bit、Even、2 stop（`EJ1N\OmronEJ1N.cpp:333-338`）；CompoWay/F（`溫控器改善報告.pdf` slide 3） | `omron-ej1n.md` | `OMROM溫控器\`（EJ1、EJ1G 手冊與型錄、`EJ1G\`、`EJ1應用範例\`） |
| Delta DTK4848 | `HEATER_CTRL_TYPE`＝4（`cmydef.cpp:226`，"KaiHuang 20190821"） | 共用溫控 COM 埠；Modbus ASCII，讀 4700H 起 2 筆、寫 4701H，站號＝序號＋1、兩位十六進位（`cpublic.cpp:205-229`） | `delta-dtk.md` | `Delta溫控器\`（`DELTA_IA-TC_DTK_I_TSET_20200709.pdf` 等） |
| Delta DTB4824 | **不是溫控迴圈的廠牌**：自動 K 溫頁（`TACTForm`）的量測器選項 `iThermoCtrlType`＝1 `Type_DeltaDTB4824`（`AutoTemperature.h:32`；0＝Agilent 34970A），"Steven 20210510 : 自動K溫使用DTB4824" | ACT 頁自己的 COM 埠（`ACTCom`，`AutoTemperature.cpp:443-447`、`:616-637`）；讀 PV `DTB4824_ReadPV`（`AutoTemperature.h:208`） | `delta-dtb.md` | `Delta溫控器\`（`DTB 操作手冊.pdf`、`DTB_Manual_Eng.pdf`、`DELTA_IA-TC_DTB_I_TSET_20140421.pdf`） |
| Delta DTM／DTME08 | `USE_16_HEATER`＝5 eht16HeaterDTME08／6 eht32HeaterDTME08（`MachineType.h:722-723`，"JimmyChiu 20210923"），只管 Index 區；SV 由 `DoSetSVOfDTME08`（`bthermo.cpp:1212-1216`）。HT9050 的溫控也是 DTM | Ethernet socket（`EJ1N\uDTME08Control.cpp` 的 `uSocketClient`）＋Modbus 指令（`EJ1N\uModbusCommand.cpp`）；HT9050 對照 `D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md` | `delta-dtm.md` | `Delta溫控器\`（`DELTA_IA-SSM_DTM_C_EN_20211022_Web.pdf`、`DELTA_IA-SSM_DTM_C_TC_20200730_web.pdf`、`DELTA_IA-TC_DTM_OM_SC_20210223.pdf`） |
| Yokogawa UT150（UT100 系列） | **沒有設定值**。只剩命名：`bUT150Install`（全樹 1229 處）、`UN150Read`、`PauseUT150Polling` 等；KT4H 的函式也叫 `UT100WordWriteNoSucm` | `TMyYokogawa`（`TempCtrl\MyTempture_UT100.cpp:20-48`）是 Yokogawa PC link 格式 `STX 站號(兩位十進位) 01 0 WRD／WWR … ETX CR`，但**不在 `HT9045.bpr`，沒編譯** | 無手冊 | — |
| RKC SRZ（Z-TIO／Z-DIO） | **程式沒用到**（grep 0 筆，證據見參照檔 §4） | RS-485；RKC communication（ANSI X3.28）或 Modbus-RTU | `rkc-srz.md` | `RKC_SRZ_溫控器\` |

補充：
- 廠牌值 3＝No Heater（`cmydef.cpp:225`），整台不加熱、溫控 COM 埠不開。
- `TempCtrl\MyTemptureSet.h:4-8` 另有一套舊編號 `TC_UT100=1`、`TC_KT4H=2`、`TC_DT4848=3`、`TC_WT404=4`、`TC_TMC401=5`，屬於沒啟用的「新溫控架構」（`USE_NEW_TEMPCTRL_FUNCTION` 在 `database.cpp:340` 強制 false；`HT9045.bpr` 的 `TempCtrl\` 只編 `TriTemp.cpp`）。**編號跟 `HEATER_CTRL_TYPE` 不同，別混用**；WT404 也沒有手冊。
- `溫控器選用差異表20181025.pptx` 裡的 Delta DTA4848、DTE10T，與報價裡的 Omron E5GC，在程式裡都是 0 筆。

## 2. 共用文件（資料夾根目錄）

### 2.1 `E:\HT9045W_相關料件技術文件\溫控器\232To485.pdf`
HighTek **HC-01 無源 RS-232↔RS-485 轉換器**使用說明，簡體中文，4 頁。
- p.1：不用外接電源，由 RS-232 第 3 腳（TXD）供電、第 7 腳（RTS）與第 4 腳（DTR）輔助；距離 0～5 km（115200～9600 bps），1.2 km 可到 57.6 kbps；RS-232 端 DB9 母、RS-485 端 DB9 公（附 5 位端子台）。
- p.2：RS-485 非同步半雙工、協定透明；腳位 RS-232 DB9F 1 DCD、2 RXD、3 TXD、4 DTR、5 GND、6 DSR、7 RTS、8 CTS；RS-485 DB9M 1 Data+、2 Data-、5 GND、6 +5V。RS-232 端是 DCE，可直接插 PC 的 COM 埠；訊號低於 +5V 時可由 DB9M 第 6 腳供 +5V。
- p.3：一對多最多 32 個 RS-485 從站，線路末端可加 120Ω 1/4W 匹配電阻。
- p.4：接法 Data+→485+、Data-→485-、GND→GND；故障排除：先查兩端接線與 RS-232 訊號電平，資料遺失或亂碼查兩端速率與格式、加 120Ω。
- 跟 KT4H 的關係：KT4H 手冊的架構就是「主機 RS-232C → 轉換器 → RS-485」（`kt4h_f412e-2.pdf` p.36）。但 KT4H 手冊說 KT4H 端**不要**接終端電阻（內建 pull-up／down，必要時接在主機側 120Ω 以上，p.37），以 KT4H 手冊為準。這些文件沒有寫 HT9045 實際用哪一款轉換器；改善報告 slide 5 的照片裡 KT4H 面板下方有一個 ADAM 模組，slide 7 的 EJ1N 成本列了 ADAM-4520。

### 2.2 `E:\HT9045W_相關料件技術文件\溫控器\溫控器改善報告.pdf`（與 `溫控器改善報告.odp` 同內容）
"HT9046A Thermo Controller Promotion Project"，Steven，2012.07.03，7 張投影片（.odp 的 7 張文字與 PDF 相同）。
- slide 2 需求：Hot Plate 2（過負載 IO 警報輸出）、Shuttle 2（同）、Chamber 1（SCR 輸出）、Socket 1、RTC 1（過溫 IO 警報輸出）、Heater 16、Heater Gun 2（SCR 輸出）。
- slide 3 KT4H 對 Omron EJ1N-TC4A：警報輸出 1～2 點 IO 對 3 組（通訊輸出）；控制點數單點對 1 對 4；顯示液晶對無（依賴通訊）；尺寸 48×59.2×61 對 31×95.4×109；通訊 RS-485 對 RS-485（CompoWay/F）；「目前 NS 採用該規格」。
- slide 4：用 Omron 取代 Index 區 16 個 Heater 控制器——省空間、轉接板與纜線減少配線、模組化好維修、程式以物件封裝可動態增減。
- slide 5：原本 16 個 KT4H 裝在前面板、SSR 在側邊，缺點是溫控器後方的設備幾乎無法維修。slide 6：改成溫控模組與 SSR 集中在控制箱，裝在 RTC LED 燈箱旁（面板標 "INDEX HEAD HEATER" A1～H1、A2～H2）。
- slide 7 成本（只算 16 Heater）：KT4H 16 台＋鈑金，對 EJ1N 4 台＋電源通訊模組＋ADAM-4520＋鈑金＋轉接板；EJ1N 方案比較便宜（金額見原檔）。
- 跟程式：`USE_16_HEATER`＝2 eht16HeaterEJ1N 的註解是 "Steven 20120220 : Omron EJ1N溫控器"（`bthermo.cpp:1207`），同一時期。

### 2.3 `E:\HT9045W_相關料件技術文件\溫控器\溫控器選用差異表20181025.pptx`
12 張投影片；slide 1 標題「溫控器市購件差異比較」「HT-電控 20180926」。
- slide 2 規格表（Panasonic KT4H；Delta DTA4848V1、DTB4824VR、DTK4848V12、DTE10T；Omron EJ1N-TC4A、E5DC）：取樣 250／500／400／100／1000／250／50 ms；顯示錶 DTE10T 與 EJ1N 沒有；通道數 DTE10T 4 或 8（T、K）／6（RTD）、EJ1N 2、4 或更多，其他 1；通訊都是 RS485（EJ1N 另有 422）；安裝機型 KT4H＝HT-9xxx、HT-3xxx，DTA4848＝HT-1xxx、HT-7xxx、水溫保護，DTB4824＝ATC3.2 水溫保護，DTK4848＝HT-3310K，DTE10T＝OVEN、ATC 水溫保護，EJ1N＝HT-7xxx、HT-9xxx、ATC5.1，E5DC＝HT-9xxx；另有各款單價比較（金額見原檔）。
- slide 3 換料成本：HT-9046LS（11 個）與 HT-3012A（24 個）把 KT4H 換成 E5DC，兩台都比較便宜（金額見原檔）。
- slide 4～6：KT4H、Delta 四款、Omron 兩款外觀照片。
- slide 7 實驗方法：8 site SLK 監控 Contact 面溫度，head 從 30˚C 升到 100˚C，用銅塊 JIG 模擬 Device；穩定標準 ±1˚C（升溫後與 Contact JIG 後都要達成）。
- slide 8 結果（四款都先 Auto tune）：升溫 30→100˚C：KT4H 8m43s、E5DC 9m02s、DTA4848 9m18s、DTK4848 7m21s；Contact 後穩定：1m51s、2m11s、2m30s、2m23s。結論：DTK 升溫最快且不 overshoot，KT4H Contact 後最好（跟 DTK 差 42 秒）；Handler 的 Hot plate、Shuttle、Hot air 少有瞬間降溫，「將 Panasonic KT4H 系列以台達電 DTK 系列取代可行性高」；客戶有品牌偏好時在制令標註，沒有就照各機型 RD 的 standard。
- slide 9～12：KT4H、E5DC、DTA4848、DTK4848 的溫度曲線圖。
- 跟程式：DTK4848 在 2019 年加入（"KaiHuang 20190821"，`cpublic.cpp:203`、`cmydef.cpp:226`），E5DC 在 2014 年（"Steven 20141030"，`cmydef.cpp:227`）。兩份文件都沒有提到 RKC。

## 3. 資料夾裡不是手冊的東西

- **報價**（`E:\HT9045W_相關料件技術文件\溫控器\OMROM溫控器\`）：`20140205鴻勁EJ1N-TC2報價.pdf`、`AA-1409150015周廷瑋報價(E5DC).pdf`、`AA-141127001周廷瑋總報價(E5GC).pdf`、`AA-141202001周廷瑋總報價(E5DC).pdf`。
- **範例程式與應用資料**：
  - `OMROM溫控器\EJ1G\CompoWayF_Example\`：VB6 專案 `CompoWayFDemo`（.vbp／.frm／.exe 等，exe 不要執行）與 `EJ1G TEST.txt`。
  - `OMROM溫控器\EJ1應用範例\`：Omron CP1H PLC 跟 EJ1 的 CompoWay/F、Modbus-RTU 讀 PV／寫 SP 範例（.cxp／.psw／.doc）、`EJ1和NS进行SAP通讯.wmv`。
  - `OMROM溫控器\EJ1G\研華\`：ADAM-4520 資料表與照片；`OMROM溫控器\EJ1G\小記_*.doc` 兩份筆記。
  - `OMROM溫控器\` 另有 `Omron 4Chanel溫控器連結測試步驟.pdf`、`Omron 鴻勁科技溫控器曲線調整.pptx`、`cj1w-cif11` 資料表。
  - `RKC_SRZ_溫控器\`：`WinUciSRZje1201.exe`、`comk_driver_a.exe`、`comk_driver_b.exe`（工具與驅動，不要執行）。
- 整個資料夾連子資料夾共 128 個項目（`find` 計數，含資料夾）。

## 4. 手冊對程式找到的疑點（20261001，交 ST01-M 登記）

> 照 RULINGS_20260927 第 1 條：golden V912 不改，只通報 Jimmy；移植樹的底層溫控檔也是 Jimmy 的（`D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §8）。
> 「推論」＝從程式與手冊推的，沒有上機量過。每條的頁碼與行號在各參照檔的 §4。
> 已登記（20261001 ST01-M `eeff91f5`，`D:\HT9045\.claude\skills\ht9050-construction\references\todo.md`）：第 1 列＝**D-036**、第 17 列＝**D-035**、第 19 列＝**C-003**，其餘各列合成 **G-035**。狀態以 todo.md 為準。

| # | 控制器 | 疑點 | 樹 | 出處 |
|---|---|---|---|---|
| 1 | KT4H | 寫 Alarm 1 種類的 task 520 **沒有任何地方會進去**（全檔沒有 `Task=520`），種類全靠面板；程式寫的是絕對溫度（140 度或 SV＋10）。⛔ 20261001 更正：面板 [38] 出廠是 `----`（No alarm action，p.20），照出廠值就**完全沒有過溫保護**；改成 High limit（`H`，偏差型）跳脫點會變 SV＋140 或 2×SV＋10；要設 Process high（`AS`）才符合程式的意思 | golden、移植樹 | `panasonic-kt4h.md` §4 #6 |
| 2 | KT4H | 出廠 7 bit／Even，程式開 8N1，每台面板都要改 | golden | 同上 #7 |
| 3 | KT4H、E5DC | ×10 換算要輸入種類是 0.1 解析度；出廠輸入種類是整數，設 125 度會變 1250 度，程式不檢查 | golden | `panasonic-kt4h.md` #10、`omron-e5dc.md` §4「小數點」 |
| 4 | KT4H | 例外回應（83H／86H）不計錯、不記錄；`A_Check_Addr` 沒人呼叫；負溫度讀成 6552.6 | golden | `panasonic-kt4h.md` #11～#13 |
| 5 | KT4H | 開了 `bEnableKT4HAlarm1`（HONPREC_QC、Allegro_Philippines、Elmos_Germany）時每次讀 PV 成功都寫一次 Alarm 1，非揮發記憶體 100 萬次 | golden | 同上 #15 |
| 6 | KT4H | 一條 RS-485 最多 31 台；`USE_16_HEATER=4` 時 Index 就 32 區（推論，要問硬體） | golden | 同上 #9 |
| 7 | E5DC | 出廠 7 bit／2 stop／Even；`cmwt`（通訊寫入）出廠 OFF，程式不開也不看 | golden | `omron-e5dc.md` §4 |
| 8 | E5DC | 正常的寫入確認只有 17 byte，會被 `Length()<30`（`bthermo.cpp:2822`）當錯誤（推論）；回覆 BCC 不驗；PV 只取後 4 位 | golden | 同上 |
| 9 | EJ1N | 送站號用 `%02X`、收回用 `atoi`（`OmronEJ1N.cpp:1733`），站號 ≥10 不一致（目前最多 8 台） | golden | `omron-ej1n.md` §4 |
| 10 | EJ1N | 只認 `TC4A`／`TC2A`（`:1813-1820`），TC4B／TC2B 那台不輪詢；TC2 的讀 PV 命令跟讀 SV 一樣 | golden | 同上 |
| 11 | EJ1N | PV／SP 只取後 3 位 hex（`:528`、`:562`），**超過 409.5 度會繞回小值** | golden | 同上 |
| 12 | EJ1N | SV 讀回比對可能每圈都判成有變而重寫（推論）；Port A 每次寫都進非揮發記憶體（10 萬次）；開機每次寫輸入種類 Pt100（`main.cpp:10952`） | golden | 同上、§5 |
| 13 | DTK4848 | 讀 4700H／寫 4701H 不在 DTK 說明書（說明書只有 1000H／1001H；4700H 只在 DTB 英文手冊當 PV 別名，4701H 哪裡都查不到）；每幀多送一個 NUL；例外回應分不出來，讀取例外時重試計數被抵銷 | golden | `delta-dtk.md` §4 #1、#9、#10 |
| 14 | DTM／DTME08 | `ErrorCodeDescription` 把 0x02 寫兩次（`EJ1N\uDTME08Control.cpp:361`，應是 0x07）；Modbus 例外回應會被顯示成 PV 錯誤；狀態解析迴圈條件用 OR 可能寫出陣列外 | golden、移植樹 | `delta-dtm.md` §4 #6～#8 |
| 15 | DTM／DTME08 | 程式不寫 RUN／STOP（Hx248）；開機 30 秒內初始設定可能整批沒寫進去且不重送；每站 8 通道只能同一種感測器 | golden | 同上 #13、#14、§5 第 3 條 |
| 16 | DTM／DTME08 | 啟動條件 `A && B \|\| C`（`main.cpp:10959-10961`），`USE_16_HEATER==6` 時不看 No Heater | golden | 同上 #18 |
| 17 | DTM／DTME08 | **設定檔路徑兩樹不同**：golden 讀 exe 資料夾（`D:\HT9045\EXE\DTME08_Control.ini`，Steven01 有這個檔）；移植樹 GATE (1) 改讀 `D:\HT9045\system\`（`EJ1N\uDTME08Control.h:135-141`），那裡沒有這個檔 ⇒ 會用預設 `127.0.0.1:59999` | 移植樹 | 同上 #16 |
| 18 | DTM／DTME08 | 移植樹檔頭寫「Panasonic DTME08」（`EJ1N\uDTME08Control.h:2`、`.cpp:2`），golden `EJ1N\fDTME08.cpp:163` 註解寫「Omron」；都是台達 DTM | 兩樹 | 同上 §0 |
| 19 | DTM（HT9050） | HT9050 表上「站 3＝DTME08」：一個群組只有一台主機，第二台 DTME08 是另一個 IP，現在的 `uDTME08Control` 只有一條連線定址不到；站 2 混 PT100／K-type 也設不了 | HT9050 | 同上 §5 第 2、3 條 |

DTB4824（自動 K 溫的量測器）與 RKC SRZ 沒有要登記的：DTB 只讀不寫（`delta-dtb.md` §4 #9 的晚回應是程式風險，順帶記），SRZ 程式沒用到。

<!-- preserved-content:end -->
