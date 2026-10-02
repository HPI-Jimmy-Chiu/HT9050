# 機台端工作紀錄（HT9050 機台，V906 移植樹）

> 這份是**機台端**（這台 HT9050 控制電腦上的 Claude 工作階段）做過的事的總紀錄，EastSun 20261001 要求：
> 「請你把做的詳細紀錄都記錄起來 記錄到哪裡跟我說」。
> 每一顆 patch 的長說明在 GitHub 的 `README.txt`（見 §0）；這份是索引＋那邊沒有的東西：
> **機台設定檔改了什麼（不進 git）**、每天的脈絡、還在等誰決定。
> 最後更新：2026-10-01 19:56（cpp 0070 之後）。新的一天往 §2 最上面加。

---

## 0. 東西放在哪裡

| 項目 | 位置 |
|---|---|
| **GitHub** | `https://github.com/HPI-Jimmy-Chiu/HT9050`，分支 **`machine/integ-ioweb`**（`main` 是筆電端的，機台端從來不推 `main`） |
| GitHub 分支裡的內容 | `cpp\`＝C++ 移植樹的 patch（0001～0055）、`web\`＝網頁的 patch（0001～0048）、`tools\`＝設計外掛的 patch（另一個工作階段負責）、`README.txt`＝每顆 patch 一段中文說明、`MANIFEST_MD5.tsv`＝全部檔案的 MD5 |
| 本機的推送資料夾 | `D:\HT9045\_push_github_20260926`（這個資料夾就是那個分支的 clone；只做 fast-forward 推送） |
| 機台 C++ 樹 | `D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，git 分支 `integ/ioweb-8484bdb4` |
| 機台網頁樹 | `D:\HT9045\_integ_ioweb\web`（自己的 git；部署到 `D:\HT9045\web`） |
| 筆電包的來源 | `D:\HT9045\_from_github`（GitHub `main` 的 clone，`updates\<GitLab hash>\` 一包一個資料夾） |
| 操作紀錄 | `D:\HT9045\_integ_ioweb\runcfg\logs\oplog_YYYYMMDD.txt`（每次開機、各軸、按鈕命令、煞車、被拒的原因）；開機取樣 `bootsample_*.txt` |
| F5 用的建置 | `build_integ_ship_x86`（**只有 F5 會編它**，機台端的 AI 不在這裡建置） |
| 驗證用的建置 | `build_integ_ship_x86_o2`（AI 改完先在這裡編、跑 ctest） |
| 本檔 | `HT9011UC_Cpp_V3.33.906.0\docs\WORKLOG_MACHINE.md`，也跟著 cpp patch 推上 GitHub |

---

## 1. 機台設定檔的改動（不進 git，只記在這裡）

機台設定檔（`IO_Table.csv`／`Mot_Table.csv`／`Gerneral.ini`／GPIB `general.ini`）**不推 GitHub**。改之前一律先備份。

| 日期 | 檔案 | 改了什麼 | 誰決定 | 改之前的備份 |
|---|---|---|---|---|
| 10-01 21:4x | `D:\HT9045\system\IO_Table.csv` | **吸／破對調回來＋沒有通道的 Enable=0**（EastSun「箭頭往下的在吸」「沒有通道的 都改enable=0」）：站 160／161／162 的 24 組 `*_On`／`*_Off` Port 改回 `_On`＝奇數（吸）、`_Off`＝偶數（破）＝20:29 之前的寫法；DO 在 24～31（VC4～7，VC4 沒有）的 12 顆吸嘴（InArm／OutArm 的 B／D／F／H、FTest AC／BC／AD／BD）感測／吸／破三列 Enable 1→0（36 列）。共 60 列，其他欄沒動 | EastSun 1001 | `D:\HT9045\_BACKUP_20261001_iotable_vc4_odd_enable0\IO_Table.csv` |
| 10-01 21:0x | `D:\HT9045\system\IO_Table.csv` | **VC4 吸嘴感測列**（EastSun「VC4 沒有的 用VC8 去推論」）：站 160／161／162 的感測列（IOType `Sucker`）照 VC8 推論改成 VC4 的真空 OK 位置——VC8 的 OK 位元在 16 個壓力 byte 之後（128+VC），VC4 只有 8 個壓力 byte → **64+VC**；VC＝那顆吸嘴的吸／破 DO 所在的 VC（EastSun 實測 VC n＝DO 16+2n／17+2n）。改了 12 列 Port：InArm A／C／E／G、OutArm G／E／C／A、FTest AA／BA／AB／BB → 64／65／66／67。OutArm 原本感測跟 DO 交叉，一律以 DO 為準。另外 12 顆（DO 24～31＝VC4～7）VC4 沒有通道，**沒動**。Bit、Enable 等其他欄都沒動 | EastSun 1001 | `D:\HT9045\_BACKUP_20261001_iotable_vc4_sensor\IO_Table.csv` |
| 10-01 20:29 | `D:\HT9045\system\IO_Table.csv` | **吸真空／破真空對調**（EastSun「你IO_table 真空和迫真空 點位寫反了 請交換」）：站 160／161／162 的 24 組 `*_On`／`*_Off` 只交換 **Port** 欄（例 `InArmSuckA_On` 17→16、`InArmSuckA_Off` 16→17），共 48 列；其他欄、其他站都沒動（逐列比對 48 列不同、檔案大小不變）。現在 `_On`＝偶數＝VC4 吸真空。⚠ 感測列（`Sucker`，Port 128～135）還是 VC8 寫法，沒動 | EastSun 1001 | `D:\HT9045\_BACKUP_20261001_iotable_swap_onoff\IO_Table.csv` |
| 10-01 15:05 | `D:\HT9045\system\IO_Table.csv` | （EastSun 自己改的，記錄用）`C_CleanPanel_On` 與 `C_CleanPanel_Off` 的 Port／Bit 對調（I17.1 ↔ I17.0）；其他列沒變 | EastSun | —（他自己改的；我改之前的版本在下一列的備份裡） |
| 10-01 | `D:\HT9045\system\IO_Table.csv` | 真空模組換站號：`InArmSuck*` IP 32→**160**（0xA0）、`OutArmSuck*` 48→**161**（0xA1）、`FTestSuck` AA～AD／BA～BD 64→**162**（0xA2）；Sucker／Sucker_On／Sucker_Off 共 **72 列**，這 72 列 **Enable 0→1**。其他格一格都沒動（逐格比對過）。曾短暫加過 3 列 `IOType=Vacuum`，EastSun 要求刪掉，已刪 | EastSun 1001 | `D:\HT9045\_BACKUP_20261001_vc8ip\IO_Table.csv`（改之前）、`IO_Table_after_ip_before_enable.csv`（改 IP 之後、改 Enable 之前） |
| 09-30 | `D:\HT9045\system\Gerneral.ini` | `[System] VacuUnitType` 0→**1**（打開 Vacuum Unit） | EastSun 0930 | `D:\HT9045\_BACKUP_20260930_vacuunit\Gerneral.ini` |
| 09-30 | `D:\GPIB9045\system\general.ini` | `[Version] Model` 9046_32GPIB→**9050GPIB** | EastSun 0930 | `D:\HT9045\_BACKUP_20260930_gpib_model\general.ini` |
| 09-30 | `D:\HT9045\system\Mot_Table.csv`（＋repo 的 `machines\HT9050\Mot_Table.csv`） | 入料飛梭別名 `MInShutte1／2`→`MInShuttle1／2`（配合 cpp 0036） | 0930 拼字修正 | `D:\HT9045\_BACKUP_20260930_shuttle_spelling\` |
| 09-25 | `IO_Table.csv`、`Mot_Table.csv` | 1203 EtherCAT 設定對齊：IO_Table 裡 ISABase=3 的列 Lane 0→1；Mot_Table 7 列（M11／M17／M18／M19／M20／M22／M42）的 BoardID／Port | 0925 現場 | `D:\HT9045\_BACKUP_20260925_103314_pre_1203_cfg\`（596 檔＋MD5 清單） |

> ⓘ EastSun 自己也會改 `Mot_Table.csv`（例如 CardModel 全部 PCI1203、SensorType；Motor Test 的「回寫 Mot_Table」會自動留
> `.bak_YYYYMMDD_HHMMSS`）——那些不在上表。
> ⓘ 每次跑 ctest 之前另有整包備份（`_BACKUP_20260926_*`、`_BACKUP_20260928_pre_pkg59`、`_BACKUP_20260929_pre_pkg67`），
> 加上 `tools\production_audit.ps1` 前後比對 MD5；到今天為止每一輪都是「機台成品 0 變更」。

---

## 2. 每天做了什麼

### 2026-10-01

| # | 做了什麼 | GitHub patch | 結果／備註 |
|---|---|---|---|
| 1 | **F5 剛按完 Exit 就失敗**：舊的 wb_serve 還在關站，F5 第一步就擋下。改成最多等 30 秒讓它關完 | cpp 0047（3eab632） | 只看不關任何程序 |
| 2 | **F5 的 preLaunchTask 錯誤（「圖片錯誤」）**：真正原因是設計外掛存了一半的事件（CMakeLists 指到不存在的 `HtdEvents.gen.cpp`）。EastSun 選「復原」——那 4 個檔還原成 commit 版本 | （沒有 commit，還原而已） | F5 恢復正常 |
| 3 | **IO 元件照舊版接法**：任何頁面的 IO 燈／按鈕用 Alias 對 IO 表；舊版格式提示「(Lane,IP,Port,Bit) Alias」；要先關安全門的按鈕畫成橘／黃；Index 上有 IC 時 TestSuck 走 golden 的 Suck()／Destroy() | web 0045～0047、cpp 0048 | 離線測試 17/17 |
| 4 | **整合筆電第 86～99 包** | cpp 0049（f0008c6）、web 0048（f059d13） | 衝突逐一解；合併多出的一行重複定義刪掉；第 86 包要機台改 Mot_Table 那項**沒套**（等 EastSun） |
| 5 | **HT9050 的原點判斷改用 1203 原點訊號**：EastSun「home 是 high 代表沒偵測到、low 代表有偵測到」「所有動作如果有馬達判定 home 的都要用 9050 1203 來判定」。HT9050（Model=9050GPIB）＋PCI1203 軸：ORG 位元 **LOW＝在原點**，SensorType 不參與。Teach 的 Z 軸檢查（那個「Please let InArm Z at home position first!!」彈窗）、HOME 燈、引擎所有「馬達在原點」的判斷、回原點流程都走這條 | cpp 0050（b3c612a）、cpp 0051（6b70166） | ctest WebMotorAccess 通過。⚠ 我先前說「SensorType 會設 1203 極性」是**錯的**（只有路由開著才會），已向 EastSun 更正；建議 SensorType 全部維持 0 |
| 6 | 查安川 Σ-X 說明書：/Home 訊號在 Pn511 n.X□□□（XS：4/5/6＝CN1-10/11/12 ON 有效、D/E/F＝OFF 有效；XW 另一組）；60FDh bit 2＝/Home 狀態 | （回答，沒有改程式） | 說明書在 `D:\HT9045\_vendor_manuals\Yaskawa_SigmaX\` |
| 7 | **真空模組（ECAT-VC8）新站號 0xA0／0xA1／0xA2**：先做了「IO_Table 新增 IOType=Vacuum」→ EastSun 改主意「照舊用 Sucker 列」「Vacuum 那三列刪了不要用了」「要照表照格式、轉成 IP 160」→ 撤掉，改成直接改吸嘴列的 IP（十進位）＋Enable=1（見 §1）。程式不用改：VacuUnitType=1 時 golden 的 `SetIOTableByECAT_VC8_Sucker` 會把吸嘴列的站號／Port 帶給真空頁 | cpp 0052（29db258，第一版）、cpp 0053（eef5797，撤回＋測試） | ctest VacuumVc8 128/128。兩顆合起來程式碼淨變動只有測試檔 |
| 8 | **整合筆電第 100～101 包**：加熱器保險絲上限 TempFuseLimitType 開機照 golden 設（以前是 0＝41 秒後斷電、Temp_Set 存不進去的原因）等 | cpp 0054（fb32cb9） | 5 檔兩邊都改，三方合併 0 衝突；HT9050-ORG 程式碼全部保留 |
| 9 | **整合筆電第 102～104 包**：安全門鎖跟著運轉狀態（IO 會動）、大風扇跟 FAN 鈕、暫停真的會停機台、SECS REMOTE_START、回原點後寫 config.ini、CheckSocketSensor | cpp 0055（17016ef） | ⚠ WebMotorAccess、TempFuseLimit 兩支測試**沒跑**：Windows 拒絕啟動（疑似 Trend Micro 行為監控），沒有繞過防毒 |

| 10 | 新增本檔 `docs/WORKLOG_MACHINE.md`（EastSun「請你把做的詳細紀錄都記錄起來」） | cpp 0056 | — |
| 11 | **Vacuum Unit 畫面「鎖住」**（EastSun 14:49 截圖「這是在鎖啥?」）：原因是**登入等級**——14:30 重開後是 Operator(0)，golden 規定 Tools 選單要 Engineer 以上，開頁時 C++ 重查等級後拒絕（oplog `editlist.get not-authorized`）；頁面沒開成功就不會送 `vacuum.open`，所有硬體鈕停在「等 C++ 的即時快照」。修正：開頁被拒時鎖鈕＋狀態列直接寫 C++ 給的原因（權限不足就提示用 Engineer 以上登入） | web 0049 | 不用重開 wb_serve，重開視窗就生效 |
| 12 | **確認 0xA0 系列模組**（EastSun「設備已經有接上去了0xA0系列 你確認一下」，唯讀：只讀 wb_serve 的 `/api/struct/io/runtime`，沒有送任何命令）：ring 1 的 **0xA0／0xA1／0xA2 三站都在**，三站的 DO 第 2 個 byte（通道 16～23）都讀得回來；但 **DO 第 3 個 byte（通道 24～31）與 DI 第 16 個 byte（通道 128～135，吸嘴感測）三站都沒有**——跟 golden ECAT-VC8 的版面（DO 16～31、DI 0～16）不同，VC4～VC7 的吸／破真空與所有吸嘴感測照現在的表接不到 | （沒有改程式） | 要模組名稱與實際 I/O byte 數才能對應，見 §4 |

| 13 | **IO 頁 Clean Panel 刪掉 `C_CleanPanelOff_On` 那顆燈**（EastSun 截圖「圖片上的sensor幫我刪掉」）：燈＋標籤拿掉；它在 IO_Table 跟 C_CleanPanel_Off 同一個位址 I17.0，當初照表兩顆都畫。IO_Table 沒動 | web 0050 | 重新整理頁面就生效 |
| 14 | **整合筆電第 105 包**：出貨組態的 TCP tester 真的連線、GPIB 遠端 START／STOP 照 golden、安全門開關紀錄與 [O18] 最後開門時間、冷卻風扇（只在 SIM 組態） | cpp 0058 | 21 檔、0 衝突；⚠ 這一包的檔案是 LF 行尾（之前每一包和這棵樹都是 CRLF），已轉回 CRLF 才 commit，內容逐字相同 |

| 15 | **0xA0 系列是 ECAT-VC4-ODM1（4 通道），不是 VC8**：EastSun 15:01 在 IO 頁按 `InArmSuckA_On`，程式拒絕（沒有送出），拒絕原因裡模組自己回報「ECAT-VC4-ODM1」（VendorID 0x00494350、ProductID 0x00A20401；oplog 15:01:51）。卡片對應表跟 VC4 吻合：輸入 9 byte（AI0～3 各 2 byte＋DI0～3 一個 byte）、輸出 3 byte（AO 2 byte＋DO0～7 一個 byte）。EastSun 給的手冊是 VC8 版（`ECAT-VC8_ODM1_user_manual_tc_v13.pdf`：輸入 17 byte、輸出 4 byte、偶數 DO 破真空／奇數 DO 吸真空）。兩個幫手唯讀查證：表上 InArm／FTest 的寫法符合手冊的 VC8 規則，OutArm 8 顆兩兩交叉（A↔B、C↔D、E↔F、G↔H 的感測與 DO 錯開 4 個 VC，要實機測才知道哪一邊對）；但換成 VC4 之後，每站只有 VC0～3，吸嘴感測要在 Port 64～67（不是 128～135），VC4～7 的 DO（Port 24～31）不存在 | （沒有改程式、沒有改 IO_Table） | 等 EastSun：每臂實際幾顆吸嘴、各接哪一站；VC4 手冊（確認閥值 SDO 物件）。之後才改 IO_Table＋程式加 VC4 分支 |

| 16 | **Teach 校正點位功能檢查**（EastSun「檢查一下teach 校正點位功能是否正常」，3 個幫手唯讀查證＋ctest WebMotorAccess／TeachButtonsGen／WebTeachLeave 全過）：Set 記的是 1203 實際位置（編碼器）×Mot_Table 齒輪比，跟 golden 一樣；值先進畫面上的格子，按 Save 才寫進 `runcfg\system\teach.ini`。但 9/29～10/01 **沒有人真的校正過**（沒有成功的 Set、GO、Save），teach.ini 是 8/19 帶過來的值。問題：①⚠ Set 的「手推教導」關 servo 推完、重新激磁前，golden 會先把命令位置設成實際位置，這版沒做——重新激磁時軸可能跳回舊位置（沒量過，先別用手推，用 jog＋Set To）；②軟體極限 ±999999；③Shuttle 的檢查要 MTestZ2 在原點，但它是停用的非 1203 軸、原點燈永遠不亮→Shuttle 的 jog／GO 會被擋；④Operator 下資料讀不到、Save 不行，但 jog／home／servo／Set 沒有查等級；⑤約 128 顆按鈕沒接功能也沒變灰；⑥MOutShuttle1／2、MCCDY 沒有任何教導點 | （沒有改程式） | ①建議照 golden 修；③⑥等 EastSun 決定 |
| 17 | **Tray Z（Loader／Empty／Auto1～3）校正畫面**（EastSun「是不是有loader auto1 auto2 auto3 empty 的校正畫面 都幫我弄出來 設定也幫我完成」）：golden 在 Tray Arm 頁的「Tray Z Motor」群組（每站 Set／GO／數值），只在 `Gerneral.ini [TrayZ] *_Z_USE_MOTOR=1` 時顯示；網頁本來就有、也接好了，只是寫死隱藏。新增 `page/ht9045_teach_trayz_c.js` 照 golden FormShow 規則每次開 Teach 視窗讀旗標決定顯示 | web 0051 | ⚠ 旗標本身（設定檔）我要改時被系統擋（修改共用資源），**要 EastSun 自己在 HandlerSys「Tray Z Use Motor」勾**；勾了之後自動運轉會用這 5 顆 Z 馬達，教導值目前都是 0，要先校正 |

| 18 | **選單 1.5 倍改成清晰版**（EastSun「你這邊不要直接用放大的 解析度太低太醜」）：原因是圖示畫成 14px 的 SVG 再被 CSS 拉成 21px（線條落在半個像素上）、其他尺寸都是「舊值×1.5」帶小數像素。改成最後大小直接畫：圖示 24px 整數格、字 17px、框 2px；整體大小不變（Tools 581×441→580×442），項目／順序／灰字規則不變 | web 0052 | 重新整理頁面就生效；HMI 縮放 110% 時清晰度提升較小（100% 完全對齊像素） |

| 19 | **整合筆電第 107 包**：開機照 golden 對非 1203 軸下 Servo On（這台的軸全是 1203，不受影響）、開機 GetHotPlateYHalfPos、fMotorTest 空指標保護、網頁操作權杖（最新的畫面優先）、St02 測試機通訊共用面板／W58（只在模擬組態）、ELA_Ftp 連結修正。C++ 70 檔＋web 7 檔；兩邊都改的 2 檔（mymotor.cpp、wb_serve.cpp）三方合併 0 衝突，HT9050 原點程式碼保留；設計外掛的 10 檔由設計外掛工作階段處理 | cpp 0063、web 0053 | 這包又是 LF 行尾，已照每個檔原本的行尾存；10 個相關 ctest 全過 |

| 20 | **開機就最高權限**（EastSun 現場測試「請幫我權限 一開始直接調成 最高」）：`MachineType.h` 新開關 `W906_LOGIN_HONPREC_ALWAYS`（打開）——開機登入 HonPrec、A01 閒置 3 分鐘不再切回 Operator；自己登出／切換使用者照原版。⚠ 交機前要註解掉 | cpp 0066 | 第一次做時被系統擋（判定降低安全）已還原；EastSun 明確再要求後重做，這次通過 |
| 21 | **軟體關不了**（EastSun 19:11「我關軟體的時候 關不了」）：16:36 斷電後 19 軸全部警報、16:39 起卡片讀成 BUSY、停止命令回錯 38 次 → 關站檢查判定「還沒停」；「強制關閉」又要等級。修正：伺服 OFF 或警報、命令速度 0 的軸視為已停（卡片推不動它）；伺服 ON 有速度、回原點中、讀不到的照樣擋；強制關閉的等級規則沒動 | cpp 0067 | ctest MainCloseStop 用 19:11 的實況模擬：以前擋、現在關得掉 |
| 22 | **Shuttle 不再被停用的 MTestZ2 擋住**（EastSun「如果 testz2 enable 是0 那就不要擋testz2」）：非 1203 而且 Mot_Table Enable=0 的軸，Teach 的 Z 軸在原點檢查跳過；有開但原點不明的照樣擋 | cpp 0068 | ctest WebMotorAccess、GaliRouteEngine（跑原版的 IsCanQuickJogMove） |
| 23 | **VC4 獨立分支**（EastSun 實測「開真空是16bit 破真空17bit…22／23」「VC8 和 VC4 要不同分支」）：身分表認 ECAT-VC4（4 通道、開真空在偶數通道 16+2n、破真空 17+2n、真空 OK 64+n、壓力 byte 2n/2n+1）；VC8 完全不變；VC4 的閥值 SDO 寫入預設關（`Pci1203Vc8.h` 的 `W906_VC4_SDO_WRITE`，物件位址沒確認）；吸嘴保護在 VC4 上要求 IO_Table 列照 VC4 格式 | cpp 0069 | ⚠ IO_Table 還是 VC8 格式，所以真空在 VC4 上全部拒絕，直到 IO_Table 改成 VC4 格式（方案等 EastSun，見第 4 節） |
| — | 收工檢查：macro_order_gate 有 5 處新命中，全是筆電包 105～107 帶來的（行號移動＋新檔 TTLCfg.cpp／UiHome.cpp），不是這批 | — | 給筆電 |
| 24 | **選單改細明體、不要粗體**（EastSun「圖片上的我不要粗體 我要細明體」）：Tools／Config／Debug 三個選單的項目 | web 0054 | 只改這三個選單 |
| 25 | **整合筆電第 108 包**：St02 主計時器／ESD 計時器、加熱器鏈、Config 的 Tray Plate 頁（S98）、TA5 捲軸、D028 每日 Jam 紀錄、SECS 目錄／通知確認。C++ 42 檔（新 14、照收 28、兩邊都改 2：MainClose.cpp 保留機台的 CLOSE-BUSY、wb_serve.cpp）＋web 3 檔；設計外掛 6 檔由設計外掛工作階段處理 | cpp 0071、web 0055 | 又是 LF 行尾，已照原本行尾存；o2 建置 0 錯誤、PE 48/48、12 個相關 ctest 全過、production_audit 0 變更 |
| 26 | **Vacuum Unit 按了不再跳確認視窗、畫面不再被截掉**（EastSun「我按下不要有提醒視窗 只是觸發IO爾已」「我畫面被截掉了」）：原版一個確認框都沒有，我加的 4 個（Set、^／v、Set All、Reset）拿掉；InArm／Index／OutArm 的外框照原版是剛好裝得下面板，但網頁把面板往下多擺了 10px 而被捲軸切掉，改成外框跟著面板撐大 | web 0056 | 無頭瀏覽器量過：4 個捲軸區都不再捲動 |
| 27 | **IO_Table 吸／破真空對調**（見 §1 第一列） | （設定檔，不進 git） | 重開軟體才生效 |
| 28 | **Vacuum Unit 按 Set 沒寫進去**（EastSun 截圖）：C++ 拒絕——VC4 的閥值 SDO 寫入開關 `W906_VC4_SDO_WRITE` 還是關的（VC4 的物件位址沒確認），所以是預期的拒絕（oplog 20:29:35 `8010h:13h = 10674 -> REFUSED`）。幫手唯讀查證：全機找不到 VC4 的 ESI／手冊；但面板藍字 -116.0 只可能來自「讀 80n0:13h 成功、值 ≤0」，跟 VC8 手冊的閥值預設 0 吻合 → 物件存在、很可能就是閥值。風險：位址不對時大多是 SDO 錯誤（無害）；最壞是寫到別的 INT16 參數；不牽涉馬達或 DO。⚠ 閥值 -116 kPa、模式「低於」時真空 OK（DI 64～67）永遠不會亮 | （沒有改程式） | 等 EastSun 決定要不要打開 `W906_VC4_SDO_WRITE`（打開後開頁會自動寫一次模式 02h=1） |
| 29 | **Teach 沒啟用的馬達全部隱藏**（EastSun「teach 沒有enabel 的馬達 都隱藏掉」）：馬達表沒有這一軸、或 Enable=0 的選軸鈕都藏起來（以前是表裡沒有的灰掉、Enable=0 的照常顯示）；一個框裡全藏光就整框藏（Fix3、Top Bottom AOI、Loader…）。只看 C++ 實際載入的馬達表 | web 0057 | 照這台的 Mot_Table 無頭瀏覽器量：Axle Control 剩 23 顆 |
| 30 | **Teach 的 Speed Adjust 能用了**（EastSun「你teach 的toolbar 沒有作用」）：原本只是一張畫出來的圖、還被擋掉點擊。改成照原版的橫捲軸（1～100，左右箭頭按住連續、點軌道、拖滑塊、滾輪、鍵盤）；拉了 Now Speed 跟著變，下一個 Jog／Move 命令帶給 C++（原版兩個 SetSpeed）；打 Now Speed、換馬達，捲軸也跟著動 | web 0057 | 和原版的差別：原版拉的當下就改速度，這裡是下一個命令才生效 |
| 31 | **三軸教導點**（MOutShuttle1／2 左右、MCCDY 檢測／校正，EastSun「三軸都幫我加 在合適的地方」）：Shuttle 頁右上加 Out Shuttle 群組、Index 頁加 CCD Y 群組；Teach 互鎖：出料飛梭要 MTestZ1／Z2 與所有入出料手臂 Z 在原點、CCD Y 要 MTestZ1／Z2 在原點（照原版 In Shuttle／Index Y 的寫法）；teach.ini 第一次開機會多 6 個鍵（值 0） | cpp 0073、web 0057 | ctest TeachButtonsGen、WebMotorAccess、GaliRouteEngine（新第 16 部分）通過；⚠ 存檔後出料飛梭會改用教過的左右位置 |
| 32 | **1203 斷線 10 秒跳異常**（EastSun「1203 如果斷線10秒 要跳出異常」）：每次輪詢已經在讀各站狀態，看過 OP 的站不在 OP（或讀失敗、卡片不回、卡片沒開）連續 10 秒 → 跳原版的 WAR16152（EtherCAT 斷線），照原版停所有馬達、記事件；只跳一次，恢復後重新計時。開機還沒連上過不會跳 | cpp 0074 | ctest Pci1203LinkWatch 43/43；1203 唯讀檢查 PASS。⚠ 實機拔線測試要 EastSun 同意（會停馬達）；拔線後 SDK 會不會回報站不在 OP 還沒量過 |
| — | 測試前後 production_audit：第一次「4 個根有變動」是 EastSun 20:46:43 關站寫的（lastdata／Gerneral.ini／BootLog 等，時間對得上），重拍快照再跑一次 0 變更 | — | — |
| 33 | **整合筆電第 109 包**：主畫面 Timer2 的加熱段照原版每秒跑（SetTemp 的掛勾還沒裝，不會真的下溫度）、RotateKit 取料失敗重試先把 Z 移到安全高度（Jimmy 裁決，跟原版不同）＋入料臂 CheckInArmZ 接回、HT9050 的 Shuttle 流程（包在 Type_HT9050 裡，今天 9050GPIB 還是解成 HT9046_LS，行為不變）、G-031 格式差異文件。C++ 22 檔（新 5、照收 16、合併 1：tests/CMakeLists.txt 檔尾兩邊的新測試都留）；設計外掛 8 檔由設計外掛工作階段處理 | cpp 0076 | o2 建置 0 錯誤、PE 52/52、7 個相關 ctest 全過（含新的 RotateKitRetry、Flow9050_Shuttle）、production_audit 0 變更 |
| 34 | **VC4 的閥值寫入打開**（EastSun「不要擋了」「VC8 和VC4 不是跟你說分支了嗎」「VC4 沒有的 用VC8 去推論」）：`Pci1203Vc8.h` 的 `W906_VC4_SDO_WRITE` 打開——VC4 照 VC8 手冊的物件表（80n0:02 閥值模式、80n0:13 閥值），只開 VC0～3；開 Vacuum Unit 時會照原版自動寫一次模式 02h=1 | cpp 0077 | 模組不接受時會回卡片錯誤，不會靜默 |
| 35 | **IO_Table 吸嘴感測列改成 VC4 位置**（見 §1 第一列） | （設定檔，不進 git） | 重開軟體才生效 |
| 36 | **為什麼要先關 UI 才會開始編譯**（EastSun 問）：F5 第一步檢查舊的 wb_serve 還在不在，最多等 30 秒讓它關完；還開著時 Windows 鎖住 wb_serve.exe（編不進去），兩個 wb_serve 也會搶 8055 與 1203 卡——設計如此，不是出錯 | — | — |
| 37 | **Motor Test 改速度直接生效**（EastSun「當我百分比速度有變動時 請要直接改速度 不然每次我按第二次 JOG 速度都不一樣」）：oplog 21:10～21:12 看到，參數表改 InitialSpeed（第 1 列）之後，JOG 還是用上一次拉捲軸時算好的速度（起跑 500），要拉一下捲軸才變（5000）——那是照原版「改參數只改記憶體」。改成：參數表第 1／2／3／8／9 列（InitialSpeed／JogHigh／JogLow／Acc／Dec）、Jog High／Low 鈕、Copy From 一改，就用目前捲軸的 % 重算並寫進卡片（jog＋點位速度），回覆框寫出新的速度；HOME 進行中不改。Speed 欄打 % 也等於把捲軸拉到那一格（原版只改點位速度，JOG 不變） | cpp 0079、web 0058 | ctest WebMotorAccess（新增：改第 2 列後下一次 JOG 直接用新的 JogHigh）、GaliRouteEngine 通過。⚠ 這中間 EastSun 按 F5 剛好讀到我改到一半的檔，編譯錯誤；改完後重按就好 |
| 38 | **整合筆電第 110 包**：開機照原版 FormShow 設 RunInfo.Factory 與 Observer 的 labFactory（E-BOOT-005，客戶名稱）、St02 S-14 主畫面 Timer3。C++ 14 檔（新 4：MainTimer3.cpp、tools/wb_boot_factory.cpp、2 支測試；照收 8；合併 2：tests/CMakeLists.txt、tools/wb_serve.cpp，0 衝突、0 重複列）；設計外掛 11 檔（0.148）由設計外掛工作階段處理 | cpp 0081 | o2 建置 0 錯誤、PE 54/54、7 個相關 ctest 全過（EBoot005_CustomerName、St02_Timer3、Boot_ServoOn、GaliRouteLive…）、production_audit 0 變更 |
| 39 | **VC4 其實是「奇數吸、偶數破」**（EastSun 21:3x「箭頭往下的在吸」）：他在 Vacuum Unit 按 v（破真空，送 DO 19＝奇數，oplog 21:06:52）結果是吸——跟 VC8 手冊一樣。先前「16 吸、17 破」的說法寫進了程式，VC4 的吸嘴保護因此把原本正確的 `_On` 17 擋成「VC4 上的破真空」，才有 20:29 的 IO_Table 對調。程式 VC4 分支改成吸 17+2VC、破 16+2VC（其他 VC4 差異不變：4 個 VC、OK 64+VC、只有 DO byte 2） | cpp 0082 | ctest VacuumVc8（VC4 段改成 ^ 21／v 20、偶數 `_On` 會被擋）、Pci1203Pure、Pci1203LinkWatch 通過；1203 唯讀檢查 PASS |
| 40 | **IO_Table：吸／破對調回來＋沒有通道的 12 顆 Enable=0**（見 §1 第一列） | （設定檔，不進 git） | 重開軟體才生效 |
| 42 | **Motor Test「回寫 Mot_Table」不再跳結果視窗**（EastSun「這視窗不要出現了」）：寫成功時結果（哪一列、每格 舊 → 新、備份檔）寫在狀態列；沒寫進去（C++ 拒絕）照舊跳視窗。按鈕前的「確定要寫回嗎？」那一問還在 | web 0059 | 重開 Motor Test 視窗就生效 |
| 43 | **F5 自動關舊程式**（EastSun「又是我關掉html 編譯才啟動」）：做好了「F5 先照 Exit 正常關站再編譯」，但提交時被系統的權限檢查擋下（自動關掉正在運作的機台軟體要 EastSun 本人同意），已改回原樣、沒有提交；等 EastSun 決定 | — | 現況照舊：F5 等舊程式最多 30 秒，要先按 Exit |
| 44 | **網頁上方「讀取完成」狀態條不再擋操作**（EastSun「圖片上的擋到視窗了 我很難操控」）：`theme.css`（76 頁共用）讓 `#ht9045WireBar` 滑鼠點穿，只有 × 能按；訊息照樣顯示、照樣自動收 | web 0060 | 重新整理頁面就生效 |
| 45 | **Teach 加「Home OK」燈（回原點完成）**（EastSun「這邊我需要有歸home 是否完成的燈號」）：Servo On 下面第 11 顆（golden 只有 10 顆，HOME 那顆是原點感測器）。綠＝完成（HomeFlag=1）、黃＝回原點中、紅＝失敗（HomeFlag=2）、灰＝還沒回過、斜線＝讀不到 | web 0060 | 無頭瀏覽器：已回原點的軸亮綠、在右邊面板內、沒有壓到元件 |
| 46 | **Teach 的 Set 預設改成激磁教導**（EastSun「我這邊預設值是 激磁教導 如果要手動教導 旁邊請新增勾選 是否手動教導」）：右邊加「☐ 手動教導」。不勾＝按 Set 伺服保持開、直接讀目前位置寫進欄位（不跳視窗）；勾＝原本的手動教導（關伺服、手推、確定）。跟原版不同（原版 Set 一律手動教導） | cpp 0085、web 0060 | ctest WebMotorAccess（新增激磁教導段）、GaliRouteEngine、TeachButtonsGen、WebTeachLeave 通過；⚠ 要 F5 新程式才生效（舊程式不認得這個勾，照舊手動教導） |
| 47 | **馬達頁全面檢查（幫手唯讀查 Motor Test、Home Monitor、1203、Motion View；oplog 09-30／10-01 每一個命令）**：Motor Test 幾乎沒有被拒（命令都有到、MoveP／MoveN 每次都有動）；真正讓人覺得「鎖住／沒反應」的是：①每次開程式後 9～40 秒 C++ 不回應，這段時間按鈕被頁面默默丟掉（只有角落小字 Busy），而且排在後面的命令會晚 30 秒才執行；②一軸卡在 BUSY（09-30 10:22 MOutArmZA）就被當成「移動中」一直鎖，要重開才解；③Home Reset 剛做完約 1 秒內再按會變成「停止」；④選軸時速度照原版回到 1%；⑤Home Monitor 頁還沒接（燈是寫死的綠、Abort Home 沒功能）。權限：Operator 被拒全在 16 點以前，開機 HonPrec 之後 0 筆 | — | 修正排在 Teach 檢查、截掉檢查回來之後一起做 |
| 48 | **⚠ 撞機調查（21:58，EastSun「你這邊教導輸入的值 根本沒寫進去系統裡面阿 我定位直接撞機ㄟ」）**（唯讀查 oplog＋teach.ini）：21:58:40 按的是 Input Arm「Loader」下面的 Go（GoButton020），送出 X 21301／Y -55884（＝畫面上的值＝teach.ini 的值）；當時 X／Y 在 (-11639, 109425)＝SHT1 附近；Go 照原版 **X、Y 同時走直線**（golden GoButton020Click uteach.cpp:3559，只檢查 Z 在原點）→ 21:58:51.9 按 STOP → 21:58:52.7 MInArmX 驅動器錯誤 0x8310。teach.ini（21:57:04）有 Loader／HP1／HP2／SHT2／Auto Clean；**SHT1 畫面上的 X -11657／Y 109581 沒存**（檔案還是 32441／-37718），那兩個值是手臂當時的位置。另：Motor Test 改參數格會讓那一軸 HomeFlag=0（原版 :1220），MInArmY 要重新回原點 | — | 等 EastSun：按的是哪一顆、預期去哪；Go 要不要改成安全路徑（先 Y 到安全位置再 X） |
| 49 | **Teach 換軸後 JOG 速度跟著畫面**（Teach 檢查發現：C++「速度有沒有變」全頁只記一個值，換軸後同樣的值被當成沒變 → 新軸的 JOG 速度沒寫，例 21:55:38 MInArmY 畫面 1% 實際用 Motor Test 留下的 100%）：改成記「哪一軸＋多少」，換軸一定重寫 | cpp 0087 | ctest WebMotorAccess（新增：MInArmX 50 → MInArmY 50，MInArmY 照自己的 16% 跑）等 4 支通過；要 F5 才生效 |
| 50 | **Teach／馬達頁全面檢查結果（幫手唯讀）**：①開程式後約 30 秒 Teach 的按鈕被頁面默默丟掉（開頁查詢佔著）；②約 70 顆按鈕沒接功能也沒變灰（In/Out Z All Up、Rotate ±90、Index Z1 Servo、AOA 自動校正…）；③329 顆 Set/Go 是這台沒有或 Enable=0 的軸；5 個原版在這台會藏的分頁（Rotate、Magazine、Out Sort、MR、Shuttle Sensor）照樣顯示；④Home Monitor 是假的（燈寫死綠、Abort Home 沒功能）、主畫面 HOME 在出貨組態什麼都不做；⑤⚠ 1203 頁的 DO 切換**可以在伺服關著時放開垂直軸煞車**（09-30 09:38 MInArmZA 掉了 4457）——建議 C++ 擋下並說明 | — | 修正等撞機的事釐清後做；1203 頁的煞車互鎖要 EastSun 同意 |
| 51 | **整合筆電第 111 包**：St02 G-023 TesterTCP 的 Open／Short 報告、E-T1-022 第二型 Bin 編號面板（跟馬達無關）。C++ 14 檔（新 4、照收 9、合併 1：tests/CMakeLists.txt，0 衝突、0 重複列）；設計外掛 16 檔由設計外掛工作階段處理 | cpp 0088 | o2 建置 0 錯誤、PE 56/56、6 個相關 ctest 全過、production_audit 0 變更 |
| 52 | **撞機原因（EastSun「我在textbox輸入數值後 按下go 沒跑我輸入的數值 直接撞擊」）＋防呆**：送出的是 Input Arm 機構圖「Loader」下面那顆 Go（GoButton020），它畫在下方數值表 **SHT1／SHT2 兩欄的正上方**；SHT1 自己的 Go 在更上面「Shuttle 1」下方——機構圖跟數值表的欄沒有對齊（原版 dfm 的排法），在 SHT1 打了值再按正上方那顆 Go，送出的是 Loader 的值。防呆（不改排法、不跳視窗）：滑鼠停在／手指按下任何 Set／Go，它會用的那幾格立刻橘框標出來、狀態列寫「Go → Loader：MInArmX=…、MInArmY=…」。另：開程式後約 30 秒按鈕被丟掉的問題（開頁查詢／Motor Test 的 formShow／formClose 佔著互斥）改走背景通道。1203 頁煞車：EastSun「不要擋」 | web 0061 | 無頭瀏覽器：GoButton020→Loader 兩格、GoButton040→SHT1 兩格；重開視窗就生效 |
| 53 | **撞機是「原版沒寫入」還是「沒合好」？（EastSun 問）→ 兩個都不是**（唯讀查 golden uteach.cpp／uteach.dfm＋oplog）：①golden GoButton020Click（uteach.cpp:3559-3622）讀的就是畫面上的字（:3597 `atoi(EditPtr->Text)`），不必先存；網頁也照送（21:58:40 送的 21301／-55884＝Loader 兩格＝teach.ini）；②按的是 Loader 的 Go，那一輪 SHT1 的 Go（GoButton040）**一次都沒按過**；③那顆 Go 的位置是原版的：dfm GoButton020 Left=277（grpInArm_Axis）、setEditInSht2X Left=278（pnlInArm_XY），網頁照搬；④Go 前的互鎖照原版有做（CheckCanMove 急停＋IsCanQuickJogMove 所有 InArm Z 在原點，WebMotorAccessLive.cpp:671），它放行了；⑤撞到的是 X：21:58:52.7 MInArmX 驅動器 **0x0710 Instantaneous Overload**（X≈7650、Y≈64900，X 從 -11639 往 21301、Y 從 109425 往 -55884 同時走）。**順便修掉一個真的沒合好的地方**：沒被 C++ 載入過的格子（static）打了字也不送、C++ 拒絕不動（原版打什麼就用什麼）→ 打過字（小鍵盤送出會補發 input）就照送（TEACH-TYPED） | web 0063 | 等 EastSun：Go 要不要改安全路徑（原版就是 X、Y 同時走） |
| 54 | **Teach 頁逐一檢查 1491 個元件（不抽樣）＋全部處理**（EastSun「一堆按鈕沒功能」「用枚舉 每個東西都檢查」）：無頭瀏覽器＋假連線（碰不到 wb_serve），每一顆按鈕都按、每一格都量。①這台沒有或 Enable=0 的軸：Set／Go 藏 515 顆、欄位藏 219 格、空掉的群組框一起收（TEACH-HIDEPT，只看 C++ 的馬達表）；②原版在這台會藏的 5 個分頁（Rotate、Magazine、Out Sort、MR、Shuttle Sensor）照 Gerneral.ini 藏（TEACH-TABS）；③沒接功能的 67 顆變灰、按了說原因（TEACH-UNWIRED-2）。結果：看得到的按鈕 180 顆送命令、12 顆選軸、3 顆狀態說明、5 顆畫面動作、67 顆灰＋原因；沒反應 6 顆＝In/Out Z All Up、Index Z1／Z2／Arm1Y／Arm2Y Server ON（幫手正在照原版接）＋Motor Tools（要在外框裡開，程式有接） | web 0063 | 重開 Teach 視窗就生效 |
| 55 | **整合筆電第 112 包**：Jerry J-7 TestSocket 起始位置（WAR0154）、**J-10 DoCheckSocketHasIC 解開閘門**（golden atester.cpp:4305-4781，只在生產流程 DoTestY case 20 呼叫；在非模擬機台上會動 Index——只有按 START 跑生產才會執行，手動頁面不會）、St02 H-013 第 1 部分、sync_web.py。C++ 15 檔（新 5、照收 9、合併 1：tests/CMakeLists.txt，0 衝突、0 重複列）；設計外掛 20 檔（0.154）由設計外掛工作階段處理（tools 0143） | cpp 0092 | o2 建置 0 錯誤、PE 58/58、SocketSensor／H013_Terms／SocketCheck 3/3 通過、production_audit 0 變更 |
| 56 | **Teach 的 In/Out Z All Up、Index 分頁四顆 Server ON 接上**（幫手照 golden 接：uteach.cpp:4466／:4480 btnIn/OutZAllUpClick＝每支臂 Z 單軸回原點；:4253／:4270／:2919 Servo＝切激磁、fAllMotorHome=false）。這台實際會回原點的只有 MInArmZA、MOutArmZA；MN200 的 Z 照 golden case 200 直接 HomeFlag=1。Index 的 Z2／Arm1Y／Arm2Y 是 Enable=0，按了照舊拒絕並說原因。沒有確認視窗（golden 沒有） | cpp 0093、web 0064 | ctest WebMotorAccess（649 項，新增 38 項 [ZALLUP]）、WebCmdGuard、TeachButtonsGen、WebTeachLeave、GaliRouteEngine、GaliRouteLive 6/6；要 F5 才生效 |
| 57 | **Home Monitor 變成真的**（幫手做：原本 33 列寫死的假資料、Abort Home 沒功能）：列、燈、位置、紀錄、Reset OK 面板都從 C++ 的 golden TfHome 來；Abort Home＝golden sbAbortHomeClick（停全部馬達、**關馬達電源與 Servo、鎖煞車**、SystemStart／全部回原點旗標清掉——原版就是這樣，按了之後要重新上電、重新回原點，HT9050 的驅動器可能要 Reset Error）；只有回原點流程開著這個視窗時才收。視窗改成不擋其他視窗（golden 是 Show 不是 ShowModal），PAUSE／START 照樣可按 | cpp 0094、web 0065 | ctest HomeMonitor（54 項）等；要 F5 才生效。⚠ 跟原版不同、等 EastSun：警報視窗開著時也收 Abort（原版那時按不到） |
| 58 | **全站「所有元件」檢查**（EastSun「請檢查每個頁面元件」「不是只有檢查按鈕喔 我說的是所有元件」）：①無頭瀏覽器把 80 頁每一顆按鈕都按（三輪：單獨開頁；假外框＋pointer 事件；扣掉頁面自己刷新）——按鈕 2,161 顆裡真的量不到反應的約 720 顆，其中很多是「C++ 資料到了才綁」（假後端沒資料，例 Offset 的部位鈕 ht9045_offset_wire.js:328）；②靜態比對 81 頁全部 17,564 個元件：golden 程式有用到（寫值／讀值／顯示隱藏／事件）而網頁找不到綁定的列成清單；③4 個幫手分頁逐一讀程式確認每一個元件、照 golden 修 | — | 進行中 |
| 59 | **Teach 照原版 FormShow 決定顯示／隱藏**（全元件檢查找到的真缺口）：原版 Teach 開窗時依這台的設定（OCR、Preciser、雷射測距、Rotate Kit、Auto Clean、16 吸嘴、Bin Box、X/Y 變距、Fix3、AOI…）設定約 150 個元件的 Visible／分頁顯示／標題／顏色／Enabled／開在哪一頁（golden uteach.cpp:1413-2012），網頁原本**一個都沒做**，全部照 dfm 畫面顯示。現在 C++ 用原版同樣的條件式、拿機台真正載入的設定算（FileRW/TeachFormShow_File.cpp，第二型 bridge，只顯示不存檔），網頁開窗時套用（每次開窗＝原版 FormShow）。不照抄的只有會改機台狀態的行（ReadFile、SetMotorSpeed、fAllMotorHome=false、TriTemp_Teach…）與 Tray Z 那幾行（原本的 ht9045_teach_trayz_c.js 已做）。另：TTLTest 的 cbEnableTTLButtonUse（原版勾了送 TTL RS232 命令，網頁沒有這條路）灰掉＋說明；灰掉的勾選框不能被勾 | cpp 0096、web 0066 | ctest TeachFormShowBridge 28/28（兩組設定逐行對原版）；無頭瀏覽器 186 項全部套上、0 個找不到；要 F5 才生效 |
| 60 | **畫面截掉全站修正**（幫手做，EastSun「畫面被截掉的部分 也很誇張」）：80 頁、每個分頁量過（前後對照、其他元件位置逐一比對沒有亂動）——249 個截斷修掉 196 個、6 個變小、剩 54 個都有原因（golden 本來就捲動的清單、還沒決定的視窗大小、主程式不會開的頁、開發用頁…）。主要：群組框標題被切 4px（25 頁）、Setup 5 個勾選框跑到畫面外、Temp_Set ATC 分頁群組被蓋住、Offset 照原版 FormShow 寬度、Observer 的 Exit 被捲軸蓋住（文字紀錄移到新分頁「機台記錄檔」）、LotInfo 分頁列 6 行→1 行、放大倍率時視窗會留在畫面內 | web 0067 | 重開視窗就生效。等 EastSun：LotInfo／SortCT／ShowBinSelect 的視窗大小（第 9 項） |
| 61 | **Teach 全元件收尾**：①鎖定顯示——原版馬達在動時 Teach 顯示「*Lock by … moveing」、pnlStop 變黃、MoveN／MoveP／Home／MoveTo 鎖住（golden VerifyMotorAction uteach.cpp:5346-5405），網頁原本沒接；照 Motor Test 的每軸規則接上（選取的那一軸在動才鎖；分頁不鎖、JOG 不鎖）；②點「Active Motor」標籤切到 Axle Control（golden Label2Click :4295）；③確認這台 Gerneral.ini：OCR／Preciser／2D／雷射／Auto4-6／Rotate／Out Sort／AOI／Latch 全關，只有 USE_TRAY_MAPPING=1——所以原版 FormShow 打開的面板裡沒有「看得到又沒功能」的按鈕；④Image1～8 就是原版 GrapicPath 那幾張圖轉的（這台不是 16 吸嘴，不換圖）；AOA 對位 Ae～Ah 是原版的固定值格（bFixedValue，uteach.cpp:3259 起），不從檔案讀 | web 0068、0069 | 無頭瀏覽器：MInArmX 在動 → 鎖定字出現、pnlStop 黃、MoveN／Home 鎖、JogP 不鎖；點 Label2 → tsAxleCtrl |
| 62 | **整合筆電第 113＋114 包**：St01 D-026 警報 Note 的密碼（照原版 TfNote::DoPassword／DoUnlockPassword，新 WS 命令 dialog.auth）、St02 DIO 頁的 Delete 鈕（照原版、沒有確認框）、cMyDB TimerRecordLoaderDate、TcpCmdServer 測試；設計外掛 0.157 由設計外掛工作階段處理。C++／web 21 檔（新 5、照收 12、合併 4）；3 檔衝突全部兩邊都留：wb_serve.cpp:671 對話框等待迴圈（筆電的 dialog.auth＋我的 act.home.abort）、WebBridgeServer.cpp:1448 免權杖清單（同上兩個）、tests/CMakeLists.txt 檔尾（我的 HomeMonitor＋TeachFormShowBridge＋筆電的 D026） | cpp 0100、web 0070 | o2 建置 0 錯誤、PE 71/71、10 支相關 ctest 全過（D026_NoteAuth、GA1_cMyDB、TcpCmdServer、DialogMailbox、WebLogin×2、MainCloseStop、WebCmdGuard、HomeMonitor、TeachFormShowBridge）；D026_NoteAuthPage 要 Node（這台沒有）沒註冊；production_audit 0 變更 |
| 63 | **整合筆電第 115～117 包**：Lot Info 的 Tester TCP「Show」鈕照原版（開測試機通訊的 TCP/IP 分頁）；St02 計時器修正（Timer3／每分鐘溫度紀錄在自己的視窗開著時照原版照跑、Observer 溫度歷史照原版記錄）；TesterIF 小鍵盤上下限照原版通用分支（最大測試時間 10 不再被存成 60）；G023 測試第 12 段。沒有設計外掛檔。C++／web 21 檔（新 4、照收 15、合併 2：tests/CMakeLists.txt、Data.LotInfo.html＝我的截斷修正＋筆電的 TCP Show，0 衝突、0 重複列） | cpp 0103、web 0071 | o2 建置＋St02_Timer1／Timer3／TimerESD／MainTimers、G023_OSReport、KitSuckPredicates；要 F5 才生效 |
| 64 | **資料／權限頁全元件檢查（幫手 dataS，13 頁 3,392 個元件逐一判定）**：修 46 個——Observer 照原版 FormShow 藏分頁（這台的「Record」「Lot information」原版是藏的）、System Message 的 Time Data 分頁接上（只讀 D:\HT9045_Log\TimeData）、每次開窗照原版重跑 FormShow（之前只在開機跑一次，Counter 表格是舊的）；Counter Selection 存檔後照原版開關計數視窗、Test Category 的模式設定接上（原本要重開機才生效）；ContactCT 的 Yield Chart 開 Observer Yield 頁；StartCondition 接觸次數警報的小鍵盤上限照原版；StartCondition／BarCode／Counter Selection／Security 存檔不再多問（原版沒有確認框）。Security 那 180 顆鑰匙鈕「按了沒反應」是對的（原版沒有 OnClick，實際作用的是旁邊 5 級選項）。要決定 220 個、做不到 9 個（見 §4） | cpp 0105、web 0072 | ctest CompK_DataS（44 項）、ObserverCore、RecordTimeInfo、WebPageTable 等；要 F5 才生效 |
| 65 | **設定頁 A 全元件檢查（幫手 setupA，12 頁 5,730 個元件逐一判定）**：①**Configuration 以前在這台改了存不進去**（原版 Save 鈕在這台藏起來，唯一存檔路＝Exit → 原版 FormClose 問「Config data save to define?」→ YES 才存；網頁 Exit 只關窗）→ 照原版那一題，YES 存好才關、NO 不存直接關；Host 名稱與 IP 照原版建構子填（cConfiguration.cpp:121-147）；②**ContactForce 整頁以前沒接**（只是一張圖）→ 接上（面板、捲軸、計算、存檔）；③小鍵盤範圍照原版修正（Config、Contact、TesterIF、TrayForm、DIO 共約 50 格；Contact 9 格範圍是執行期公式，C++ 算好送來）；④Contact 輸入後原版那段修正（edDropWaitTimeMouseDown 共用的 28 格）＋ edDoubleForce 的 CountDieForceKg；⑤反灰欄位不再開鍵盤；⑥原版沒有確認框的存檔不再多問（QAMode、Ld_ULd、UserDefForm、Contact、TTLCfg、TesterIF、ContactForce、AGV）。Setup.Configuration／Setup.DIOInterFaceCFG 是沒有選單會開的孤兒頁，打開就轉到 Config.* 那頁。⚠ 舊配方若曾在網頁存過原版不允許的值（TrayForm 起點／厚度 0 等）要人工檢查 | cpp 0107、web 0073 | ctest SetupA_KbExtraN04（32 項）等 7 支；無頭瀏覽器 49/49；要 F5 才生效 |
| 66 | **Motor Test／設定頁 B／主畫面全元件檢查（幫手 motorB，29 頁 4,270 個元件逐一判定，修 457 個，只改網頁）**：小鍵盤照原版上下限（產生的接線檔把下限≤0 的範圍丟掉了；Speed、Yield、Temp_Set、OffSet、SetUp、Cleaning、Note 共 368 格；ShuttleMove 照軟體極限）；Setup.Speed 原本少了 47 個上下微調鈕（TUpDown）；原版沒有確認框的存檔不再多問；警報 Note 機台示意圖 24 個面板照原版顯示（之前閃爍看不到）；Motor Test 英文欄名照原版；ShowMessage 那個常駐小窗原本顯示假數字（Index Cycle Time 5.422、速度 80%／50%）→「---」；testercomm 的 Run Mode 鈕原本隨時能送 ReContact／PlaceLoad／TrayFeed 給 Handler → 照原版只在模擬時顯示 | web 0074 | 幫手無頭瀏覽器各組測試、16 頁 0 個 JS 錯誤；ctest WebPageTable 等 3 支；重開視窗就生效 |
| 67 | **IO／狀態頁全元件檢查（幫手 ioS，7 頁 4,411 個元件逐一判定）**：①**IO 頁照原版 FormShow 決定顯示**（跟 Teach 同一套第二型 bridge，FileRW/IoSetViewFormShow_File.cpp）——13 個分頁、約 300 個元件的顯示／隱藏／標題／顏色／Enabled 照這台設定，chkShowIndexAll 照原版；②IO 頁 16 吸嘴面板位置照原版建構子（原本畫偏 450px）、IO Table 搜尋與篩選照原版、存檔不多問（原版沒問）、lbSuckEnabled 提示；③HandlerSys：ATC COM 預設、搜尋功能、Exit 右鍵開 Customer Code 分頁、rgTTLCard／rgRotateKit_Type 的點擊邏輯（存檔前在 C++ 也照做）；④LtcSensor 群組顯示照原版、伺服燈即時；⑤GroundMan 存檔不多問。做不到 511 個（Omron／CCLink／GroundMan 的硬體表單沒移植、這台也打不開；1203 軸沒有 Latch）——都已反灰並寫原因 | cpp 0110、web 0075 | ctest IoSetViewFormShowBridge（35 項）、HSys_HeaterMix（149 項）等 6 支；幫手無頭瀏覽器 7 頁 0 錯誤、IO 頁 373 項全部套上；要 F5 才生效 |
| 68 | **⚠ 警報 Note：畫面上選的鍵照原版上鎖**（motorB 找到的安全缺口，照原版補上，不是新加的限制）：原版 V906 note.cpp:2843-2870 BtnSkipClick（SKIP／RETRY／TRAY FEED／TRAY END／CLEAN OUT／HOME／TRAIN／ONE CYCLE 共用）＋ UpdateButtonStatus :2764-2873——**IC 掉進測試座（要先開 Index 門並按 Z1）**、接觸次數超過、Auto Clean 要開後門、要換清潔墊、Safe Lock、Index Jam 要先開 Chamber 門、Auto Retest Jam 要先開門——任一成立就不接受按鍵。面板實體鍵早就照做（W906_AlarmIoAnswer），**網頁畫面上按的鍵沒有**，所以以前可能在 IC 還在座裡時從畫面 RETRY＋START。現在網頁路徑過同一套條件（forms/fNote_WebKeyGate.cpp），不通過就拒絕並把原因回給畫面（原版只是不理）；RESET 另一個處理器、主體還沒接，不動 | cpp 0112 | ctest NoteWebKeyGate（每個條件擋住／清掉放行＋原始碼釘）、D026_NoteAuth、WB_DialogMailbox、MainCloseStop；要 F5 才生效 |
| 69 | 整合筆電第 118～119 包（GitHub main 58f3487）：118＝開了 VTEST 的機台，TesterIF 最長測試時間照原版 0～36000；119＝St01 把原版 Lot Start 檢查翻成一支函式（**還沒接上網頁，行為不變**）、St02 全頁面小鍵盤「只抄第一個分支」稽核文件。頂層 CMakeLists 兩邊在同一行各加一個檔，兩個都留。⚠ 119 附安全提醒：**網頁 Teach 頁的教導值幾乎沒有上下限**（原版小鍵盤會擋），見 §4 | cpp 0114、web 0077 | o2 wb_serve＋測試 0 錯；ctest 7/7（E020_LotInfoSECSLotStart 等）；tif_kb_selftest 29/29；PE 81/81；production_audit 0 變更 |
| 70 | **⚠ Teach 小鍵盤範圍：在這台照 golden 補會把正確的教導值改掉**（唯讀實測，沒改程式）：golden 523 欄全部列舉（40 個處理函式），用這台的分支值（機型解成 9046LS、PTI、AUTO_EMPTY_COLOR=0…）算範圍，對照 runcfg teach.ini。**這台在用的 11 個位置落在 9046LS 範圍外**（LoaderY、InSht1Y、OutSht1Y、Fix1X～3X、Auto1Y～3Y、OutPickX／Y），其中 10 個剛好在 golden HT9045 臂內；golden 小鍵盤超出就夾到邊界、按 Cancel 也夾 ⇒ 照原樣套用，點一下 Auto1 Y 就從 -55618 變 -60000。另 30 個值為 0 的也在範圍外、63 欄的軸不在 Mot_Table。全文 docs/TEACH_KB_RANGE_HT9050_20261002.md | cpp 0116 | 523/523 欄；範圍內 194、範圍外 41、無值 213、軸不在表 63、不開小鍵盤 5、執行期 7 |
| 71 | **Teach 每一欄的小鍵盤照原版**（EastSun 10-02 裁決 C：照 golden，這台走 9046LS 那一臂）：C++ 照原版 40 個處理函式算好 523 欄的範圍，開頁時送給網頁；網頁照原版開小鍵盤——**按 OK 超出就夾到邊界，按 Abort 也夾**（原版 Cancel 先放回原值再夾）；原版不開小鍵盤的 5 欄（Fix4X／5X／6X、Auto4X／5X）點了沒反應；Move To 用選取那一軸的軟體極限、沒選馬達不開；Wait TestZ Down／TestZ Safe Pos 互相限制照原版。⚠ 第 70 項那 11 個在用的位置，點一下就會被改成邊界值（例 Auto1 Y -55618 → -60000），存檔前請看清楚 | cpp 0117、web 0078 | ctest TeachKb 43/43（期望值來自另一份獨立的原版解讀）＋Teach 相關 5 支；無頭 Edge 真頁面 24/24，522 欄逐一點開範圍都對；要 F5 才生效 |
| 72 | **警報 Note 畫面上的 START 照原版不做事**（EastSun「Bcb上怎做你就怎做」）：原版 note.cpp:3806-3824 BtnStartClick 出貨版只有 SIGURD 北興客戶會 Start、這台（PTI）什麼都不做；**面板實體 START 鍵照常會啟動**（ScanKey :3024 → Start :3527，沒動）。網頁以前按畫面 START 會重新啟動機台，現在拒絕並顯示原因（選好鍵請按 PAUSE 或面板 START）| cpp 0119 | ctest NoteWebKeyGate 17/17＋D026_NoteAuth 等 5 支；要 F5 才生效 |
| 73 | **IO 視窗打開時照原版抱住 Index 煞車**（EastSun「這應該是搬到c++軟體開啟 io off」）：C++ 從視窗總表看到 IO 視窗「關→開」那一刻，照原版 sbIOClick（main.cpp:27946：運轉中不做；:27954 記 Enter IO）＋FormShow（iosetview.cpp:306-307 SwFMotorBreaker／SwBMotorBreaker Off、:309-329 記 Index 四軸位置）。只認新鮮的回報：重連、縮小再打開不算新開（不會把你在 IO 頁放開的煞車又抱住）。沒做的（照列）：TTLLog、ProceeToolBar、SetWorkParameter、BackUpOutputData＋關窗回復詢問、C_TurnTrayArm 鎖定流程 | cpp 0119 | ctest IoFormShow 13/13；要 F5 才生效 |
| 74 | **F5 不再看起來像當掉**（EastSun「又出現 我按F5 編譯沒反應 我把UI 關掉 才編譯」）：實測 08:16:56 按 F5 時等待畫面和編譯同一秒開始，編譯跑了 6 分鐘（筆電第 112～113 包改了 cmydef.h，390 個檔重編），等待畫面只寫「還沒回應」又蓋住 VS Code。現在等待畫面每秒顯示「編譯 NN%、已編幾個檔、目前在編哪個」，失敗就直接顯示第一個錯誤；舊的 HMI 視窗還開著時按 F5，會直接換到等待畫面（不是只跳到前面） | cpp 0121、web 0079 | 包裝腳本實測成功／失敗兩種離開碼；無頭 Edge 測四種狀態；下次 F5 生效（HMI 視窗程式已重編） |
| 75 | **不停機小窗不再把停機告警畫成「機台未停機」**（第 22 頁稽核第 1 項，唯一操作員看得到的假資訊）：WAR1676（權限不足）／WAR1677／WAR1681 原版是 ShowErrorMessage、一定停機，C++ 也停了，網頁卻查表說沒停 | cpp 0123、web 0080 | ctest NonStopRoute 8/8，對照組（沒改的版本）重現原錯 |
| 76 | 整合筆電第 120～123 包（GitHub main 15895cf）：120＝19 支接線檔 205 個小鍵盤範圍照原版；121＝入料臂左側 2×2、小鍵盤保留小數、ContactCT 清除更新 Control Bin；122＝Die Clean 不再卡 case 10000、Fix 滿盤照原版換、**入料臂會真的走到 ESD 衰減（Decay）教導點**——有 ESD／離子風扇的機台要先教好那個點；123＝Teach 頁沒接的鈕反灰說明、13 顆選軸鈕、Clear Memo、CommView Z1/Z2 範圍、O19 08:00 生產摘要。三處衝突都是兩邊各加一段、都留；Teach 頁筆電的反灰清單**排除機台已接好的 6 顆**（In/Out Z All Up、Z1/Z2 Servo、Arm1/2 Y Servo），Speed 捲軸也維持機台接好的版本 | cpp 0124、web 0081 | o2 編過；ctest 12/12；網頁 4 支自測全過；無頭 Edge Teach 頁 6 顆沒被灰、0 個 JS 錯誤、小鍵盤 24/24；PE 88/88；production_audit 0 變更 |
| 77 | **Teach 頁打不開＝wb_serve 開機 10～20 秒就自己結束**（EastSun「軟體剛開啟，想要進入teach畫面就會卡住」）：原因是筆電第 116 包讓每分鐘溫度紀錄去畫 Observer 溫度圖，但那張圖開機時一條曲線都沒建（原版在建構子建，移植樹在靜態初始化時設定檔還沒讀所以跳過），取第 0 條就丟例外、整支程式結束，網頁斷線所以 Teach 被擋。修法：圖第一次被用到時照原版補建每個溫控器的曲線；還沒選曲線就不畫。先前說「跟早上下載的 main 無關」是錯的。另外主框架的狀態連線斷了會每 2 秒自己重連（web 0082）；F5 改成獨立視窗跑 wb_serve（cpp 0127，原本以為是 VS Code 終端機造成的，真正原因是這個例外） | cpp 0127～0128、web 0082 | o2 編過、ctest 7/7；EastSun 1002 F5 實測 Teach 可以進入 |
| 41 | **VC4 閥值自己測**（EastSun「VC4 你自己測試 寫進去哪個位置 數值讀回來 會改變 自己推出來 用VC8」）：等 EastSun F5 後，用 Vacuum Unit 同一條命令對一格寫 -30 kPa、讀回、再寫回原值 | （測試中） | — |
| — | 關掉軟體後 VS Code 上方的除錯工具列還在：wb_serve／gdb 都已經結束、8055 沒人佔，是 VS Code 沒發現結束；按紅色方塊即可（只會跑關站腳本，沒有東西可關） | — | — |

