> 保存來源：`.claude/skills/ht9045-temperature/references/main-screen-display.md`，main `84233b648`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 主畫面的溫度顯示：畫面格子 → 名稱 → 實體溫控器位址（20261002）

> Steven 1002：「我們來看一下, 溫度怎麼顯示在主頁上……先看看我們現在 顯示 --> 名稱 --> 實體溫控的addr 是怎麼對應的」。
> 樹：golden V912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（G，cp950）；HT9050 golden（V910 HT9050）＝`D:\HT9045\HT9011UC_Code_V3.33.910.0_20260820_HT9050\`（H）；
> 移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（P）；網頁＝`D:\HT9045\web\page\`。
> 來源：20261002 ST01-E2 派三位唯讀工程師查 golden 版面、ShowThermo／ATC、HT9050 與 V906 網頁，ST01-E2 抽查 `asTempCtrl`、`iTempCode`、
> `Index16Heater`、EJ1N 對應（腳本逐格比對 G `bthermo.cpp:3903-3918`／`:3943-3958`）與網頁格子。〔V〕＝讀過程式；〔推〕＝推論。
> 各廠牌通訊與面板設定在本資料夾 `controllers\`；這份只講「畫面上哪一格是哪一顆溫控器」。

## 0. 先講結論

1. **一條鏈**：機台設定 → 哪些通道「有裝」（`bUT150Install[]`，`TfMain::Index16Heater` 依測試模式與 socket 開關決定）→ 畫面名稱（`asTempCtrl[]`，`ShowHotName` 每個 Timer2 拍改名）
   → 面板（`FormShow` 依機型開關決定哪幾群看得到）→ 數值（`Timer1Timer` 每 100 ms 對 71 個通道都呼叫 `ShowThermo(i)`）→ 實體位址（看廠牌，§3）。
2. **版面跟「有沒有加熱」無關**：No Heater、SubMachineType、測試模式都不改版面，只讓沒裝的格子顯示「---」〔V〕。真正改版面的是 `USE_16_HEATER`（舊窗 4 組／新窗 16 組／新窗兩列 32 組）
   與幾個選配（CCD、Heat Gun、ATC 熱風、DUT 2／4 顆、L/B、ESD、Door、TriTemp）。
3. **Index 區的名字**：通道名 `tc{排}{欄}{臂}`——排 A／B＝socket 第 0／1 排、欄 a～h＝第 0～7 欄、臂 1／2＝Index 臂 0／1（`LastSet.bUseTestSocket[臂][排][欄]`，G `main.cpp:19231-20878`）〔V〕；例外：1x4 只有一排，Aa1、Ba1、Ab1、Bb1＝第 0～3 欄（`main.cpp:19529-19536`）。
   **畫面字母是按欄排的**：Aa1＝A1、Ba1＝B1、Ab1＝C1、Bb1＝D1……Bd1＝H1；32 組的 Ae1＝I1……Bh1＝P1（G `cmydef.cpp:89-109`）〔V〕。
4. **實體位址看廠牌**：KT4H／E5DC／DTK4848 的站號＝通道序號＋1；TC401＝第（序號÷4）＋1 台、通道 序號%4；Index 區走 EJ1N 或 DTME08 時另有對照（§3 表）。
5. **HT9050 沒有自己的溫度畫面**：H 的 `cTemperFrom.cpp`／`.dfm` 跟 V912 逐位元組相同；H 的 DTM 路徑仍照 HT9045 Index 的順序排（`iTempCode`），跟 HT9050 的 3 站配線對不上（§6）〔V〕。
6. **網頁現況**：`Status.TemperFrom.html` 11 格全是「---」、沒有資料來源；而且 Arm1／Arm2 那幾格的 id 放錯（§7）〔V〕。

## 1. 版面：三個容器（G `cTemperFrom.dfm`）

| 容器 | 什麼時候看得到 | 內容 |
|---|---|---|
| `gbOldTempWindow`（舊窗，一列，每格 57 寬） | `USE_16_HEATER`＝0（eht4Heater）（`cpp:406`） | Plate 1、Plate 2、SH 1、SH 2、Head 1/2、Head 3/4、Head 5/6、Head 7/8、Dut、〔Chamber〕、〔CCD〕、〔Heat Gun 1/2〕 |
| `palNewTempWindow` → `pnl16Heater`（新窗第 1 列） | `USE_16_HEATER`≥1（`cpp:407-412`） | Hot Plate（Plate 1／2）、Shuttle（SH 1／2）、Arm1（8 格）、Arm2（8 格）、Index（Chamber、Socket）、〔CCD〕、〔ATC air〕、〔Heat Gun〕、〔Door〕 |
| `palNewTempWindow` → `pnl32Heater`（新窗第 2 列） | 32 組（3／4／6）一定開（`cpp:494-496`）；16 組只在有 CCD2／L/B／ESD／L/B Up-Down 時開 | 〔Chamber〕〔Index DUT 1～4〕〔Base〕、Arm1（Ae～Bh 8 格）、Arm2（8 格）、〔CCD2〕〔L/B〕〔ESD〕〔L/B Up／Down〕 |

- 每一格都是 alLeft，看不到的格子不佔位置，所以位置會跟著開關往左擠〔推〕。
- Arm 群組裡的 8 格：預設上排 Aa Ab Ac Ad、下排 Ba Bb Bc Bd，畫面讀起來上排 **A1 C1 E1 G1**、下排 **B1 D1 F1 H1**；1x4（`QualSite1X4`／`_8Site1X4`）且不是 1 對 2 線材 kit 時，
  `SetIndex16HeaterPos` 改成上排 Aa Ba Ab Bb、下排 Ac Bc Ad Bd（畫面 A1 B1 C1 D1／E1 F1 G1 H1）（`cpp:1869-1914`，`cSetUp.cpp:2967-2973` 讀 setup 檔時呼叫；eNewATCSystem 時不做）〔V〕。Ae～Bh 那 16 格從不重排。
- 數值格的文字：`%5.1f`；沒裝「---」；讀值太低「...」；通訊錯誤「ERR」或 999。顏色：範圍內綠、偏低黃、過溫紫（`ShowThermo` `cpp:587-1480`）〔V；門檻見 §5〕。

## 2. 各機型顯示哪些格子

| 機型 | 判斷 | 看得到的格子（左 → 右） | Index／Head 誰有裝 |
|---|---|---|---|
| **無溫控**（`HEATER_CTRL_TYPE`＝3） | `cTemperFrom` 沒有任何 No Heater 判斷（grep 0 筆） | 跟它的 `USE_16_HEATER` 值一樣 | `Index16Heater` 一開始把 Head1～4、Aa1～Bd2、Ae1～Bh2 全設沒裝就 return（`main.cpp:19236-19248`）⇒ 這些格「---」；DoThermo 也直接 return（`bthermo.cpp:1177`），`UN150Read` 不會更新。Plate／Shuttle／Chamber／Socket 的「有裝」不看 No Heater〔V〕 |
| **Index 4 組**（`USE_16_HEATER`＝0，eht4Heater） | 舊窗（`cpp:406-407`） | Plate 1、Plate 2、SH 1、SH 2、**Head 1/2、Head 3/4、Head 5/6、Head 7/8**、Dut、〔Chamber：ATC 沒裝時，`cpp:364`〕、〔CCD：`REAL_TIME_CCD`，`cpp:207-212`〕、〔Heat Gun 1/2：`INSTALL_HEAT_GUN`>0，`cpp:362-363`〕；DUT 2／4 顆時 Dut 換成 gb_Index（DUT1～4）（`cpp:299-359`） | Head1／Head2（`tcHead1`、`tcHead2`）＝臂 0，Head3／Head4＝臂 1；SingleSite＋`bSingleHeater` 時只有 Head1、Head3（`main.cpp:20846-20864`）。Index 區（Aa1～）全部沒裝 |
| **Index 16 組**（1 KT4H／2 EJ1N／5 DTME08） | 新窗第 1 列（`cpp:407-424`） | Hot Plate、Shuttle、**Arm1（Aa1～Bd1）、Arm2（Aa2～Bd2）**、Index（Chamber、Socket）、〔CCD〕〔ATC air〕〔Heat Gun〕〔Door〕；有 CCD2／L/B／ESD 才開第 2 列 | 依測試模式（下表 §4）；Head1～4 沒裝 |
| **Index 32 組**（3 EJ1N／4 KT4H／6 DTME08） | 新窗兩列（`cpp:426-438`、`:494-496`） | 第 1 列同 16 組；第 2 列 〔Chamber〕〔DUT〕〔Base〕、**Arm1（Ae1～Bh1）、Arm2（Ae2～Bh2）**、〔CCD2〕〔L/B〕〔ESD〕 | 32 組只有 2x8＋`TestIF_File.bUse32Heater` 是一 socket 一區（Ae～Bh＝第 4～7 欄，`main.cpp:20406-20445`） |
| **DUT 2／4 顆**（`SocketBasedAdd4Temp` → `iSocketBaseTempCount`） | `cpp:251-262`、`:285-292` | Index 群組（Chamber／Socket）藏起來，第 2 列出現 gb_Index（DUT 1～4；2 顆時 DUT3／4 藏），Chamber 只有 ATC 沒裝時出現 | HT9046_LS 且 SubMachineType 是 None／AU／CR 時強制 4 顆（`database.cpp:1396-1402`）——這是 SubMachineType 唯一的影響 |
| **TriTemp**（HT-1032，`Tri_Temp_Machine`＝1） | `cpp:376-400` | 第 1 列 HP 1、HP 2、Shuttle 1、Shuttle 2、Arm1、Arm2、Out Sht、〔Door〕；第 2 列 〔DUT〕、Base、Arm1、Arm2、〔CCD2〕、「Air Stream」、〔ESD〕、L/B | 名稱改從 `uTemp_Set.cpp:98-149` 來（「Plate 1-1」等），Index 區顯示原始名 Aa1～Bh2 |
| **ATC** | 版面只有少數開關：eNewATCSystem 且 `iATC_Use_Heat_Count`≥16 也開 32 組那兩群（`cpp:429-430`、`:436-437`）；Chamber 只在 ATC 沒裝時出現；eNewATCSystem 不做 `SetIndex16HeaterPos` | 見 §5 | 見 §5 |
| **HT9050** | 跟 V912 同一份 `cTemperFrom` | 照它的 `USE_16_HEATER`（DTME08 是 5／6）走上面的版面 | 見 §6 |

其他選配：CCD（`REAL_TIME_CCD`；第二組 `RTC_TemperNumber`＝2 時通道 52 顯示「CCD1」）、`CCD2_TEMPER`（通道 49 tc2D 顯示「CCD2」）、`LB_TEMP`、`LB_TEMP_UpDown`、`Index_ESDAir`、`IndexDoorHeater`、
`INSTALL_HEAT_GUN`、`INSTALL_ATC_HEAT_GUN`，都在 `D:\HT9045\system\Gerneral.ini [System]`／`[ATC]`（G `database.cpp:625`、`:634-639`、`:684`、`:691-698`、`:1081-1082`、`:1394`、`:1514`、`:1523`）。

## 3. 71 個通道一覽：畫面名稱 → 面板 → 實體位址

- 畫面名稱：0～26、33～48 由 `ShowHotName`（`cpp:1782-1828`）換成 `asTempCtrl[i]`（沒裝就名稱和數值都「---」）；27～32、49～70 不改名，用 dfm／FormShow 的字。
- 實體位址：**整台廠牌**看 `[TempCtrl] HEATER_CTRL_TYPE`，V912 另有逐通道 `HeaterInsOpt_<通道>`（`ht9045-heater-control` §2）；全部共用溫控 COM 埠。
  - KT4H／DTK4848：站號＝序號＋1，封包用兩位十六進位，**面板設十進位**；E5DC：同一個號碼，封包用兩位十進位（`controllers\index.md`、`controllers\omron-e5dc.md` §1.3）。
  - TC401：一台 4 通道，第（序號÷4）＋1 台、通道＝序號%4（`cpublic.cpp:422-461`）。
  - **Index 區（Aa1～Bh2）例外**：`USE_16_HEATER`＝2／3 走 EJ1N（自己的 COM 埠、台號＝SW1）、＝5／6 走 DTME08（Ethernet、站＝內部站號），不走溫控 COM 埠（`bthermo.cpp:1331-1367`）；＝1／4 才照序號＋1 走溫控 COM 埠。
  - DTME08 的「站」是內部站號：0＝DTME08 主機，1～3＝DTMN08 的旋鈕（`controllers\delta-dtm.md` §1.3）；CH＝手冊的 CH1～CH8。
  - ⛔ 20261002（E-029，移植樹，golden 沒有）：任何通道都可以是 5 Omron EJ1N／6 Delta DTM（`[TempCtrl] HeaterInsOpt_<通道>`）。「各溫控器不同」存 `HeaterInsAddr_<通道>`（EJ1N＝台號、DTM＝內部站號）＋`HeaterInsCh_<通道>`（CH）；Index 區不填＝上表 EJ1N／DTME08 兩欄（golden `iTempCode` 接線），Index 區以外 golden 沒有對照、一定要填。規則：`D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §9。

