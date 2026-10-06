# Index Z 扭力讀值、正負號與自動測高（國際牌 → 安川 Σ-X）

> 20261003 ST01-M 整理（Steven 1003 21:4x～22:3x 的問答與裁決 Q87）。改 Index 測高、扭力門檻、HT9050 的 1203 扭力讀值之前先讀這份。

## 1. golden 怎麼讀國際牌（Panasonic MINAS）的扭力

全在 golden 0618 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\rs232.cpp`（COM2 直接跟 Index 伺服驅動器講；Galil 只負責移動，不讀扭力）：

| 步驟 | 位置 | 內容 |
|---|---|---|
| 選哪一軸 | `:841-851` | `chkReadTorque1`＝Z1（位址 0）、`chkReadTorque2`＝Z2（位址 1） |
| 交握 | `:899-985` | ENQ → 驅動器回 EOT → 送請求 → 驅動器回 ACK＋ENQ → 送 EOT → 收資料 → 回 ACK |
| 請求封包 | `:833`、`:909-910` | `datatrq[4]={0x00, 位址, 0x52, 0xAE-位址}`：資料長度 0、軸位址、**命令 0x52（＝(模式<<4)|命令＝命令 2／模式 5「Read out of present torque output」，註解「要求讀取扭力值」；1004 更正：以前寫成命令 5／模式 2；單位＝額定扭力 2000，所以 k/20＝額定扭力 %（A5II p.427），詳見 `references/panasonic-rs232/`）**、檢查碼 |
| 回覆解碼 | `:1799-1813` | `k = str[4]<<8 \| str[3]`（16 位元有號）；**`if(k<0) k=0;`**（負值丟掉）；**`Torque = k/20.0`**；`%5.2f` 寫進 `fMain->edTorue0`（`:975`）與 `fContact->PnlTorue0` |
| 逾時／錯誤 | `:858-866`、`:887-895` | 收不到回覆 `Torque=-9999`、重開 COM；連續 10 次 → 「Rs232 Read Index Z1 Torque error!!」 |

- 0x52 在國際牌手冊上的正式名稱（扭力命令還是實際扭力、原始單位）程式看不出來；`÷20` 推算原始單位＝0.05 %（2000＝100 %）。**ANSWERED（1004）：** Q1 answered by the manual: 0x52 = command 2 / mode 5, rated torque = 2000, k/20 = % (A5II p.427, A4 p.294); no machine check needed (ST01-M, TO_ES02 main 4a4bf63c). The E-10 measurements (M14 holding 6077h, 2704h, saturation, following error, fIndexDownPos vs socket top) still stand.
- `rs232.cpp:4415-4440` 是鴻勁自製通訊卡（HP card）讀「目前設定的扭力」，不是即時扭力，別搞混。
- 測高本身在 golden 0618 `cContact.cpp`：`DoTestContactFunction` case 400／800 → `Do_Z1_AutoGetHeight`／`Do_Z2_AutoGetHeight`；case 536 用 COM2 寫扭力上限（預設 40）；case 550／555 等 `edTorue0`（**沒有逾時**，`:5766-5771`）；停在 `TorqueData>=門檻` 或高度 `<= fIndexDownPos` 連 10 次（`:5789-5790`）；國際牌另外取絕對值（`:5786-5787`，因為解碼已把負值歸 0，實際上是空操作）；每步一個 Galil 絕對移動（`:6059`／`:6070`）。

## 2. 安川 Σ-X（HT9050）對應的欄位

HT9050 Index Z＝M14 `MTestZ1`，驅動器安川 **SGDXS-200AA0A0002**（EtherCAT，3.0 kW 一般規格），經 PCI-1203。依安川「Σ-XS SERVOPACK with EtherCAT Communications References Product Manual」**SIEP C710812 02H**（2025/10）：

| 項目 | 安川 | 手冊位置 |
|---|---|---|
| 對應國際牌讀值的欄位 | **6077h Torque actual value**（INT16、唯讀、預設在 TxPDO 1A00h）——伺服驅動器上＝扭力命令值（送進電流迴路的值），不是量到的；量到的電流是 6078h（千分之一額定電流） | §15.13.5 p.686、§15.13.6 p.687 |
| 單位 | Trq. unit＝**2704h:1／2704h:2 %**，預設 1／10 ⇒ **1 count＝0.1 % 額定扭力**；2704h 存 EEPROM，可能被改過 | §13.1.6 p.588、§5.14.5 p.220-221 |
| 正負號 | 跟驅動器的參考座標走（正轉＝正）；Pn000.0 只改實體轉向；沒有 607Eh 極性物件 | §5.4 p.186 |
| 扭力上限 | 6072h（0.1 %）、60E0h／60E1h（0.1 %，預設 8000）、Pn402／Pn403（1 %，預設 800）、Pn404／Pn405（1 %，預設 100，6040h bit11／bit12 開）——**取最小的生效** | §14.7 p.620、§6.7 p.260-263 |
| 「正在限扭力」 | 6041h bit14 Torque Limit Active（是否＝正在限要上機確認）；硬體輸出 /CLT（Pn50F／Pn5B4 指定腳位） | §15.6.3 p.667-668、§6.7.3 p.264 |

port 現況（main，NB2-1 R197）：扭力**上限**已接好（`rs232.cpp:950` 判 `MOT[MTestZ1].CardType=="PCI1203"` → `W906_Ht9050TorqueLimit`，送門檻×10 寫 60E0h／60E1h 並讀回）；扭力**讀值**沒有來源（1203 監看器有讀 6077h 但只送網頁，不進 `edTorue0`）；測高函式只有宣告（`forms/fContact.h:1348-1349`／`:1507`），呼叫在 `csystem.cpp:31320-31322` 的 `#if 0`。

## 3. 換算規則（Steven 1003 Q87 裁決）

Steven 原話：「把安川讀回來的值，轉換成跟原本國際牌馬達一樣的比例」「正負號是要比較的」「向下的地心引力是負的，向上的反作用力是正的」「機台端會根據motor_table的設定，去決定目前馬達是向下移動（負的）或是向上移動（正的）」。

1. **比例**：國際牌 `k/20`＝％ ⇔ 安川 `6077h × 2704h:1 ÷ 2704h:2`＝％（預設 ÷10）。golden 的門檻數字照用。
2. **正負號要比**：方向以 M14 的 Mot_Table 設定為準（往下＝負、往上＝正）。6077h 是馬達自己出的力：往下壓時馬達扭力朝下（負），socket 的反作用力朝上（正），兩者等大反向。換成「下壓方向為正」後，**負值當 0**（＝golden `k<0 → 0`），不取絕對值——方向錯了就不會提早觸發。
3. **馬達出力 vs 壓力**：扭力（N·m）經螺桿變直線力（F＝2π·η·T／導程）；6077h 是命令值，穩態≈實際扭力、加減速時有差；垂直 Z 空降時馬達一直出力撐重量，所以真正壓在 socket 上的＝扣掉重量與摩擦後的部分。golden 國際牌路徑沒有扣基準值——port 若要扣「空降穩定值」，算 port 新增，要標明並請 Steven／EastSun 確認。
4. **port 專用防護**（不是照翻，要註明）：扭力來源未確認前拒絕執行＋警報；case 555 等不到 → `ST`＋警報；高度下限 `fIndexDownPos` 照舊是最後一道；可選下壓時把下壓方向的 Pn404／Pn405 設成門檻＋餘量。
5. **機台上解開前**：EastSun 照安川手冊／上機確認 M14 的 2704h 實際值、Index Z 往下時 6077h 的正負（TO_ES02 E-10，main `3b63f43c`／`e3403015`）；golden 的「40」是額定扭力的 40 % 還是公斤也一起確認。

## 4. 進度

- 裁決：`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`「20261003 22:1x Steven 裁決：Q87」。
- 實作：todo **E-038**（St01／ST01-E，安全相關，MR 標題要有 safety），側分支 `v906/st01-e038`；認領筆電檔（fContact／csystem／rs232／1203 監看器）先貼 FROM_STEVEN §1。
- 來源：NB2-1 R197（`origin/v906/nb2-assist` `docs/nb2_assist/README.md:158` 起）；HT9050 馬達表 `ht9050-hw/references/motors-9050.md`。

## 5. 機台端的歸零／伺服 ON／放煞車規則（Steven 1003 22:5x，Q88／Q89；跟 golden 分開）

Steven 原話：「斷電後就必須歸零」「Alarm如果可以clear, 基本上也是需要歸零」「Alarm如果不能clear,就必須斷電重啟後歸零」「機台只要不斷電，基本上不需要歸零」「這個動作跟golden是分開的無誤」「一直沒斷電：已經歸零過的軸不用再歸零。但是要把encoder pulse回寫給command pulse」「比照golden的 servo on功能」「Q89 應該要先servo on後才能放煞車」。

