> 保存來源：`.claude/skills/ht9045-temperature/references/controllers/delta-dtk.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# 台達 Delta DTK 系列（重點 DTK4848）（溫控器手冊參照）

> 手冊放在 Steven01 本機的 `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\`（不在 repo 裡；其他電腦沒有 E: 這顆硬碟，要看原檔請到 Steven01）。

樹的代號：**golden V912**＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，Big5，用 `iconv -f BIG5 -t UTF-8` 讀）；
**移植樹 V906**＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（C++17，UTF-8）。下文的 `檔名:行號` 前面都標樹名。

---

## 1. HT9045 在哪裡用

### 1.1 設定鍵與值

| 項目 | 內容 | 程式位置 |
|---|---|---|
| 廠牌代碼 | `const int DTK4848 = 4;`（0=TC401、1=KT4H、2=E5DC、3=NoHeater、4=DTK4848） | golden V912 `cmydef.cpp:222-226`；顯示字串 `MachineTypeUtility.cpp:19-25` |
| 全機廠牌（舊鍵） | `Gerneral.ini [TempCtrl] HEATER_CTRL_TYPE`，預設 KT4H(1) → `TC401HeaterControl` | golden V912 `database.cpp:433`；移植樹 V906 `database.cpp:541` |
| 逐通道廠牌（20260618 RogerYang 移植創發 2type） | `[TempCtrl] HeaterInsOpt_<通道名>`（HotPlate1、Shuttle1、Head1…71 個），值 `-9999`＝沒設 → 回退 `HEATER_CTRL_TYPE` | golden V912 `HandlerSys.cpp:50-60`（`HeaterInsOpt_Read`）、`MachineTypeUtility.cpp:31` 起的 `g_tHeaterInsInfo[]` |
| 溫控 COM 埠 | `[TempCtrl] COM_PORT`，預設 `"COM2"` → `HSys.sTempComPort` | golden V912 `database.cpp:522`；移植樹 V906 `database.cpp:637` |

本機 `D:\HT9045\system\Gerneral.ini`（Steven01）目前是 `HEATER_CTRL_TYPE=2`（E5DC）、`COM_PORT=COM12`、`HeaterInsOpt_*=-9999`（:266-358），也就是**這台沒有用 DTK**。

### 1.2 通訊介面與參數

- 走溫控共用序列埠 `COM2->Comm2`：**9600 bps、8 data、None、1 stop**（golden V912 `rs232.cpp:266-283`；`USE_NEW_TEMPCTRL_FUNCTION` 被 `database.cpp:340` 強制成 false，所以一定走這段）。
- 同一條埠也接 KT4H／E5DC／TC401（依各通道廠牌選擇呼叫哪個函式）。
- 協定：**Modbus ASCII**，LRC 用 `DTK4848_LRC`（golden V912 `cpublic.cpp:130-145`）。

### 1.3 站號規則

- `Addr`＝通道序號（`eTempControll`，0 起算；例 `tcHotPlate1`=0），送出時 `Addr+1`，用 **`%02X` 兩位十六進位**（Steven 20210511 由 `%02d` 改，註解在 golden V912 `cpublic.cpp:210`、`:223`）。
  例：HotPlate1→`01`、HeatGun1（序號 27）→`1C`。
- 移植樹 V906 另有「各溫控器不同」模式可指定站號 1～247（`FileRW/HSys_Heater.h:160-162`、`FileRW/HSys.cpp:834-850` `W906_HeaterStationIdx`），**但 bthermo 還沒用它**（見 §4）。

### 1.4 程式位置

| 動作 | golden V912 | 移植樹 V906 |
|---|---|---|
| 寫 SV | `cpublic.cpp:205-216` `DTK4848WordWriteNoSucm`：`":%02X06%04d%s" + LRC + "\r\n"`，暫存器 `4701` | `cpublic.cpp:350-366`，送到 `g_pDTKComm` |
| 讀 PV | `cpublic.cpp:218-229` `DTK4848WordReadNoSucm`：`":%02X03%04d0002" + LRC + "\r\n"`，暫存器 `4700`、2 個 word | `cpublic.cpp:367-381` |
| 溫控主迴圈 | `bthermo.cpp:2532-2535`（溫度變了→寫）、`:2566-2569`（否則→讀）、`:2595-2598`（Task=2500） | `bthermo.cpp:2696-2699`、`:2758-2761`、`:2787` |
| 回應解析 Task 2500 | `bthermo.cpp:2973-3091`（`:2980` 看第 5 字元是不是 `"6"`；`:2987-2988` 取第 8～11 字元 ÷10） | `bthermo.cpp:3234-3370` |
| 設定頁手動測試 | `cConfiguration.cpp:5631-5634`（寫）、`:5651-5654`（讀）、`:5741-5755`（解析用 `GetEveryCode`＋`Change_Tempture_Value`，`cpublic.cpp:147-175`） | — |
| 序列埠接口 | `COM2->Comm2` | `g_pDTKComm`（`cpublic.cpp:157` 初值 `nullptr`；`cpublic.h:390-394`）。**全樹只有 `tests/test_cpublic_foundation.cpp:91`、`:122` 會設它**，所以移植樹實際上什麼都沒送出 |

已有的通道整理：`D:\HT9045\.claude\skills\ht9045-heater-control\references\golden-v912-channels.md:108`。

---

## 2. 手冊清單

| 絕對路徑 | 語言 | 版本／日期 | 內容 | 先讀哪幾頁 |
|---|---|---|---|---|
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-TC_DTK_I_TSET_20200709.pdf` | 繁中／簡中／英／土耳其文（單張 2 頁，橫式大張） | 2020-07-09（條碼 5014004404-DTK4） | DTK 操作說明書：選購碼、規格、參數表、警報模式、感測器表、**RS-485 通訊表**、開孔 | PDF p.1 **左半＝繁中**（「RS-485 通訊」表在左半第 2 欄下方）、右半＝簡中；PDF p.2 左半＝英文「RS-485 Communication」 |
| `E:\HT9045W_相關料件技術文件\溫控器\Delta溫控器\DELTA_IA-SS_DT_C_TC_20180208_web.pdf` | 繁中 | 2018-02-08 | 台達 DT 系列型錄（DT3、DTK、DTA、DTB、DTC、DTD、DTE、DTV；**沒有 DTM**） | DTK：PDF p.10（印刷頁 9，簡介）、p.11（印 10，電氣規格＋警報）、p.12（印 11，參數表，**沒列通訊參數**）、p.13（印 12，感測器表、開孔、端子） |

