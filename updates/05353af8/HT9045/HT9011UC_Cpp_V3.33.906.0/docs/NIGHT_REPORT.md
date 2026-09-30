# 夜間報告 2026-09-29（二）晚起 —— **進行中版本**（每推一批整份更新；最後更新 0930 12:4x）

> 使用者 0929 17:5x 下班：「完成手頭任務且無任務情況下，自動切換到Loop周末下班任務」；18:0x：「不用限制，我如果需要停止Loop會主動說明白」
> ⇒ **沒有收尾時間**，一直做到使用者說停；早上使用者在也照跑。迴圈：cron `4ca5c42b`（每小時 :07／:27／:47，session 閒置才觸發）。
> ⚠ session-scoped：視窗關掉、或筆電被公司 AutoTools 強制關機（`night-loop` 技能 §5.1，0928 17:08 發生過）就會中斷；狀態全在 git 與本檔。
> 0924～0929 週末那一輪的舊報告在 git 歷史（`f455a765` 之前的版本）。
> **0930 09:3x 使用者不小心下了 `/clear`**：只清掉對話記憶；commit、各批 gate 紀錄（上一輪暫存區 `ef8cf4db…\scratchpad\iz3`／`aml3`／`flow4`）、NIGHT_REPORT／INBOX、cron 心跳 `4ca5c42b` 都還在，已從磁碟接回，**沒有遺失任何工作**。

### 怎麼讀

| 節 | 內容 |
|---|---|
| **§0** | **要你決定／處理的**（只放真的需要你的） |
| §1 | 做完了什麼（附證據與 commit） |
| §2 | 佇列（照 INBOX 111→117 的順序） |
| §3 | 紅燈／哨兵 |
| §4 | 清理與還原 |
| §5 | commit／push 清單 |
| §6 | 我自己犯的錯與更正 |

---

## §0 要你決定／處理的

| # | 事項 | 為什麼需要你 | 預設（你沒回之前照這個做） | 在哪 |
|---|---|---|---|---|
| 1 | **Motion View 要不要照 golden 加「Simulte Enable」勾選框** | golden 的 Motion View 只有勾了主畫面 Motion View 分頁上的「Simulte Enable」（CheckBox2，golden `main.cpp:8676-8686`）才會讓手臂圖跟著馬達動；沒勾就不動。網頁版（Steven 的設計）只要頁面是 LIVE 就一直跟著動。<br>例：開機後直接打開 Motion View —— **A**：跟 golden 一樣，手臂圖不動，要先勾「Simulte Enable」；**B**：一打開就跟著動（現在的樣子）。 | **B（保持現在的樣子）**：可逆，而且你 19:1x 問的正是「會不會跟著動」。你選 A 我再加勾選框 | `web/page/ht9045_mv_motor.js` 檔頭 |
| 1b | **HT9050 馬達表的軟體極限全是 ±999999（占位值）——武裝引擎馬達走 1203 之前要填真的值** | 這台 `system\Mot_Table.csv` 跟 repo 的 `machines/HT9050/Mot_Table.csv` 一樣：**19 個啟用軸的 SoftLimitN／P 全是 -999999／999999**。golden 拿它做好幾件事：MotorMove 超限就拒絕、Motor Test 的「移到極限」、路由寫到 1203 卡上的軟體極限，還有出料臂讓位的位置——`aoutarm.cpp:902` 讓位 X＝SoftLimitN＋iOutArmXBase×2000＋100，SIM 量到出料臂真的跑到 **-995899**（St02 21:43 問的就是這個）。引擎路由現在是關的（馬達命令到不了卡），所以今天不會動；**一旦武裝，出料臂讓位會往 -995899 跑、而且沒有軟體極限擋**。<br>例：Fix 盤滿要讓位 → golden 算出 -995899 → 真機一路撞到硬體極限。 | **先不動表**（那是機台設定，要 EastSun 量實際行程）；我在 GitHub README 最上面的機台通知加一行，請 EastSun 上機武裝前填好。你要換別的做法（例如先填保守值）再跟我說 | `machines/HT9050/Mot_Table.csv`；`aoutarm.cpp:902` |
| ~~1c~~ | ~~**引擎馬達走 1203：停止指令送不出去時，要不要照 golden 跳 WAR16122 警報**…~~ | ✅ **已決定**：你 0930 13:5x：A（只記錄＋畫面看得到，不另跳警報；第 6 條） | — | `RULINGS_20260930.md` |
| ~~1d~~ | ~~**INBOX 115：HIGH 那 92 個 `#if 0` 裡，有 10 個相依已經都在、但碰到安全面，要不要照 g…~~ | ✅ **已決定**：你 0930 13:5x：**全部開**（第 4 條）——agent 照分級文件逐項照 golden 解中 | — | `RULINGS_20260930.md` |
| 2 | **GitLab 上的 MR !7 要在網頁上按「Close」** | St01 和筆電都沒有 API 權杖，只能用網頁關；它已經被筆電的直接合併取代 | 放著不影響任何東西 | `gitlab.honprec.com/.../ht9045/-/merge_requests/7` |
| 4 | **機台端 session 不在線** —— 「引擎馬達走 1203 改由筆電做、機台不要改那 5 個檔」只寫在 GitHub README 最上面 | 如果機台上今晚有人在開發，最好口頭說一聲 | — | GitHub README 最上面 |
| ~~5~~ | ~~**主畫面 RESET 鈕與 Site 格點擊要不要接上 golden 動作**（FLOW-4 做完另外三顆，這兩個刻意…~~ | ✅ **已決定**：你 0930 13:5x：A（維持不接；第 5 條） | — | `RULINGS_20260930.md` |
| ~~6~~ | ~~HT9050 的 Loader／Empty／Auto1～3 升降 Z 軸：馬達還是氣缸~~ | ✅ **已決定**（你 0930 13:0x：「Z軸是氣缸，維持A」＝RULINGS_20260930 第 2 條）：不改設定，模擬裡那 5 個軸不動是對的。馬達表那 5 列 `Enable=1` 跟「是氣缸」不一致，寫進 GitHub README 請 EastSun 看 | — | `RULINGS_20260930.md` 第 2 條 |

