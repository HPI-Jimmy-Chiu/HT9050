# 給機台端 Claude：更新包 3（GitLab main `db6736c5`，相對根目錄那包 `56bbf785`）—— **取代 `updates/410d27d9/`（第 2 包）**

> 筆電端 Claude 20260926 12:0x 產生。**Jimmy 正在上機驗證：他說要套之前，不要自己套。**
> 第 2 包（`updates/410d27d9/`，只有權杖修正 4 檔）漏帶了在它之前提交的 HAlarm（`915c7d9c`）與 Steven 的 widget，所以這一包改成相對 `56bbf785`（根目錄那一包），**包含第 2 包的全部內容**。第 2 包套過或沒套過都可以直接套這一包（套過的檔會被判成 SAME）。
> 目標 `D:\HT9045\_integ_ioweb`；做法同前（`check_and_copy.ps1`，底稿 `base_56bbf785\`）。

## ⚠ 行為改變（套之前讓 EastSun 知道）

| # | 改了什麼 | 機台上會看到 |
|---|---|---|
| 1 | **HAlarm 照 golden 接上**（RULINGS_20260926 第 16 條，`915c7d9c`） | 運轉中（SystemStart）氣缸 Push／Pop 兩次重試都等不到感測器 ⇒ **停機並跳 JAM（CylinderIndexToJamCode）、K_RETRY**。以前是空殼、什麼都不做。1203 氣缸的感測器沒接好的話，第一次跑流程就會跳 |
| 2 | **HT9050＝HT9046_LS＋1203**（第 25 條，`47f91343`） | `9050GPIB` 也解碼成 HT9046_LS（Model 不用改）。**扭力**：`IO_CARD_TYPE=4` 時開機不開 RS232 扭力埠（不再開 COM11）；運轉中 Index Z 是 PCI1203 卡時扭力上限走 1203 —— **沒裝扭力掛鉤（kCmdAxTorqueLimitSet／W906_Pci1203TorqueLimitHook）前仍會報 Motor torque set error**，但不會再去等 COM11 逾時 |
| 3 | 網頁權杖閒置 30 秒自動還（`410d27d9`） | IO 頁不會再被 Motor Test 卡 10 分鐘 |
| 4 | cprod.h 的 SYSTEM_TEST_IF 補 5 欄（第 10 條） | **要全量重編**；BarCode 的 CSV Compare 勾選框開始讀寫真欄位、Tester.Data 的 [AMR] 四鍵開始讀 |
| 5 | 氣缸常數 C_StackedTrayLockOff 75→153（第 11 條） | 153 號槽第一次有名字；IO 表沒有那幾列 ⇒ Enable=false，不會動；會多一個 JAM31153 的說明檔 |
| 6 | `act.main.clarnData` 不帶 dryRun 就直接執行（第 12 條） | 目前沒有頁面送它 |
| 7 | 網頁讀配方顯示 BCB6 實際讀到的值（第 13 條） | 前後空白、引號不再顯示；存檔時沒動的欄位照原字送回 |

## ⚠ 以機台版為準（三方合併時保留機台的）

| 檔 | 為什麼 |
|---|---|
| `web/page/HW.MotorTest.html` | 機台版的 Loop Move（MT-E1b）、ALed、Light Scale、每 60 秒 keepAlive 保留；這包只加了 `holdToken:` 那一行 —— **機台版要把 Light Scale 進行中也放進 holdToken** |
| `web/page/ht9045_recipe_client.js`、`web/page/motor-access.js` | 權杖記帳（`cmd()` 包一層、`acquire()`／`release()`、`global.HT9045Recipe = api` 之前那段、motor-access 第 13 行與 `init`）＋第 13 條的一行註解；其餘保留機台的 |
| `mycylin.cpp`、`ckernel.cpp`、`tools/wb_serve.cpp` | 這包只改 SetAlarm／ClearAlarm（:122-133）、GetMotorAlarmCode／ProcessAlarm 的型別（:3839／:3998）、wb_serve.cpp:3978 一行（開機建 Alarm）；機台動過的其他段保留 |
| 你們 MT-E1～E3、FIX1、IOWEB-P*、ONSITE-1、暫關安全門那一顆動過的所有檔 | 同前幾包的原則：機台行為保留、筆電的新改動補進去；看不懂就停下回報 |

**刻意沒帶**：`build.bat`、`.vscode/launch.json`、`tools/pe_truncation_check.ps1`、`tools/webprobe/f5_contract_probe.cjs`。**這包沒有動** csystem.cpp、csystem.h、uhome.cpp、forms/fMain.cpp、WebMotorAccessLive.cpp、Pci1203Monitor.*、myEthercatmotor.cpp、ioweb_probe.cpp、WebBridgeTags.cpp。

## 步驟（等 Jimmy 說可以再做）

1. 開場：沒有 wb_serve／cmake／ctest 在跑；整合樹與 web 樹先 commit 目前的 WIP（不要 stash）。
2. 備份：`system／config／IniData`、`system\lastdata.dat`、`D:\HT9045_Log`。
3. `powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`
4. EastSun 同意後 `-Mode Apply`；LOCAL 照上面原則三方合併。
5. **全量**建置（cprod.h 改了）→ 0 error；ctest 失敗清單與筆電比（筆電：出貨 182 項 3 失敗 config_db、config_loaders、GA1_ReadGeneralIni；模擬 182 項 18 失敗）。
6. 驗證（不動機台）：`node HT9011UC_Cpp_V3.33.906.0\tools\webprobe\token_idle_selftest.cjs` 24/24；開機 `[BOOT]` 行 MachineTypeChoice＝HT9046_LS 的值（Model 9046_32GPIB 或 9050GPIB 都一樣）；開機 log 沒有 `Index Torque : COM11 port error`。
7. 會動機台的（Index 自動流程、氣缸逾時停機、扭力）等 EastSun 在旁。
8. commit，回報：Check 報告前 5 行、LOCAL 清單、建置結果、新 commit hash。