⚠ 型錄 PDF p.7（印 6）那張「RS-485 通訊」表是 **DT3** 的（三組警報、`103CH` 執行／停止），不是 DTK，別抄錯。
⚠ 資料夾裡**沒有 DTK 完整使用手冊**，只有單張說明書；說明書只列 8 個暫存器。

---

## 3. 通訊重點（手冊內容）

出處都是 `DELTA_IA-TC_DTK_I_TSET_20200709.pdf`（下稱「DTK 說明書」），除非另外註明。

### 3.1 協定與通訊參數

- RS-485，**2,400～38,400 bps**，**Modbus ASCII／RTU**；說明書寫的功能碼只有 **03H（讀暫存器，最多 8 個 word）**（DTK 說明書 PDF p.1 繁中「RS-485 通訊」、p.2 英文 "DTK supports baudrate 2,400 to 38,400 bps, Modbus ASCII/RTU protocol, function code 03H and reads maximum 8 words"）。
- 06H（寫單一 word）在 DTK 說明書**沒有寫**；同家族 DTB 手冊有寫 06H（見 `delta-dtb.md` §3）。
- 選購碼 `DTK□□□□ □ □ □`：第 6 碼「通訊選配」0＝無、1＝RS485（DTK 說明書 PDF p.1「選購資訊」）。**要買第 6 碼＝1 的型號才有 RS-485**。
- 設定模式裡的通訊參數（DTK 說明書 PDF p.1 參數表「設定模式」欄）：`CoSH` 通訊寫入許可／禁止、`C-SL` ASCII／RTU 選擇、`C-no` 通訊位址、`bPS` 鮑率、`LEN` 資料長度、`PrtY` 同位元、`StoP` 停止位元。
  說明書**沒寫出廠預設值，也沒寫站號範圍**。

### 3.2 暫存器（DTK 說明書 PDF p.1 繁中表、p.2 英文表）

| 位址 | 名稱 | 說明 |
|---|---|---|
| 1000H | PV 目前溫度 | 以 0.1 刻度為單位 |
| 1001H | SV 溫度設定值 | 以 0.1 刻度為單位 |
| 1005H | 控制方式 | 0：PID、1：ON/OFF、2：手動 |
| 1012H | 輸出 1 輸出量讀取 | 0.1% |
| 1013H | 輸出 2 輸出量讀取 | 0.1% |
| 1016H | 溫度誤差調整值 | −99.9～+99.9，0.1 |
| 1018H | 控制執行／停止 | 0：停止、1：執行（預設） |
| 1022H | 讀寫自動調諧狀態 | 0：停止（預設）、1：開始 |