> 已經照安全預設做、不用你回的：St01 的 review6 原本先不合（St02 20:24 審出 2 個機台安全的 major）；**St01 01:19 兩個都修好了**（頁面消失只放開還按著的 jog、絕不送 STOP；Exit 關機過程讓 Light Scale 的停止通過），02:4x 已合進 main（取 St01 gate 過的 `1e5316eb`，後面新加的 B5 重新登入等 St01 自己 gate 完再說）。

---

## §1 做完了（0929 下午 → 0930）

| 時間 | 事項 | 證據 |
|---|---|---|
| 0930 12:4x | **第二批上 GitLab main（`b21ca17e`）＝GitHub 第 85 包**：① **INBOX 118 馬達撞到（JAM）時照 golden 停機**（golden `note.cpp:1052-1169` `ShowMotorErrorMessage`，`forms/fNote_ShowError.cpp` 檔尾）：停所有馬達、Galil ST、Index 煞車關，再跳警報（走既有的 kcode==0 通知路徑，不擋主迴圈），**框送出去就立刻暫停**（你 11:4x 裁決「撞機後立刻暫停，照你的預設做」＝RULINGS_20260930 第 1 條）；閘住 3 行缺相依的（ShowErrorUnit、FTP 存 Jam 檔、瑞薩 FT-CT）；新 ctest `NoteMotorError`。⚠ 警報框目前還關不掉（Jerry J-5 → INBOX 119 進行中）。② Jerry J-1／J-2／J-4（INBOX 120 ✅）。③ 回 St01 B8 的 6 題（其中 (6) 發現他的評估用的是 Steven01 開發機的設定，HT9050 這台沒有 OTD／E84／讀碼器／RTC／ATC）。④ CLAUDE.md 的 START 普查數字更正成 34／31／3。 | 全新 build dir 兩組態 gate：出貨 4 項＝基準、模擬 19 項＝基準（各 256 個測試，沒有多也沒有少）；執行期資料夾 0／0／0。GitHub 第 85 包 `8b4a7a9`（17 檔，掃描 0 處；README 機台通知改成「118 已在第 85 包、119 進行中」） |
| 0930 12:1x | **常溫模擬卡住的根因找到了（INBOX 111 的驗收）**：11:00～11:15 用模擬跑 S1（Initial Start→HOME→Lot Start→START），跟昨晚一樣，START 後 2 分鐘除了 Tray 臂全部停住。原因：golden 開機時把約 50 個料盤馬達「綁到畫面元件」（`SetHTrayPanel`），這個綁定的副作用是打開「盤面資料跟著補盤更新」的開關；移植樹把整段當成「只是畫面」用 `#if 0 // GATE n5-G3` 關掉 ⇒ **Loader 每補一盤，盤子是有了、格子卻全是空的**：入料臂以為沒 IC 可取，一直等；Tray 臂以為這盤取完了，搬走再換一盤，無限循環。直接量到：11:10:25 的快照 `MMTrayY.fHasTray=true` 同時 `Tray.HasIC()=false`；18 次補盤、18 次都被當成空盤搬走。golden 真機紀錄（2025-12-11）補盤後是 `HasIC()=true`。**模擬與真機組態都有這個問題**（golden 是無條件綁定）。 | workflow 5 個 agent（兩路追蹤、一路綜合、兩路反向驗證都駁不倒）；報告 `docs/AMBIENT_STALL_HTRAY_20260930.md`（跟修正一起 commit）；修正＝INBOX 121 進行中 |
| 0930 11:0x | **四批上 GitLab main（`e2dac07e`）＝GitHub 第 84 包**（上一輪 0930 03:5x～07:0x 在三棵 worktree 做完、各自 gate 過，`/clear` 之後我接手合成一批再 gate 一次）：<br>① **FLOW-4（INBOX 109）**：主畫面 **ONE CYCLE／TRAY FEED／ALARM RESET 三顆鈕照 golden 真的會做事**（golden 906 `main.cpp:4332-4380`／`:13944-13947`／`:22159-22166`，本體在 `cCleanOut.cpp`；wb_serve 主迴圈每一圈取 St01 的按鈕事件、按一下跑一次）。原本呼叫空殼的引擎路徑（Auto Clean 前的 one cycle、低良率、GPIB／SECS ONE_CYCLE、SECS TRAY_FEED 約 20 處）也跟著照 golden 走。RESET 與 Site 格沒接 ⇒ §0 第 5 題。<br>② **AMB-L3（INBOX 111 第二層收尾）**：入料臂最後 8 個 `{}` 空殼換成 golden 本體（`ainarm9045.cpp` 檔尾，golden `:2231-2525`、`:4175-4191`）。最明顯的差別：**Auto Skip 次數到了會照 golden 放棄 Loader 盤**（之前計數器永遠不動）。<br>③ **INDEXZ-1203 重做＋第二輪審查修正（INBOX 113），開關 `WB_ENGINE_INDEXZ_1203` 維持關**：暫停後的恢復移動只剩 golden 那一發（DoSystem 門與電源檢查過之後的 VS/SP，G22 照 golden 解）、DP 對位照 golden、安裝器移到 1203 監控器建立之後、回原點被停住算失敗、操作員停止與警報掃描的停止分開；31 個突變全被測試抓到。開關關著時唯一的行為差異是 G22：每次 START 後送一發 `VS…;SP…` 給 Galil 層（沒有 Galil 卡＝不做事）、運轉中每輪清手動吸嘴旗標（今天沒有讀者）。**武裝前還差 INBOX 118**。<br>④ **St02 S-10 Tray Edit 頁**（golden `uTrayEditForm` 逐行＋`act.trayEdit`＋`HW.TrayEdit`；12 處直接讀 `TrayEditForm->fShow` 附理由列入基準，St01 三個條件都滿足）＋S-09 第一批剩下兩個（bthermo 常數副本、`btnClearCountClick(NULL)`）。<br>⑤ 你早上的「上傳週報」commit `4b7300ec`（只有 `.claude/commands`、CLAUDE.md、代理檔；不進 GitHub 包）。 | 合併樹 `e2dac07e`，**全新 build dir 兩組態 gate**：出貨 ＝4 項基準（config_db、ini_helpers、config_loaders、GA1_ReadGeneralIni；255 個測試）；模擬 ＝19 項基準（`PTW1_MyStringList` 啟動偶發 Not Run 0 秒，單獨重跑 0.6 秒通過）。三批合併前各自的 gate 也都＝基準（`iz3\gate_*2.log`、`aml3\after_*.log`＋單獨重跑、`flow4\gate_*.log`）。執行期資料夾（system、config、IniData，2386 檔）gate 前後比對＋還原後 0 差異。衝突只有 3 處（`CMakeLists.txt` 來源清單一行、`tests/CMakeLists.txt` 兩邊各自附加的測試、INBOX 第 107～110 列），逐列保留兩邊。GitHub 第 84 包 `c6280e6`（48 檔；README 最上面加了 INDEXZ／INBOX 118 的機台通知） |
| 0930 11:0x | 回 Steven（TO_STEVEN §4）：St02 near-miss 認領 **Q1 的 11 列同意**（Command／uHGem／cObserver／fLotInfo，相依都在、逐行對 golden；1121 的 `INDEXCYCLETIME?` 會開始回真的 index cycle time＝golden 行為）；**Q3 fConfiguration 選 A**（不建全域，寫進頁面真正用的 FileRW 編輯表；FileRW 兩支是 St01 的，要 ST01-E 同意）；提醒 St01 主畫面三顆鈕已經接上、他網頁的「實際動作還沒接」提示字要改 | 同一顆 commit |
| 0930 11:0x | **回 Jerry（新開 `docs/handoff/TO_JERRY.md`）**：他 0929 18:24 推了 5 項回報（`v906/jerry-handoff`；他能推公司 GitLab、測的是 main `aafa3953`／`3a93a28f` ⇒ 原本 §0 第 3 題已經不用問），**夜間迴圈當時沒讀他的分支，漏了 16 小時**，已補進 `night-loop` 技能的每輪讀取清單。J-1 輪詢閘門同意合（下一批 gate）、J-2／J-4 筆電修（INBOX 120）、J-3 在筆電這台會過（他那台環境）、**J-5「kcode==0 的通知框關不掉、馬達已停，只能重開程式」排成 INBOX 119，緊接在 118 後面**（118 之後每次撞機都會碰到） | 同一顆 commit |
| 03:3x | **INBOX 111b 兩個後續上 main（照 golden）**：① 出料臂 `DoOutArmSuckPreOn`（golden `aoutarm9045.cpp:678-706`）——實機＋「下降時吸」打開＋沒按手動單步時，**出料臂下降途中離目標 50 脈波內就先開真空**（之前要等 Z 到位才開，比 golden 晚）；② 入料臂三個過期閘（golden `ainarm9045.cpp:5606`／`:5768`／`:6841`）——每次取料前照 golden 重算 X 方向 pitch（之前沿用上次的值）。④ 2DID 順序檢查**沒解**：寫那個步驟計數器的是 golden `cContact.cpp:20401-20787`（整段檢查流程還沒翻），只解這一段是半套；它是 F33 選配功能，排後面。 | `30736a76`；兩組態 gate＝基準（出貨 4、模擬 19，第一次 gate 我漏了一個 include 編不過，補上重跑）；執行期資料夾 0／0／0；GitHub 第 83 包 `3d9ab3b` |
| 03:2x | 回 St02 兩題（S-09 第一批剩下的）：bthermo 那份常數副本改用 `forms/fTemp_Set.h`（定義 `uTemp_Set.cpp:228-244` 的 0～16 跟副本逐一相同）、`btnClearCountClick(NULL)`（本體只拿 Sender 比對那顆按鈕 ⇒ 跟 golden 一樣走自動清除）都同意 | `da4ec373` |
| 02:4x | **St01 review6（到 `1e5316eb`）＋St02 S-09（`53868b63`）上 main**（188 檔）。review6：網頁視窗關掉／縮小時放開還按著的 jog（**不會送 STOP**）、Exit 關機時讓 Light Scale 的停止通過、14 個設定視窗關閉照 golden FormClose、D-012 關機步驟、無畫面的停止警報送 MES16441。S-09：`cBinSel.cpp` 50 個寫檔 gate 照 golden 解開（GPIB `SETOSBIN_`、遠端命令、開機自動設 ContFail 這些路徑會寫 Bin 設定檔；網頁 BinSelect 存檔本來就會寫）、32 個過期 gate 照 golden（遠端命令回覆、SECS SV／EC 登錄、紀錄檔寫入）、`SaveASECLTestLogInfo`／`SetOEEState` 照 golden。合進來後出貨組態多紅 2 個、模擬多紅 1 個，**都是測試跟不上解閘，不是行為錯**：`Command.cpp:9889` 兩個裸讀改包 `W906_FormShowing`（跟 `ckernel.cpp:1974` 同寫法）；SecsCatalogue 的 741→769、219→229 逐筆對過 golden 四個區塊（+28 筆 SV，其中 10 筆 TObject*）。St02 02:37 說要自己修同兩處，我 02:3x 先推一列交接請他不要重做。 | 修正 `527ce723`；兩組態 gate：出貨＝4 項基準（HSys_HeaterMix 負載逾時、單獨重跑過）、模擬＝19 項基準，兩個修過的測試兩組態單獨重跑都過；執行期資料夾（system、config、IniData）對備份 0／0／0；St01 代理 gate 結果相同。GitLab `910892e4`（review6 開 MR !10、gpib-widget 是 MR !6）；GitHub 第 82 包 `9bcfea5`（147 檔） |
| 01:1x | INBOX 115 A 類 4 個上 main | GitLab `7a4aea10`；GitHub 第 81 包 `e4f0f29` |
| 00:5x | **INBOX 112「引擎馬達走 1203」進 main，開關 `WB_ENGINE_MOTOR_1203` 預設關**（你 18:5x 的決定）。關著時跟之前行為一模一樣（複審逐項確認：沒裝路由時新程式一律直接回去、不改任何狀態）；打開後引擎的馬達命令（移動、停止、回原點、速度、軟體極限）會經過新的路由送到 1203 卡。做了三路對抗式審查＋一次修正＋一次複審：Motor Test／警報路徑直接送出的停止現在路由看得到、不會把中途被停的移動記成到位；停止失敗每軸鎖存；23 個突變全被測試抓到；新測試兩組態都過（EcatMotorRoute 143／143、Pci1203MotorRoute 218／218）。**武裝前還要處理的 4 件**寫在設計書 §9.4（最重要的：停止失敗目前畫面看不到，見 §0 第 1c 題）。 | `5bffea92`、`4cc243e9`、`d84d6e74`（l2e 的 f5341505／d13494b9／541fa7e7）；兩組態 gate＝基準（245 個測試）；`pci1203_control_gate.ps1` 只剩原本的 2 個 FAIL、確認開關沒開；GitLab `e8dda454`；GitHub 第 80 包 `19ec3bc`（README 機台通知已改成「已進 main、預設關」） |
| 00:2x | St02 Motion View 自審修正（`a539e0f6`：飛梭沒讀到位置時不飛出畫面、手臂讀數顯示馬達脈波）上 main；回 St02 S-09 在筆電檔裡的認領（大部分同意，會改變操作行為的 4 類先不解） | GitLab `8f9edf73`；GitHub 第 79 包 `d7a58d2`；交接 `0f6ded24` |
| 23:5x | **St02 INBOX 117 第二步上 main**：Motion View 的手臂位置改照 golden `SetScreenScale` 的兩個教點換算（C++ 發 `motionView.screenScale`；`mymotor.h:156` 一個唯讀 getter，筆電 21:5x 同意）。筆電的 `ht9045_mv_motor.js` 繼續餵位置，換算交給 St02 的 `liveMech()` | merge `06505b34`；兩組態 gate 剛好＝基準；GitLab `3a26d332`；GitHub 第 78 包 `d6368dd` |
| 23:0x | **INBOX 114 PumpInit 寫死的機台設定全部拿掉**（12 個：`USE_OUT_SORT_ARM`、`AUTO_EMPTY_COLOR`、`AUTO3_IS_MAGAZINE`、`TRAY_VIBRATION`、`SUPPORT_2_EMPTY_EMPTY`、`bUseAuto2Empty`、`TrayForm.bEnableAMR`，和 5 個 golden 從不載入的執行期旗標）。golden 沒有這種寫死；拿掉後改用 Gerneral.ini／配方讀到的值。**量過行為不變**：這台的 `Gerneral.ini` 就是 HT9050 機台那份（MD5 相同；「HT-9045W」只是那份檔的型號字串，所以 INBOX 原本說「筆電設定會變」是過期的前提），模擬開機印出的實際值 `USE_OUT_SORT_ARM=0 AUTO_EMPTY_COLOR=0 … bEnableAMR=0` 跟原本寫死的一模一樣。`bEnableAMR` 不再被強制關掉 ⇒ golden 的 AMR 對接互鎖恢復。 | `b8d6c956`、`33052919`（開機訊息第一版掉進註解，見 §6）；兩組態 gate 剛好＝基準；GitLab `283c4567`；GitHub 第 77 包 `44b793b`（README 機台通知加了軟體極限那一行） |
| 21:4x | **St02 MR !6 第三批上 main**：S72 尾巴（golden `SetLotStart` 的 `slEventLog->SetLotData` 照跑）、S-12 模擬組態的加熱執行緒（`HeaterSimTick.cpp`，只在 SOFT_SIMULTE 下；你 16:1x 選 A）、S-09 `cprod.cpp:3009` 閘退役 | merge `927740a5`；兩組態 gate 剛好＝基準（出貨 4、模擬 19，243 個測試）；GitLab `d40fa5a0`；GitHub 第 76 包 `e72da27` |
| 21:1x | **INBOX 117 Motor View／Motion View 畫面驗證**（你 19:1x：「確認 motor view 畫面是否能正常顯示並且更新數據」「確認 motion view 畫面是否能因為 motor 位置不同而更新內容」「可以透過今天跑模擬的方式驗證」）。**兩個都原本不行，都改好、都用模擬驗過。**<br>① **Motor View 原本數字不會動**：頁面只在開頁時讀一次 `JSON/Motor-runtime.json`，C++ 根本沒在寫這個檔（`D:\HT9045\web\JSON` 那份是 09-02 的快照，provider「TBD」）。改成讀 C++ 的即時值（`/api/struct/motor/runtime`），每 500 ms 更新一次（跟 Motor Test 頁一樣）。<br>② **Motor View 的 Current 欄有幾軸一直是 0**：原本優先顯示 encoder，模擬時 `MOutArmX` 的命令位置是 -995899、encoder 是 0，畫面就一直是 0（`MOutArmY`、`MTestZ1` 也一樣）。改成照 golden `UpdateMotorScreen`（`main.cpp:8363-8384`）：顯示命令位置，只有 4 個光學尺軸跟 Galil 的 Index 軸才用 encoder。<br>③ **Motion View 原本手臂完全不動**：頁面其實有畫手臂位置的程式（`liveMech()`），但沒有人把馬達位置餵給它（St02 的 `ht9045_mv_trays.js` 只送托盤格子）。新檔 `web/page/ht9045_mv_motor.js` 每 500 ms 把命令位置餵進去。<br>**實測數據**：模擬 S1 流程（HOME→Lot Start→START，20:53～21:01）API 的位置一直在變（`MInArmX` 0→30732→4203→35399→5326；`MTrayX` 0→39662→61662→96912→4562）；改之前 Motion View 整段 SVG 從頭到尾一模一樣。頁面層級的直接證據（不重新開頁，只改 API 回應）：Motor View 表格 0/0 → 11111/22222 → 33333/44444；Motion View `MInArmX`=0 → 30000，SVG 有 28 行不同（例：入料臂橫樑 x 69.1→219.1），改回 0 又一模一樣。<br>⚠ **這台筆電的 `Gerneral.ini` 機種是 `HT-9045W`**，所以 HMI 開的是 HT9045 版的 Motion View；HT9050 版（`Main.MotionView9050.html`）目前沒有任何即時資料來源（頁面自己寫「wb_serve 無 producer」），它的手臂圖也還不會動——要做就是另一件事（交給 St02，見 §2）。 | `9c0c3209`、`4f1a080d`、`4de9813e`；量測資料 `%TEMP%\claude\…\scratchpad\mv117\`（api.jsonl 233 筆、pages.jsonl、page_probe2.json） |
| 21:1x | **INBOX 111 常溫接線第二層（四組）上 main**：<br>A 出料臂 `PCIL112_OutArmXYMove` 照 golden、`MotorMoveShuttleShake` 生效；B `GetOutShuttleStatus_9045` 照 golden；C 入料臂 4 個過期閘退役、`InArmSuckReset`、Clean Out 收尾、`DoInArm_SuckerMap`／`DoSiteMappingResult` 真本體；**D 整機退料 `DoTrayFeedProcess` 第一次接上**＋`TfMain::InitialTrayFeedTask`。<br>審查抓到的都處理了：DoTrayFeed 裡 AMR 等待（G21）與 P53 的 MESxx24（G22/G23）三個閘理由過期 → 照 golden 打開；`SYN_TEK_MOTION_MODULE` **刻意維持 0**（golden 預設 0xA7 會讓 1203 機台走 M204 latch 模式，但 latch 感測器在移植樹還是離線替身，出料飛梭殘料判斷會讀到空資料）；`GetInArm2DIDMapping` 暫停呼叫（本體 85% 還在閘裡）。沒做的列在 INBOX 111b。 | `1b726051`、`78a4e58f`、`6c0c27e9`、`290ecfa8`、`d707083d`、`40386775`；gate：出貨＝4 項基準、模擬＝19 項基準（3 項負載逾時／exe 被拿走，單獨重跑都過） |
| 20:4x | 回 St02：`SetOEEState` 放 St02 自己的 `LogObjects.cpp` 可以；review6 暫不合（見 §0 註） | `533b7c8d` |
| 19:5x | 回 St02：S-10 Tray Edit 筆電那 3 處（CMakeLists／wb_serve 分派／替身 Close）同意 | `5d27de65` |
| 14:24～19:14 | S-08 伺服器端、St02 MR !6（兩批）、WSLINK-B、原生畫面（OFF）、機台 OPLOG／SR-WIRE、St01 MR !8、常溫接線靜態盤點（79 列） | 見 §5 |
| 18:5x | **`WB_ENGINE_MOTOR_1203` 維持預設關、上機再開**（你的決定，RULINGS_20260929 第 14 條） | — |
| 18:3x | **0018 TOKEN-OFF 不收**（你的決定，RULINGS_20260929 第 13 條） | INBOX 116 結案 |

---

## §2 佇列（照 INBOX 111→117）

| 順序 | 項目 | 狀態 |
|---|---|---|
| ✅ | INBOX 112 引擎馬達走 1203 | 00:5x 上 main（`e8dda454`，開關關著） |
| ✅ | INBOX 113 INDEXZ-1203 重做（Index Z1） | 0930 11:0x 上 main（`e2dac07e`，開關關著）；**武裝前還差 INBOX 118** |
| ✅ | INBOX 114 PumpInit 寫死的機台設定 | 23:0x 上 main（`283c4567`） |
| 4 | INBOX 115 HIGH 那 92 個 `#if 0` | 🔧 01:0x A 類 4 個改好（`f614db40`：`mymotor.h` 外觀、`myMN200motor.cpp` 的訊息框判斷〔只在出貨組態編進去〕、`csystem.cpp` 汽缸自我測試讀資料＋畫格子），兩組態 gate 在跑；B 10 個在 §0 第 1d 題 |
| — | INBOX 111b（第二層審查留下的 8 項後續） | ①② 03:3x 上 main；④ 要連 golden 的 2DID 檢查流程一起翻（F33 選配，排後面）；⑥ 註解已改、全樹行號表等 CPU 空了重產；⑦ `fMain->Pause`／`SetLotState`／`WriteIniData` 屬 START/PAUSE 與客戶碼功能，照規則排最後；③⑤⑧ 不變 |
| — | **HT9050 版 Motion View 接即時資料**（`Main.MotionView9050.html`：沒有 producer、軸讀數表的值固定「–」） | 已在 TO_STEVEN §4 21:0x 告知 St02；St02 也被請看 `liveMech()` 的 `/100` 換算（模擬時 `MOutArmX`=-995899 會畫到畫面外） |
| — | Kevin K-01（golden 模擬錄 State Record，常溫、FT005054_9050）／K-02（9050 Index BCB6 流程） | Kevin 18:04 認領 K-01，之後沒有新推送（21:3x 量：3.5 小時；K-01 沒擋到筆電的工作，所以不收回） |
| — | St02 S-10 Tray Edit（會一起帶 St01 的 `e6e537c2`）、S-12（加熱，只接模擬） | St02 在做；(A) SetOEEState／SaveASECLTestLogInfo 已在 02:4x 那批上 main |
| — | St01 review6 後段（`2829694e`：B5 重新登入 Q45「甲」、hub m11／m13） | St01 說自己 gate 完才可以合（02:40～04:00） |
| ✅ | St02 S-10 Tray Edit | 0930 11:0x 上 main（`e2dac07e`）；St02 要的 **T1 重播**（補 golden PAUSE 之後那段 Tray Edit）還沒做：重播會換入／換回真實 system／config，排在 CPU 空下來之後 |
| 🟡 | INBOX 109 FLOW-4 | 0930 11:0x 三顆鈕上 main（`e2dac07e`）；RESET 與 Site 格 ⇒ §0 第 5 題 |
| ✅ | INBOX 118 馬達撞到時照 golden 停機＋警報 | 0930 12:4x 上 main（`b21ca17e`）；撞機後立刻暫停（RULINGS_20260930 第 1 條） |
| 🔧 | **INBOX 119 通知型告警框關不掉**（Jerry J-5） | 11:5x 開工（worktree l2k）：C++ 新指令 `dialog.notifyAck`＋ctest；網頁那一半（`ht9045_dialog_host.js`）等 C++ 上 main 後把契約給 St01。**兩條 1203 路由武裝前的最後一件** |
| ✅ | INBOX 120 Jerry J-1／J-2／J-4 | 0930 12:4x 上 main（`b21ca17e`） |
| 🔧 | **INBOX 121 料盤馬達照 golden 綁定（常溫卡住的根因）** | 12:2x 開工（worktree l2m）：`SetHTrayPanel` 本體照 golden、`SetSimuScreenPara` 補 golden 那約 50 個綁定呼叫（畫面元件傳 NULL）、改正錯的閘理由、新 ctest＋突變測試；做完**重跑同一個模擬流程**，看入料臂會不會開始取料、出料那側會不會跟著動 |
| 🔜 | St02 near-miss 11 列（`232cb4e3`）＋St02 Contact 開 Tray Edit（`08e16cf1`） | 排進第三批（跟 119／121 一起 gate） |
| — | St01 review6 `42e6607e`（B5 重新登入、hub m11／m13、BarCode Exit、D-012 A3W、INBOX 108） | St01 兩組態 gate 都＝基準；**等 Steven 回 St01 的 Q59** 才會在 §2 放行，放行後筆電合 |
| — | **上機清單**（要人在機台旁）：G14 TCP_IP_MODE＋ON_LINE 的 SOT 有沒有真的送出；引擎路由第一次上機（EastSun 在旁）；一個瀏覽器一條連線；**這一包的出料臂 XY、整機退料、Out Shuttle 殘料判斷** | 等上機 |