| 狀況 | 要怎麼做 |
|---|---|
| HOME 途中馬達斷電（含急停、開門） | 停止 HOME、所有軸 HomeFlag 清掉＝要重新歸零；電回來**不可**自動重置警報＋激磁＋繼續（機台 cpp 0170 M-a 要改） |
| 警報清得掉 | 清一次；清掉的軸要重新歸零（不是重試 20 次後當沒事繼續，cpp 0168 M-b 要改） |
| 警報清不掉 | 停止 HOME，提示「請斷電重開後再歸零」 |
| 一直沒斷電 | 已歸零的軸不用再歸零；**伺服 ON 時比照 golden `ServoOnOff(true)` 把 encoder 回寫成 command**（不然驅動器朝舊命令位置走、整段彈回＝NB2 !151 M2） |
| 放煞車 | **先伺服 ON（本輪 SVON 完成）→ 才放煞車**；條件＝馬達電源已上＋本輪伺服 ON＋延遲數完；開機不可因上一輪留下的 SVON 就放 8 支 Z 軸煞車（NIGHT_REPORT §0 #88，NB2-1 R200） |

已寫進 TO_ES02（main `e2931f5a`、`10ab026f`）給 EastSun／MC01 改機台 patch；裁決在 decisions-decided.md「20261003 22:5x Steven 裁決：Q88／Q89」。驅動器回原點（cpp 0165）建議另外檢查 DS402 原點到位旗標（St01 V-6 M-c）。

## 6. Phase A 實作（E-038，St01／ST01-E，20261003 23:xx；分支 `v906/st01e-e038`，不併 main／review6）

> 範圍與理由：`D:\AI_TempFile\st01e-e038-scope-20261003.md`。ST01-M 暫行裁決（等 Steven 隔天回覆）：基準值選項存在但預設關（照 golden）；不加 Pn404／Pn405；Phase B（翻 `Do_Z1_AutoGetHeight` 等）不接；Q1／Q3／Q4 經 TO_ES02 問 EastSun；CONFIRMED 旗標預設 0、唯讀、程式永不回寫。

### 6.1 資料路徑（只在出貨組態、`MOT[MTestZ1].CardType=="PCI1203"`）

```
MainProc → COM2->ReadTorque()（csystem.cpp:30399）
  → TCOM2Shim::ReadTorque（rs232.cpp:356；:358 同一行的 1203 分支，在行尾 // 之前）
    → W906_Ht9050TorqueRead()（IndexZTorque1203.cpp，ht9045_sm）
      → W906_Pci1203TorqueReadHook（EtherCAT/Pci1203TorqueRead.cpp，wb_serve；跟扭力上限掛鉤同一行安裝：Pci1203TorqueHook.cpp:79）
         → MotorAccessLiveBackend().Resolve("MTestZ1") → Pci1203Monitor()->SetTorqueFocusAxis(槽) ＋ axis(槽)／card().pollCount
      → ht9045::idxz::BridgeStep（IndexZTorqueCore.h，純函式）
    → fMain->edTorue0、fContactForm->PnlTorue0、Torque[0]、COM2->asReceiveTorue；chkReadTorque1／2 清掉；autoTask=999
```

- 回傳碼：-1＝模擬組態或不是 1203 列 → 照 golden 走 RS-232（行為不變）；0＝已武裝、這一拍沒有值；1＝寫了一個值（＝golden case 40，0618 rs232.cpp:991-995）；2＝沒武裝（autoTask=1，＝golden :854-858）。
- **一次武裝只給一個值**，而且只用「武裝之後的下一次監看器輪詢」的樣本（武裝那一輪的值不用）。
- 只在 Z1 已武裝、而且旗標／驅動器型態／方向／掛鉤都通過時才叫掛鉤 ⇒ 未確認的機台**完全沒有** 6077h 信箱流量。
- 只收 **SDO 6077h**（`torqueSrc==2`，這一輪的即時值）；PDO 值即使「已驗證」也不收（可能凍結）。另外要 2704h 讀到、伺服 ON、無警報、扭力焦點在 M14（Motor Test 頁選別軸時拿不到值 → 等 → 逾時）。
- 讀不到值：10 秒跳一次 golden 樣式的「1203 Read Index Z1 Torque error!! ＜原因＞」（golden 0618 rs232.cpp:861-871 是 RS-232 失敗超過 10 次才跳）。

### 6.2 換算與正負號

- `v = (raw − B) × 2704h:1 ÷ 2704h:2 × pressSign`；`pressSign = −1`（Mot_Table M14 `Direction=0` ⇒ 卡片座標＝流程座標，往下＝負）；Direction=1 ⇒ 不給值、拒絕（路由本來就拒絕那一列）。
- `v<0 → 0`（＝golden `k<0 → 0`，寫成 `v>0 ? v : 0`，所以 −0.0 也印成 ` 0.00`），`"%5.2f"`（＝golden :1812）。原始值**從不取絕對值**。
- B＝0（golden，沒有扣重力基準；觸發時實際壓力＝kg＋重力份量）。`[IndexDriver] HT9050_INDEXZ_TORQUE_BASELINE=1` 才扣基準：B＝`W906_IndexZTorqueBaselineReset()`（Phase B 的 case 536 呼叫點）之後在待命高度 5 個不同輪詢的中位數，收集時不給值（不會依這 5 筆往下走）。

### 6.3 旗標（只讀、不回寫）

- `D:\HT9045\system\Gerneral.ini` `[IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED`：缺鍵＝0＝不給值。EastSun 做完 E-10 量測後**手動**設 1。
- 讀法：`INIFileGeneral->ValueExists` ＋ `ReadInteger`（write-through TIniFile 每次重讀檔、不寫檔）；**不可**改成 `CheckAndReadIniDataGeneral`（common.cpp:1643-1655，缺鍵會把預設值寫回共用的 Gerneral.ini）或任何會回寫的載入器。ctest [T12] 在暫存目錄建 Gerneral.ini，缺鍵／=1／=0 三種讀完 SHA-256 都不變；換成回寫路徑會變紅（已實測，見 6.6）。
- 第一次讀到（`INIFileGeneral` 已開）就快取，主控台印一行 `[torque-1203] Gerneral.ini [IndexDriver] ...`；改了要重開 wb_serve。

### 6.4 給 Phase B 的呼叫點（還沒有人呼叫；都標 NOT GOLDEN）

| 函式 | 放哪裡 | 做什麼 |
|---|---|---|
| `W906_IndexZHeightGate(iContactMode, arm)` | `Do_Z1/Z2_AutoGetHeight`、`Do_LoadCellAutoHigh` 的 case 1 | 模式 1／2／8、Z1 是 1203 時：未確認、非國際牌、Direction≠0、沒掛鉤、Z2 → 拒絕＋看得到的訊息；RS-232 機台一律放行 |
| `W906_IndexZTorqueWaitTimedOut(arm, edTorue0=="")` | case 555、7170、Load Cell 與 Do_Z2 的等待處 | 等 5 秒（`kTorqueWaitMs`）沒值回 true 一次 → 呼叫端送 `ST`＋訊息（可帶 `W906_IndexZTorqueLastWhy()`）＋照 case 536 失敗的出口離開 |
| `W906_IndexZTorqueBaselineReset()` | case 536 成功後 | 只在基準值選項開時有作用 |

`csystem.cpp:31320-31322` 的 `#if 0` 要等 E-10 量完、旗標設 1、Jimmy 決定（NIGHT_REPORT §0 #86）才解。

### 6.5 ⚠ 上機要看（human-review A）

- **旗標設 1 之後，同一個值也會進 golden 的其他讀者**：量產的 `DoTestHeadMotor` 12110（atester.cpp:6907-6935，現在就是活的）與 `DoZ1PickFromShuttle` 560（還沒翻）。這是 golden「一個讀取器餵所有讀者」的行為，但在 HT9050 是新的，要 EastSun 看量產啟動那一段。
- 行為改變（B）：HT9050 不再跑 RS-232 讀扭力（不會再對關著的埠逾時）；錯誤訊息從「Rs232 Read Index Z1 Torque error」變成「1203 Read Index Z1 Torque error!! ＜原因＞」。
- 旗標設 1 之前要量（E-10）：往下時 6077h 的正負、M14 的 2704h、靜止與慢速下降時的撐重扭力 g 是否小於 kg（HT9050 設定 15 %；60E0h／60E1h 兩個方向都限，g≥kg 會讓 Z 下沉）、卡死時 6077h 是否停在上限、追隨誤差。

### 6.6 測試（ctest `IndexZTorque1203`，`tests/test_indexz_torque_1203.cpp`，兩個組態）