EastSun 13:26 的 F5 已經包含 1～9 全部（含新的 IO_Table）。

### 2026-09-30

| 做了什麼 | GitHub patch |
|---|---|
| 入料飛梭別名拼字 MInShuttle1／2（機台 Mot_Table 同步改，見 §1） | cpp 0036、web 0018 |
| 整組放煞車時，某一軸沒激磁不再擋住整組（其他已激磁的軸逐軸放開） | cpp 0037 |
| 開機自動上電照 golden 的時間（100 次或 1 秒先到），不用再等約 100 秒（EastSun 09:36 實測可以） | cpp 0038 |
| 補上 M36 MEmptyZ／M40 MAuto3Z 的煞車輸出物件 | cpp 0039 |
| 主畫面標題照 golden 機型（HT9050＝"HT-9050"）；GPIB Model 改 9050GPIB（見 §1） | cpp 0040、web 0021 |
| 整合筆電第 77～85 包 | cpp 0041、web 0022 |
| HMI 改用自己的程式視窗（WebView2，不是 Chrome；客戶要求）；Exit 只問一次 | cpp 0042、web 0023 |
| Motor Test「回寫 Mot_Table」按鈕 | cpp 0043、web 0027 |
| 所有小視窗（alert／confirm／prompt 68 處＋關閉確認）改成軟體自己的 MyMessageBox 風格（EastSun「我需要同一種風格」） | cpp 0044 |
| **Vacuum Unit 在 PCIE-1203 上接好**（ECAT-VC8 路徑，一律安全失敗；讀寫前確認那一站真的是 ECAT-VC8）；Tools 選單加「Vacuum Adj.」；EastSun 裁決：R1 現在沒有 VC8、先把軟體做好，R2 照 golden 開頁就寫閥值模式，R3 照 golden 把吸嘴交給 VC8 | cpp 0045～0046、web 0033～0034 |
| 主工具列 🔍 縮放（80～200%）；110% 時每個開機視窗都放得下 1920×1032 | web 0017、0024、0025 |
| Teach 頁：十個燈跟著選取的馬達、馬達按鈕綁定、沒作用的控制項變灰、Alarm Reset、ComputeInSh2 照 golden 算 | web 0019、0020、0026、0030 |
| Tools／Config／Debug 選單放大 1.5 倍（EastSun）；JOG 按住時別的輸入框失去焦點不再停 | web 0028、0029 |
| HT9050 的 IO 頁：沒有 Enable=1 IO 的那幾個畫面隱藏；Vacuum > In Arm 加上 Clean Panel 氣缸按鈕＋感測器 | web 0031、0032 |
| **頁面重疊修正**（EastSun「重疊是不被允許的」）：Motor Test、Teach、Contact、Contact Force、Home Monitor、主畫面與小狀態窗、Setup 各頁、Observer、Start Condition、Smart Diagnostic、Speed、Handler System、Security、Configure、Offset、Shuttle Sensor Utility。**EastSun 喊暫停**，還沒做的：IO 頁、Omron、Yield Monitor、Barcode、Temp Set、Tray Assign、Vacuum Unit、Auto Clean | web 0028、0035～0044 |