| # | 通道（eTempControll） | 畫面名稱 | 面板（golden dfm） | KT4H／E5DC／DTK4848 站號 | TC401 第幾台／通道 | Index 走 EJ1N（2／3）台／CH | Index 走 DTME08（5／6）站／CH |
|---:|---|---|---|---:|---|---|---|
| 0 | `tcHotPlate1` | Plate 1 | hlTempPlate1_2 ／ 舊窗 hlTempPlate1 | 1 | 1／通道 0 | — | — |
| 1 | `tcHotPlate2` | Plate 2 | hlTempPlate2_2 ／ hlTempPlate2 | 2 | 1／通道 1 | — | — |
| 2 | `tcShuttle1` | SH 1 | hlTempShuttle1_2 ／ hlTempShuttle1 | 3 | 1／通道 2 | — | — |
| 3 | `tcShuttle2` | SH 2 | hlTempShuttle2_2 ／ hlTempShuttle2 | 4 | 1／通道 3 | — | — |
| 4 | `tcHead1` | Head 1/2 | 舊窗 hlTempHead12 | 5 | 2／通道 0 | — | — |
| 5 | `tcHead2` | Head 3/4 | 舊窗 hlTempHead34 | 6 | 2／通道 1 | — | — |
| 6 | `tcHead3` | Head 5/6 | 舊窗 hlTempHead56 | 7 | 2／通道 2 | — | — |
| 7 | `tcHead4` | Head 7/8 | 舊窗 hlTempHead78 | 8 | 2／通道 3 | — | — |
| 8 | `tcSocket` | Dut（DUT 2／4 顆時不改名） | hlTempDut_2 ／ hlTempDut | 9 | 3／通道 0 | — | — |
| 9 | `tcChamber` | Chamber | hlTempChamber_2（或第 2 列 hlTempChamber_3）／ hlTempChamber | 10 | 3／通道 1 | — | — |
| 10 | `tcCCD` | CCD | hlTempCCD_3 ／ hlTempCCD | 11 | 3／通道 2 | — | — |
| 11 | `tcAa1` | A1 | hlTempAa1（gbArm1） | 12 | 3／通道 3 | 1／CH1 | 0／CH1 |
| 12 | `tcAb1` | C1 | hlTempAb1（gbArm1） | 13 | 4／通道 0 | 1／CH3 | 0／CH3 |
| 13 | `tcAc1` | E1 | hlTempAc1（gbArm1） | 14 | 4／通道 1 | 2／CH1 | 0／CH5 |
| 14 | `tcAd1` | G1 | hlTempAd1（gbArm1） | 15 | 4／通道 2 | 2／CH3 | 0／CH7 |
| 15 | `tcBa1` | B1 | hlTempBa1（gbArm1） | 16 | 4／通道 3 | 1／CH2 | 0／CH2 |
| 16 | `tcBb1` | D1 | hlTempBb1（gbArm1） | 17 | 5／通道 0 | 1／CH4 | 0／CH4 |
| 17 | `tcBc1` | F1 | hlTempBc1（gbArm1） | 18 | 5／通道 1 | 2／CH2 | 0／CH6 |
| 18 | `tcBd1` | H1 | hlTempBd1（gbArm1） | 19 | 5／通道 2 | 2／CH4 | 0／CH8 |
| 19 | `tcAa2` | A2 | hlTempAa2（gbArm2） | 20 | 5／通道 3 | 3／CH1 | 1／CH1 |
| 20 | `tcAb2` | C2 | hlTempAb2（gbArm2） | 21 | 6／通道 0 | 3／CH3 | 1／CH3 |
| 21 | `tcAc2` | E2 | hlTempAc2（gbArm2） | 22 | 6／通道 1 | 4／CH1 | 1／CH5 |
| 22 | `tcAd2` | G2 | hlTempAd2（gbArm2） | 23 | 6／通道 2 | 4／CH3 | 1／CH7 |
| 23 | `tcBa2` | B2 | hlTempBa2（gbArm2） | 24 | 6／通道 3 | 3／CH2 | 1／CH2 |
| 24 | `tcBb2` | D2 | hlTempBb2（gbArm2） | 25 | 7／通道 0 | 3／CH4 | 1／CH4 |
| 25 | `tcBc2` | F2 | hlTempBc2（gbArm2） | 26 | 7／通道 1 | 4／CH2 | 1／CH6 |
| 26 | `tcBd2` | H2 | hlTempBd2（gbArm2） | 27 | 7／通道 2 | 4／CH4 | 1／CH8 |
| 27 | `tcHeatGun1` | Gun 1（舊窗 Heat Gun 1） | hlTempHeatGun1_2 ／ hlTempHeatGun1 | 28 | 7／通道 3 | — | — |
| 28 | `tcHeatGun2` | Gun 2（舊窗 Heat Gun 2） | hlTempHeatGun2_2 ／ hlTempHeatGun2 | 29 | 8／通道 0 | — | — |
| 29 | `tcDUT1` | DUT 1 | hlTempDut_A1（gb_Index） | 30 | 8／通道 1 | — | — |
| 30 | `tcDUT2` | DUT 2 | hlTempDut_A2（gb_Index） | 31 | 8／通道 2 | — | — |
| 31 | `tcDUT3` | DUT 3 | hlTempDut_A3（gb_Index） | 32 | 8／通道 3 | — | — |
| 32 | `tcDUT4` | DUT 4 | hlTempDut_A4（gb_Index） | 33 | 9／通道 0 | — | — |
| 33 | `tcAe1` | I1 | hlTempAe1（gbArm1_2） | 34 | 9／通道 1 | 5／CH1 | 2／CH1 |
| 34 | `tcAf1` | K1 | hlTempAf1（gbArm1_2） | 35 | 9／通道 2 | 5／CH3 | 2／CH3 |
| 35 | `tcAg1` | M1 | hlTempAg1（gbArm1_2） | 36 | 9／通道 3 | 6／CH1 | 2／CH5 |
| 36 | `tcAh1` | O1 | hlTempAh1（gbArm1_2） | 37 | 10／通道 0 | 6／CH3 | 2／CH7 |
| 37 | `tcBe1` | J1 | hlTempBe1（gbArm1_2） | 38 | 10／通道 1 | 5／CH2 | 2／CH2 |
| 38 | `tcBf1` | L1 | hlTempBf1（gbArm1_2） | 39 | 10／通道 2 | 5／CH4 | 2／CH4 |
| 39 | `tcBg1` | N1 | hlTempBg1（gbArm1_2） | 40 | 10／通道 3 | 6／CH2 | 2／CH6 |
| 40 | `tcBh1` | P1 | hlTempBh1（gbArm1_2） | 41 | 11／通道 0 | 6／CH4 | 2／CH8 |
| 41 | `tcAe2` | I2 | hlTempAe2（gbArm2_2） | 42 | 11／通道 1 | 7／CH1 | 3／CH1 |
| 42 | `tcAf2` | K2 | hlTempAf2（gbArm2_2） | 43 | 11／通道 2 | 7／CH3 | 3／CH3 |
| 43 | `tcAg2` | M2 | hlTempAg2（gbArm2_2） | 44 | 11／通道 3 | 8／CH1 | 3／CH5 |
| 44 | `tcAh2` | O2 | hlTempAh2（gbArm2_2） | 45 | 12／通道 0 | 8／CH3 | 3／CH7 |
| 45 | `tcBe2` | J2 | hlTempBe2（gbArm2_2） | 46 | 12／通道 1 | 7／CH2 | 3／CH2 |
| 46 | `tcBf2` | L2 | hlTempBf2（gbArm2_2） | 47 | 12／通道 2 | 7／CH4 | 3／CH4 |
| 47 | `tcBg2` | N2 | hlTempBg2（gbArm2_2） | 48 | 12／通道 3 | 8／CH2 | 3／CH6 |
| 48 | `tcBh2` | P2 | hlTempBh2（gbArm2_2） | 49 | 13／通道 0 | 8／CH4 | 3／CH8 |
| 49 | `tc2D` | CCD2 | hlTemp2D_2（grp2DID） | 50 | 13／通道 1 | — | — |
| 50 | `tcLB` | L/B | hlTempLB | 51 | 13／通道 2 | — | — |
| 51 | `tcIndexESD` | ESD | hlTempESD | 52 | 13／通道 3 | — | — |
| 52 | `tcCCD_2` | CCD1 | hlTempCCD_2_2 | 53 | 14／通道 0 | — | — |
| 53 | `tcATCHotAir1` | Gun 1（ATC air 群組） | hlTempATCHeatGun1_2（gbATCHeatGun） | 54 | 14／通道 1 | — | — |
| 54 | `tcATCHotAir2` | Gun 2（ATC air 群組） | hlTempATCHeatGun2_2 | 55 | 14／通道 2 | — | — |
| 55 | `tcOutSht1` | Out Sht 1 | hlTempOutShuttle1 | 56 | 14／通道 3 | — | — |
| 56 | `tcOutSht2` | Out Sht 2 | hlTempOutShuttle2 | 57 | 15／通道 0 | — | — |
| 57 | `tcBase1` | Base 1 | hlTempBase1（grpBase） | 58 | 15／通道 1 | — | — |
| 58 | `tcBase2` | Base 2 | hlTempBase2（grpBase） | 59 | 15／通道 2 | — | — |
| 59 | `tcBase3` | Base 3 | hlTempBase3（grpBase） | 60 | 15／通道 3 | — | — |
| 60 | `tcBase4` | Base 4 | hlTempBase4（grpBase） | 61 | 16／通道 0 | — | — |
| 61 | `tcBase5` | Base 5 | hlTempBase5（grpBase） | 62 | 16／通道 1 | — | — |
| 62 | `tcBase6` | Base 6 | hlTempBase6（grpBase） | 63 | 16／通道 2 | — | — |
| 63 | `tcHotPlate3` | Plate 1（只有 TriTemp 會改名） | hlTempPlate_3（grp_HopPlate2） | 64 | 16／通道 3 | — | — |
| 64 | `tcHotPlate4` | Plate 2（同左） | hlTempPlate_4 | 65 | 17／通道 0 | — | — |
| 65 | `tcShuttle3` | Shut 1（同左） | hlTempShuttle_3（grp_Shuttle2） | 66 | 17／通道 1 | — | — |
| 66 | `tcShuttle4` | Shut 2（同左） | hlTempShuttle_4 | 67 | 17／通道 2 | — | — |
| 67 | `tcDoor1` | Door 1 | hlTempDoor1 | 68 | 17／通道 3 | — | — |
| 68 | `tcDoor2` | Door 2 | hlTempDoor2 | 69 | 18／通道 0 | — | — |
| 69 | `tcLBUp` | L/B Up | hlTempLBUp | 70 | 18／通道 1 | — | — |
| 70 | `tcLBDown` | L/B Down | hlTempLBDown | 71 | 18／通道 2 | — | — |