[T1] 2704h 換算　[T2] 正負號（反方向永不觸發、對的方向在 kg 觸發，raw −2000…2000 掃描）　[T3] 重力，golden 預設　[T4] 基準值選項　[T5] `"%5.2f"`／atoi　[T6] 一次武裝一個值、只用之後的輪詢　[T7] 有效性＋10 秒讀取錯誤　[T8] 5 秒等待逾時　[T9] 進入點拒絕　[T10] 出貨組態：1203 路徑 vs 國際牌 RS-232 路徑　[T11] 接線原始碼釘子（rs232.cpp:358 的呼叫在行尾 // 之前、讀取掛鉤跟上限掛鉤同一行、旗標不走回寫路徑）　[T12] Gerneral.ini 唯讀（SHA-256 不變）。
「壞了會紅」實測（20261004 04:11-04:18，SIM 建置；故意改壞→紅→還原→綠）：正負號改 +1 → [T1]／[T2]／[T3]／[T4] 共 9 條紅；對原始值取絕對值 → [T2] 2 條＋[T3] 1 條紅；旗標改用 `CheckAndReadIniDataGeneral` → [T12]「缺鍵」SHA-256 變了＋[T11] 紅；還原後全綠。結果：出貨組態 60 條、模擬組態 52 條全過；全量 ctest 模擬 18 紅／出貨 3 紅＝基準，沒有新的紅。

### 6.7 還開著的問題

- ~~Q1 國際牌 0x52 的單位（k/20＝％？）——要國際牌手冊。~~ **ANSWERED：** Q1 answered by the manual: 0x52 = command 2 / mode 5, rated torque = 2000, k/20 = % (A5II p.427, A4 p.294); no machine check needed (ST01-M, TO_ES02 main 4a4bf63c). The E-10 measurements (M14 holding 6077h, 2704h, saturation, following error, fIndexDownPos vs socket top) still stand.
- Q2 重力基準要不要開——Steven／EastSun（量完 E-10 #5 再定）。
- Q3 1203（Advantech DS402）有沒有在哪裡反轉命令極性——EastSun 上機。
- Q4 HT9050 Z 的撐重扭力是否小於 kg（15 %）——EastSun 上機。
- Q5 Phase B（翻 Auto Height 本體，約 8,200 行 golden）誰做——筆電 CT-3c 清單上，Steven 決定。

## 7. Phase B 第一段 B1（E-042，St01／ST01-E，20261004；分支 `v906/st01-e042`，從 Phase A `e6cec741` 開，不併 main／review6）

> 計畫：`D:\AI_TempFile\st01e-e042-plan-20261004.md`（§2 認領、§3 防護、§6 測試、§7 步驟、§11 手動測高兩種做法、§12 重啟紀錄）。認領：FROM_STEVEN §1 `1c50ce7f`（筆電 OK）。**B1 沒有任何執行期改變**：翻好的本體沒有呼叫者，`csystem.cpp:31320-31322` 照舊 `#if 0`，ctest 釘住。10/05 18:00 前不進任何機台包。

### 7.1 做了什麼

- **新檔 `forms/fContact_AutoHeight.cpp`**：golden 0618 `cContact.cpp:5384-8168`（5 個檔案層計時器＋`TfContact::Do_Z1_AutoGetHeight`）**逐行照搬、行號固定偏移**（port 行＝golden 行 −5284，[B10] 釘 108 個 `case N:`）；由 `D:\AI_TempFile\st01e-e042-run\gen_autoheight.py` 從 golden 產生（9 處同一行修改、31 個敘述 GATE，重跑位元組相同）。檔尾：`GetAutoHeightMaxKGTorque`（委派計算核心 `ComputeAutoHeightMaxKGTorque`）、`TestZ_CompensationHight`（委派 `ComputeTestZCompensationHight`，寫 golden 全域 `iTotalOffset_1/2`）。
- **新檔 `forms/fContact_AutoHeight.h`**：只有 extern（golden :93-98 的 `iIndexStatus` 等），給筆電 CT-3c 的 Index jog 讀。
- **GATE 掉的（沒翻、或沒有實體）**：ATC 通知（`ATC_InterfaceForm->UseTSD_Function／HandlerArm`，port 只有 shim；HT9050 沒 ATC）、`ATC_SwitchTjSignal`（S-26，B3 決定）、`fTestCategory／fDynamicTemp->BringToFront`（只是視窗前後）、`DoCalibrateAboveHeightZ1`（S-43，B3 要翻；**在那之前非 1203 機台等於「沒勾 Calibrate Above」**，B6 解閘前一定要補）。
- **共用檔（都是同一行、行數不變）**：`cContact.h:160` 行尾 `//` 之前加 `const int CONTACT_TEST = 3;`（golden :77）；`Command.cpp:317` 自己那份改成同一行註解；`CMakeLists.txt:2910` 加 `forms/fContact_AutoHeight.cpp`；`forms/fContact.h:1348`、`:1508` 註解改 ACTIVE；`tests/CMakeLists.txt` 檔尾一段。`BarCode/BarCode_Shuttle2_CCDScan.h:187-189` 也有一份，但沒有 TU 同時看到兩份（兩組態全編過），**不動**。⚠ `forms/fContact.h:1556`（`TestZ_CompensationHight`）不在認領行，註解還寫 GATE (S-51)，本體其實已在新 TU——告知筆電。

### 7.2 防護（全部 `AI(W906-E042) NOT GOLDEN`；只在出貨組態且 `MOT[MTestZ1].CardType=="PCI1203"`；國際牌／RS-232 機台與模擬組態一行都不變；**停止（`ST`）永遠不被擋**）

| 編號 | 放哪裡（golden 行） | 做什麼 |
|---|---|---|
| P7 | `switch(Task)` 那一行（:5423），**每一拍第一句** | M14 驅動器 ALM、ERROR_STOP、伺服 OFF、監看器樣本無效連 3 輪、輪詢計數凍結 >2 秒（單拍間隔 >1 秒不算，golden `MySleep` 會擋住 tick）、路由鎖住的失敗、沒有健康掛鉤 → **同一拍**送 `ST`＋訊息（每次跳一次）＋golden case 536 的出口（`fAllMotorHome=false; CarlibrationTask=1; return false;`）。Steven 1004 07:5x「io 馬達的裝置有異常error的時候，機台就不可以動」 |
| P4＋P8 | case 1（:5425），在任何輸出之前 | CONFIRMED≠1、BASELINE=1（Q90）、非國際牌、Direction=1、沒掛鉤、驅動器異常（這一項在 Do_Z1 裡輪不到：P7 在 switch 那一行先跑、走中途出口；它是給 B4 的 `DoTestContactFunction` case 1 用的）、**模式不是 1（Auto Height）或 3（Contact Test）**、不是單 shuttle（`iShuttleMode=1, iShuttle_Sel=0`）、Arm2／32-site／ROI／Calibrate Above／latch teach／ASE teach 選項 → 訊息＋`SystemStart=false; CarlibrationTask=1;`，什麼都沒動。Steven 1004 08:4x：白名單＝模式 1＋3、先單臂；Load Cell 拒絕；手動測高（2）在 HT9050 繼續拒絕 |
| P5 | case 555 等扭力（:5767）、case 7170（:6410），在 golden 那一行的前面 | 5 秒沒有扭力值 → `ST`＋訊息＋536 出口（golden 會永遠等）。Phase A 的 `W906_IndexZTorqueWaitTimedOut` 也改成只在 1203 列有作用 |
| P9 | case 560 國際牌下降那一行（:6069） | 下一步的**命令位置**已經在 `fIndexDownPos` 或以下 → `ST`＋訊息＋536 出口（golden 只看 encoder，Z 卡住時命令會一直往下走）；golden 的 encoder 下限（P6）照舊 |
| P3 | 不接 | 重力基準值程式碼保留、預設關、**沒有呼叫點**（[B9] 釘住）；設 `HT9050_INDEXZ_TORQUE_BASELINE=1` 會被 P4 拒絕並引用 Q90 |

健康來源：`EtherCAT/Pci1203TorqueRead.cpp` 的 `HealthThunk`（跟讀值掛鉤同一行安裝，**不搶** 6077h 焦點、不讀 SDO，只看最後一輪的 `motionIO／state／valid／pollCount` 與 `Pci1203GaliRouteFaultFor`）。純判斷在 `IndexZTorqueCore.h`（`HealthFault`、`RunAllowed`、`CommandAtOrBelowFloor`）。

### 7.3 手動測高（模式 2）為什麼在 HT9050 拒絕

golden 兩種：A＝`bD10ManualHeightComptibleWithNS=1`（HT9050 就是這個）面板鍵 jog，送 Galil `JG`，1203 路由不認、鎖失敗（`Pci1203GaliRouteCore.cpp:808`），P7 會停；B＝`bD10=0` 按住鍵時 `MOY` 伺服 OFF 用手推，路由會真的關伺服，但 P7 會跳、而且 M14 是垂直軸（有煞車就推不動、沒煞車會掉）。細節與日後選項：計畫 §11。

### 7.4 測試（ctest `IndexZAutoHeight1203`，`tests/test_indexz_autoheight_1203.cpp`）