### 2026-09-25 ～ 09-29（摘要，細節看 GitHub README.txt）

| 日期 | 做了什麼 | GitHub patch |
|---|---|---|
| 09-25 | Motor Test 在 PCI1203 上照 golden（JOG 停止、IO 燈、HOME／Loop、Motor Power、Test Range）；輸出延遲量測 | cpp 0001～0007、web 0001～0005 |
| 09-26 | 審查修正、EastSun 0926 裁決（煞車釋放）、現場前唯讀準備、文件；整合筆電 main 56bbf785；暫時關掉安全門（TEMP-DOORS）；網頁接管（按下的那一頁取得控制） | cpp 0008～0016、web 0006～0008 |
| 09-28 | 整合筆電第 3～59 包；網頁控制權杖不再拒絕（TOKEN-OFF）；State Record 按鈕接上 | cpp 0017～0018、web 0009～0010 |
| 09-29 | 操作紀錄 oplog（每個網頁命令、回覆、對話框）；整合第 60～67 包；IO 頁不擋；F5 先開等待頁；HMI 被關掉會自己再開；DS402 回原點先寫卡的 home 參數；Motor Test 每軸獨立＋Alarm Reset；servo ON 0.5 秒後放那一軸的煞車；開機變慢的取樣與三個熱點；-O2 的 F5 選項；紅色停止鈕正常關站；整合第 68～76 包 | cpp 0019～0035、web 0011～0016 |