## 4. Index 區：socket → 通道 → 畫面字母（G `TfMain::Index16Heater`，`main.cpp:19231-20878`）

這支函式依測試模式決定哪幾區「有裝」；每一區對應 socket `LastSet.bUseTestSocket[臂][排][欄]`，那個 socket 有開（或 L17「關 Site 也加熱」）才裝〔V〕。

| 測試模式 | 16 組時一區管幾個 socket | 舉例 |
|---|---|---|
| 1x4（`QualSite1X4`） | 一區一個 | Aa1、Ba1、Ab1、Bb1＝socket 第 0～3 欄（只有一排，這裡的 B 不是第 1 排；`main.cpp:19529-19536`）；畫面配合 `SetIndex16HeaterPos` 重排成 A1 B1 C1 D1 |
| 2x2（`QualSite2X2`） | 一區一個 | Aa／Ba／Ab／Bb（`main.cpp:19581-19597`） |
| 2x4（`_8Site2X4`） | 一區一個 | 排 A／B × 欄 a～d |
| 2x8（`_16Site2X8`），16 組 | **一區管相鄰兩欄** | Aa1＝第 0、1 欄，Ab1＝第 2、3 欄……（`main.cpp:20457-20473`） |
| 2x8，32 組（`TestIF_File.bUse32Heater`） | 一區一個 | Aa1～Ad1＝第 0～3 欄、Ae1～Ah1＝第 4～7 欄（`main.cpp:20406-20445`） |
| Index 4 組（eht4Heater） | 不用 Index 區 | Head1／Head2＝臂 0、Head3／Head4＝臂 1（`main.cpp:20846-20864`） |

