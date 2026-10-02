# golden V912 逐通道溫控器：71 通道全表、讀存檔逐行、通訊框

> golden＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（BCB6，cp950；VS Code 要用「Reopen with Encoding」選 Big5）。只寫 `:行號` 的，是同一段前面那個檔。行號以 `git -C D:\HT9045 show HEAD:<路徑>`（HEAD `89ccb4cc`，20260927）為準。
> 下表由 golden `MachineTypeUtility.cpp:30-102`（`g_tHeaterInsInfo`）與 `:241-294`（`GetCtrlItemVisProp`）逐列產生，站號欄照 `cpublic.cpp` 的公式算（§3），不是設定值。

## 1. 71 通道全表

- 序號＝`eTempControll`（golden `MachineType.h:638-655`），也是 bthermo 的 `Addr`、`UN150Read[]`／`bUT150Install[]` 的索引。
- 「有下拉」＝`occupy`=true：HandlerSys「Heater」分頁建了下拉，存檔寫下拉的值。沒有下拉的 48 個存檔一律寫 -9999（`HandlerSys.cpp:782-788`）。
- 「下拉顯示條件」只決定下拉看不看得到；看不到的下拉存檔**照樣寫它的值**（`TypeIdxToShowIdx` 只看 occupy，`HandlerSys.cpp:76-86`）。
- Index 32 區＝序號 11～26（Aa1～Bd2）與 33～48（Ae1～Bh2），全部沒有下拉。