- 小數點：顯示可選小數點一位或無（DTK 說明書 PDF p.1「電氣規格」顯示刻度），但 1000H／1001H 的單位寫死 0.1。
- 警報：2 組、9 種模式（0～9，9＝斷線警報）（DTK 說明書 PDF p.1「警報輸出」）。DTK 說明書**沒有列 PV 錯誤碼**（DTB 的 8002H～8007H 見 `delta-dtb.md`）。

### 3.3 檢查碼與例外碼

- DTK 說明書**沒有寫 LRC／CRC 算法，也沒有寫 Modbus 例外回應**。
- 同家族手冊的寫法：LRC＝從「機器位址」加到「資料內容」、捨去進位、取 2 的補數（DTB `DTB 操作手冊.pdf` PDF p.2：`01H+03H+10H+00H+00H+02H=16H`，補數 `EA`；DTM 操作手冊 PDF p.72，印 7-13：`H01+H03+H41+HFF+H00+H02=H146`→`H46`→`HBA`）。CRC＝初值 FFFFH、多項式 A001H（DTB 操作手冊 PDF p.2）。

---

## 4. 與程式對照

| # | 項目 | 手冊 | 程式 | 判斷 |
|---|---|---|---|---|
| 1 | **PV／SV 暫存器位址** | DTK 說明書只有 **1000H（PV）、1001H（SV）** | 讀 **4700H**（2 word）、寫 **4701H**（golden V912 `cpublic.cpp:207`、`:220`） | ⚠ **不一致**。資料夾裡只有 `DTB_Manual_Eng.pdf` PDF p.13（印 12）寫「PV read back 1000H/4700H」，把 4700H 當 PV 別名；**4701H 在這 8 份手冊裡都查不到**。程式從 KaiHuang 20190821 起就這樣寫，若現場有在用，表示該批 DTK 韌體接受，但要換新韌體／新機台前**應上機確認**，或改用說明書有寫的 1000H／1001H（屬於改 golden，不在本文件範圍）。 |
| 2 | 位址字串格式 | 4 位十六進位 | `%04d` 印十進位數字 4700／4701 | 因為 4、7、0、0、1 剛好都是 0～9，印出來的字元等於 0x4700／0x4701，**碰巧正確**；若以後照抄這個寫法去讀含 A～F 的位址（例 DTB 的 102EH 錯誤狀態），`%04d` 就印不出來。 |
| 3 | 寫入功能碼 | DTK 說明書只寫 03H | 寫 SV 用 **06H**（`cpublic.cpp:210`、`:213`） | DTK 說明書沒記載 06H；DTB 手冊有。實務上 06H 是標準單一寫入，但 DTK 部分仍屬「手冊未記載」。 |
| 4 | 站號 | 說明書沒寫範圍 | `%02X`、`Addr+1`（1～71） | 格式正確（Modbus ASCII 位址 2 個十六進位字元）。範圍要對實機 `C-no`。 |
| 5 | 比例 | PV／SV 0.1 刻度 | 寫 `Temp*10`（bthermo `:2534`）；讀 `HexStrToInt(...)/10.0`（`:2988`） | ✓ 比例相符。 |
| 6 | 負數 PV | 1000H 為 0.1 刻度，Pt100 可到 −200 °C | 主迴圈用 `HexStrToInt` 當無號數（`bthermo.cpp:2988`），0 °C 以下會變成 6000 多 °C（例 −1.0 °C＝FFF6H→6552.6）；設定頁的 `Change_Tempture_Value`（`cpublic.cpp:157-175`）有處理 `F` 開頭的負數 | ⚠ 兩個解析器結果不同。HT9045 只加熱，實務影響小。 |
| 7 | 負數 SV | — | `IntToHex(Value,4)` 遇負數會產生 8 個字元（`cpublic.cpp:210`、`:213`），而 `DTK4848_LRC` 只算前 12 字元（`:133`） | ⚠ 負數 SV 會送出錯誤封包；只加熱時不會發生。 |
| 8 | LRC | 同家族手冊：位址～資料加總取 2 補數 | `0xFF-LRC+1` 再取最後 2 個十六進位字元（`cpublic.cpp:130-145`） | ✓ 算法相符（取最後 2 字元等於丟掉進位）。 |
| 9 | 送出長度 | ASCII 幀到 CR LF 結束 | `WriteCommData(..., strlen+1)`（`cpublic.cpp:215`、`:228`），**每幀多送一個 0x00** | ⚠ 手冊沒提；ASCII 接收端通常會丟掉「:」之前的雜字元，現場若沒出事可維持，但換其他廠牌共用同一條 RS-485 時要留意。 |
| 10 | 回應判斷 | 標準 Modbus：例外回應功能碼是 0x83／0x86（Modbus 規範，非 DTK 手冊內容） | 只看第 5 個字元是不是 `"6"`（`bthermo.cpp:2980`），不驗 LRC、不驗站號 | ⚠ `83` 與 `03` 的第 5 字元都是 `3`、`86` 與 `06` 都是 `6`，**例外回應分不出來**。寫入被拒（`86`）會被當成寫入成功；讀取例外（`83`）時第 8～11 字元是「LRC＋CR LF」，`HexStrToInt(char*)`（golden V912 `EJ1N/TextProcess.cpp:323-345`）由右往左遇到非十六進位字元就停 → 回 0 → 進 `Read==0` 分支（`bthermo.cpp:2989-3009`）；但該分支結束後**沒有 `break`**，`:3010` 起仍把 `CommRetry` 重設、把讀值 0 寫進 `UN150ReadReal`／`UN150Read`、`Task=300`，重試計數等於被抵銷（這是程式本身的問題，與手冊無關）。 |
| 11 | 讀 2 個 word | 03H 最多 8 word | 讀 2 個（4700H、4701H），只用第 1 個（`bthermo.cpp:2987`） | 數量在範圍內。 |
| 12 | 通訊格式 | 2,400～38,400 bps、ASCII／RTU | 9600、8N1、ASCII（golden V912 `rs232.cpp:268-273`） | ✓ 在支援範圍；DTK 本體要設 `bPS`=9600、`LEN`=8、`PrtY`=無、`StoP`=1、`C-SL`=ASCII。 |
| 13 | 移植樹 | — | `g_pDTKComm` 永遠是 `nullptr`（移植樹 V906 `cpublic.cpp:157`），bthermo 判斷用全機 `TC401HeaterControl==DTK4848`（`bthermo.cpp:2696`、`:2758`、`:2787`），不是 golden 的逐通道 `IsValEqual_HeaterInsOpt` | 移植樹的 DTK 目前**不會真的通訊**；逐通道廠牌與指定站號（`W906_HeaterStationIdx`）也還沒接進 bthermo（`FileRW/HSys.cpp:66` 註明「溫控底層翻完之前，機台溫控仍只看 HEATER_CTRL_TYPE」）。 |