另外：`IniConfig.bD58UseArm1PickPlaceArm2Test` 且臂 1 不加熱時，臂 1 那 16 區全部沒裝（`main.cpp:20867-20876`）；ATC 開主動冷卻時，部分模式只開 ATC 管的那幾區（同函式各模式的 ATC 分支）。
完整 15 種測試模式的逐行對照沒有展開，要用時直接讀這支函式。

## 5. ATC

**ATC 不改版面、不改名稱，改的是數值從哪來。** 溫度視窗一律顯示 Tc，從不顯示 TJ〔V〕。

| 情況 | 哪些格子改顯示 ATC | 值從哪來 | 斷線時 |
|---|---|---|---|
| `bATCActiveCooling` 且 ATC_SYSTEM＝eNewATCSystem／eATCHonPrecType／eWinWay | **Index 區 Aa1～Bd2、Ae1～Bh2**（G `bthermo.cpp:1316-1330` 改走 Task 270「讀取 Site 溫度」） | `DOUN150ReadTemp(Addr)`（`bthermo.cpp:4140-4370`）：ATC 通道＝`fTemp_Set->iAddrToATC[Addr]`；New ATC＝`ATC_InterfaceForm->dTC[ch]/10.0`；HonPrec＝`GetATCSiteNowTemperature(ch)`；WinWay＝`fWinway->arrATC_Site[k]->GetPT()` | **9999**（不是 999）⇒ 畫面顯示「9999.0」紫色，不會是「ERR」〔推，從程式順序〕 |
| eATCSiliconType（序列 ATC） | Task 260 | `fATCReadBuffer[Addr]`（`bthermo.cpp:3093-3150`） | 999 |
| eATC60／eATC30 | Index 16 區 | `DoATC60Temperature`：`ATC_60_SYS.ReadTemp[0..15]` 照 Aa1、Ba1、Ab1、Bb1…交錯排（`bthermo.cpp:3471-3496`） | 999 |
| TriTemp＋New ATC（40 通道） | 再加 HotPlate1～4、Shuttle1～4、HotAir1／2 | `dTC[...]/100.0`（`bthermo.cpp:4283-4367`） | -999 |
| ATC 熱風槍捷徑（eNewATCSystem＋主動冷卻＋`bActiveHeatGun`＋IS_ATC33() 或 ATC_TYPE_61） | HeatGun1／2 | 加熱器讀值，一律綠色、不比範圍（`cTemperFrom.cpp:771-784`）——不是 ATC 讀值 | — |