| 序號 | 通道 | 鍵 `[TempCtrl]` | 有下拉 | 下拉顯示條件（`GetCtrlItemVisProp`） | KT4H／DTK4848／E5DC 站號 | TC401 第幾台・通道 | golden 行 |
|---:|---|---|:-:|---|---:|---|---:|
| 0 | HotPlate1 | `HeaterInsOpt_HotPlate1` | 有 | 一律 | 1 | 1・0 | :31 |
| 1 | HotPlate2 | `HeaterInsOpt_HotPlate2` | 有 | 一律 | 2 | 1・1 | :32 |
| 2 | Shuttle1 | `HeaterInsOpt_Shuttle1` | 有 | 一律 | 3 | 1・2 | :33 |
| 3 | Shuttle2 | `HeaterInsOpt_Shuttle2` | 有 | 一律 | 4 | 1・3 | :34 |
| 4 | Head1 | `HeaterInsOpt_Head1` | 有 | `USE_16_HEATER`≠0 | 5 | 2・0 | :35 |
| 5 | Head2 | `HeaterInsOpt_Head2` | 有 | `USE_16_HEATER`≠0 | 6 | 2・1 | :36 |
| 6 | Head3 | `HeaterInsOpt_Head3` | 有 | `USE_16_HEATER`≠0 | 7 | 2・2 | :37 |
| 7 | Head4 | `HeaterInsOpt_Head4` | 有 | `USE_16_HEATER`≠0 | 8 | 2・3 | :38 |
| 8 | Socket | `HeaterInsOpt_Socket` | 有 | 一律 | 9 | 3・0 | :39 |
| 9 | Chamber | `HeaterInsOpt_Chamber` | 有 | 一律 | 10 | 3・1 | :40 |
| 10 | CCD | `HeaterInsOpt_CCD` | 有 | `RTC_TemperNumber`≥1 | 11 | 3・2 | :41 |
| 11 | Aa1 | `HeaterInsOpt_Aa1` |  | —（沒有下拉，存檔寫 -9999） | 12 | 3・3 | :42 |
| 12 | Ab1 | `HeaterInsOpt_Ab1` |  | —（沒有下拉，存檔寫 -9999） | 13 | 4・0 | :43 |
| 13 | Ac1 | `HeaterInsOpt_Ac1` |  | —（沒有下拉，存檔寫 -9999） | 14 | 4・1 | :44 |
| 14 | Ad1 | `HeaterInsOpt_Ad1` |  | —（沒有下拉，存檔寫 -9999） | 15 | 4・2 | :45 |
| 15 | Ba1 | `HeaterInsOpt_Ba1` |  | —（沒有下拉，存檔寫 -9999） | 16 | 4・3 | :46 |
| 16 | Bb1 | `HeaterInsOpt_Bb1` |  | —（沒有下拉，存檔寫 -9999） | 17 | 5・0 | :47 |
| 17 | Bc1 | `HeaterInsOpt_Bc1` |  | —（沒有下拉，存檔寫 -9999） | 18 | 5・1 | :48 |
| 18 | Bd1 | `HeaterInsOpt_Bd1` |  | —（沒有下拉，存檔寫 -9999） | 19 | 5・2 | :49 |
| 19 | Aa2 | `HeaterInsOpt_Aa2` |  | —（沒有下拉，存檔寫 -9999） | 20 | 5・3 | :50 |
| 20 | Ab2 | `HeaterInsOpt_Ab2` |  | —（沒有下拉，存檔寫 -9999） | 21 | 6・0 | :51 |
| 21 | Ac2 | `HeaterInsOpt_Ac2` |  | —（沒有下拉，存檔寫 -9999） | 22 | 6・1 | :52 |
| 22 | Ad2 | `HeaterInsOpt_Ad2` |  | —（沒有下拉，存檔寫 -9999） | 23 | 6・2 | :53 |
| 23 | Ba2 | `HeaterInsOpt_Ba2` |  | —（沒有下拉，存檔寫 -9999） | 24 | 6・3 | :54 |
| 24 | Bb2 | `HeaterInsOpt_Bb2` |  | —（沒有下拉，存檔寫 -9999） | 25 | 7・0 | :55 |
| 25 | Bc2 | `HeaterInsOpt_Bc2` |  | —（沒有下拉，存檔寫 -9999） | 26 | 7・1 | :56 |
| 26 | Bd2 | `HeaterInsOpt_Bd2` |  | —（沒有下拉，存檔寫 -9999） | 27 | 7・2 | :57 |
| 27 | HeatGun1 | `HeaterInsOpt_HeatGun1` | 有 | `INSTALL_HEAT_GUN`>0 | 28 | 7・3 | :58 |
| 28 | HeatGun2 | `HeaterInsOpt_HeatGun2` | 有 | `INSTALL_HEAT_GUN`>0 | 29 | 8・0 | :59 |
| 29 | DUT1 | `HeaterInsOpt_DUT1` | 有 | `iSocketBaseTempCount`＝eDut2ea／eDut4ea | 30 | 8・1 | :60 |
| 30 | DUT2 | `HeaterInsOpt_DUT2` | 有 | `iSocketBaseTempCount`＝eDut2ea／eDut4ea | 31 | 8・2 | :61 |
| 31 | DUT3 | `HeaterInsOpt_DUT3` | 有 | `iSocketBaseTempCount`＝eDut4ea | 32 | 8・3 | :62 |
| 32 | DUT4 | `HeaterInsOpt_DUT4` | 有 | `iSocketBaseTempCount`＝eDut4ea | 33 | 9・0 | :63 |
| 33 | Ae1 | `HeaterInsOpt_Ae1` |  | —（沒有下拉，存檔寫 -9999） | 34 | 9・1 | :64 |
| 34 | Af1 | `HeaterInsOpt_Af1` |  | —（沒有下拉，存檔寫 -9999） | 35 | 9・2 | :65 |
| 35 | Ag1 | `HeaterInsOpt_Ag1` |  | —（沒有下拉，存檔寫 -9999） | 36 | 9・3 | :66 |
| 36 | Ah1 | `HeaterInsOpt_Ah1` |  | —（沒有下拉，存檔寫 -9999） | 37 | 10・0 | :67 |
| 37 | Be1 | `HeaterInsOpt_Be1` |  | —（沒有下拉，存檔寫 -9999） | 38 | 10・1 | :68 |
| 38 | Bf1 | `HeaterInsOpt_Bf1` |  | —（沒有下拉，存檔寫 -9999） | 39 | 10・2 | :69 |
| 39 | Bg1 | `HeaterInsOpt_Bg1` |  | —（沒有下拉，存檔寫 -9999） | 40 | 10・3 | :70 |
| 40 | Bh1 | `HeaterInsOpt_Bh1` |  | —（沒有下拉，存檔寫 -9999） | 41 | 11・0 | :71 |
| 41 | Ae2 | `HeaterInsOpt_Ae2` |  | —（沒有下拉，存檔寫 -9999） | 42 | 11・1 | :72 |
| 42 | Af2 | `HeaterInsOpt_Af2` |  | —（沒有下拉，存檔寫 -9999） | 43 | 11・2 | :73 |
| 43 | Ag2 | `HeaterInsOpt_Ag2` |  | —（沒有下拉，存檔寫 -9999） | 44 | 11・3 | :74 |
| 44 | Ah2 | `HeaterInsOpt_Ah2` |  | —（沒有下拉，存檔寫 -9999） | 45 | 12・0 | :75 |
| 45 | Be2 | `HeaterInsOpt_Be2` |  | —（沒有下拉，存檔寫 -9999） | 46 | 12・1 | :76 |
| 46 | Bf2 | `HeaterInsOpt_Bf2` |  | —（沒有下拉，存檔寫 -9999） | 47 | 12・2 | :77 |
| 47 | Bg2 | `HeaterInsOpt_Bg2` |  | —（沒有下拉，存檔寫 -9999） | 48 | 12・3 | :78 |
| 48 | Bh2 | `HeaterInsOpt_Bh2` |  | —（沒有下拉，存檔寫 -9999） | 49 | 13・0 | :79 |
| 49 | 2D | `HeaterInsOpt_2D` | 有 | `CCD2_TEMPER`>0 | 50 | 13・1 | :80 |
| 50 | LB | `HeaterInsOpt_LB` | 有 | `LB_TEMP`>0 | 51 | 13・2 | :81 |
| 51 | IndexESD | `HeaterInsOpt_IndexESD` | 有 | `Index_ESDAir`>0 | 52 | 13・3 | :82 |
| 52 | CCD_2 | `HeaterInsOpt_CCD_2` | 有 | `RTC_TemperNumber`≥2 | 53 | 14・0 | :83 |
| 53 | ATCHotAir1 | `HeaterInsOpt_ATCHotAir1` | 有 | `INSTALL_ATC_HEAT_GUN`>0 | 54 | 14・1 | :84 |
| 54 | ATCHotAir2 | `HeaterInsOpt_ATCHotAir2` | 有 | `INSTALL_ATC_HEAT_GUN`>0 | 55 | 14・2 | :85 |
| 55 | OutSht1 | `HeaterInsOpt_OutSht1` |  | —（沒有下拉，存檔寫 -9999） | 56 | 14・3 | :86 |
| 56 | OutSht2 | `HeaterInsOpt_OutSht2` |  | —（沒有下拉，存檔寫 -9999） | 57 | 15・0 | :87 |
| 57 | Base1 | `HeaterInsOpt_Base1` |  | —（沒有下拉，存檔寫 -9999） | 58 | 15・1 | :88 |
| 58 | Base2 | `HeaterInsOpt_Base2` |  | —（沒有下拉，存檔寫 -9999） | 59 | 15・2 | :89 |
| 59 | Base3 | `HeaterInsOpt_Base3` |  | —（沒有下拉，存檔寫 -9999） | 60 | 15・3 | :90 |
| 60 | Base4 | `HeaterInsOpt_Base4` |  | —（沒有下拉，存檔寫 -9999） | 61 | 16・0 | :91 |
| 61 | Base5 | `HeaterInsOpt_Base5` |  | —（沒有下拉，存檔寫 -9999） | 62 | 16・1 | :92 |
| 62 | Base6 | `HeaterInsOpt_Base6` |  | —（沒有下拉，存檔寫 -9999） | 63 | 16・2 | :93 |
| 63 | HotPlate3 | `HeaterInsOpt_HotPlate3` |  | —（沒有下拉，存檔寫 -9999） | 64 | 16・3 | :94 |
| 64 | HotPlate4 | `HeaterInsOpt_HotPlate4` |  | —（沒有下拉，存檔寫 -9999） | 65 | 17・0 | :95 |
| 65 | Shuttle3 | `HeaterInsOpt_Shuttle3` |  | —（沒有下拉，存檔寫 -9999） | 66 | 17・1 | :96 |
| 66 | Shuttle4 | `HeaterInsOpt_Shuttle4` |  | —（沒有下拉，存檔寫 -9999） | 67 | 17・2 | :97 |
| 67 | Door1 | `HeaterInsOpt_Door1` |  | —（沒有下拉，存檔寫 -9999） | 68 | 17・3 | :98 |
| 68 | Door2 | `HeaterInsOpt_Door2` |  | —（沒有下拉，存檔寫 -9999） | 69 | 18・0 | :99 |
| 69 | LBUp | `HeaterInsOpt_LBUp` |  | —（沒有下拉，存檔寫 -9999） | 70 | 18・1 | :100 |
| 70 | LBDown | `HeaterInsOpt_LBDown` |  | —（沒有下拉，存檔寫 -9999） | 71 | 18・2 | :101 |