直接呼叫 `Do_Z1_AutoGetHeight`（私有成員用 explicit-instantiation 存取規則，不動標頭），出貨組態接**真的 `TGaliRouteCore`＋假 1203（有「表面」）**：[B3] 下降到門檻（限值 150、每步 300 counts、10 次讀值觸發、觸發後不再下一步）、[B4] P5、[B5] P7 七種故障中途發生 → 同一拍 `ST`、之後不再動；Do_Z1 一進 case 1 就 ALM＝**中途形狀**（golden 是在 `DoZ1PickFromShuttle` 動過 Z1 之後才進 Do_Z1 case 1，所以送 `ST`＋536 出口，不送任何移動）；真正的入口判斷 `W906_IndexZRunRefused`（B4 放在 `DoTestContactFunction` case 1）遇 ALM 拒絕、**連停止都不送**、[B6] P9、[B2] 入口白名單（模式 1、3 會跑 golden case 1；2／4／8、兩個 shuttle、Arm2、CONFIRMED 缺／0、BASELINE=1 都拒絕且不送任何指令）、[B8] 停止永不被擋、[B1] RS-232 列 60 秒等不到值也不逾時。兩組態：[B7c] 純判斷、[B9] 普查（先剝掉註解、字串／字元常值內容、raw string、`#if 0` 區塊，再找 11 個函式名：tests 以外 0 個活的呼叫者、csystem 那行仍在 `#if 0`、P 呼叫點都在 golden 行的行尾 `//` 之前、基準值 0 呼叫點、新 TU 沒有回寫 ini 路徑）、[B10] 偏移、[B12] 模擬組態走 golden 的 `edTorue0=30`。
**結果（20261004 restart 2，St01）：**
- 增量建置兩組態 BUILD=0（20:16）。指定集 19 個：出貨 19/19；模擬 18/19，`Rs232Torque` 的 [F]（20 ms ACK／ENQ 時序）在負載下紅，單獨重跑 2/2 綠。新測試 `IndexZAutoHeight1203`：出貨 74/74、模擬 21/21。
- 全量 ctest（21:17-22:37，兩組態同時跑，各 356 個）：模擬 20 紅＝基準 18＋`IniFiles`、`common`；出貨 4 紅＝基準 3＋`Jam_Rules`。這三個單獨重跑全綠（兩組態同時跑、共用 `%TEMP%` 的固定檔名）——**沒有新的紅**。
- 壞了會紅（出貨，`D:\AI_TempFile\st01e-e042-run\red_proof.py`，紀錄 `red_proof.log`）：P7 每拍那一句拿掉 → 13 條紅；P7 晚一拍 → 7；P8 放寬到模式 2 → 3；P8 收窄（拿掉模式 3）→ 3；P9 拿掉 → 6；P5 漏到 RS-232（兩種改法）→ 各 1（[B1]）；普查：在 scratch 複本把 `csystem.cpp` 的 `#if 0` 打開 → [B9] 普查（指到 `csystem.cpp:31321`）＋csystem 那根釘子 2 條紅，未改的複本（對照組）全綠；還原後 74/74。真的 `csystem.cpp` 從頭到尾沒有寫過。
- restart 2 修掉的三個紅（細節：計畫 §13）：[B9] 普查誤判——`csystem.cpp:31327` 是 `#ifdef DEBUG_TRY_CATCH` 裡的 log 字串、不是呼叫，普查改成先剝掉註解、字串／字元常值、raw string、`#if 0`；[B5]「入口 ALM」是**測試期待錯**——golden 是在 `DoZ1PickFromShuttle` 動過 Z1 之後才進 Do_Z1 case 1，所以那裡是中途，P7 送 `ST`＋536 出口才對，程式不改、檢查改得更嚴（另外釘住 B4 要用的入口判斷：拒絕、連停止都不送）；[B12] 模擬組態是**測試前提沒建**——golden 的無卡 `Gali_MotMove` 用 `TMyMotor::speed` 走步，模擬組態只有「軸停用」時 `SetMotorScaleSpeed` 才設它，測試把 M14 啟用了，`speed`=0，case 530 永遠到不了；測試給 golden 自己的最小模擬步 100。

### 7.5 B2 以後

B2 `SetContactMode`（**已做，見 §8**）；B3（**已做，見 §9**）`forms/fContact_IndexPickPlace.cpp`（`DoZ1/Z2PickFromShuttle`、`DoZPlaceToShuttle`、`DoArm1/2PlaceToShuttle` 含 V912 的 D1 修正、`CheckIndexArmStatus`）＋`Do_Z2_AutoGetHeight`＋`DoCalibrateAboveHeightZ1`＋`ATC_SwitchTjSignal` 決定；B4 `forms/fContact_ContactSM.cpp`（`DoTestContactFunction`，case 1 放 P4／P8；普查改成釘 `DoTestContactFunction` 的呼叫者）＋`Do_LoadCellAutoHigh`；B5 兩組態 gate＋MR；B6 解閘要 10/05 18:00 後＋E-10＋CONFIRMED=1＋Jimmy #86＋S-26 R4。12110 逾時是另一張卡 E-044；B7（V910 DoTestHead）10/05 18:00 後。

## 8. Phase B 第二段 B2（E-042，St01，20261004 23:xx；分支 `v906/st01-e042`，接在 B1 `291cb0a5` 之後，不併 main／review6）

> **B2 沒有任何執行期改變**：`SetContactMode` 沒有呼叫者（連結器連 wb_serve 都不會拉進這個 .o），`csystem.cpp:31320-31322` 照舊 `#if 0`。10/05 18:00 前不進任何機台包。認領：FROM_STEVEN §1 `1c50ce7f`（只用到 A 列 `CMakeLists.txt:2910` 同一行＋St01 自己的檔）。

### 8.1 做了什麼

- **新檔 `forms/fContact_ContactSM.cpp`**：golden 0618 `cContact.cpp:15341-15434` `TfContact::SetContactMode` **逐行照搬、一行都沒改**（port 行＝golden 行 −15293，檔頭寫著、測試讀它；產生器 `D:\AI_TempFile\st01e-e042-run\gen_contactsm.py`，重跑位元組相同）。B4 會把 `DoTestContactFunction`（golden :11755-13963）放進同一個 TU，屆時更新檔頭的偏移即可。
- `CMakeLists.txt:2910`（`forms/fContact.cpp` 那一行，認領 A 列）同一行加 `forms/fContact_ContactSM.cpp`，行數不變。
- **沒動**：`forms/fContact.h:1464`（`SetContactMode` 宣告，註解還寫 `GATE (X-10)`；不在 ST01-M 給的認領行內 → 告知筆電，同 B1 的 :1556）、`cContact.h`／`Command.cpp`（`CONTACT_TEST` 在 B1 已解）、`csystem.cpp`、`FileRW/*`。
- **V912 比較**（:15566-15696）：只多兩種**功能**——ASE_CL Contact 高度反灰 ×4（E-030／RULINGS_20261002 #23-6 已裁「照 906 不做」）、`VISUAL_DETECTION_TEST`（模式 12，Q-C 未決，`cContact.h` 沒有這個常數）。不是修正 ⇒ 照 0618，#20 不適用。

### 8.2 為什麼算安全項

- 它是**模式切換**（`forms/fContact.h:441-446` 的風險註記）：寫全域 `iContactMode`（`cmydef.cpp:3346`），MainProc 活讀（`csystem.cpp:31302`：Contact 頁開著且 `iContactMode!=CONTACT_NORMAL` → `DoServoOn()`），所有接觸／測高狀態機都依它分岔。本體沒有馬達、IO、寫檔、訊息、`SystemStart`／`fAllMotorHome`。
- golden 的呼叫點：`FormShow` :1163、`FormClose` :1872、`rbModeNormalClick` :15338（port 都還 GATE (X-08)／(X-09)／(X-11)，沒定義）、`DoTestContactFunction` case 1 :11790（B4）。所以今天**沒有呼叫者**；原本「宣告不定義＝連結器當互鎖」那道 X-10 閘，改由 ctest [B13] 普查釘住（tests 以外 0 個活的 `SetContactMode(` 呼叫者，整個識別字比對，C 路的 `DF_SetContactMode` 不算）。

### 8.3 HT9050 的拒絕放哪裡（Steven 1004 08:4x：模式 1＋3、先單臂；手動測高 2 繼續拒絕；Load Cell 8 拒絕）

- **不放在 `SetContactMode` 裡**：golden 讓操作員選任何模式，這份也一樣（網頁今天也能選）；在 UI 切換裡擋會改掉 golden 的畫面行為。
- 擋在 P8＝`W906_IndexZRunRefused`（`IndexZTorque1203.cpp`），B4 放在 `DoTestContactFunction` case 1、`SetContactMode()`（:11790）與 golden 的 `CONTACT_NORMAL` 拒絕（:11813-11818）之後、任何輸出（`SetAllMotorSpeed`／`SwTesterAirCooling`／`ADAM_WriteMaxData`，:11832-11843）之前。[B13] 在 `SetContactMode` 的輸出上釘住這條鏈：HT9050 只有 `rbAutoHeight`／`rbContactTest` 能開始，`rbManualHeight` 拒絕（「Manual Height stays refused」）、`rbLoadCellAutoHigh` 拒絕（「Load Cell」），拒絕時什麼都不送（連 `ST` 都不送）；國際牌／RS-232 機台同樣的模式值、一個都不拒絕。
- P7（驅動器異常就不能動）在入口照查：M14 ALM 時模式照切（UI），但入口連模式 1 都拒絕（「drive error」）。

