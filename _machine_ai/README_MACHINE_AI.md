# 給機台端 Claude：合併新包（筆電 main 66cb14e0 → 56bbf785）—— **取代第二包與第三包**

> 筆電端 Claude 20260926 10:0x 產生。機台上一次合進整合樹的是第一包（66cb14e0，整合樹 `95c398b`）。
> **這一包從 66cb14e0 起算、涵蓋到 GitLab main 56bbf785 的全部變動（277 檔）**，第二包（ff4b1d8b）、第三包（afc9e3c7）**都不要再套**。
> 目標一樣是 **`D:\HT9045\_integ_ioweb`**；做法與第一包相同（`check_and_copy.ps1`，底稿 `base_66cb14e0\`）。

## ⚠ 先看：以機台版為準的地方（三方合併時保留機台的，不要被筆電蓋掉）

| 檔 | 為什麼 |
|---|---|
| `HT9011UC_Cpp_V3.33.906.0/csystem.cpp` 的 `IsIndexMotorOutOfPower()` | 機台 8929d13 的版本（MTestZ1 是 PCI1203 時只看 EMG／SnMotorPower）才對；main 還是 golden 本體（NB2 R63，USB 04 帶回後 main 會跟上） |
| `web/page/HW.MotorTest.html` 的 Loop Move 位置讀取 | 機台 MT-E1b（d288f1f，web a5454b9）已修「pos1/pos2 恆 0」；main 版還是 NB2 R49 說的舊寫法 |
| 你們 MT-E1～E3、FIX1、IOWEB-P*、ONSITE-1 動過的所有檔 | 照前兩包的原則：機台行為保留、筆電的新改動補進去；衝突看不懂就停下回報 |

**刻意沒帶**：`build.bat`、`.vscode/launch.json`、`tools/pe_truncation_check.ps1`、`tools/webprobe/f5_contract_probe.cjs`（建置輸出搬到 `<repo>\Obj\V906` 那組；你們正照 RULINGS_20260926 第 1 條換 oracle 建置線）。
**沒動你們要求別碰的檔**：Pci1203Monitor.*、WebBridgeTags.cpp 的 tag（Steven 有加 SortCT／LotInfo／TestCategory 三段 tag 發布，是另一段，會進 LOCAL 合併）、ioweb_probe.cpp、myEthercatmotor.cpp、database.cpp／cinitial.cpp 讀馬達表與 INDEX_MOTION_CARD 那一帶。

## 這包帶了什麼（都過筆電兩組態 gate：出貨 180 項失敗 3 項、模擬 180 項失敗 18 項＝基準）

| 類別 | 內容 | 對機台的影響 |
|---|---|---|
| 安全修正（今天） | **R33**：HandlerSys 頁存檔把 IO_CARD_TYPE 4 寫回 2 —— C 路引擎「填不進就整頁拒寫」＋rgIOCard 補第 5 項 PCIE-1203 IO（`3349e1a6`）；**R46／R53**：HT9050 不探測、不開 RS232 扭力埠（`3e1aec8d`）；**R61**：aTester_Rear 三處 FTestSuck（`a2cfd4cb`） | HandlerSys 頁在 HT9050 上 4 能原樣存回；開機不再碰 COM1 |
| 第二包原本的內容 | 教導頁運動鈕（W5-b）、YES/NO 照 golden 等操作員回答（**新 wb_serve 必須配新的 `web/page/dialog-page.js`**）、Index Z 扭力（HT9050 走 1203 掛鉤；掛鉤沒裝前一律回 Motor torque set error）、HT9050＝HT9046 家族分派、DIO 存檔 | 見第二包 README |
| W2 解閘（0926） | IC 料況搬移、生產紀錄、陷阱 5、A4-6 同類 10 處、SetTestRunMode＋ModifyTester、G-5／G-6 生產紀錄 CSV、G03、**h4-G4 回原點前等吸嘴吹氣完成（安全互鎖恢復）** | ⚠ **第一次跑 Index 自動流程要 EastSun 在旁**；JAM／WAR 會開始寫 `D:\HT9045_Log\Production_Log\*.csv`、Auto Clean 寫 `CleanPad_Log` |
| Steven C 路畫面（合進 main） | Setup.Cleaning／QAMode／TesterIF／BarCode、HW.VacuumUnit／ShuttleMove、Status.TowerLight／Security Jam、Data.LotInfo 各分頁、ShowMyMessage 網頁版、換配方（recipe.change） | 行為改變（照 golden）：開機讀 Tester.Data（例 TesterType 0→1、iTestBinCount 16→33）；BarCode 第一次存檔補 12 個缺鍵、Check duplicate code by shuttle 0→1；Jam 設定改讀真實 JAM0000.dat；密碼值不再送到瀏覽器 |
| LastSet 結構 | 補 V912 尾欄位（S45），`sizeof(LAST_GENERAL_SET)` 178896→179928 | 舊 `lastdata.dat` 照 golden 短讀；**第一次寫檔會把檔案延長**（多出來的是 0）⇒ 套用前先備份 `system\lastdata.dat` |
| 其他 | M108、ZSafePos 照 ini、開機 [BOOT] 摘要、database.cpp 中文標題、F5／launch 修正（不帶）、文件（RULINGS 0925／0926、W2／W3 進度表、INBOX、NB2 R30～R63） | — |

## 步驟

1. 開場：沒有 wb_serve／cmake／ctest 在跑；整合樹與 web 樹先 **commit 目前的 WIP**（不要 stash）。
2. **備份**：`system／config／IniData`（＋`runcfg`）、`system\lastdata.dat`、`D:\HT9045_Log`（生產紀錄會開始寫）。
3. 檢查（唯讀）：`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`
4. EastSun 同意後 `-Mode Apply`。LOCAL 的三方合併：base 先找機台 git 歷史裡最近一個筆電已知版本，找不到用包內 `base_66cb14e0\`。上面「以機台版為準」那幾處一律保留機台的。
5. **全量**建置（LastSet.h、cprod.h 都改了）→ 0 error；ctest 失敗清單和筆電比。
6. 驗證（不動機台）：開 HW.IoSetView、HW.MotorTest、HW.teach、HW.HandlerSys（⚠ 先看 IO_CARD_TYPE 顯示第 5 項 PCIE-1203 IO；存檔後檢查 ini 還是 4）；看開機 `[BOOT]` 行。**不要跑 w4_motor_probe.py／w5_teach_probe.py**。
7. 會動機台的（Index 自動流程、回原點吹氣互鎖、Loop Move）等 EastSun 在旁。
8. commit，回報：Check 報告前 5 行、LOCAL 清單與合法、建置結果、新 commit hash。