---

## §3 紅燈／哨兵

- **0930 11:0x 哨兵**：配方數 66 個目錄／65 份有 Contact.Data（＝基準）；控制字元：這一批改到的 54 個檔 0 處；接線 `check_deployed.py --selftest` 通過。
- **0930 12:4x 哨兵**：兩組態 gate＝基準；執行期資料夾 0／0／0；GitHub 包掃描 0 處。
- **03:1x 配方數哨兵：66 個目錄／65 份有 Contact.Data**（基準 65／64）——多的那個是 `FT005054`（0928 10:13 建立，就是 HT9050 模擬用的工單），預期中；0929 pagewire 在它加進來之後量過，沒有新的接線回歸。新基準 66／65。
- **03:1x 控制字元哨兵**：我們今晚寫的檔 0 處；命中的是產生的手冊（NUL）、技能裡從 PDF 抽出來的參考檔、以及三棵樹都有的 `atester_ProcessCount.cpp`（golden 帶來的），都是舊的。
- **接線哨兵（pagewire `check_deployed.py`）23:5x：自我測試通過；實跑 14 項紅，跟 INBOX 102 記的一樣**（`sync_web.py` 的 OURS 清單 0922 之後沒更新，St01／St02 0925～0928 加的接線 js 都不在裡面；client repo 已凍結，沒人在跑同步）⇒ 今晚沒有新增的接線回歸，筆電新加的 `ht9045_mv_motor.js` 沒被標。
- **23:1x～23:50 額度用完**：112 複審與 115 分級兩個 agent 被中斷（沒有產出），23:5x 重派；gate 是背景程序，照跑完。
- **機器負載造成的 ctest 逾時**（不是回歸，都有重跑證據）：21:0x 那次 `TesterComm_TcpCmdServer`（146 秒逾時，單獨 16.5 秒過）、`HSys_HeaterMix`（119 秒逾時，單獨 14.9 秒過）。
- **`IniFiles_Win32Diff` 在模擬組態「沒啟動」**：`build_sim` 裡那支 exe 剛連結完就不見了（Process not started／operation not permitted），像是被防毒拿走；重新連結後單獨跑 18 秒通過。出貨組態那支一直都在。再發生就去看 Defender 的隔離紀錄。
- **無頭瀏覽器的 `document.hidden` 是 true**：Motor View、Motor Test、Motion View 的輪詢在「看不到的視窗」會停（這是對的），所以用無頭瀏覽器驗證頁面輪詢時，要先告訴頁面它是看得到的，不然會量到假的「沒更新」。