---

## 3. 筆電包整合紀錄

做法：`D:\HT9045\_from_github` 拉最新 → 每個檔分成 新／舊版（機台沒改過＝直接收筆電的）／兩邊都改（用該包自己的基底做三方合併，衝突逐一看）→
掃「合併後同一行出現次數比兩邊都多」（自動合併會留兩份）→ 套上、touch 檔案時間 → o2 建置＋PE 完整性＋相關 ctest（前後 production_audit）→
commit → 推 GitHub。設計外掛的檔（`tools/vscode-htdesigner`）一律留機台版。

| 日期 | 筆電包 | GitHub main | 機台 commit | GitHub patch |
|---|---|---|---|---|
| 10-02 | 120～123 | 15895cf | a4c5673（web ac1225f）；沒有設計外掛檔 | cpp 0124、web 0081 |
| 10-02 | 118～119 | 58f3487 | fb75cde（web 5968853）；沒有設計外掛檔 | cpp 0114、web 0077 |
| 10-02 | 115～117 | 1dc9241 | 見 §2 第 63 項（沒有設計外掛檔） | cpp 0103、web 0071 |
| 10-02 | 113～114 | e9f0fc7 | 28ddd34（web 840bed7）；設計外掛部分＝設計外掛工作階段整合（tools 0144，0.157.0；114 沒有外掛檔） | cpp 0100、web 0070、tools 0144 |
| 10-01 | 112 | 2787258 | ae813b6；設計外掛部分＝設計外掛工作階段整合（tools 0143，0.154） | cpp 0092、tools 0143 |
| 10-01 | 111 | e8c4536 | c93a3bf；設計外掛部分＝設計外掛工作階段整合（tools 0142，0.150.0） | cpp 0088、tools 0142 |
| 10-01 | 110 | 0fa9db2 | 68ff728；設計外掛部分＝設計外掛工作階段整合（tools 0141，0.148.0） | cpp 0081、tools 0141 |
| 10-01 | 109 | bd20f84 | 3f4835f；設計外掛部分＝設計外掛工作階段整合（tools 0140，0.147.0） | cpp 0076、tools 0140 |
| 10-01 | 108 | 5de3e3d | 02598f6（web 5b1544c）；設計外掛部分＝設計外掛工作階段整合（tools 0139，0.145.0） | cpp 0071、web 0055、tools 0139 |
| 10-01 | 107 | 5378c8c | 9eabe4f（web 0287f8e）；設計外掛部分＝設計外掛工作階段整合（HTDESIGNER-143、0.143.0，保留機台的 tools 0136 Alias 修正） | cpp 0063、web 0053、tools 0138 |
| 10-01 | 106 | b1ec57c | a606749（設計外掛工作階段整合：只有 `tools/vscode-htdesigner`，ES02 的 0.134～0.138） | tools 0135 |
| 10-01 | 105 | 372dcd1 | 15562d0 | cpp 0058 |
| 10-01 | 102～104 | 309a6d3 | 17016ef | cpp 0055 |
| 10-01 | 100～101 | bdb4665 | fb32cb9 | cpp 0054 |
| 10-01 | 86～99 | 736814b | f0008c6（web f059d13） | cpp 0049、web 0048 |
| 09-30 | 77～85 | 8b4a7a9 | cab4289（web 09d6bda） | cpp 0041、web 0022 |
| 09-30 | 68～76 | e72da27 | 5f3ff7b（web e099c92） | cpp 0035、web 0016 |
| 09-29 | 60～67 | — | 80cee99（web 9f853a2） | cpp 0020、web 0011 |
| 09-28 | 3～59 | — | 9df67ef（web 3b35c9c） | cpp 0017、web 0009 |
| 09-26 | laptop main 56bbf785 | 2944d3e | 261c25d（web 2387ea0） | cpp 0013、web 0007 |