### 8.4 ⚠ 給 B4

- 成員 `SetContactMode` 讀的是 **facade 的單選鈕**（`forms/fContact.h:1089-1229`），**網頁不驅動它們**。網頁驅動的是 C 路那一份：`FileRW/DeviceForm_File.cpp` `g_ct3aRb` → `DF_rbModeNormalClick` → `DF_SetContactMode`（`FileRW/DeviceForm_File.gen.inc`，V912 底、含模式 12），也寫 `iContactMode`。
- facade 單選預設全不勾（vclcompat `TRadioButton()`；facade 建構子只建欄位），所以 B4 的 case 1 呼叫成員時**不會改掉**網頁設的 `iContactMode`（golden：沒有分支成立）——[B13] 釘住這一點。但 case 1 之後讀的 `chkDailyCorrelation`，以及 P8 讀的 `cbRTCModel`／`cbCalibrateAboveHeight`／`cbTeachInSHSen`／`chkTeachInOutArmZ` 也都是 facade 那一份。B4 要決定橋接（從 C 路同步到 facade，或照現狀並寫明）。
- 網頁可以設模式 12（`VISUAL_DETECTION_TEST`，V912 才有）：HT9050 被 P8 擋；國際牌機台在 0618 的 `DoTestContactFunction` 沒有這個模式——B4 要處理。

### 8.5 測試（ctest `IndexZAutoHeight1203` 加 [B13]，兩個組態）

- (a) 10 顆單選各自 → golden 的值（0／9／1／2／3／4／5／8／10／11，寫成數字）＋Memo 那一行；K Temp 勾 `chk_K_Temperature`；Daily Correlation 只在 Step 啟用；PTI（HT9050）的 One-Touch 隱藏＋清掉。(b) 同時勾多顆時照 golden 的分支順序。(c) 全不勾 → `iContactMode` 不動、Memo 沒有行。(d) KYEC_LEE／KYEC_XILINX／MAXIM_THAILAND 的 One-Touch。(e) KYEC_CHEN＋A16 的 `ChangeContactMode`。(f) 以上全部：卡片收到 0 個指令、0 個訊息、`fAllMotorHome`／`SystemStart`／`CarlibrationTask` 不動。(g) 純 P8 鏈（兩組態）。(h) 活的 P8 鏈（出貨組態：HT9050 拒絕 8 顆、各一個訊息、什麼都不送；ALM 時入口拒絕；`CardType` 空＝國際牌：同值、全放行）；模擬組態：全放行（golden SOFT_SIMULTE）。
- 原始碼釘子：普查（上面 8.2）；`forms/fContact_ContactSM.cpp` 每一行程式碼＝golden 0618 :15341-15434（92 行程式碼，去註解、空白合併後逐行比，golden 沒程式碼的行 port 也不能有）；本體只能是 UI 狀態（沒有 `MOT[`／`SW[`／`Gali_`／`ADAM_`／`WriteIniData`／`ShowMyMessage`／`SystemStart`／`fAllMotorHome`／`W906_`…）；`TfContact::SetContactMode` 只定義一次。

### 8.6 結果（20261005 00:10-02:59，St01）

- 增量建置兩組態 BUILD=0；指定集 19 個：出貨 19/19、模擬 19/19。`IndexZAutoHeight1203`：出貨 92/92（[B13] 18 條）、模擬 35/35（[B13] 14 條）。
- 全量 ctest（各 356 個）：模擬 18 紅＝基準 18（名字逐一相同）；出貨 4 紅＝基準 3＋`W7_C1_CleanOutFinish`（全量裡 "Not Run 0.00 sec"，單獨重跑 2/2 綠）——**沒有新的紅**。
- 零執行期改變佐證：`nm -C wb_serve.exe` 兩組態 `TfContact::SetContactMode` 0 筆（只有 C 路的 `DF_SetContactMode`）。
- 壞了會紅（出貨，`D:\AI_TempFile\st01e-e042-run\red_proof_b2.py`，紀錄 `red_proof_b2.log`）：rbManualHeight 寫成 AUTO_GET_HEIGHT → 8 條紅（全是 [B13]）；模式切換裡塞 `SystemStart=false;` → 3；P8 放寬到 Load Cell → 7（[B13] 4）；P8 漏到 RS-232 → 2（[B13] 1）；scratch 複本加一個活的 `fContactForm->SetContactMode()` → [B13] 普查紅；csystem `#if 0` 打開（B1 那根，重跑）→ 2 條紅；對照組（未改複本、加 `DF_SetContactMode()`／`SetContactModeFor9045()` 的近似呼叫）全綠；還原後 92/92。真的樹沒寫過（普查只動 scratch 複本）。
- W-44（只有 In／Out shuttle 都在 home 時 Index Z1 才能壓）／W-56：B2 不碰；放置位置寫在計畫 §14（golden 的壓下 case 沒有檢查 shuttle home → B3／B4 的 NOT GOLDEN 防護）。

## 9. Phase B 第三段 B3（E-042，St01，20261005 02:xx-04:xx；分支 `v906/st01-e042`，接在 B2 `936b9c99` 之後，不併 main／review6）

> **B3 沒有任何執行期改變**：新翻的 8 個狀態機都沒有呼叫者（golden 的呼叫者是 `DoTestContactFunction` case 300／330／900，B4 才翻），`csystem.cpp:31320-31322` 照舊 `#if 0`。認領：FROM_STEVEN §1 `1c50ce7f`（只動 A 列 `CMakeLists.txt:2910` 同一行＋St01 自己的檔）。10/05 18:00 前不進任何機台包。

### 9.1 翻了什麼（golden 0618 逐行照搬、每段固定偏移，產生器 `D:\AI_TempFile\st01e-e042-run\gen_b3.py`＋`b3_edits.py`，重跑位元組相同）

- **新檔 `forms/fContact_IndexPickPlace.cpp`**：A 段 :2384-5252（`CheckIndexArmStatus`、計時器、`DoZ1PickFromShuttle`、`DoZ2PickFromShuttle`）、B 段 :11354-11739（`DoZPlaceToShuttle`）、C 段 :19277-19501（`DoArm1/2PlaceToShuttle`）。
- **`forms/fContact_AutoHeight.cpp` 重產**：主段延長到 :10510（加 `Do_Z2_AutoGetHeight`，偏移照舊 −5284），檔尾加 :22325-22606（`DoCalibrateAboveHeightZ1`＋`Z2`；Z2 是因為 Do_Z2 會叫它，不翻會連結失敗）與 :18565-18626（`ATC_SwitchTjSignal`）。B1 的 LEAF-S43 三個 GATE 拿掉（本體已翻）。
- **ATC_SwitchTjSignal 的決定＝翻**：它是給測試機的「哪支 arm 下壓」通知（GPIB `MSG_CMD_Arm1/2Down`、SVID `iWhichArmDown`、`SwTjSignal01-08` 輸出），沒有動作；只有兩行 ATC `HandlerArm` 照舊 GATE（ATC 表單是 shim）。逐點 GATE 會讓國際牌機台默默少掉 golden 的 GPIB／IO 通知。
- **缺相依的同行 GATE**（原因寫在行上）：`ContactIndexSuckDestroy1`（KYEC D73 快速模式 → 當成關＝golden 的 else 分支）、`fBarCode->DoBarcodeCCDAutoTeach`（`cb2DMatrix` 當成沒勾）、`fBarCode->CleanBarcodeError`（Steven 20260421 清條碼錯誤旗標；B6 前條碼機台要補）、`StopAllMotor()` 寫成 `StopAllMotor(true)`（兩個宣告會歧義，本體相同）。
- **#20 D1**：`DoArm1PlaceToShuttle` case 300 先 `hContactDeley.SetSecAndOn(2)` 再進 case 400 等它（V912 :19648）；同一行三段註解 [906]／[V912]／[why]。

### 9.2 防護（全部 `AI(W906-E042) NOT GOLDEN`；只在出貨組態＋`MOT[MTestZ1].CardType=="PCI1203"`；停止永遠不擋）

| 編號 | 放哪裡 | 做什麼 |
|---|---|---|
| P7 | 8 個新狀態機的 `switch(Task)` 那一行 | M14 異常 → 同一拍 `ST`＋訊息＋golden 536 出口 |
| P4/P8 | `Do_Z2_AutoGetHeight` case 1 | arm 2 在 1203 Index Z 一律拒絕（HT9050 只有一支 index arm） |
| P5 | Do_Z2 555／7170、取料 560（Z1／Z2） | 5 秒沒扭力值 → `ST`＋出口 |
| P9 | Do_Z2 560、取料 550 下降步（Z1／Z2） | 命令位置到 `fIndexDownPos` 就停（golden 在 `bNeedSuck` 時會一直往下走） |
| **W-44** | **Do_Z1／Do_Z2 的 `switch(Task)` 那一行** | **只管壓 socket**（ST01-M 1005 03:4x）：除了 golden 的下降前 task（1／100／110／150／151／200／520／525／2200），In shuttle 1（M11）不在 `InSHT[0].iLeft` 或 Out shuttle 1 X（M17）不在 `OutSHT[0].iRight`（±100 counts，B3b 起；讀不到＝不在）→ `ST` Z1／Z2＋訊息＋出口。`W906_IndexZShuttlesHomeRefused` 是 B4 `DoTestContactFunction` case 1 用的入口版（拒絕、什麼都不送）。**取料／放料不加**：它們要 shuttle 停在 index 下方，照 golden 0618 |