---

## 5. 注意事項

1. **寫 SV 前要開 `CoSH`（通訊寫入許可）**。DTK 說明書只列了參數名稱沒寫預設；DTB 手冊寫明「通訊寫入預設禁止」（`DTB_Manual_Eng.pdf` PDF p.10，印 9，0810H），DTK 上機時一定要確認，否則 06H 寫不進去、溫度不會變。
2. 共用溫控 COM 埠：KT4H／E5DC／DTK／TC401 混用時站號不能撞。Steven 20260928 裁決 R113「不同廠牌共用同一個站號要擋，目前有遇到 E5DC + KT4H + DTK4848 混用了，新的機台架構是改用全機 DTM」（移植樹 V906 `FileRW/HSys.cpp:632-633`；`D:\HT9045\.claude\skills\ht9050-construction\references\archive\decisions-decided-202609.md:1120` R113）。
3. 新機台方向是 **DTM**（見 `delta-dtm.md`），DTK 主要是舊機維護。`E:\HT9045W_相關料件技術文件\溫控器\溫控器選用差異表20181025.pptx` 的比較結論是「DTK 升溫最快且不 overshoot，KT4H 可用 DTK 取代」、DTK4848 安裝機型 HT-3310K（同資料夾 `index.md` §2.3 的摘錄；本文件未另外核對）。
4. 若要查 47xxH 這組相容位址的正式定義，需要另外下載 **DTK 完整使用手冊**（本資料夾只有單張說明書，說明書最後一行請上 `www.deltaww.com` 下載）。
5. 溫度範圍（DTK 說明書 PDF p.1「溫度感測器種類及溫度範圍」）：Pt100 −200～850 °C、K −200～1,300 °C 等；取樣 0.1 秒。

<!-- preserved-content:end -->