## 2. 讀、存、按鈕（golden `HandlerSys.cpp`）

| 動作 | 行 | 做什麼 | 寫檔 |
|---|---|---|---|
| 建構子 | `:70-112` | occupy 的 23 個通道各建 Label＋ComboBox，每欄 10 個（`COUNT_OF_ITEM_IN_COL`，`:30`）；`SetCtrlItemProp(GetCtrlItemVisProp(i), …)` 決定可見 | 否 |
| `HeaterInsOpt_Read` | `:50-60` | `HEATER_CTRL_TYPE`（缺鍵預設 KT4H）→ 逐鍵讀，預設 -9999；讀到 -9999 記憶體換成 `HEATER_CTRL_TYPE` | **缺鍵就寫**（`CheckAndReadIniDataGeneral`，golden `common.cpp:1444-1455`） |
| 開頁 `FormShow` → `LoaderSystemSet` | `:132-135`、`:262-278` | 呼叫上一列；有下拉的全同→Heater Type 顯示那個，否則顯示 `HEATER_CTRL_TYPE`；回填各下拉；回填期間擋 `rgHeaterTypeClick`（`s_bSuppressHeaterTypeEvent`，`:36`、`:266`、`:277`） | 同上（只寫缺的鍵） |
| Load 鈕 | `:1127-1130` | 再跑一次 `LoaderSystemSet` | 同上 |
| 存檔 `SaveSystemSet` | `:779-794` | 71 鍵：有下拉寫 ComboBox 的 `ItemIndex`、沒有寫 -9999，記憶體同值（`:787`）；`HEATER_CTRL_TYPE`＝有下拉全同的那個值，否則 Heater Type 單選的值 | 是 |
| 存檔鈕 | `:1121-1125` | `SaveSystemSet`＋"Please restart the program to active new parameters." | 是 |
| 離開鈕 `ExitBtnClick` | `:1179-1185` | `HSys.ReadGeneralIni()`（只讀 `HEATER_CTRL_TYPE`，golden `database.cpp:433`）＋`SaveSafeDoorSet` | 否 |
| 點 Heater Type `rgHeaterTypeClick` | `:1519-1540` | 有下拉的通道設成點的廠牌、沒有下拉的設 -9999（`:1531`）；各下拉改畫面 | **立刻寫** 71 鍵（`:1536`）＋`HEATER_CTRL_TYPE`（`:1538`） |
| 溫控迴圈第一次 `DoThermo` | golden `bthermo.cpp:1167-1175` | static 旗標，只呼叫一次 `HeaterInsOpt_Read` | 缺鍵就寫（等於開機後不久就寫） |

