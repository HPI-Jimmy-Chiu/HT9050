# Index Z 扭力讀值、正負號與自動測高（國際牌 → 安川 Σ-X）

> 20261003 ST01-M 整理（Steven 1003 21:4x～22:3x 的問答與裁決 Q87）。改 Index 測高、扭力門檻、HT9050 的 1203 扭力讀值之前先讀這份。

## 1. golden 怎麼讀國際牌（Panasonic MINAS）的扭力

全在 golden 0618 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\rs232.cpp`（COM2 直接跟 Index 伺服驅動器講；Galil 只負責移動，不讀扭力）：

| 步驟 | 位置 | 內容 |
|---|---|---|
| 選哪一軸 | `:841-851` | `chkReadTorque1`＝Z1（位址 0）、`chkReadTorque2`＝Z2（位址 1） |
| 交握 | `:899-985` | ENQ → 驅動器回 EOT → 送請求 → 驅動器回 ACK＋ENQ → 送 EOT → 收資料 → 回 ACK |
| 請求封包 | `:833`、`:909-910` | `datatrq[4]={0x00, 位址, 0x52, 0xAE-位址}`：資料長度 0、軸位址、**命令 0x52（＝(模式<<4)|命令＝命令 2／模式 5「Read out of present torque output」，註解「要求讀取扭力值」；1004 更正：以前寫成命令 5／模式 2；單位＝額定扭力 2000，所以 k/20＝額定扭力 %（A5II p.427），詳見 skill ht9045-panasonic-rs232）**、檢查碼 |
| 回覆解碼 | `:1799-1813` | `k = str[4]<<8 \| str[3]`（16 位元有號）；**`if(k<0) k=0;`**（負值丟掉）；**`Torque = k/20.0`**；`%5.2f` 寫進 `fMain->edTorue0`（`:975`）與 `fContact->PnlTorue0` |
| 逾時／錯誤 | `:858-866`、`:887-895` | 收不到回覆 `Torque=-9999`、重開 COM；連續 10 次 → 「Rs232 Read Index Z1 Torque error!!」 |

- 0x52 在國際牌手冊上的正式名稱（扭力命令還是實際扭力、原始單位）程式看不出來；`÷20` 推算原始單位＝0.05 %（2000＝100 %），**要對國際牌 MINAS RS-232 手冊確認**。
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

- Q1 國際牌 0x52 的單位（k/20＝％？）——要國際牌手冊。
- Q2 重力基準要不要開——Steven／EastSun（量完 E-10 #5 再定）。
- Q3 1203（Advantech DS402）有沒有在哪裡反轉命令極性——EastSun 上機。
- Q4 HT9050 Z 的撐重扭力是否小於 kg（15 %）——EastSun 上機。
- Q5 Phase B（翻 Auto Height 本體，約 8,200 行 golden）誰做——筆電 CT-3c 清單上，Steven 決定。

## 7. 量產 Index 扭力等待逾時（E-044，Steven 1004 08:4x「加逾時：報警並停機」，not-golden）

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