### 9.3 ⚠ 上機要看（human-review A）＋給 B4

- **W-44 只管壓 socket**：請 EastSun 確認取料／放料下降時 shuttle 仍停在 index 下方、照常下降。
- ~~W-44 的後果：HT9050 上 golden 取料之後沒有程式把 shuttle 移回 home~~ **⛔ 更正（B3b，ST01-M 1005）**：golden Do_Z1 case 150/151 會先把 In shuttle 1 移到 `InSHT[0].iLeft`、Out shuttle 1 X 移到 `OutSHT[0].iRight`；HT9050 原本因 `USE_OUT_SHT_MOT=0` 走錯分支（case 100），B3b 已修，見 §9.5。
- B4 還要：`DoTestContactFunction`（case 1 放 P4／P8／W-44 入口）、`Do_LoadCellAutoHigh`、facade 與網頁的橋接（ST01-M：V912 `DF_SetContactMode` 當唯一入口，facade 單選經它走，附 906 註記）、上面三個缺相依的負責人。

### 9.4 結果（20261005 03:20-06:2x，St01）

- 建置兩組態 BUILD=0；指定集 19 個：出貨 19/19、模擬 19/19。`IndexZAutoHeight1203`：出貨 115/115（[B14] 23 條）、模擬 44/44（[B14] 9 條）。
- 全量 ctest（各 356 個）：出貨 3 紅＝基準 3；模擬 20 紅＝基準 18＋`WB_State`、`HSys_HeaterMix`（負載下，單獨重跑 2/2 綠）——**沒有新的紅**。
- 零執行期改變：`nm -C wb_serve.exe` 兩組態 B1-B3 的 TfContact 成員與 `CheckIndexArmStatus` 0 筆；W-44 的三個 helper 跟著 `IndexZTorque1203.o` 進 wb_serve，但全樹只有 E-042 的 TU 呼叫它們（沒有活的呼叫者）。
- 壞了會紅（出貨，`D:\AI_TempFile\st01e-e042-run\red_proof_b3.py`，紀錄 `red_proof_b3.log`）：W-44 從 Do_Z1 拿掉 → 6 條紅；W-44 壓下 task 收窄 → 6；home 容許放寬到 10000 → 6；取料 P7 拿掉 → 2；取料 P5 拿掉 → 2；取料 P9 拿掉 → 2；D1 拿掉 → 1；Do_Z2 case 1 arm 2 拒絕拿掉 → 1；scratch 複本加活的 `DoZ1PickFromShuttle`／`DoArm1PlaceToShuttle` 呼叫 → 普查紅；csystem `#if 0` 打開（重跑）→ 2；對照組全綠；還原後 115/115。

### 9.5 B3b 修正（St01，20261005 06:xx；ST01-M 回覆 W-44）

- **Do_Z1 case 1 在 HT9050 走 golden 的 shuttle 分支**（golden :5528 同一行加 `|| W906_IndexZLive1203()`，NOT GOLDEN）：case 150/151 先把 In shuttle 1（M11）移到 `Prod.InSHT[0].iLeft`、Out shuttle 1 的 X（M17）移到 `Prod.OutSHT[0].iRight`，再 110 → 200 → 520 → 530 壓 socket。原本 `USE_OUT_SHT_MOT=0` 會走 case 100（Index Y，HT9050 沒有）。`USE_OUT_SHT_MOT` 不改；910 HT9050 :5539 是同一行 golden。Do_Z2 case 1 沒有 shuttle 分支（一律 100），HT9050 在 case 1 就拒絕。
- **W-44 的目標改成教導位置**：M11 在 `InSHT[0].iLeft`、M17 在 `OutSHT[0].iRight`，±100 counts（就是 golden case 151 剛移到的位置），不是 0；讀不到照舊＝不在（`ST`＋出口）。M18 是 **Out shuttle Y（M18, MOutShuttle2）**，跟 M17 同一顆雙軸驅動器，**不檢查**（W-62／EastSun）；HT9050 沒有第二個 shuttle，M12 MInShuttle2 停用。
- human-review A：EastSun 確認教導的 `InSHT[0].iLeft`／`OutSHT[0].iRight` 離開 socket（＝Steven 說的 home），且 M11＝MInShuttle1、M17＝MOutShuttle1（機台 Mot_Table 第 13／19 列；Steven 的參考表 `D:\HT9045\system\Mot_Table_9050.csv`）。
- **還開著（B4，ST01-M 在問 Jimmy／Steven，E-050）**：golden 放回（`DoZPlaceToShuttle`／`DoArm1/2PlaceToShuttle`）不動 shuttle；HT9050 壓 socket 前 In shuttle 已移到 iLeft，沒人移回 iRight，又沒有 Index Y ⇒ Z1 會放在沒有 shuttle 的地方。B4 先讓 HT9050 的放回在 In shuttle 不在 iRight 時**拒絕**（警報＋停、不下降）並釘測試。
- 結果：測試 `IndexZAutoHeight1203` 出貨 117/117、模擬 44/44；指定集兩組態 19/19；全量出貨 3 紅＝基準＋`WB_Server`、`HSys_HeaterMix`（負載，單獨 2/2 綠）、模擬 18 紅＝基準＋`HSys_HeaterMix`（單獨 2/2 綠）。壞了會紅（`red_proof_b3b.log`）：目標改回 0（In）→ 3 條紅、（Out）→ 4、B3b 那一行拿掉 → 1；普查兩根照舊紅；對照組綠。

## 10. Phase B 第四段 B4（E-042，St01，20261005 09:xx-；分支 `v906/st01-e042`，接在 B3b `519297e3f` 之後，不併 main／review6）

> **B4 仍沒有執行期改變**：`DoTestContactFunction` 沒有呼叫者（MainProc 那行 `csystem.cpp:31320-31322` 照舊 `#if 0`，ctest 普查釘住），`nm` 確認 wb_serve 裡沒有 B1-B4 的 TfContact 成員。唯一進 wb_serve 的新東西是 `FileRW/DeviceForm_File.cpp` 檔尾的單一入口包裝＋它在靜態初始化時登記的函式指標（沒人叫）。B5 的 MR 在 10/05 18:00 之後。

### 10.1 翻了什麼（golden 0618 逐行照搬，產生器 `D:\AI_TempFile\st01e-e042-run\gen_b4.py`，重跑位元組相同）

- `forms/fContact_ContactSM.cpp` 重產：B2 的 `SetContactMode` 段照舊，加上 :11752-13963（`WaitTime`／`iRealTimeCCD` 計時器＋`DoTestContactFunction`）與 :17197-17997（`Do_LoadCellAutoHigh`），每段在標記行寫自己的偏移。
- **葉子狀態機沒翻的呼叫**（`Do_AutoContactTest`、`DoStepContact*`、`DoDeviceMapCheck`、`DoContactDeviceLoopTest`、`DoContactKTemperatureTest`、`Do_ROILearning`、`DoRTCAutoTuning`、`DoFullViewCheck`、`Do_ContactTest_32Site`、`DoZ1PickFromSocket`、`DoSocketSensorCheckRemainIC`、`DoIndecxCHECkFunction`、`CheckInShuttleSensor_Latch_Contact`、SFC／tray map auto tune、In-shuttle latch teach 的 `fShuttleMove`）→ `E042Leaf("名稱")`：訊息＋`SystemStart=false`＋`CarlibrationTask=1`，流程停下，**不會默默跳過**。
- 其他同行修改：`FormHS->CheckATCTempWait()` → `false`（port 沒有 TFormHS 實例；只有 KYEC_LEE 會成立，同 `csystem.cpp:33374`）、`CheckShuttleSensor_9045()` 寫成 `(0)`（golden 預設值）、`palSFCInformation->Font->Color` 只是字色（GATE）。

### 10.2 case 1 的入口（全部 NOT GOLDEN，在 golden 的 `CONTACT_NORMAL` 拒絕之後、任何輸出之前，:11819 同一行）

| 順序 | 檢查 | 機台 |
|---|---|---|
| 1 | 還沒翻的模式：4／5／9／10／11（葉子沒翻）、**12 VISUAL_DETECTION_TEST**（0618 沒有這個分支；訊息含「TODO E-052: port V912 Task 60」） | **所有機台** |
| 2 | P4＋P8＋P7（`W906_IndexZRunRefused`）：CONFIRMED、只准模式 1／3、單臂、驅動器健康 | PCI1203 Index Z |
| 3 | W-44 入口（`W906_IndexZShuttlesHomeRefused`）：In shuttle 在 `InSHT[0].iLeft`、Out shuttle X 在 `OutSHT[0].iRight` | PCI1203 Index Z |
| 4 | 放回步驟歸零（新的一輪） | — |