- **通道 → ATC 通道**：`TfTemp_Set::InitialAddrToATC()`（G `uTemp_Set.cpp:6437` 起）建 `iAddrToATC[tc]`（預設 -1）與 `iSiteToATC[臂][排][欄]`、`iSiteToOfs`（預設臂 1 `j+i*8`、臂 2 `j+16+i*8`）；
  eht4Heater 時 Head1～4 → 0～3（`:6465-6485`），其他依測試模式各一張表（例 `:6549-6646`）〔V〕。`iATCUseChannel` 在 G 不存在〔V〕。
- **範圍與顏色**：ATC 區的允差改用 `IniConfig.iATCTemperatureRange`（AmbientHot 用 `fAmbientHotGuartbent`）（`cTemperFrom.cpp:1000-1017`）；讀值 <18 顯示「...」，只有 IS_ATC33()／ATC_TYPE_61 例外（`:1171-1186`）。
- **顯示的 ATC 值不含 offset**：`dATCTempAdjustmentOffset` 只算不加（`bthermo.cpp:3157-3200`）〔V〕。
- **真正的 ATC 溫度面板在 LotInfo**：`TfTemperFrom::Timer1Timer` 每拍呼叫 `fLotInfo->ShowATCThermo()`（`cTemperFrom.cpp:1689`），依型別分到 `ShowATC70Thermo`（eATCHonPrecType＋`bATC70Active`）、
  `ShowATC20Thermo`（eATCHonPrecType）、`ShowNewATCThermo`（eNewATCSystem）（G `uLotInfo.cpp:5574-5629`）。這裡的第二欄才會顯示 TJ（`bShowTJTemp`，`:6559-6571`；ATC7.0 因為 `||` 優先序永遠顯示 TJ）。
  LotInfo 的 `ATCPtr[i]` 照 **ATC 原始通道**排、不經 `iAddrToATC`，所以兩個畫面的順序可能不同〔V〕。溫度視窗關掉（`FormClose` 停 Timer1，`cTemperFrom.cpp:1696-1699`）時 LotInfo 的 ATC 面板也停更新〔推〕。
- **主畫面本身**沒有即時溫度：只有設定值 `edWorkTemperBase`（`fWorkTemperBase`）、`edATCAmbientTemper`、`edSoakTime`、模式字 `lblTemperatureMode`（「ATC Mode／Ambient Mode／Hot Mode」）、
  `labATC`（「ATC Off Line／On Line／On Line with TJ mode」）（G `main.cpp:13330-13601`、`:27713-27735`）〔V〕。
- `bATC32UseTJMode` 只改 GPIB 回的字串（Tc 取代第二點），不改格子（`cTemperFrom.cpp:1274-1288`）〔V〕。
- `iATC_Use_Heat_Count`＝2 有對照表，但 `DOUN150ReadTemp` 沒有 `case 2` ⇒ 回 0.0、畫面「...」〔推〕。

## 6. HT9050

