# I-03b HT9050 DTM 通道表：方案（只寫文件，沒改程式）

> 作者：Ifor01（IFOR-NB2），20261005。認領：FROM_IFOR §1 1005 10:4x（`0c669e93`）。
> 狀態：**Ifor 1005 13:4x 定 D1～D4「照建議」**（FROM_IFOR §3），第二步程式在 `v906/ifor-i03b`；實作跟方案不一樣的地方見 §7。
> Jimmy 若要改（例如改 B），請在那張 MR 上說。
> 依據：Jimmy RULINGS_20261002 第 20a 條（溫控照 V912）、RULINGS_20261003 第 11 條（HT9050 加熱等 I-03b）、
> Ifor 1005 09:3x（`USE_16_HEATER` 先維持 0）、St01 `docs/handoff/ST01_W_ANSWERS_20261004.md` W-06／W-19、
> Steven 0924 `.claude/skills/ht9050-hw/references/temp-dtm-map.md`（以下簡稱「對照表」）。
>
> 行號的樹（一律寫明）：
> - **移植樹**：`HT9011UC_Cpp_V3.33.906.0\` = GitLab main `966965c0`（20261005 10:31）。
> - **golden 906**：`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\`（cp950）。
> - **V912**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（cp950）。
> - **910 HT9050（Frank）**：`origin/ref/frank-910-9050` 的 `HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch\`。

---

## 0. 一句話

HT9050 的溫控**全部**在台達 DTM 上（3 站 24 通道），golden 906／V912／Frank 910 都只有「Index 加熱器用 DTM、其他溫區走序列埠」的做法，
**三棵樹都沒有 HT9050 的 DTM 通道表**（`iTempCode[]` 三棵樹逐字相同，`cmydef.cpp:111-117`）。所以這是新設計。
現在把 `USE_16_HEATER` 設 5 或 6 會**加熱錯的加熱器**（§2 第 1 點），所以維持 0 是對的。

**要 Jimmy 決定的四件事**（我的建議在 §3）：

| # | 題目 | 我的建議 |
|---|---|---|
| D1 | 用哪個方式接進程式（A：HT9050 專用 DTM 表／B：照 V912 逐通道廠牌加一個「DTM」選項） | **A**，但表的形狀讓 B 以後能直接接手 |
| D2 | 用什麼開關打開 | **新開關 `[TempCtrl] DTM_CHANNEL_MAP=HT9050`**（預設空＝golden 行為），不新增 `USE_16_HEATER` 值 |
| D3 | 表寫在程式裡還是設定檔 | **寫在程式裡**（一個新檔、有單元測試），站 3 在 SLK 順序確認前不寫 SV |
| D4 | SLK-1～8 用哪個溫控代號 | **沿用 `tcAa1`～`tcBd1`**（不改 `eTempControll`，`tcTotalCount` 維持 71） |

---

## 1. HT9050 的「DTM 站／CH ↔ `eTempControll`」草案

來源：對照表 §1／§2（硬體工作簿 `HP-9050開發機資料-20260717.xlsx` 分頁 04_溫控器站號、03_加熱與感溫）。
名稱照 V912（第 20a 條）；`eTempControll` 在 V912 `MachineType.h:638-655` 跟移植樹逐字相同（`FileRW/HSys_Heater.h:32` 20260926 比過）。

`iCh` = DTM 全域通道（0 起算），程式拆成 `站 = iCh/8`、`CH = iCh%8`（golden `EJ1N/fDTME08.cpp:266-270` GetStationAndNumber）。

| iCh | 站 | CH | 型號 | 溫區 | `eTempControll`（值） | 感測器 | 狀態 |
|---:|:-:|:-:|---|---|---|:-:|---|
| 0 | 1 | 1 | DTME08 | Hotplate 1 | `tcHotPlate1`（0） | PT100 | 圖面已答 |
| 1 | 1 | 2 | DTME08 | Hotplate 2 | `tcHotPlate2`（1） | PT100 | 圖面已答 |
| 2 | 1 | 3 | DTME08 | In Shuttle 1 | `tcShuttle1`（2） | PT100 | 圖面已答 |
| 3 | 1 | 4 | DTME08 | In Shuttle 2 | `tcShuttle2`（3） | PT100 | 圖面已答 |
| 4 | 1 | 5 | DTME08 | DUT 1 | `tcDUT1`（29） | PT100 | 圖面已答 |
| 5 | 1 | 6 | DTME08 | DUT 2 | `tcDUT2`（30） | PT100 | 圖面已答 |
| 6 | 1 | 7 | DTME08 | DUT 3 | `tcDUT3`（31） | PT100 | 圖面已答 |
| 7 | 1 | 8 | DTME08 | DUT 4 | `tcDUT4`（32） | PT100 | 圖面已答 |
| 8 | 2 | 1 | DTMN08 | Chamber | `tcChamber`（9） | PT100（`HTSPT425M47F1`） | 圖面已答，看標籤確認 |
| 9 | 2 | 2 | DTMN08 | Hot Air 1 | `tcHeatGun1`（27） | **K-type**（熱風槍自帶） | 圖面已答，看標籤確認 |
| 10 | 2 | 3 | DTMN08 | Hot Air 2 | `tcHeatGun2`（28） | **K-type** | 圖面已答，看標籤確認 |
| 11–15 | 2 | 4–8 | DTMN08 | （空） | —（不註冊） | — | 保留 |
| 16–23 | 3 | 1–8 | DTME08 | SLK-1～8 | `tcAa1`～`tcBd1`（11～18），**順序未定** | PT100 | **要上機** |

SLK-1～8 的兩種可能（對照表 §3；兩種開機都正常、畫面都正常，**錯的只有哪一顆 socket 在加熱**，不能猜）：

| SLK | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|
| 一排一排（row-major，`Command.cpp:568` 推的） | Aa1 | Ab1 | Ac1 | Ad1 | Ba1 | Bb1 | Bc1 | Bd1 |
| 前後交錯（跟 `iTempCode[]` 一樣） | Aa1 | Ba1 | Ab1 | Bb1 | Ac1 | Bc1 | Ad1 | Bd1 |

另外要對：HT9050 測試模式下「site → 溫控代號」的換算（`Command.cpp` 的 `asTempArmOrder`／`iSiteMap` 那幾段，例如 :410-440 的 2×2 NN）
跟 SLK 接線要是同一個方向，否則畫面上 site 1 顯示的溫度是別顆的。這一項跟 SLK 順序一起確認。

---

## 2. golden 906 的做法，跟 HT9050 對不上的五處

golden 906（V912、Frank 910 相同）的 DTM 只管 Index 加熱器：

- **寫 SV／讀 PV**：`DoSetSVOfDTME08()` 第 i 個 DTM 通道 ↔ `iTempCode[i]`（golden `bthermo.cpp:4701-4776`；移植樹 :5302-5398；V912 :4879 起）。
  `frmDTME08->SetSettingSV(i, Temp[Addr])`、`UN150Read[Addr]=…GetPalGroup(i)->GetPV()`（golden :4775-4776）。
- **其他溫區走序列埠**：`DoThermoReal()` 的 Task 100 只把 Index 區（`tcAa1`～`tcBd2`；32 組再加 `tcAe1`～`tcBh2`）跳過（Task=300＝換下一個通道），
  Hotplate／Shuttle／DUT／Chamber／熱風槍照樣用 `HEATER_CTRL_TYPE` 的序列溫控器讀寫（golden :1156-1182；移植樹 :1439-1462、:3495-3500）。

對不上的地方：

1. **通道意義不同（最危險）**。HT9050 站 1 CH1 是 Hotplate 1，但 golden 把 DTM 通道 0 當 `tcAa1`。設 5／6 的話，
   Hotplate 1（1600 W）會收到 Index Aa1 的 SV，畫面上 Aa1 顯示的是 Hotplate 1 的溫度；站 2 的 Chamber 收到 `tcAa2` 的 SV。
2. **非 Index 溫區沒有 DTM 路**。HT9050 沒有序列溫控器：`HEATER_CTRL_TYPE=4`（COM12 DTK4848）讀成 999（TO_IFOR 1003 11:3x）；
   改成 `3`（NoHeater）的話 `DoThermo()` 一進來就 return（golden :1001-1002；移植樹 :1264-1265；V912 :1177 `IsNoHeaterMachine()`），DTM 也一起停。
   ⇒ golden 沒有任何設定能做到「全部走 DTM、不碰序列埠」。
3. **站數**。golden 的 DTM 通道數只有 16（2 站）或 32（4 站）（golden `EJ1N/fDTME08.cpp:62-64`；移植樹 `forms/fDTME08.cpp:98-101`），
   每輪照站數跑（golden :659；移植樹 :1092-1103）。HT9050 是 3 站：16 會漏掉站 3（SLK 全部不控）；32 會去問不存在的站 4，
   每輪等 `DTMTimeout` 並記「time out 10 Sec」（移植樹 :174-186）。站 4 沒裝時 DTM 實際回什麼要上機看，但不論哪種都不對。
4. **感測器型別一個面板只能一種**。`InitialDoSetSensorType()` 用同一個 `rgSensorType` 把每一站 8 個通道都設成 PT 或 K
   （golden :352-369；移植樹 `forms/fDTME08.cpp:773-790`）。HT9050 站 2 是 PT100（Chamber）＋K-type（熱風槍）混用。
   設錯型別讀值會偏很多，可能讓控制器以為還沒到溫而一直加熱。
5. **站 3 可能是另一台主機（另一個 IP）**。St01 W-06：hub 只規劃 DTM #1（port 2）、DTM #2（port 3）兩個 port，
   比較像「兩台 DTME08 各一個 IP，DTMN08 掛在第一台旁邊」。golden 只有一條 `uDTME08Control` 連線，
   站號只是 Modbus 起始位址（`站 × 0x1000`，`EJ1N/uDTME08Control.cpp` GetStationCode），**定址不到另一個 IP**。要上機確認。

V912 有一個相關的新東西：**逐通道溫控器廠牌**（`EN_HEATER_SHEET=1`，V912 `MachineType.h:657-714`、`HandlerSys.cpp:50-60` `HeaterInsOpt_Read`、
`bthermo.cpp:1167-1177`、:2520-2595 `IsValEqual_HeaterInsOpt(Addr, …)`）。選項只有 TC401／KT4H／E5DC／NoHeater／DTK4848，**沒有 DTM**。
移植樹只有 St01 E-029 的頁面與讀寫檔（`FileRW/HSys_Heater.h:16-26`），溫控流程還是 906 的單一 `TC401HeaterControl`；
那份註解寫明溫控底層是 Jimmy 的（第 37 條）。

---

## 3. 接進程式的選項

### 選項 A：HT9050 專用 DTM 通道表（建議）

- 新檔（例如 `EJ1N/DtmChannelMap.{h,cpp}`）放 §1 的表：每列＝站、CH、`eTempControll`（空＝-1）、感測器型別、是否武裝。
- 開關：`Gerneral.ini [TempCtrl] DTM_CHANNEL_MAP=HT9050`（D2）。**沒設＝golden 行為完全不變**。
- 打開時：
  - `DoSetSVOfDTME08()` 改照表跑（SV 寫到表上的站／CH、PV 讀回表上的 `eTempControll`），不再用 `iTempCode[]`。
  - `DoThermoReal()` Task 100：表上有的通道一律 Task=300（不碰序列埠）；表上沒有的照 golden。
    ⇒ `HEATER_CTRL_TYPE` 維持 4 也不會去讀 Hotplate 等溫區；不用改成 3（改 3 會讓 DoThermo 整個停）。
  - `TfrmDTME08`：通道數＝表上的站數 × 8（HT9050＝24 ⇒ 3 站，:1095 那條「還有下一站嗎」不用改）；
    感測器型別改照表每個通道設（站 2 CH1 PT100、CH2-3 K）。
  - `USE_16_HEATER` 仍然只描述 Index 的版面（§4），這個開關不靠它。
- 好處：改動集中、其他機型不受影響、可以單元測試。壞處：多一個 golden 沒有的開關（要記進我的 V912 帳本）。

### 選項 B：照 V912 逐通道廠牌，加一個「DTM」選項

- 把 V912 的 `IsValEqual_HeaterInsOpt(Addr, …)` 搬進移植樹的 `bthermo`／`rs232`（第 37 條，Jimmy 的底層），再加第 6 個選項 DTM；
  `DoThermoReal()` 遇到 DTM 的通道就跳過，DTM 路照一張「站／CH ↔ Addr」表跑。
- 好處：跟 V912 的方向一致（第 20a 條）；HandlerSys 頁已經能逐通道設。
- 壞處：範圍大（V912 改了 `bthermo`／`rs232`／`cConfiguration`／`OmronEJ1N` 多處）、碰 Jimmy 的底層；
  「DTM」這個選項 V912 也沒有，一樣是新設計；而且 B 仍然需要 A 的那張站／CH 表。

### 選項 C：只改 `iTempCode[]` 內容，設 `USE_16_HEATER=6`（不建議）

解決不了 §2 第 2～5 點（序列埠照讀、站 4 逾時、感測器混用、第二個 IP），而且改 `iTempCode[]` 會改到所有 DTME08 機型。

### 建議

**A**，表的欄位照 B 會需要的樣子設計（每列一個 `eTempControll`＋站／CH）。以後 Jimmy 做 B 時，「這個通道是不是 DTM」改成看 HeaterInsOpt，
表本身不用動。另外三個小決定：

- **D2 開關**：不新增 `USE_16_HEATER` 的值。`USE_16_HEATER` 在移植樹 25 個檔、652 處決定「Index 有幾組加熱器、畫面怎麼排」
  （`Command.cpp` 389 處），新值等於要逐處審。用獨立開關，`USE_16_HEATER` 留給 Index 版面。
  也不建議用機型判斷（`MachineTypeChoice==Type_HT9050`／`9050GPIB`）當開關：機台 0210 剛換過機型（TO_IFOR 1005 08:2x），設定檔明確打開比較安全。
- **D3 表放程式裡**：設定檔打錯一格就是加熱錯的東西；放程式裡有測試保護。SLK 順序確認後改一行＋MR。
  在確認前，站 3 那 8 列標「不武裝」：不寫 SV、只讀 PV（讀值不會造成危險，還能幫忙對接線）。
- **D4 沿用 `tcAa1`～`tcBd1`**：新增 `tcSLK1..8` 要改 `eTempControll`（註解：「有增加請搜尋：溫控器要一起改」「超過 100 的話 Alarm Code 要重新處理」），
  而 Index 的畫面、配方、site 對應都已經用 `tcAa1` 這組。

---

## 4. `USE_16_HEATER` 5／6 的影響（Ifor 1005：先維持 0）

- **現在不能設 5 或 6**：§2 第 1 點（加熱錯的加熱器）＋第 3 點（漏站 3 或問站 4）。維持 0 時 `frmDTME08` 不建、DTM 不連線、不加熱。
- **以後（選項 A 做完）**：`USE_16_HEATER` 只管 Index 版面。0 ＝ 4 組加熱器版面（`tcHead1`～`tcHead4`），HT9050 Index 有 8 組 SLK，所以到時候
  多半要設 5（16 組版面、只裝 `tcAa1`～`tcBd1` 8 組，其餘 `bUT150Install` 關掉）。這要跟 HT9050 的 site map 一起看，**做完 A 再定**，現在不動。
- 加熱測試若要在 I-03b 之前做：照 Jimmy 1003 11:3x，EastSun 先改 `HEATER_CTRL_TYPE=3`（整個溫控不跑）。

---

## 5. 武裝前的安全順序

每一步過了才走下一步；任何一步不對就停在那一步。

| 步 | 做什麼 | 誰 | 風險 |
|:-:|---|---|---|
| 0 | `USE_16_HEATER=0`（現況）；要做加熱測試前 `HEATER_CTRL_TYPE=3` | EastSun | 無 |
| 1 | 拍照／抄標籤：三台的型號與站號旋鈕、站 3 是不是另一個 IP、`D:\HT9045\EXE\DTME08_Control.ini` 內容（hub port 2／3）、SLK-1～8 接線標籤、站 2 三個感測器標籤（給 EastSun 的上機清單第 3、4 項） | EastSun | 無 |
| 2 | 依第 1 步定 SLK 順序與連線數（一個或兩個 IP），我改表、MR | Ifor01 | 無 |
| 3 | **只讀**：開 `DTM_CHANNEL_MAP=HT9050`，全部 SV 不寫，看 19 個通道的 PV 是不是都接近室溫、K-type 兩通道讀值正常 | EastSun＋Ifor01 | 低 |
| 4 | 寫感測器型別（站 2 混用），再讀一次 PV | EastSun | 低 |
| 5 | **一次一個通道、低溫**（例如 SV 40 ℃）：確認實際變熱的是那一顆（手持溫度計），過溫保險絲在位；SLK 用標籤定的順序，這一步只是複核，不是用加熱去找順序 | EastSun（有人在旁） | 中 |
| 6 | 全部通道對過之後才開 AT（`InitialDoSetAllAT` 會讓控制器自動調參） | EastSun | 中 |
| 7 | 正常溫度設定 | — | — |

---

## 6. 做程式時的工作量與測試（選項 A）

**工作量**：約 1～1.5 個工作天（1003 13:2x 估約 1 天，加上 §2 第 4 點的逐通道感測器型別）。站 3 若是第二個 IP，
`TfrmDTME08` 只有一個 `dtme08` 物件，要加第二條連線，**另加 0.5～1 天**，等第 1 步的答案再估。

**會動的檔**（實際動之前在 FROM_IFOR §1 逐檔認領，別人的檔先問）：

- 新檔 `EJ1N/DtmChannelMap.{h,cpp}`（表＋查詢函式），加進 `CMakeLists.txt` 的 `ht9045_comms`（:1841 起，跟 `EJ1N/uDTME08Control.cpp` :1868 同一組）。
- `bthermo.cpp` `DoSetSVOfDTME08()`、`DoThermoReal()` Task 100（各加一個開關分支，golden 那段原樣保留）。
- `forms/fDTME08.cpp` 通道數（:98-101）、`InitialDoSetSensorType`（:773-790）、`HeaterSVLog(iTempCode[iCh], …)`（:897）。
- `FileRW/HSys.cpp` `HmDefaultUnitCh`（:605-615，HandlerSys 頁顯示的 DTM 接線，現在用 `iTempCode[]`；St01 的檔）。
- 讀開關的地方（`Gerneral.ini [TempCtrl]`，跟 `HEATER_CTRL_TYPE` 同一段）。

**測試**（第 15 條：每項附反向驗證）：

1. `test_dtm_channel_map`：表跟對照表的機器可讀版 `.claude/skills/ht9050-hw/data/HT9050-TempMap.json` 逐列相同；24 列、
   `eTempControll` 不重複、站 2 CH4-8 是空的、站 2 CH2-3 是 K-type。反向：對調兩列 ⇒ 紅。
2. 沿用 `test_dtme08_cycle` 的模擬 DTM：開關打開時 Hotplate 1 的 SV 寫到站 1 CH1、站 2 CH2 的 PV 進 `UN150Read[tcHeatGun1]`、
   跑完站 3 就回第 1 站（不問站 4）、站 3「不武裝」時不寫 SV。反向：改成 32 通道 ⇒「問了站 4」那項紅。
3. `DoThermoReal`：開關打開時表上的通道不送序列命令。反向：拿掉分支 ⇒ 紅。
4. 開關沒設：`DoSetSVOfDTME08` 仍照 `iTempCode[]`、通道數仍照 `USE_16_HEATER` 16／32（golden 行為不變）。
5. 模擬、出貨兩組態都跑；完整 gate 交給筆電。

---

## 7. 實作（I-03b 第二步，`v906/ifor-i03b`）跟上面方案不一樣的地方

照方案 A 做；以下幾處是寫程式時發現、改得更貼 golden 或更保險的：

1. **設定溫度照序列埠那條路算，不是讓 `DoSetSVOfDTME08` 照表跑。** §3 原本寫「`DoSetSVOfDTME08()` 改照表跑」，但它只會用工作溫度；
   golden 給序列溫控器的設定溫度在 `DoThermoReal()` case 100 裡算，還包含 Hotplate／Shuttle 分段加熱、DUT 固定溫度、加熱模式、
   加熱門開著歸零、加熱繼電器沒開歸零等。所以改成：表上的通道照樣走 case 100 算 SV，**只把送 SV／讀 PV 換成 DTM**
   （`bthermo.cpp` `W906_DtmMapExchange`，位置跟 V912 逐通道換協定的地方一樣，V912 `bthermo.cpp:2520-2595`）；
   讀回照 golden case 2500（台達 DTK4848）的換算；DTM 斷線連 5 次以上記 999＋`UN150CommError`。開表時 `DoSetSVOfDTME08()` 直接返回，
   免得用 Index 的設定溫度去寫同一組面板。
2. **站 3（SLK）不武裝＝SV 一律寫 0，PV 照讀。** §3 原本寫「不寫 SV」；改成寫 0 比較保險：控制器裡若留著之前手動設的溫度，也會被關掉。
3. **開表時每個面板開機 SV＝0。** golden 開機第一輪會把面板的 edSV（預設 "30"）寫進控制器；HT9050 那會寫到 Hotplate／Chamber／
   還沒確認的 SLK。改成 0，等溫控執行緒算出真的設定溫度再送。
4. **還沒做的（照 §5，另開卡）**：
   - 武裝（`W906_DTME08_FormShowArm` 由 wb_serve 呼叫）——條件已經加上表，但**還沒有人呼叫**，所以現在機台上 DTM 不會連線。
   - §5 第 3 步的「只讀模式」（全部 SV 不寫）——跟武裝一起做。
   - 站 3 若是第二個 IP（第二台 DTME08）——等上機確認，需要第二條連線。
   - HandlerSys 頁顯示的 DTM 接線（`FileRW/HSys.cpp` `HmDefaultUnitCh`，St01 的檔）仍照 `iTempCode[]`；開表時應該改看表，請 St01 評估。
5. **測試**：`tests/test_dtm_channel_map.cpp`（ctest `DtmChannelMap`）——表、表對 JSON、開機（24 通道、逐通道感測器型別、開機 SV 0、
   站 4 一次都沒問）、`DoThermo` 全程對假 DTM（DUT 1 的 SV＝固定溫 60 算出來的，不是工作溫度 80；SLK 一直 0 但 PV 照讀；
   表上的通道一次都沒走序列埠）、斷線 999、原始碼釘選；反向驗證六項見 MR 說明。

---

## 附：這份文件沒做的事

- 沒改任何程式、設定檔、對照表。
- 站 3 型號與旋鈕、SLK 順序、DTM 實際 IP：照舊等 E-05（W-06）上機。