拒絕時 `SystemStart=false; return;`，**什麼都不送**（連 `ST` 都不送）。`switch(Task)` 那一行另有 P7（case 1 以外每一拍）。

### 10.3 模式單一入口（Steven 1003 常設：V912 比較好就用 V912＋906 註記；ST01-M 1005）

- golden 0618 :11790 `SetContactMode();` → `if(W906_ContactModeSingleEntryHook) W906_ContactModeSingleEntryHook(-1); else SetContactMode();`。
- 掛鉤由 `FileRW/DeviceForm_File.cpp` 檔尾在靜態初始化時登記成 `FileRW_Contact_SetContactModeSingleEntry`：在網頁選的單選（`g_ct3aRb`）上跑 V912 的 `DF_SetContactMode`（含模式 12），再把那顆映到 TfContact facade 的單選，並把 case 1／P8 接著要讀的 `chkDailyCorrelation`、`cbOneTouchAutoContactHight`、`chk_K_Temperature` 從 C 路抄到 facade。
- 用掛鉤而不是直接呼叫：`DeviceForm_File.cpp` 是 wb_serve 的原始檔，不在 `ht9045_sm`；直接呼叫會讓 B6 之後每個連 `ht9045_sm` 的程式連結失敗。C 路沒連進來時退回 golden 的成員 `SetContactMode()`。

### 10.4 HT9050 放回步驟（Steven 1005 09:2x Q101：HT9050 沒有 Index Y；放回前 In shuttle 回到 iRight）

- 位置：golden case 900 :13350，`DoZPlaceToShuttle()` 前面同一行：`W906_IndexZPlaceBackPrep`，回 1＝就緒（golden 放回照跑）、0＝移動中（下一拍再來）、−1＝失敗（警報＋`ST`＋golden 出口）。
- 動之前的條件：Z1 在安全高度（`Prod.TestZ1_Safe`，−100 counts 以內或更高）＋Out shuttle X（M17）在 `OutSHT[0].iRight`±100；讀不到或不成立 → 警報＋`ST`＋出口，**In shuttle 不動**。
- 條件成立 → In shuttle（M11）移到 `InSHT[0].iRight`、確認 ±100 → 放料（golden）；放料期間 In shuttle 離開 iRight → 失敗。之後 golden case 1700 把它移回 `iLeft`（第 7 步）。
- 只在 PCI1203 Index Z（`W906_IndexZLive1203`）；國際牌／RS-232 機台一律回 1（golden：Index Y 把 IC 帶回 shuttle 上方）。
- ⚠ human-review A：EastSun 第一次以 1% 速度看這段移動。

### 10.5 Load Cell

`Do_LoadCellAutoHigh` 的 `switch(Task)` 有 P7，case 1 有 `W906_IndexZRunRefused(iContactMode, iIndex, ...)`：HT9050 拒絕（Steven 1004 08:4x）。P5／P9 沒加：在 HT9050 到不了，其他機台照 golden。

### 10.6 缺相依的負責人（給 ST01-M）

- `fBarCode->CleanBarcodeError`／`DoBarcodeCCDAutoTeach`（還有 B4 的 SFC auto tune `InitialSFCAutoTune1/2`、`DoSFCAutoTune_1/2`）：TfBarCode 在 `aHotPlateSubstrate.h`，**要先問 Steven**，B4 不碰那個檔，GATE 照舊。
- `fiosetview->ContactIndexSuckDestroy1`：GATE 照舊，ST01-M 找負責人。
- 另外（B4 新出現）：`fTrayMapping->DoTrayMapAutoTuneCCD`（TfTrayMapping）、`fShuttleMove->iShuttleMoveTask`／`DoInShuttleChkStackAction`（`forms/fShuttleMove.h`）。

### 10.7 結果（20261005 09:4x-12:4x，St01）

- 建置兩組態 BUILD=0；指定集 19 個：出貨 19/19、模擬 19/19。`IndexZAutoHeight1203`：出貨 142/142（[B15] 25 條）、模擬 55/55（[B15] 11 條）。測試也把 C 路原始檔（`FileRW/DeviceForm_File.cpp` 等，照 `test_b8_ct3a_contactflags`）編進來，單一入口是真的跑 V912 的 `DF_SetContactMode`。
- 全量 ctest（各 356 個）：出貨 3 紅＝基準 3；模擬 18 紅＝基準 18（名字逐一相同）——**沒有新的紅**。
- 零執行期改變：`nm -C wb_serve.exe` 兩組態 B1-B4 的 TfContact 成員 0 筆；單一入口包裝＋掛鉤指標在 wb_serve 裡（2 筆），沒人呼叫。
- 壞了會紅（出貨，`D:\AI_TempFile\st01e-e042-run\red_proof_b4.py`，紀錄 `red_proof_b4.log`）：放回條件拿掉 Z1 安全高度 → 2 條紅、拿掉 Out X 在 iRight → 2、目標改成 iLeft → 1、:13350 的放回步驟拿掉 → 1；模式 12 不擋 → 3；W-44 入口拿掉 → 2；P4/P8/P7 入口拿掉 → 4；Load Cell case 1 拒絕拿掉 → 2；單一入口改回 golden 成員 → 1；scratch 複本加活的 `fContactForm->DoTestContactFunction()` → 普查紅；csystem `#if 0` 打開 → 2；對照組全綠；還原後 142/142。

## 11. 量產 Index 扭力等待逾時（E-044；main 原 §7，E-042 合 main 時改號為 §11，Steven 1004 08:4x「加逾時：報警並停機」，not-golden）