- **畫面**：H `cTemperFrom.cpp`（2030 行）跟 G 逐位元組相同，`.dfm` 也相同；H 的 `cTemperFrom.cpp` 跟 V908 只差 ATC33／`bATC32UseTJMode` 那幾行（`:774`、`:1172`、`:1275`）。
  cTemperFrom／main／bthermo／EJ1N 裡沒有任何 HT9050 專用的溫度畫面；H 裡 HT9050 的程式都在 Loader／Tray（`database.cpp:411-418` 等）〔V〕。⇒ **HT9050 的主畫面溫度要長什麼樣子，golden 沒有答案，要定。**
- **配線**（`D:\HT9045\.claude\skills\ht9050-hw\references\temp-dtm-map.md`）：3 站 24 通道——站 1 DTME08：Hotplate1／2、In Shuttle1／2、DUT1～4；站 2 DTMN08：Chamber、Hot Air 1／2；
  站 3（表上寫 DTME08）：SLK-1～8（推成 tcAa1～tcBd1，順序還沒確認，SLK-SITE-ORDER）。
- **H 的程式怎麼開 DTM**：`DoThermo` → `DoSetSVOfDTME08()`（H `bthermo.cpp:4714-4792`），對 `i<INDEX_HEAT_COUNT` 用 `Addr=iTempCode[i]`，也就是 **HT9045 Index 的順序**（Aa1、Ba1、Ab1…）；
  16 組時 `Addr>=tcAe1` 跳過（`:4782`）〔V〕。⇒ 照 H 的程式，站 1 CH1 拿到的是 tcAa1 的設定值而不是 HotPlate1；16 組時站 3 不會被讀寫；HotPlate／Shuttle／DUT／Chamber／HeatGun 仍走溫控 COM 埠〔推，程式已讀、沒上機〕。
  HT9050 的通道表要另外做，不能照抄 `iTempCode`。逐通道廠牌的選項也沒有「Delta DTM」（G `MachineType.h:656`、`MachineTypeUtility.cpp:19-24`）。
  ⛔ 20261002 更正（E-029，St01；Steven 1002「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」）：移植樹的 HW.HandlerSys「Heater」現在有「Omron EJ1N」（代碼 5）與「Delta DTM」（6）——「各溫控器不同」每個通道 7 個廠牌都能選（HT9050 的 Hotplate／Shuttle／DUT 可以設成 DTM＋站＋CH），「全機相同」時只在 Index 下拉。golden V912 仍沒有；底層（bthermo 等）還不讀 5／6。見 `D:\HT9045\.claude\skills\ht9045-heater-control\SKILL.md` §9。
- **網頁**：`Main.MotionView9050.html`、`Alert.MotionView9050.html`、`IDE.MotionView9050-*.html` 都沒有顯示溫度、也不讀溫度 tag；HotPlate 只是版面上的一塊（`MotionView9050-layout.json:26`）〔V〕。

## 7. V906 網頁現況（移植樹 HEAD 20261002）

| 畫面 | 有什麼 | 資料來源 |
|---|---|---|
| `D:\HT9045\web\page\Status.TemperFrom.html`（主畫面下方，`D:\HT9045\web\background.html:439`，x 0／y 720） | 11 格溫度：Hot Plate、Shuttle（各合成一格）、Arm1 4 格、Arm2 4 格、CCD | **全部「---」，沒有 producer**（`:85-93`） |
| `D:\HT9045\web\page\main.html` | `edWorkTemperBase`（temp.sv）、`edSoakTime`（temp.soak）、`lblTemperatureMode`（temp.mode） | 設定值，不是量測值（P `WebBridgeTags.cpp:1010-1012`） |
| （沒有頁面讀） | temp.pv、zone.hotplate／shuttle／index／heatgun | `kUnloadedTags`，一律 null（`WebBridgeTags.cpp:220-233`） |
| （沒有頁面讀） | temp.zone.<ch>.pv／comm／inst（71×3＝213 個） | P `JsonBridge\StageThermo.cpp:171-200`，`tools\wb_serve.cpp:2888` 有在發，但 live＝false 一律 null |

- ⚠ **`Status.TemperFrom.html` 的 Arm 格子 id 放錯**：表頭寫「Arm1（A1 C1 E1 G1）」，底下四格是 `tempAa1`、`tempAc1`、`tempAe1`、`tempAg1`（`:59`、`:63`）。
  golden 的 A1 C1 E1 G1 是 **Aa1、Ab1、Ac1、Ad1**（§3 表）；Ac1＝E1、Ae1＝I1、Ag1＝M1，而且 Ae1／Ag1 是 32 組第 2 列（gbArm1_2）的通道。Arm2 同樣錯。改的時候照 §3 表。
- 移植樹 `cTemperFrom.cpp`（1433 行）：建構子、`ShowThermo`（`:234-1156`）、`ShowHotName`（`:1332-1381`）有翻但**沒有正式呼叫端**（只有 `tests/test_temperfrom_core.cpp`）；WAR15 告警那段 `#if 0`（GATE T1，`:977-1006`）；
  `FormShow`、`ChangeFormSize`、`SetIndex16HeaterPos`、`Timer1Timer` 沒翻（`forms\fTemperFrom.h:72-91`）〔V〕。
- **`bUT150Install[]` 已經有真值**（I-01，Ifor）：`forms\fMain_Heater.cpp` 翻了 `Index16Heater`／`IndexHeatMode`／`HotplateHeatMode`（寫 `bUT150Install[]`），開機 `tools\wb_serve.cpp:4233` 跑一次 `IndexHeatMode`（golden `TfMain::FormShow`），之後每秒 `WebBridgeTags.cpp:601` 的 `W906_Timer2HeaterTick`（golden `Timer2Timer` 加熱段）〔V，review6 `067d4c7a`；Ifor 1002 確認〕。但 `temp.zone.*.inst` 仍發 null（`kThermoFields` 的 live＝false）。
  ⚠ `StageThermo.cpp:50-67`（`kThermoFields` 的 inst 列）的說明（「寫入點只在 main.cpp、沒移植」）已經過時；檔案是筆電的，由 Ifor 在接 inst 的同一個 commit 改。
  **inst 先單獨上線**（Steven 1003 17:3x「可以先推了」；Ifor 1002 原本建議等 pv）：Ifor 在 `PublishThermoTags` 讀 `bUT150Install[]`，inst 列 live 改 true；pv／comm 維持 false。1003 17:34 已寄信通知 Ifor（副本 Steven）。
  上線後：沒裝的通道顯示 ---、標「沒裝」；有裝的通道在 pv 接上前仍是 ---、標「沒有資料」（`web\page\ht9045_temperfrom_strip.js:147-149`）。