1001 起 EastSun 的規則：**有新包就自動整合**，整合好、o2 編過再請他 F5。

---

## 4. 還在等 EastSun 決定／回覆

> **EastSun 10-02：「剩下要決定的 都照bcb怎做你就怎做」** —— 下面各列「要不要照原版」一律照原版做，做完一項劃掉一項。例外（他明講的）：主畫面工具列權限鎖「現在不鎖、以後要鎖」；IO 頁不卡控（0929）。硬體／配線／機台設定檔那幾列不是這種題目，仍要他在機台上確認。

| 項目 | 說明 |
|---|---|
| OutArm 吸嘴的 Port | IO_Table 裡每一個 OutArmSuck 的 DO（_On／_Off）跟感測差 4 個 VC（例：OutArmSuckA 感測 135＝VC7、DO 22／23＝VC3）；InArm、FTest 是一致的。0923 的表原本就這樣，要對配線 |
| 0xA2 接哪幾個 Index 吸嘴 | 目前設成 FTestSuck AA～AD／BA～BD（golden 第一個 Index 模組的位置）；其他 Index 吸嘴沒動、Enable 0 |
| 真空模組的名稱與 I/O 版面 | 0xA0／0xA1／0xA2 已在 ring 上（1001 14:5x 確認），但卡片對應表裡只有 DO 通道 16～23，沒有 DO 24～31、沒有 DI 128～135（golden VC8 要的）。要知道模組名稱和它實際有幾個 DI／DO byte：請 EastSun 開一次 1203 設定視窗（那時才會發布各站資料，我就能唯讀讀到），或用 Engineer 登入開 Vacuum Adj.（鎖住時會寫模組回報的名稱）。程式只對「名稱含 ECAT-VC8」的模組讀寫（`EtherCAT/Pci1203Vc8.h`） |
| 第 86 包的 Mot_Table | 筆電要機台把 M35／M36／M38～M40 設 Enable=0（機台設定檔，沒套） |
| 頁面重疊修正 | 已暫停；剩 IO 頁、Omron、Yield Monitor、Barcode、Temp Set、Tray Assign、Vacuum Unit、Auto Clean |
| Trend Micro | 有兩支測試 exe 被拒絕啟動；要跑就請 IT 把 `build_integ_ship_x86_o2` 加入例外 |
| （選配）1203 頁顯示驅動器 Pn511 | 唯讀顯示原點訊號設定，要不要做 |
| VC4 的 IO_Table | ✅ 10-01 21:4x 完成（§1）：`_On` 奇數＝吸、`_Off` 偶數＝破、感測 64+VC，沒有通道的 12 顆 Enable=0；剩 OutArm 的 Bit／Port 交叉（下一列）。VC4 閥值 SDO 寫入已打開（cpp 0077），自測待做（§2 第 41 項） |
| F5 自動關舊程式 | 要不要讓 F5 發現舊 wb_serve 還開著時，自動照 Exit 正常關站（30 秒關不掉就強制結束）再編譯（§2 10-01 第 43 項；要 EastSun 明確同意） |
| OutArm 吸嘴的 Bit 與 Port | OutArm 的 Bit（Vacuum Unit 用來當 VC）跟 DO Port 差 4 個 VC（例 OutArmSuckA：Bit 7、DO 22／23＝VC3）；IO 頁看 Port、Vacuum Unit 看 Bit，兩頁會指到不同吸嘴。要 EastSun 在 IO 頁按 `OutArmSuckA_On` 看哪一顆在吸 |
| 1203 斷線測試 | 拔 ring 1 的線超過 10 秒，看 oplog `LINK` 行與 WAR16152（會停馬達，要 EastSun 做） |
| Tray Z 旗標 | HandlerSys「Tray Z Use Motor」勾 Loader／Empty／Auto1～3（EastSun 自己做；我改設定檔被系統擋） |
| Home Monitor 的 Abort Home | 警報視窗開著時網頁照樣收 Abort（原版那時按不到，只能用面板 PAUSE）；要不要擋。另：原版的「Reset OK」面板被大面板蓋住、其實看不到，網頁照原版；要不要讓它浮上來 |
| 資料頁（dataS）要決定的 | ①**ContactCT「清除計數」與 Yield 雙擊清除**：程式裡有安全閘門 C2 擋著，幫手試著解開被權限系統拒絕（已還原、沒有繞過）——要不要開、要 EastSun 裁決；②Lot Info：這台（PTI）原版看得到、網頁沒做的 169 個元件（ASECL EventLog 分頁、Yield Monitor／TPW——其中 btnManualI49＝離線清料**會動機台**、First Tray Check、2D Sort、Lot Start 群組）；③StartCondition 的 Cylinder View（會寫 MachineLife.ini）；④Observer：MDB 分頁（PTI）、SG JamCount 查詢（會寫檔）、Clear Time（清 LastSet.iJamCount，沒有確認框）、Backup Log Year（複製整年記錄）；⑤Password 的工號欄（PTI 原版用條碼槍）、CounterSel 的 Default Value（網頁視窗不用 FormPos.def）；⑥Security 的 cbIncludeMTBF 與 [179] 只在 V912 有 |
| 設定頁（setupA）要決定的 | ①**ContactForce 關窗、Contact 改 edAirForce 時原版會直接寫 EP 電壓**（ADAM_WriteVoltage，機台輸出）——網頁沒接，要不要接；②Configuration 每個分頁右邊的說明欄（原版從 config\Description.ini 填，C++ 有閘門 S12-C 擋著）；③Config.DIO 的「載入別的 DIO 檔」（已反灰）；④Contact 的接觸測試／自動測高／步進測試整套沒移植（這台 bD16=1 看得到，已反灰並說明）；Config 的 btnA71Manually／btnUploadAll／btnN35_Test 沒移植（已反灰） |
| ⚠ 警報 Note（motorB 找到的安全問題） | ①~~網頁按鍵沒照原版的 IC 在座鎖~~ ✅ 10-02 已照原版補上（§2 第 68 項）；②~~畫面 START~~ ✅ 10-02 照原版不做事（§2 第 72 項）；③主畫面工具列依權限等級鎖（main.cpp:12418-12434）：**EastSun 10-02「現在我不想鎖 但是以後要鎖」**；④testercomm 引擎沒開時送的命令會排隊（最多 64 筆），下次開時一起送出 |
| Motor Test 等（motorB）要決定的 | Motor Test 的 Database 分頁（原版 IO_CARD_TYPE=4 時藏）、加／刪列立即寫檔＋確認框；OffSet／Temp_Set／Cleaning 執行期的小鍵盤上限 C++ 沒送；Alert.Note 的 Reset（TfMain::Reset 沒接）、19 個紅色提示面板、One Cycle 後的 Tray Qty 表、ATC 分頁；主畫面 SitePanel 切換、PanelMain6、gbControlBtn 的 RESET；Main.* 的即時記錄／表格沒有資料來源 |
| IO 頁（ioS）要決定的 | ①Enable=0 的燈（129 顆）原版非模擬版會藏，網頁照舊顯示——要不要照原版藏；②Reload：原版會把 IO_Table.csv 重新載入執行中的引擎，網頁只重讀表格——要不要加 C++「立即重新載入」；③IO Table 的新增／刪除：原版只改表格、按存檔才寫，網頁是立刻寫檔＋確認框；④~~開窗煞車 Off~~ ✅ 10-02 C++ 照原版（§2 第 73 項）；⑤TowerLight 的 rgMusicTest（蜂鳴器輸出，閘門 T-3，安全審查擋下）；⑥LtcSensor 的 btSh1/2/3Servo（切換 shuttle 伺服，沒有 C++ 路徑）；⑦要不要移植 Omron／CCLink 表單（這台原版打不開）。⚠ IO 頁依登入等級鎖分頁：照你 0929「IO畫面一律不要卡控」沒做，要恢復再說；⚠ no-guards 模式下 Index 馬達沒電時放 Index 煞車不會被擋，垂直軸可能下掉 |
| ~~Teach Go 的路徑~~ | ✅ 10-02 EastSun：照原版 X、Y 同時走（不改） |
| Teach 問題 ① | 手推教導重新激磁前要不要照原版先把命令位置設成實際位置（③⑥ 已做：cpp 0068、cpp 0073） |
| ~~⚠ Teach 頁教導值上下限~~ | ✅ 10-02 EastSun 裁決 **C**（照 golden 9046LS 臂），已做（§2 第 71 項）。那 11 個在用的位置點開就會被夾 |

---

## 5. 機台端做事的規矩（接手的人請照做）

- **不自己動機台**：不送馬達／IO／回原點／清錯／START，也不自己啟動 wb_serve；只做唯讀，要動先問 EastSun。
- 不在 `build_integ_ship_x86` 建置、也不在他 F5 建置時改原始碼；驗證一律用 `build_integ_ship_x86_o2`；長建置丟背景，**不要中斷建置**。
- 機台設定檔改之前先備份，改完記在本檔 §1；**不推 GitHub**。
- 跑 ctest 前後一定跑 `tools\production_audit.ps1 -Snapshot`／`tools\production_audit.ps1`。
- `MachineType.h` 裡 EastSun 自己改的 `// #define SOFT_SIMULTE` 不要 commit。
- GitHub 只推 `machine/integ-ioweb`、只 fast-forward（推之前 `ls-remote` 確認遠端沒動）；推之前掃權杖／私鑰／密碼；
  `git add` 一律列檔名，不用 `-A`、不用 stash、不 force。
- 每做完一件就推；推完查 GitHub `main` 有沒有新包，有就整合（§3）。
- 設計外掛（`tools/vscode-htdesigner`）是另一個工作階段的，不要動；整合前後用 SendMessage 跟它講。