- 位置：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp` `DoTestHeadMotor` case 12110 第一行（:6908）與 14110（:7538，臂 2 雙胞胎）同一行插入；本體 `Ht9050TorqueWait.cpp`／`Ht9050TorqueWait.h`（St01）；ctest `E044TorqueWait`。側分支 `v906/st01e-e044`。
- golden（0618 atester.cpp:6436-6468）在這裡等 edTorue0 有值；唯一的錯誤出口（:6456-6460）要 `GetReadTorueTask()==999`，只有讀成功才會設（golden rs232.cpp:991-995）。HT9050 沒有 RS-232 扭力（rs232.cpp:263-266 不開埠），B7（V910 DoTestHead）或 E-038 CONFIRMED=1 之前什麼都等不到 ⇒ 永遠壓著等（S-26 10-1）。
- 只 HT9050：`MOT[MTestZ1].CardType=="PCI1203"`（同 rs232.cpp:950）是 atester 那行 && 的第一個運算元；其他機種停在 CardType，什麼都不跑＝照 golden 永遠等。SIM（SOFT_SIMULTE）不作用（12110 在 SIM 到不了）。
- 5 秒沒有值 ⇒ 清 chkReadTorque1/2（RS-232 讀取器與 E-038 bridge 都停，不再跳「Read Index Z1 Torque error」）→ `ShowErrorMessage`（golden 停機：wb_serve 先 StopAllMotor(true)＋SoftStop/SoftStart/SystemStart=false，再記警報、再出框）→ golden 12110 自己的出口 `fAllMotorHome=false; Task=1;` ⇒ 下次 START 先回原點。
- 只在自動運轉推著這段等待時才會報（ST01-M 1004 15:4x 變更，ST01-E2 R4 計畫 §3）：`SystemStart && !SoftStop && fAllMotorHome && iHome!=1`＝自動運轉梯子自己的守衛（csystem.cpp:1686／:1934）＋不在 HOME（iHome==1，csystem.cpp:31224；golden 的 HOME 路徑與停機分支在 [I01] 時也會呼叫 DoTestHeadMotor，:31220-31231、:32632-32648）。停機、HOME、驅動器警報後（fAllMotorHome=false）都不報，而且視窗歸零。
- 計時：自己的視窗、`GetTickCount`（測試可換假時鐘）；**每次進入 12110／14110 都重新起算**：golden 的武裝條件（bFirstTime＋DoTestHeadMotorDelay 未到）、或跟上一拍之間隔了 2 個以上 MainProc 拍（`GetMainProcCallCount`，＝狀態機去過別處或停過機），都算新的一次進入；有值的一拍也歸零。停機後再按 START＝重新算 5 秒。不能用 atester 裡的 MyTickCount（:5861 被替身成常數 0）。
- 5 秒（Steven 1004 14:1x 確認「照設計 5 秒」）＝E-038／E-042 P5 的 kTorqueWaitMs；比 E-038 的 10 秒「source not confirmed」早，操作員只看到一個有碼的警報。golden 的 ReadTorqueDelay 10 秒沒動。
- 警報碼：暫用 WAR0361（臂 1）／WAR0362（臂 2）＝golden 既有「Index arm 1/2 contact torque monitor error.」；KCode 照 golden Index 慣例（bIndexAreaOnlyCanUseSkip ? K_SKIP : K_RETRY）。**TODO：Jimmy 選**（WAR0361/0362 或新 WAR0363/0364；K_SKIP 或 K_RETRY），只改 `Ht9050TorqueWait.cpp` 兩個字面值，不用再認領筆電檔。
- ⚠ **R4（todo E-045：移動中驅動器警報要照 golden ShowMotorErrorMessage 報一次）應該比 E-044 早或一起上**；沒有它，Z1 下壓中驅動器警報不會報出來，等 5 秒後會被當成扭力逾時。E-045 誰做、何時做由 Steven 決定。R4 報了之後 E-044 不會再報第二個：ShowMotorErrorMessage 先把 fAllMotorHome 設 false（golden note.cpp:1059；移植樹 forms/fNote_ShowError.cpp:663），等待就不符合條件（ctest E044TorqueWait [T14]）。
- 拿掉條件：B7 上線、HT9050 量產不再走 12110／14110 時，同一個 commit 刪兩行插入、翻 ctest E044TorqueWait 的 pin；E-042 不在 12110／14110 另加 P5（避免雙警報）。
- 上機（EastSun，human-review A）：教導做完、無 IC，START → Z1 下到 12110 → 約 5 秒警報（碼＝暫用碼）、全軸停、之後不再跳 Read Torque 訊息、什麼都不動（這台 [I01]=1）→ START 先 HOME。

## 20261005 補記（ST01-E，分支 `v906/st01-skills-1005`）：E-042 進 MR !218、E-044 的「拿掉條件」不成立、MTestZ1 只有 `Gali_*` 到得了 1203

- **E-042 B1～B4＝HT9045 MR !218**（`v906/st01-e042` tip `94f99c19`＝B4 `847b9dc7`＋兩次併 main）：**執行期零改變**——`MainProc` 呼叫 `DoTestContactFunction` 那一句仍是 `#if 0`（ctest census 釘住），`nm -C wb_serve.exe` 兩組態都沒有 B1～B4 的 TfContact 成員。已放進筆電第 72 批（gate b72a 1005 21:59 起，`ba8021dd`）；`tests/CMakeLists.txt` 檔尾的合併由筆電處理，St01 不再把 main 併進 e042。合進 main 之後，本檔上面的 §7（E-044）會變成 §11，§7～§10 是 E-042 的段落。真的動 Z（B6）要 E-10＋`CONFIRMED=1`＋Jimmy #86＋S-26 R4，不在 !218。
- **§7（合 !218 後 §11）E-044 的「拿掉條件」不成立了**：那一點寫「B7 上線、HT9050 量產不再走 12110／14110 時，刪兩行插入」。B7＝ST01-C（910 `DoTestHeadMotorFP`）上線後，一般 `DoTestHeadMotor` 的 12110／14110 在 HT9050 **仍走得到**：
  1. MainProc 的 [I01] 兩處照 910 呼叫一般 `DoTestHeadMotor()`（Jimmy `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20261005.md` 第 14 條「#116 照 B」＝decisions-decided Q103；機台 `bI01TesterFinishThenHome=1`）；
  2. `W906_HT9050_AS_LS` 的建置把 9050GPIB 解成 Type_HT9046_LS，整台走一般流程（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\MachineType.h`:1780、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\database.cpp`:520）。
  ⇒ `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\atester.cpp`:6908／:7538 的 E-044 插入**留著**；FP 自己的 12110 另外呼叫一次 E-044（同樣是 `W906_Ht9050TorqueWaitTimedOut`＋`W906_Ht9050TorqueWaitAlarm`，出口 `fAllMotorHome=false; Task=1;`），**永遠不加 E-042 的 P5**（一個等待只跳一個警報）。
- **F1：HT9050 的 MTestZ1 只有 `TMyMotor::Gali_*` 會到 1203**（ST01-C 計畫 `D:\AI_TempFile\st01e-c-fp-plan-20261005.md` §1.5）：M14 MTestZ1 在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cinitial.cpp`:4031 建成 `TMyGALILMotor`（INDEX_MOTION_CARD=0）。一般的 `MOT[MTestZ1].MotorMove()` 走到 `TMyGALILMotor::MoveToPos`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Motor\myGALILmotor.cpp`:1129），沒有 Galil 卡（`bGali_CardInstall=false`，:736）就直接回 false、什麼都沒送到 1203；ctest FP9050_Index [F4]（ST01-C slice 1，還沒 commit）量到它最後還回**假的「到位」**；一般 `ReadPos()` 讀 0。只有走 `W906_GaliRouteOwns(Mot_Name)` 分支的 `Gali_*`（同檔 :1622、:1676、:1758、:1957、:2151 等）會被 `EtherCAT\Pci1203GaliRoute` 送到 1203。SIM 裡 Galil 物件關著、`MotorMove` 用模擬——SIM 綠燈證明不了接上了。
  ⇒ 910 原樣的 `MoveIndexZ`（910 `Motor\mymotor.cpp`）在機台上 Z1 不會動、流程停在第一次 `MoveIndexZ(TestZ1_Safe)`、沒有訊息。ST01-C 用路由的 `Gali_MotMove(iPos, MOT[MTestZ1].GailSpeed)`＋`Gali_ReadPos()`（Steven 1005 23:1x，decisions-decided Q113）。順帶：`SetMotorScaleSpeed` 對 Galil 的 Index 軸只寫 `GailSpeed`，一般 `MotorMove` 不看，走 `Gali_MotMove` 才會真的變慢。EastSun 1005 在 `acarry.cpp` `W906_ShtIndexZ1Safe` 的註記用一般 `ReadEncoderPos` 讀 Z1，跟這個量測要對一下（Frank F-4＝decisions-pending Q130）。
- **W-44 的定義改了**（Steven 1005 23:1x，decisions-decided Q114）：「安全 X 座標」——飛梭 X 在 Index 安全區之外，Z1 就可以下壓；不再要求兩支飛梭都在原點（出料飛梭可能正在做 5S＝CCD 五面檢查）。保護歸 Frank01；E-042 B3／B4 的 `W906_IndexZShuttlesAtHome`／`W906_IndexZShuttlesHomeRefused`（原點 ±100）**要在 B6 之前改成呼叫 Frank 的判斷**（提案名 `W906_Ht9050ShuttlesClearOfIndex(AnsiString* why)`）。流程見 `D:\HT9045\.claude\skills\ht9045-index-flow\references\ht9050-index-fp-flow.md` §5。
- 閘與防護看馬達類別的狀態、不看 1203（Steven 1005 23:2x，decisions-decided Q119；`D:\HT9045\.claude\skills\ht9045-motor-control\SKILL.md`〈安全機制〉的常設規則）：本檔各段用 `MOT[MTestZ1].CardType=="PCI1203"` 當**開關**的防護（E-042 的 P4～P9、E-044），要跟 E-043 計畫第 2 版一起看；讀扭力走哪條路（E-038 資料路徑）不是閘，不受影響。

## ⛔ 20261006 更正（Q130／Q133，Steven 1006 08:2x；decisions-decided 20261006 節）

- **F1 那段（「MTestZ1 只有 `Gali_*` 到得了 1203、ST01-C 用路由的 `Gali_MotMove`」）要照 Q130 看**：Steven「3軸或4軸 index使用的是 Galil卡片, 1軸的index使用的是 MyMotor的 MotorMove. 跟是不是1203沒有關係!」。golden 只要 `INDEX_MOTION_CARD==0` 就把 Index 四軸（MTestY1／Z1／Z2／Y2）都建成 `TMyGALILMotor`（`InitialMotorParameter`，原註解「for HT-502 II」）；HT9050 機台的 `Gerneral.ini` 是 `INDEX_MOTION_CARD=0`，所以 1 軸的 Z1 才變成 Galil 物件、要繞 1203 路由。ST01-C slice 1 本來就照 `INDEX_MOTION_CARD` 分支（0＝`Gali_*`，否則 910 的 `MotorMove`），所以這是**機台設定／類別的決定**（改了會牽動 Z1 的 1203 回原點、E-038 扭力來源、E-042 P7、E-044 的 `CardType=="PCI1203"`、`IsIndexMotorOutOfPower`），給 Frank01＋EastSun＋Jimmy 定；說明在 `D:\AI_TempFile\st01e-c-revision-note-20261006.md` §1。
- **W-44 那段（Q114「安全 X 座標」）再改為 Q133**：「應該是禁止shuttle進入socket區域 (in shuttle的右側 跟out shuttle的左側), 而不是禁止index動作」「一切行為以index的需求為優先」——防護擋**飛梭的移動**（入料飛梭往右、出料飛梭往左進 socket 區），Index 下壓前**等**飛梭離開 socket 區、到安全區（不一定是 0）；E-042 的 `W906_IndexZShuttlesHomeRefused`（原點版）在 B6 前要改成「等到清空」。細節在 `D:\HT9045\.claude\skills\ht9045-index-flow\references\ht9050-index-fp-flow.md` §9。