小工具（golden `MachineTypeUtility.cpp`）：`IsValEqual_HeaterInsOpt(通道, 廠牌)`（`:109-120`，廠牌不在 0～4 或通道越界回 false）、`IsNoHeaterMachine()`（`:124-127`，看全域）、`IsAllValEqual_HeaterInsOpt`（`:129-157`）、`GetFirstHeaterInsOpt`（`:159-179`）、`IsAllSame_HeaterInsOpt`（`:181-196`）、`GetStaTable_HeaterInsOpt`（`:198-226`）、`IsExistVal_HeaterInsOpt`（`:229-239`，第一次呼叫算完就快取，之後改表不會更新）。`THeaterInsInfo` 的建構預設是 KT4H（`MachineType.h:671`、`:690`）。

## 3. 通訊框（golden `cpublic.cpp`，全部經 `COM2->Comm2` 送到 `[TempCtrl] COM_PORT`）

| 廠牌 | 寫設定溫度 | 讀目前溫度 | 站號欄位 |
|---|---|---|---|
| KT4H | `UT100WordWriteNoSucm(Addr, 0x0001, 溫度×10)` `:179-190`：`:%02X%02X%04X%4s00` CR LF，功能碼 6，LRC 補在 `[13]`、`[14]` | `UT100WordReadNoSucm(Addr, 0x0080)` `:192-201`：功能碼 3 | `Addr+1`，`%02X` |
| DTK4848 | `DTK4848WordWriteNoSucm` `:205-216`：`:%02X06%04d%s%s\r\n`，暫存器 4701，送出長度多 1（含 NUL） | `DTK4848WordReadNoSucm` `:218-229`：暫存器 4700、2 個字 | `Addr+1`，`%02X`（註解：Steven 20210511 把 `%02d` 改成 `%02X`） |
| E5DC | `E5DCWriteTemp` `:480-488`：`%02d0000102C100030000010000%04X`，STX…ETX＋BCC | `E5DCReadTemp` `:463-478`：`%02d0000101C00000000002` | `Addr+1`，`%02d`（十進位，兩位） |
| TC401 | `TMC401WriteTemp(台, 通道, 溫度×10)` `:422-441`：`[台+1, 0x06, 0x00, 0xC8+通道, 高, 低, CRC 低, CRC 高]` | `TMC401ReadTemp(台, 通道)` `:443-461`：`[台+1, 0x03, 0x00, 通道, 0x00, 0x01, CRC…]` | 呼叫端傳 `(Addr/4, Addr%4)`（golden `bthermo.cpp:2522`、`:2556`） |