## §4 清理與還原

- 21:0x 跑了 3 次模擬 wb_serve（S1 流程一次、頁面短測兩次，第一次短測因為 tag 撞名沒啟動）。每次開關機都會補寫 `Gerneral.ini`、`config.ini`、`lastdata.dat`、`machinerecord.dat`、工單 FT005054 的 6 個檔、換 MD5 檔 ⇒ `flow_run.py` 照 sysguard 快照 `ht9050s1b_0929` 逐檔還原、log 與 StateRecord 新檔移走；**最後 sysguard 0／0／0**。

## §5 commit／push 清單

| GitLab main | GitHub 包 | 內容 |
|---|---|---|
| `4a4040ce` | 69 `ba3b57b` | S-08 伺服器端、ST02 工作卡 S-09～S-11 |
| `a84d25cc` | 70 `60afc75` | St02 MR !6 |
| `3a93a28f` | 71 `9227de3` | WSLINK-B、原生畫面（OFF）、測試修正 |
| `180657c7` | 72 `7a708de` | 機台 OPLOG／SR-WIRE、INBOX 111～116、TO_KEVIN、機台通知 |
| `052194b2` | 74 `7153246` | St02 MR !6 新一批＋St01 MR !8 |
| `5d27de65`、`533b7c8d` | （只有交接檔，併進 75） | 回 St02 的兩列 |
| `64d8554e` | 75 `320a0c5` | INBOX 111 四組＋INBOX 117 三顆（16 檔；引擎走 1203 不在這包） |
| `d40fa5a0` | 76 `e72da27` | St02 MR !6 第三批（11 檔） |
| `283c4567` | 77 `44b793b` | INBOX 114、111b 註解、交接檔（7 檔） |
| `3a26d332` | 78 `d6368dd` | St02 INBOX 117 Motion View 換算（5 檔） |
| `8f9edf73` | 79 `d7a58d2` | St02 Motion View 自審修正（4 檔） |
| `e8dda454` | 80 `19ec3bc` | INBOX 112 引擎馬達走 1203（開關關著，18 檔） |
| `7a4aea10` | 81 `e4f0f29` | INBOX 115 A 類 4 個 |
| `910892e4` | 82 `9bcfea5` | St01 review6 到 `1e5316eb`＋St02 S-09 `53868b63`＋兩個測試修正（147 檔） |
| `16aafc23`、`da4ec373` | （只有文件，併進 83） | INBOX 狀態更新＋FLOW-4 認領、回 St02 兩題 |
| `30736a76` | 83 `3d9ab3b` | INBOX 111b ①②（2 檔） |
| `e2dac07e` | 84 `c6280e6` | FLOW-4、AMB-L3、INDEXZ-1203（關）、St02 S-10＋S-09 剩下兩個、交接檔（48 檔）；另含你的「上傳週報」commit（不在包裡） |
| `b21ca17e` | 85 `8b4a7a9` | INBOX 118、Jerry J-1／J-2／J-4、裁決 0930 第 1 條、INBOX／報告（17 檔；交接檔與 CLAUDE.md 不在包裡，包只帶移植樹與 web） |