- 量測值（`UN150Read[]`）與通訊異常（`UN150CommError[]`）唯一的寫入路徑是 `DoThermo`。加熱執行緒本身仍不啟動（本 skill SKILL.md §2），但 **1003 起出貨版由 serve loop 的快鐘每 20 ms 跑一次 `DoThermo`**（Jimmy `153f9a5e` FASTCLK step 3：`FastClockJobs.cpp:8`、`:22`；`MachineType.h:1852` `W906_FASTCLK_HEATER`）〔V，main `b111825c`〕。
  模擬版照舊不跑：`HeaterSimTick.cpp` 刻意不跑 `DoThermo`（`:18`、`:49`）；它的出貨版分支仍是空函式（`:26`），出貨版改由 `FastClockJobs.cpp` 跑〔V〕。
  pv／comm 仍發 null：Jimmy 在 `StageThermo.cpp:28` 註明要上機量過才改 live（`DoThermoReal` 讀哪些通道要看 `bUT150Install[]`）。Ifor 的計畫：上機確認後，同一個 commit 在 `PublishThermoTags` 加 pv／comm 讀值並把 live 改 true；`StageThermo.cpp` 是筆電的檔，先逐行認領。
- 新溫度條（E-027，§9.1）**已經在 main**：Steven Q70＝A，review6 到 `81a106d2` 為止的內容 1003 合進 main（交付包 130）。
- **HT9050 的格子要等 I-03b**（DTM 站號／CH → `eTempControll` 對照表）：golden DTM 讀值寫到 `UN150Read[iTempCode[i]]`，`iTempCode` 是 HT9045 Index 的順序（Aa1、Ba1…，§6），溫度條的 HT9050 版讀 `tcHotPlate1`／`tcShuttle1`／`tcDUT1`／`tcChamber`／`tcHeatGun1`，對不上 ⇒ 底層接值之後 HT9050 也要 I-03b 做完才有數字；I-03 第二段（MR !115）因此還不啟動 DTME08 輪詢（Ifor 1002）。

## 8. 意外發現（golden 原樣，只記不改）

1. 舊窗（4 組）會多一條 70 px 空白：`grpChamber`／`gb_Index` 在 dfm 預設看得到，讓 `pnl32Heater->Visible=true`（`cpp:497-506`），而它的上層已經藏起來，`ChangeFormSize` 仍加 70 px（`cpp:1862-1865`）〔V 程式／推 畫面〕。
2. 三個叫 CCD 的格子分在三個通道：10「CCD」、52「CCD1」、49（tc2D）「CCD2」；grp2DID 的「2D」字樣永遠看不到〔V〕。
3. tcSocket 的格子 dfm 寫「Socket」，執行時被 `ShowHotName` 改成「Dut」；16／32 組＋DUT 1 顆時 `ShowTempComp[tcDUT1]` 指到舊窗（藏起來）的 `hlTempDut`，等於沒顯示（`cpp:268-269`）〔V〕。
4. ATC 熱風那兩格（通道 53／54）名稱一直是「Gun 1／Gun 2」，`asTempCtrl` 的「ATC hot air1／2」永遠不會出現（ShowHotName 不改 53／54）；4 組＋DUT 1 顆時 ATC 熱風根本不顯示（`cpp:341-345` 只在 DUT 4 顆分支搬）〔V〕。
5. `SetIndex16HeaterPos` 1x4 用 `TestIF_File.iTestMode`、8Site1x4 用 `TestIF.iTestMode`；不看 `IniConfig.bL30Use1CableLayoutKitByConfig`（`main.cpp:19518-19519` 有看）；Ae～Bh 從不重排（`cpp:1874-1911`）〔V〕。
6. `Timer1Timer` 的防重入寫成 `if(bEnter) bEnter=true;`，從不 return（`cpp:1635-1636`）〔V〕。
7. 其他程式把格子上的字當資料讀：TriTemp 解析格子文字（`TriTemp.cpp:825-866`、`:2772-2781`），SECS 讀 `RunInfo.ShowTempComp`（`cpp:1651`）——網頁重做時改格子名稱或位置會影響這些邏輯〔V〕。
8. `SECSGEM\SECSGEM.cpp:682` 用了不存在的 `fTemperFrom->hlTempCCD_2`（只有 `hlTempCCD_2_2`，`cTemperFrom.h:244`），在 `#ifdef SECS_GEM` 裡〔V〕。

## 9. 新版顯示提案（20261002，待 Steven 選；todo E-027）

Steven 1002 13:xx：「客戶比較期望可以一眼看到全部站的溫度 / 你可以做成縮小版 (只有PV) 跟張開全部顯示的版本, 再提案一次」。
ST01-E2 做的設計畫布（私人連結，只有 Steven 看得到）：https://claude.ai/artifact/PJgHjdd3HrMgtGEyY1fcQV ，五張：

| 圖 | 內容 |
|---|---|
| HT9050 縮小版 | 925×140（＝現在 `Status.TemperFrom.html` 的位置），19 區只顯示 PV；左邊設定值、模式、正常／偏低／過溫／異常計數、「展開全部」 |
| HT9050 展開版 | 依區域（每格 PV、SV、Δ、範圍條、MV、站／CH）或依溫控器（站 1～3 × CH1～8，空的 CH 也列） |
| 9 系列 Index 16 組 縮小版 | Plate、SH、Arm1／Arm2 各 2 排 × 4 欄、Index、CCD |
| 9 系列 Index 32 組 縮小版 | 一條摘要＋32 區一排 |
| 9 系列 展開版 | 兩個 Arm 照 socket 實際位置（上 A 排、下 B 排、欄 a→h），格子寫畫面站名＋溫區代號＋實體位址；可切 Index 溫控器（KT4H／EJ1N／DTME08）與 ATC |

- 顏色照 golden 的意思（範圍內綠、偏低黃、過溫紫紅）再加 ▼▲! 符號；跟 golden 不同的是 32 組改成一個 Arm 一張 2×8。
- 位址公式用腳本對過 §3 表（32 區 × 3 廠牌 0 不符）。畫面數值是示意；機台上現在拿不到 PV（§7），要先有資料來源。
- 待確認：SLK 1～8 的實際排法、HT9050 站 3 型號（todo C-003）。Steven 選定後，再改 `Status.TemperFrom.html`（St01）。