接收（golden `rs232.cpp:742-762`）依 `g_iHeaterTypeIdx_SendCmd` 那個通道的廠牌：TC401 抄 8 byte 到 `READBUFF`；E5DC 整串進 `Com2Buffer`；其他第一個字不是 `:` 就 return（不設 `Com2ReceiveOK`）。

通訊錯誤（golden `bthermo.cpp:2700-2717`，KT4H 那段）：重試用完就 `StopComm`、`CommunCTErr[Addr]++`，超過 5 次 `UN150CommError[Addr]=true`、`UN150Read[Addr]=999` ⇒ 畫面溫度 999。站號送錯（例如溫控器撥的站號跟「序號＋1」不同）就是這個結果。

## 4. 溫控迴圈的 Task（golden `bthermo.cpp` `DoThermoReal`）

| Task | 行 | 做什麼 |
|---|---|---|
| 1 | `:1274-1288` | `Addr=HotPlate1`、`OldTemp[]=-1`、重試計數重設 → 100 |
| 100 | `:1289-2599` | Tri_Temp／ATC／Index 區分支（`:1290-1377`）→ 算設定溫度 → 溫度變了就寫（`:2492-2538`）、沒裝就 300（`:2539-2543`）、否則讀（`:2544-2570`）→ 依廠牌選 200／250／255／2500（`:2583-2598`）；**都不符就停在 100** |
| 200 | `:2601` 起 | TC401 回覆處理 |
| 250 | `:2687` 起 | KT4H 回覆處理 |
| 255 | `:2804` 起 | E5DC 回覆處理 |
| 2500 | `:2974` 起 | DTK4848 回覆處理 |
| 300 | `:3209-3217` | `Addr++`（到 71 繞回 0）、`g_iHeaterTypeIdx_SendCmd=Addr`、→ 100 |
| 500 | `:3218` 起 | KT4H 送警報值；不是 KT4H 就 300（`:3219-3223`） |

`PauseUT150Polling` 為真時一進來就 `Task=1` return（`:1255-1259`）。