## §6 我自己犯的錯與更正

- **03:0x 用 `sed -i` 改了 INBOX 一個字，整份工作副本從 CRLF 變成 LF**（INDEXZ agent 剛提醒過同一個坑）：git 存的本來就是 LF，所以 commit 沒被影響；工作副本已改回 CRLF。之後這類檔一律用 Python 以二進位讀寫。
- **02:5x 第一次 111b gate 編不過**：`DoOutArmSuckPreOn` 用到 `Sen[]`，但本檔 `mysensor.h` 要到 :2401 才 include；補在上面現成的空行（:80，行號不動）後重跑通過。
- **03:3x 第 83 包的密碼掃描一度跟複製同時跑**（平行下指令），結果可能看到半份資料夾；複製完重掃一次，12 個檔 0 處。
- **02:2x 執行期資料夾檢查一度回報「IniData 少了 1807 個檔」**：檔案其實都在。備份快照是連 IniData 一起拍的，我檢查時沒帶 `SYSGUARD_DIRS`，工具只走了 system／config，於是把快照裡的 IniData 全部當成消失。已改工具（`sysguard.py`：沒帶環境變數時照快照裡記錄的資料夾檢查；帶了但不一致會先印警告），重跑是 0／0／0。
- **22:2x INBOX 114 那行開機訊息，第一版根本沒編進去**：我把 `printf` 接在 `InitAllProcessTask();` 那一行**原本的行尾註解後面**，整段變成註解（記憶裡早就記過「同一行附加要插在第一個 `//` 之前」）。是用短的模擬 wb_serve 抓那行字抓不到才發現的；已移到註解前面，前處理輸出確認有那個字串，重跑兩組態 gate。今晚其他同一行附加逐一看過：程式碼都在註解前面（托盤的 `#if 1`），其餘本來就是註解行。
- **21:0x 我一度把「Motor View 會更新」當成證據**：那一輪的探針每次都重新開頁，數字會變是因為重讀，不是頁面自己在輪詢；而無頭瀏覽器的 `document.hidden` 是 true，頁面的輪詢根本沒跑。同一輪「Motion View 畫面變了」也不能歸功給轉接檔。改用「告訴頁面它看得到＋不重新開頁、只改 API 回應」重量一次，才拿到上面 §1 那組數據。
- **WSLINK-B 第一版的前提錯了**（14:2x）：以為關 Motor Test 分頁會斷線；已照各自視窗的 golden FormClose 重做，TO_STEVEN §4 已更正。
- 端到端驗證前兩次判「失敗」都是我的腳本錯（沒帶數字 id、讀了還在緩衝的 log）；我一度說「指令被擋」，其實 `allowCmd` 從 0918 起就恆為 true，已當場更正。
- GitHub 第 72 包 README 我先寫了一句「套完機台的權杖會恢復擋人」，推之前發現錯了才改掉。