### 9.1 已整合進畫面（20261002，ST01-E2；branch `v906/steven-st01e2-e027-tempdisplay`，`f4600609`，待 ST01-E 合進 review6）

Steven 1002：「那個配置挺不錯的, 協助整合進去畫面中吧, 要注意原本就有四種的佈景主題」。

| 檔（`D:\HT9045\web\`） | 做了什麼 |
|---|---|
| `page\Status.TemperFrom.html` | 舊的 11 格表（Arm 格子 id 放錯，todo E-026）換成 `#tzCompact`（縮小版）／`#tzExp`（展開版）。7 個功能 chips 與 palLed 三顆燈（HandlerSys 隱藏入口，E-023 TP-2）原樣搬到同一列；palLed 從 `.srcnote` 搬出來——release 模式 `ht9xxx.css:115` 會把 `.srcnote` 整條藏起來，燈也跟著不見 |
| `page\ht9045_temperfrom_strip.js`（新） | 依 `machine.gpibModel`（`9050GPIB`＝HT9050，用 §6 的 3 站表）或 `Gerneral.ini`（`USE_16_HEATER`、`SocketBasedAdd4Temp`、CCD、熱風、ATC air、LB、ESD、Door）排格子；訂閱 `temp.zone.<tc>.pv／comm／inst`、`temp.sv`、`temp.mode`；位址照 §3（`HEATER_CTRL_TYPE`／`HeaterInsOpt_`、EJ1N、DTM）；判讀是 golden ShowThermo 的子集（§1）：著色只做設定值＝`fWorkTemperBase`、允差＝`config.ini [Tempture] iL04TemptureRange` 的通道，且只在 Hot／AmbientHot |
| `page\ht9045_temperfrom_strip.css`（新） | `--tz-*` 狀態色，classic／dark／steel／contrast 各一組；順手修 dark 下 `.unk` chip 看不到字 |
| `background.html` | 新訊息 `{resizeMe:{h}}`／`{resizeMe:null}`：送出訊息那一頁自己的視窗往上放大／恢復（`RESIZE_ME_IDS` 只開 `temperf`）。**沒有新增頁面、沒有動 WebPageTable**（St02 LI-9 F1 正在改列數） |

- 驗證：`E023_StatusPages`（node）33/33；node 邏輯檢查（各機型格子數、KT4H／TC401／E5DC／HeaterInsOpt／EJ1N／DTM／HT9050 位址、判讀規則）；
  無頭 Edge 截圖 16／32／HT9050 縮小版 × 4 主題、展開版（假資料的測試頁放 scratch，沒 commit）。上機要 EastSun 看（human-review A）。
- 今天 C++ 對 `temp.zone.*` 一律發 null（§7），所以機台上格子全是 ---；資料來源接上後不用改頁面。
- 跟 golden 一樣（沒有另外處理）：只有 999 與通訊旗標顯示 ERR；ATC 斷線回的 9999 照數值顯示「9999.0」（golden 原樣，§5、todo 已登記）。
- 後續（20261002）：
  - **已合進 review6**：ST01-E 合成 `5a7c1bba`，含 `339ac421` 與 `dc3a0b64`。SIM／SHIP 下讀頁面的 27 個 ctest 都過。
  - **位址寫法**：照 Steven「第x台 CHx → CHx-x」改。EJ1N 寫 `EJ1N CH6-1`（台-CH）；DTM 寫 `DTM CH0-2`（內部站號-CH）；TC401 寫 `TC401 CH1-0`（台-通道）；HT9050 寫 `CH1-1`（站-CH）；KT4H、E5DC、DTK 寫 `KT4H #12`。
  - ⛔ 20261002（E-029，St01）：**逐通道代碼 5／6**——Index 區以外的通道 `HeaterInsOpt_`＝5 寫 `EJ1N CH台-CH`、＝6 寫 `DTM CH站-CH`（站＝內部站號，0＝DTME08 主機），台／站與 CH 來自「各溫控器不同」（`HeaterInsMode=1`）存的 `HeaterInsAddr_<通道>`／`HeaterInsCh_<通道>`；沒存（或「全機相同」）就寫 `EJ1N 站號未設定`／`DTM 站號未設定`（詞條 `Temp: Station not set`，不猜）。Index 區：USE_16_HEATER 是 EJ1N／DTME08 時照上面的 golden 公式，「不同」模式存了台／站＋CH 就用存的；COM 埠廠牌在「不同」模式存了站號也用存的（`KT4H #50`、`TC401 CH3-1`）。ctest `E029_HeaterPages`（node）[D] 驗這些字。
  - **多語系**：`588412a1`，Steven「你有做多國語言支援嗎?」。
    - 介面字經 `web\page\i18n.js` 的 `HTI18N.t` 翻譯，支援 en、zh、ja、ko，跟著 `HT_LANG` 換語言。
    - 新增的 35 個詞條放在 `terms`，key 一律是 `'Temp: '`＋英文原文（`2615ce75`；`terms` 是全站共用字典，「OK→正常」不能漏到別頁的 OK 鈕），字典編輯器重新產生檔案時會保留。
    - 通道名稱照 golden 英文不翻。
- 下一步：接溫度讀值（1002 15:1x，Steven「我們做好的東西, 直接通知ifor看怎麼串接」）。
  - **分工**：St01 只做到網頁與 tag 名稱。溫度值接 tag 歸 Ifor，接法已寄信通知（ifor@honprec.com，副本 Steven），todo E-027 的下一步。
  - **時機**：等 Ifor 的 I-03 讓讀值真的跑起來之後再接。
  - **改哪裡**：在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JsonBridge\StageThermo.cpp` 的 `PublishThermoTags`：
    - pv 讀 `UN150Read[]`
    - comm 讀 `UN150CommError[]`
    - inst 讀 `bUT150Install[]`
  - **同一顆 commit**：把 `kThermoFields` 對應欄位的 live 改成 true。
  - **網頁不用改。**
  - **前提**：加熱執行緒要先有人啟動（todo D-029），`DoThermo` 才會寫 `UN150Read`。

<!-- preserved-content:end -->
