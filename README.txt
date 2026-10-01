HT9050 機台端 → 筆電：機台自己的 commit（format-patch），分支 machine/integ-ioweb（orphan，不含機台整合樹的歷史）
產生：20260926，機台端 Claude（EastSun 同意推送；範圍照 Jimmy 20260926 選的 B：只放機台自己的 patch）

機台樹（本分支不含它們的歷史，只有 patch）：
  C++  D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0  分支 integ/ioweb-8484bdb4  HEAD ee3254e
  web  D:\HT9045\_integ_ioweb\web                          HEAD 9f853a2
  ⇒ 機台目前 = 筆電 GitHub main e4e4a69（第 67 包，GitLab main e882ad39）＋ 下面列的機台自己的修改。
  ★ 這個分支只有機台端推（EastSun 20260929：「開一個你專門用來上傳的 branch，避免互相蓋版本」）；筆電請只讀它，main 機台不碰。

套用順序（筆電 main 還沒收過任何一份機台 patch 的前提下）：
  1. usb01_P1_P25c\   （原 USB 包 01，20260925 16:25 產生）
       machine_P1_P25b_vs_8484bdb4.patch                   C++，基準 = 筆電 feat/v912-port 8484bdb4（機台 IOWEB-P1..P25b）
       machine_merge_resolution_and_P25c_vs_64e5e80.patch  C++，機台合進筆電 66cb14e0 時的衝突解法 ＋ P25c
       machine_web_P17_P24_vs_07c3dc25.patch               web，基準 = 07c3dc25（IOWEB-P17..P24）
  2. usb02_web_P28_P29\machine_web_P28_P29_vs_laptop_66cb14e0.patch
       web：筆電 66cb14e0 的 5 個網頁檔 → 機台 d183f1d（P28 IO 頁 HT9050 分頁、P29 Tools→1203 Setting、img/homemode 16 張）
  3. cpp\  git format-patch 95c398b..ab33f7a（16 顆，git am；95c398b = 機台合進筆電 66cb14e0 的那一顆）
       0001 4b21dbe MT-E1   0002 9fd97ff MT-E1b  0003 2c1afab MT-E2   0004 eef1a78 LAT-1   0005 b90b0ca MT-E3a
       0006 0d253a0 MT-E3b  0007 5a0f02d MT-E3c  0008 5dc6706 MT-FIX1 0009 8929d13 MT-FIX1b 0010 7961939 MT-FIX1a
       0011 b1c9c25 ONSITE-1（60E0h/60E1h 讀回、ioweb_probe watch）  0012 6423746 DOCS-M1（三份文件）
       0013 261c25d MERGE-56bbf785（筆電 56bbf785 合進機台：17 個 LOCAL 手動合併＋審查修正；commit 訊息列了未完成項）
       0014 0b3344b TEMP-DOORS —— ⚠ 暫時關 8 扇安全門，上線前 revert（Jimmy 20260926：「是對的，暫時先這樣」）
       0015 2094b1b TEMP-DOORS-2 —— ⚠ 暖氣門 SnHeaterDoor／SnHeaterDoor2 也暫時關（EastSun 看到 MES1625），跟 0014 一起 revert
       0016 ab33f7a WSFANOUT + TAKEOVER —— WebBridgeServer 的 tag patch 每個基準只算一次（原本每條 WS 連線各比對、各複製一次約
                    6,400 個 tag，網路執行緒 100%、每個 HTTP 要 0.7 s）；wb_serve maxConnections 16→64（實測 23 條連線，
                    超過 16 的排在 backlog 永遠等不到 → Motor Test 整個不動）；新命令 control.takeover（接管單一操作員權杖）
  4. web\  git format-patch d183f1d..dbe0d98（8 顆，路徑相對 web\：git am --directory=web 或 git apply --directory=web）
       0001 aaf0694 MT-E1  0002 d288f1f MT-E1b  0003 8336746 MT-E2  0004 ef20fb8 LAT-1  0005 b4c613e MT-E3
       0006 a5454b9 MT-FIX1  0007 2387ea0 MERGE-56bbf785 (web)  0008 dbe0d98 TAKEOVER (web)（操作員按的都 takeover）
  ⚠ 0013 與 web 0007 裡的「筆電那一半」就是筆電 56bbf785 自己的內容；筆電套的時候，那些檔在 main 上已經是那個樣子，
    git am 可能要 -3（三方）或逐檔確認。真正屬於機台的差異是 LOCAL 17 檔的合併解法與審查修正（見 0013 的 commit 訊息）。
  ⚠ 原 USB 包 03、04（MT-E2_E3、MT-E3_FIX1）的內容就是上面 cpp 0003-0010、web 0003-0006，不另外放。
  ⚠ 原 USB 包 05（機台正本 IO_Table／Mot_Table／Gerneral.ini／Pci1203Io.ini）沒有放：這是公開 repo，機台設定檔要
    EastSun 另外同意才放；需要的話用 USB 包 D:\HT9045\TO_LAPTOP_USB_20260926\05_machine_data_20260926。

機台端驗證（0b3344b，build_integ_ship_x86 = WinLibs i686 16.2、C++14、出貨組態、HAVE_PCI1203）：
  全量建置 exit 0；ctest 182 支失敗 9 支 = config_db、ini_helpers、config_loaders、GA1_ReadGeneralIni（桶 A）、
  dfm2rc_rc_compiles／fidelity／idempotent（沒有 rc.exe）、MachineSuckers_HT9050（手算段釘筆電 IO 表）、
  GA2_C1_cinitial（只因 TEMP-DOORS：它釘 golden 的 SnSafeDoor1 Enable）。production_audit 0 變更；
  pci1203_readonly_gate PASS；control gate 只剩既有 START_RING；pe_truncation 178 完整。oracle 線這次沒有重建。
20260926 下午追加（cpp 0015-0016、web 0008）：EastSun 在機台上實測通過 —— 極限燈跟著感測器、沒有 3 秒延遲；開機直接進 IO 頁
  按輸出成功；IO 頁 → Motor Test Servo Off 成功。ctest 網頁橋接子集 20/20、production_audit 0 變更、pe_truncation 178 完整。
  另：斷馬達電／重開機後驅動器會鎖 A.A12（EtherCAT Output Data Synchronization Error），ERROR_STOP 讓 VerifyMotorAction
  一直鎖 MoveN/MoveP/LoopMove；對那幾軸 pci1203.ax.resetError 就好（EastSun 同意後由機台端送，5 軸全回 READY）。
推送前掃描：權杖／私鑰／7z 密碼參數 0 筆。

===== 20260928～29 追加（cpp 0017-0021、web 0009-0011）=====
  cpp 0017 9df67ef MERGE-8b5a91b5 —— 筆電第 3～59 包（GitHub 1d25eb6..d2f400b）一次合進機台：NEW 224／OLD 276／LOCAL 5（全乾淨）。
                    LOCAL 5 檔保留的機台修改：cinitial.cpp（TEMP-DOORS）、wb_serve.cpp（maxConnections 64）、
                    WebBridgeServer.cpp（WSFANOUT＋TAKEOVER）、ht9045_io_do.js、ht9045_recipe_client.js（TAKEOVER）。
                    筆電刪掉的 8 個檔機台還留著（沒建進任何 target）。⚠ 絕大部分內容就是筆電自己的包，筆電不用套這一顆。
  cpp 0018 8ca6123 TOKEN-OFF —— MachineType.h 檔尾 #define W906_WEB_TOKEN_ENFORCE 0：網頁權杖暫時不擋人（EastSun 20260928：
                    「這部分先不要在機台端卡控，這是要做，但等我動作流程完成才進行」）。WebBridgeConfig::enforceControlToken 預設
                    true，ctest 不受影響。⚠ 這是機台端的暫時設定，main 要不要收請 Jimmy 決定；要恢復就改成 1。
  cpp 0019 e1a16cf OPLOG —— 操作 LOG：%W906_OPLOG_DIR%\oplog_YYYYMMDD.txt（網頁指令、回覆、各軸狀態變化、Motor Test 鎖／馬達電源），
                    帳密類指令內容不記；State Record 會把當天的 oplog 一起收進快照（cStateRecord.cpp 一行，非 golden）。
                    WebBridgeServer.cpp 的 g_W906OpLogHook 預設 0（沒裝就完全不動）。建議 main 收（排查機台問題用）。
  cpp 0020 80cee99 MERGE-e882ad39 —— 筆電第 60～67 包（GitHub d2f400b..e4e4a69）：NEW 22／OLD 81／LOCAL 4；兩處衝突都是位置重疊
                    （WebBridgeServer.cpp 豁免那行筆電改了註解、wb_serve.cpp 兩邊都加在檔尾），機台修改一行不少。
                    ⚠ 其中一個機台改動在合併裡：第 62 包的 Q2-OBS（observer.get Yield 動作的權杖檢查）前面加
                    `W906_WEB_TOKEN_ENFORCE != 0 &&` —— 不加的話權杖不擋人時 Yield 還是回 not-operator。
                    第 67 包 RULINGS_20260928 #6：Index Z 的 Gali_* → 1203 由筆電做，機台端沒有重做。
  cpp 0021 ee3254e OPLOG-2 —— 操作 LOG 也記伺服器推給畫面的框（ALARM／MODAL／QUERY 與答掉的時間）。
  web 0009 3b35c9c MERGE-8b5a91b5 (web) —— 同 cpp 0017 的網頁部分；合併後多一處：takeover() 也掛閒置計時（armTokenIdle）。
  web 0010 41cd83e SR-WIRE (web) —— 主畫面 State Record 鈕原本走 state-record.js 的離線通道（只寫 localStorage、等一個 C++ 從來
                    不寫的 ack 檔 ⇒ 沒有紀錄、卡在 Busy #1 180 秒），改成直接送 WS act.main.stateRecord。建議 main 收。
  web 0011 9f853a2 MERGE-e882ad39 (web) —— 同 cpp 0020 的網頁部分（5 檔全是 OLD）。
  機台端驗證：build_integ_ship_x86 wb_serve 每一步 exit 0、pe_truncation 178 完整；EastSun 0929 09:32 F5 實跑，oplog 正常產生
  （START／MOT／PAGE／CMD／OK／NG）。ctest 這兩天沒跑。State Record 新接法還沒在機台上按過。
  掃描：權杖／私鑰／7z 密碼 0 筆；帳密字樣與內網 IP 只出現在 0017／0020 裡筆電翻譯的 golden 預設值（GitHub 上的更新包本來就有），
  機台自己寫的 0018／0019／0021、web 0010 是 0 筆。
  沒有放：MachineType.h 裡 EastSun 0928 手動註解掉的 SOFT_SIMULTE（沒 commit，出貨組態本來就用 W906_NO_SOFT_SIMULTE 關掉）。
===== 20260929 追加（cpp 0022-0034、web 0012-0015）=====
  ⓘ 機台樹同一天另有 47 顆 HTDESIGNER-*（tools/vscode-htdesigner，VS Code 設計器外掛，另一個工作階段；使用者說那些先不推）
    沒有放進來；它們只動 tools/vscode-htdesigner/，下面的 patch 不依賴它們，照編號 git am 即可。
  cpp 0022 a179025 IO-NOGUARD + MT-ENABLE + SR-HANG + OPLOG-3 —— IO 頁一律不卡控（EastSun「IO畫面一律不要卡控，讓我測試」，
                    JsonBridge/IoBtnPanelClick.cpp W906_IO_PAGE_NO_GUARDS 1，回覆帶 guardsBypassed）；Motor Test 馬達格只列
                    Mot_Table Enable=1；State Record 的 1.bat／7z 改成 job 物件＋逾時（原本 system() 卡死整個程式）。
  cpp 0023-0024 68f0a21 / 4bbcec8 BOOTWAIT —— F5 先開等待畫面（web/boot_wait.html），伺服器起來自動切 HMI。
  cpp 0025 bff7e58 HMI-KEEP —— wb_serve 跑著時 HMI 被關掉 5 秒就自動再開（預設 http 瀏覽器，W906_HMI_URL）。
  cpp 0026 e90deb6 HOME-VENDOR —— DS402 歸零先寫卡片的 PAR_AxHomeVel*（研華 Home 範例、golden SetHomeSpeed）。
                    ⚠ 20260929 查到歸零慢的真因是單位：驅動器用 2702h（64/1）解讀 6099h，卡片速度是 2701h 單位（MTrayX 慢 374 倍）；
                    EastSun 要先用 Utility 比對，換算還沒做。
  cpp 0027 1680ba1 MT-AXISLOCK + MT-ALMRST + HMI-KEEP-2 —— EastSun「每個軸都是獨立可控的」：Motor Test 的鎖只看選取的那一軸
                    （ERROR_STOP 不算在動），HOME／LoopMove／伺服鈕只擋同一軸；新 action resetAlarm（Acm_AxResetError 選取軸）；
                    阻塞框等回答時 HMI 看守也跑。test_web_motor_access 545/0。
  cpp 0028 2ea7c90 + 0030 f6f0914 BRAKE-AXIS —— EastSun「SERVO ON 就必須要先激磁 0.5秒後 觸發io的煞車」：每一軸自己的煞車輸出，
                    SVON 滿 0.5 s 才放（沒 EMG、SnMotorPower 亮）；Servo Off 先鎖煞車、100 ms、再關激磁；驅動器自己掉 SVON 立刻鎖。
                    對應表（由名稱推，EastSun 看過沒反對）：MInArmZA/SwInArmZBreaker、MOutArmZA/SwOutArmZBreaker、MTestZ1/SwFMotorBreaker、
                    MLoaderZ/SwCassetteLDMotBreaker、MAuto1Z/Auto1、MAuto2Z/Auto2。⚠ IO_Table 另有 SwCassetteEmptyMotBreaker、
                    SwCassetteAuto3MotBreaker，但 cmydef.cpp 沒有這兩個 SW[]（golden V906 只到 366）→ MEmptyZ／MAuto3Z 的煞車送不出去。
  cpp 0029 c077e63 BOOTSPEED-1 —— 開機取樣器（診斷，只印訊息；W906_BOOTSAMPLE_SEC=0 關）。
  cpp 0031 5761489 BOOTSPEED-2 —— F5 不接 gdb（noDebug）、gdb 設定加 _NO_DEBUG_HEAP=1、建置 -j 6。
  cpp 0032 97edbd6 BOOTSPEED-2 —— 開機熱點：寫穿式 TIniFile（不存在的檔先問 OS、不再存記憶體副本、同內容只更新時間、一次掃描的
                    寫入演算法）＋生產紀錄標題只組一次。舊寫入演算法保留為 W906_Win32ProfileApplyRef；新舊 404,970 組 0 差異，
                    IniFiles_Win32Diff 3,027 組對 kernel32 0 不符。實測開機到 START：68～98 s → 約 10 s（含不接 gdb）。
                    ⚠ 這台缺 D:\HT9045\Error\English\（JAM0000.dat），golden 的建檔迴圈每次開機都跑；alarm 說明與 Security Jam 設定也存不下來。
  cpp 0033 62e2f37 BOOTSPEED-3 —— 另一條 -O2 建置線（build_integ_ship_x86_o2，保留 -O0 語意的旗標）與 F5 選項；機台上還沒驗證。
  cpp 0034 16ab6a6 STOPBTN —— 不接除錯器時紅色停止鈕／工作「關閉程式」走主畫面 Exit 的正常關站（命名事件，wb_serve 檔尾）。
  web 0012-0013 7090832 / ab0b960 BOOTWAIT (web) —— boot_wait.html（就是 background.html 的開機畫面）。
  web 0014 5adeac8 MT-AXISLOCK + MT-ALMRST (web) —— 鎖看選取軸、Alarm Reset 鈕、motor-access.json 49 條。
  web 0015 fd2ec54 MT-RESEL (web) —— Motor Test 視窗重新打開時自動重選上一軸（golden FormShow 把 ActiveIndex 清成 -1，畫面就不更新）。
  另：EastSun 20260929 改了 Mot_Table 的 BoardID／Port（BoardID＝1203 連線 ID＝Acm_AxOpenbyID 的站號，Port＝站內第幾軸）；
    程式本來就是這樣對應，沒改程式。機台設定檔照舊沒有放。
  掃描：權杖／私鑰／7z 密碼 0 筆。沒有放：MachineType.h 的 SOFT_SIMULTE（EastSun 手動、未 commit）。

===== 20260930 追加（cpp 0035-0041、web 0016-0022）=====
  ★ 機台現在套到第 85 包：機台 = 筆電 GitHub main 8b4a7a9（GitLab main b21ca17e）＋ 下面列的機台自己的修改。
     機台樹 HEAD：C++ cab4289、web 09d6bda（C++ 同一天另有 HTDESIGNER-*，另一個工作階段、使用者說先不推，沒放）。
  cpp 0035 5f3ff7b MERGE-d40fa5a0 —— 筆電第 68～76 包合進機台（三方合併，機台修改保留）。⚠ 絕大部分是筆電自己的內容，不用再套。
  cpp 0036 56bc828 SHTSPELL —— 入料飛梭別名改成 MInShuttle1／MInShuttle2（原 MInShutte1／2）；teach.ini 兩種拼法都讀得到。
                    機台的 D:\HT9045\system\Mot_Table.csv 同步改名（設定檔，沒有放）。
  cpp 0037 dcc1323 BRAKE-GROUP —— golden 整組放煞車（G05／G16／HOME）因為組內某一軸沒激磁被拒時，組內已激磁 0.5 s 的軸逐軸放開。
                    根因：開 Motor Test 會跑 DoMotorPowerOn 鎖住整組卡匣煞車，MAuto2Z 沒激磁 ⇒ 整組永遠不放，操作員要切激磁才會動。
  cpp 0038 ebeb04e G31A-TIME —— golden 開機自動上電的「數 100 次」改成照 golden 的時間（100 次或 1 秒先到）：golden MainProc 約 1 ms，
                    wb_serve 是 500 ms，原本要等約 100 秒才准放煞車。✅ EastSun 0930 09:36 實測：開機後不用切激磁就放開。
  cpp 0039 484b82a BRAKE-EMPTY-AUTO3 —— 補 SW[] SwCassetteEmptyMotBreaker(369)／SwCassetteAuto3MotBreaker(370)，MAX_SWITCH_ITEM 372；
                    加進 golden Cassette 群組與逐軸煞車表（M36 MEmptyZ、M40 MAuto3Z）。IO_Table 本來就有這兩列，只是程式沒有物件。
                    golden V906／V912 都沒有這兩個名字。機台上還沒實測。
  cpp 0040 df97a7e CAPTION —— tag machine.caption＝golden fMain->Caption 依機型（main.cpp:9216-9253／:9590-9607）＋ HT9050 分支
                    （W906_GpibModel=="9050GPIB" ⇒ "HT-9050"，非 golden）。
                    ⚠ 機台設定：EastSun 0930 選擇把 D:\GPIB9045\system\general.ini [Version] Model 從 9046_32GPIB 改成 9050GPIB
                    （設定檔，沒有放；依 RULINGS_20260926 第 25 條兩者都解碼成 HT9046_LS，行為相同；HandlerSys 機型下拉沒有 9050）。
  cpp 0041 cab4289 MERGE-b21ca17e —— 筆電第 77～85 包合進機台：NEW 35／OLD 162／LOCAL 13（11 乾淨、2 處衝突都是「兩邊都加」：
                    WebMotorAccessLive.cpp 同一空白行的煞車 note 與 Pci1203GaliRoute.h include；wb_serve.cpp 檔尾 HMI-KEEP／STOPBTN
                    與 JAM-STOP host）。機台修改一個都沒少（逐檔關鍵字比對）。README 的路由檔照筆電原樣、沒改；兩個 1203 路由開關維持關。
                    ⚠ 絕大部分是筆電自己的內容，不用再套。-O2 建置 exit 0；機台上還沒實跑。
  web 0016 e099c92 MERGE-d40fa5a0 (web)、0022 09d6bda MERGE-b21ca17e (web) —— 同上兩顆合併的網頁部分。
  web 0017 abaf098 UIZOOM —— 主工具列 🔍 整個 HMI 縮放 80～200 %（localStorage，每台自己記）。
  web 0018 7d4377e SHTSPELL (web) —— 教導表、Motion View、Home 頁的 MInShuttle1／2。
  web 0019 41681a5 TEACH-LED、0020 627eeeb TEACH-SEL／TEACH-UNWIRED —— 教導頁 10 顆燈跟著選的馬達；release 模式下馬達選取鈕綁不上
                    （theme.js 把 title 移到 data-htitle）已修；這台沒有的軸、沒接好的按鈕（Suck On/Off、IO Check、Auto Teach Z、
                    速度捲軸）改成灰色＋點了說明原因。
  web 0021 215b806 CAPTION (web) —— 主視窗標題列跟著 machine.caption。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；新 patch 裡的 8 個 .csv 全是筆電包本來就在 GitHub 上的測試樣本與 docs。
  沒有放：MachineType.h 的 SOFT_SIMULTE（EastSun 手動、未 commit）；機台設定檔（Mot_Table／IO_Table／general.ini）。
  cpp 0042 1954b25 HMI-SHELL + EXIT-NOHOME —— HMI 改用自己的程式視窗（tools/hmi_shell，ht9045_hmi.exe，WebView2），不再用 Chrome：
                    客戶要求「外框不是 chrome、不要讓人員有額外操作的空間、按快捷鍵不會改到畫面、工作列是軟體的圖標」。
                    沒有標題列／邊框、貼齊螢幕工作區；瀏覽器快捷鍵在引擎層關掉（重新整理、縮放、找、列印、上一頁、DevTools…）；
                    工作列是鴻勁 LOGO（D:\HT9045\web\assets\img\logo.jpg 的標誌，去背）；✕／Alt+F4 問一次「確定要關閉軟體嗎？」後走
                    主畫面 Exit；wb_serve 正常關站／VS Code 停止鈕會一併關掉它。HMI-KEEP、MODAL-WAKE、F5 等待頁都先開它，找不到才開瀏覽器。
                    自己的 build_hmi_shell.bat（沒有動 CMake 來源清單）。附 Microsoft.Web.WebView2 1.0.4258.31 的標頭與 x86
                    WebView2Loader.dll（BSD 類授權，LICENSE.txt 同放）；WebView2 Runtime 是 Windows 11 內建。
                    Exit：golden「請在關閉程式前,執行歸零步驟」只在 #define W906_EXIT_REQUIRE_HOME 時檢查（MachineType.h 檔尾，預設關，
                    EastSun 0930「我不需要確認都歸零」）；運轉中、機台內有 IC、Q44 停機照舊。離線測過，機台上還沒實跑。
  web 0023 cbc7920 HMI-SHELL + EXIT-ONECONFIRM (web) —— Exit 只問一次「確定要關閉程式??」（第二框 "Sure To Exit?" 當 YES）；
                    background.html 給程式視窗的 ✕ 用的 HT9045ShellCloseRequest，document.title 跟著 machine.caption。
  cpp 0043 cbd3da5 MT-SAVEMOT —— Motor Test「回寫 Mot_Table」：選取軸 Settings 表的 10 個值寫回 Mot_Table.csv 那一列（非 golden；
                    golden 的 Settings 表只改記憶體）。只改值真的不同的格子、其餘位元組不動、先備份 .bak_YYYYMMDD_HHMMSS、
                    暫存檔＋讀回＋MoveFileEx；多種拒絕（運轉中、Alias 重複、CardModel 不符、空格子、Index 列 Acc/Dec…）。
                    tests/test_mt_savemot.cpp（ctest MtSaveMotTable）64/64，用真檔的副本；機台上還沒按過。
  web 0024 dcf921d LAYOUT110 —— 110% 時開站 9 個視窗都放進 1920×1032（不重疊、不出畫面），程式碼對應 footer 與標題的（xxx.dfm）一律不顯示。
  web 0025 a83604f LAYOUT110-2 —— BinSelect／Lot Info 的開發說明（.sbsNote、.nowire）不顯示。
  web 0026 977e759 TEACH-ALMRST —— 教導頁 Active Motor 區加 Alarm Reset（同 Motor Test 的 resetAlarm），指令表加 uteach 那一列。
  web 0027 f149905 MT-SAVEMOT (web) —— Motor Test「回寫 Mot_Table」按鈕、確認框與回覆顯示，指令表加 btnSaveMotTable。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0044 8fc60c7 HMI-DLG —— 程式視窗裡所有小視窗（網頁的 alert／confirm／prompt 共 68 處，加上 ✕ 的「確定要關閉軟體嗎？」）
                    改成軟體自己的 MyMessageBox 風格，不再出現瀏覽器的「127.0.0.1:8055 說」：HMI 漸層標題列、凸起面板、深藍字、
                    藍灰按鈕（確定／取消），Enter＝確定、Esc＝取消、拖標題可移動；顏色與縮放每 3 秒從頁面讀（跟著主題與 110% 等）。
                    離線編譯過，機台上還沒看過。
  web 0028 24f2b4f MENU15 + NOOVERLAP-1 —— Tools／Config／Debug 三個下拉選單全部放大 1.5 倍（按鈕 186×44、字 16.5px、圖示 21px）；
                    Motor Test「回寫 Mot_Table」往下移（原本蓋住 Torque）、Torque 說明字不再超出面板、select 框的「All」不再壓線。
  web 0029 af58936 JOGBLUR —— 按住 JOG 時，如果剛才游標在輸入框，按下那一刻輸入框失焦會被當成放開、寸動馬上停（Teach 與 Motor Test
                    共用）。現在只有整個視窗失焦才算放開；放開滑鼠／手指、頁面被藏起來照舊送停止。離線 A/B 測過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0030 a35ec22 TEACH-SH2 —— 教導頁 ComputeInSh2 照 golden 算偏移（InArm X/Y 目前位置 − Sh2 X/Y 欄位），只填兩個 Offset 欄、不送命令。
  web 0031 e3b8244 IO9050HIDE —— HT9050 開機後 IO 頁把 Stack 3 頁、Fix Tray 頁、Vacuum→Others 的 Tray Arm／RT Arm／第二個 Tray Arm 分頁（全都沒有 Enable=1 IO）藏起來；
                    只看 wb_serve 即時 IO 表（hw.enable），讀不到或不是 HT9050 就一個都不藏；TrayMobile 有 Enable=1 所以留著。
  web 0032 2ccab9b IOCLEANPANEL —— IO 頁 Vacuum → In Arm 加 Clean Panel：C_CleanPanel／C_CleanPanelOff 兩顆汽缸鈕＋三顆感測燈（HT9050）。
                    ⚠ IO_Table 裡 C_CleanPanel_Off 與 C_CleanPanelOff_On 是同一個輸入位址（17/0/0），照表畫、表沒改。
  web 0033 6c1d8b5 VACUNIT-MENU —— Tools 加 Vacuum Adj.（golden sbVacuumUnit），VacuUnitType≠0 才顯示，讀不到就灰字顯示原因；開 HW.VacuumUnit.html。
                    機台端同日把 D:\HT9045\system\Gerneral.ini [System] VacuUnitType 0→1（EastSun 要求；只改那一個字元，先備份）。
                    設定檔本身沒有放進這裡。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。

20260930 傍晚追加（tools 0001-0077）：小工具「HTML 視覺設計工具」VS Code 外掛（tools/vscode-htdesigner）
  EastSun 20260930：「關於小工具部分，編譯沒問題就能commit and push 到github」。
  tools\  機台樹裡碰過 tools/vscode-htdesigner 的 commit，一顆一個檔（77 顆，9484e81..d60160c，git format-patch）：
       每一顆都只碰 tools/vscode-htdesigner；第一顆（9484e81）建立整個資料夾，之後沒有別的 commit 動過它。
       ⇒ 照編號 git am（或 git apply）就是機台端現在的樣子，不會碰到其他檔案。
       0001 9484e81 第一版（設計檢視、DFM 屬性、事件）……0073 d3edc33 事件表照 WPF 的事件分頁（兩欄、空白＝空、Enter 新增、
       下拉選已有函式、右鍵重設）  0074 6ee8c8d 屬性面板上方：名稱方塊、〔屬性〕〔⚡事件〕  0075 d1a5872 工具箱 Enter＝新增
       0076 c69b535 雙擊＝預設事件（空的＝新增）、Shift+F7、元件樹右鍵選單  0077 d60160c 設計畫面左下工具列、上層路徑可點
  版本 0.79.0。裝法：cd tools\vscode-htdesigner → powershell -File pack.ps1 → code --install-extension dist\ht9045-html-designer-0.79.0.vsix
  （dist\ 不在版控裡，要自己打包）。怎麼用：README.md、CHEATSHEET.md、CHANGELOG.md（同資料夾）。
  測試（機台端，離線）：四層全過 —— 程式庫 159、假 VS Code 166、面板、真的 VS Code 148；不動機台、不存檔、不需要 wb_serve。
  ⚠ 新增事件時外掛會「提議」改 tools/wb_serve.cpp（htd.event 分支）、CMakeLists.txt、HtdEvents/HtdEvents.gen.cpp——
    都是編輯器裡還沒存檔的修改，使用者存了才算；這些 patch 本身沒有改那幾個檔。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。

  tools 0078 5f76d94 HTDESIGNER-80（0.80.0）—— 屬性值右邊的小方塊（WPF 的 property marker：空心＝跟 DFM 一樣、實心＝改過，點一下＝重設選單）；
                    「改回 DFM」的位置／大小一律寫回原始碼（畫面還沒跟上時以前會漏）。四層測試全過（程式庫 159、假 VS Code 166、面板、真 VS Code 148）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0079 HTDESIGNER-81（0.81.0）—— 旁邊的 HTML 游標在元件的標籤、文字或沒有 id 的子元素裡都會選取它（WPF 的分割檢視），
                    在 <div class="form"> 裡＝表單；「排列：依名稱／依類別／DFM 順序」移到屬性清單最上面。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0080 HTDESIGNER-82（0.82.0）—— Font.Name 有下拉清單（WPF 的 FontFamily）；README 加「跟 WPF 設計工具的操作對照」表
                    （逐項對微軟說明；做不到的：工具箱直接拖到畫面上——VS Code 的限制）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0081 HTDESIGNER-83（0.83.0）—— 開始「自己找跟 WPF 不一樣的地方直接改」（EastSun 20260930），每次改動記在 tools/vscode-htdesigner/WPF_DIFF_LOG.md：
                    元件樹 F2／Ctrl+C／X／V／Delete；HTML 右鍵「移到事件處理函式」；一打開頁面屬性面板就顯示表單。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0082 HTDESIGNER-84（0.84.0）—— 設計畫面上方的資訊列（WPF 的 Information Bar）：頁面有 JS 錯誤就顯示數量與第一個，「看全部」＝輸出面板，×＝關掉。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0083 HTDESIGNER-85（0.85.0）—— 屬性格線分類（WPF 的 Common／Layout／Appearance／Text）：可以改的屬性分 一般／版面／外觀／文字，排列預設「依類別」、依名稱＝A→Z 不分組，兩區一起套用。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0084 HTDESIGNER-86（0.86.0）—— 工具箱最上面的「指標」（WPF 的 Pointer，放好元件後回到指標）、屬性類別可以收合、滑鼠停在屬性名稱上＝說明。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0085 HTDESIGNER-87（0.87.0）—— 按住 Ctrl 拖曳元件＝複製一份放到放開的地方（WinForms／WPF），原本的不動、新名稱、一次 Ctrl+Z；Ctrl＋點照舊是加選。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0086 HTDESIGNER-88（0.88.0）—— 隱藏／鎖定快捷鍵 Ctrl+H／Shift+Ctrl+H／Ctrl+L／Shift+Ctrl+L（WPF 文件大綱）、縮放 12.5%～800%、工具列「背景」亮／暗切換；元件樹上 F2 改名改的是樹上那列。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0045 7860efc VACUNIT-1203 —— Vacuum Unit 視窗在 PCIE-1203 上接好（ECAT-VC8 路徑，一律安全失敗）：讀／寫前都確認 ring 1 那一站存在、
                    是 ECAT-VC8（身分表 EtherCAT/Pci1203Vc8.h）、OP 狀態、位址在卡片對應表；寫還要 VacuUnitType=1、沒有在運轉。
                    Set／^／v／Set All（三臂的 Tag 改由 DFM 表決定，解開 VacuumUnit.cpp 原本 gated 的 btnSetInArmClick）／Reset、
                    現值／閥值／Event／LED 都走這條；開頁照 golden 寫閥值模式（EastSun R2）；吸嘴照 golden 交給 VC8
                    （#define W906_VC8_SUCKER_REMAP，EastSun R3），而且那 144 個吸嘴別名只有在確認是 VC8 時才送 —— BTestSuck 在 0x50／0x51
                    DO 16..19 跟上料／Auto1 氣缸同通道，以前 IO 頁的吸嘴鈕打得到氣缸線圈，現在擋住。這台今天 ring 上沒有 VC8：
                    全部 999.0／Error5、按鈕鎖住並寫原因。ctest VacuumVc8 119/119；1203 兩個 gate 跟改前一樣（control gate 2 個既有 FAIL）。
                    改到 EastSun 的 Pci1203Monitor／Control／IoRoute（只有加內容）；筆電端的檔一個都沒碰。機台上還沒實跑。
  cpp 0046 57c3d45 VACUNIT-1203 —— 拒絕訊息「每個物件每分鐘一行」的表 32 → 256 格（沒有 VC8 時 48 個物件把表擠爆，變成每秒都印）。
  web 0034 b9000ea VACUNIT-1203 (web) —— Vacuum Unit 頁：每秒輪詢、顯示 C++ 算出的值、只有 available 的鈕才解鎖、寫之前確認框、回應逐筆標示。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0035-0044 NOOVERLAP-A／B／C —— EastSun 20260930「重疊是不被允許的」：離線掃描工具量 72 個視窗、590 種分頁狀態找到 759 處，
                    先修 golden 執行時會藏／會對齊（alBottom／alRight／FormShow Visible）卻被畫在設計位置的元件，其次才微調網頁字寬／圓鈕大小。
                    EastSun 同日喊暫停，已完成的頁面（掃描後 → 剩下、剩下的都判定為誤判或 golden 本來如此）：
                    Teach 45→2、Contact 43→3、Contact Force 43→0、CC-Link 68→0、Offset 67→22、Configure 46→4、Handler System 27→1、
                    Speed 21→0、Security 7→6、Motor Test 6→0、Home 16→0、Observer 16→1、Setup 9→1、Ld/ULd 7→4、Tray Form 13→0，
                    以及主畫面、Contact CT、Counter Clear、Show Message、Bin Select、Sort CT、Lot Info、Comm View、Tower Light、Tester IF、
                    AGV、Start Condition、Smart Diagnostic 全部 0。
                    還沒做：IO 頁、Omron、Yield Monitor、Auto Clean（改到一半已還原）、Barcode、Temp Set、Tray Assign、Vacuum Unit。
                    只動版面，指令／IO／馬達的程式都沒改（Offset 加了一個 Scan AOI 勾選展開面板的小處理，照 golden）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（命中的都是畫面上的 Password: 標籤與欄位名）；沒有機台設定檔。
  tools 0087 HTDESIGNER-89（0.89.0）—— 右鍵「版面重設」變子選單（WPF 的 Layout > Reset）：只重設位置／只重設大小／全部版面／全部改回 DFM。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0088 HTDESIGNER-90（0.90.0）—— 照 XAML 設計工具快捷鍵補五個：Ctrl+Shift+A 取消選取、F9 藏控制點、Alt＋方向鍵複製一份、Ctrl+N 新增元件、Shift＋角落控制點等比例。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0089 HTDESIGNER-91（0.91.0）—— 設計畫面上 F2＝直接在元件上改文字（WPF 的 Edit control text），Enter 寫入、Esc 不改；改名稱改用名稱方塊／元件樹 F2／右鍵。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0090 HTDESIGNER-92（0.92.0）—— 左下工具列的格線分成「格線」（畫不畫）和「吸附」（吸不吸）兩個按鈕，跟 WPF 一樣。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0091 HTDESIGNER-93（0.93.0）—— 拖曳時文字的基準線也會對齊（WPF 的對齊線），紅色虛線；比邊緣近時用基準線。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0092 HTDESIGNER-94（0.94.0）—— Blend 快捷鍵：Ctrl+Shift+1／2／9 同寬／同高／同大小、Ctrl+=／Ctrl+- 縮放、Ctrl+0／9 符合選取、Ctrl+1 實際大小（只在設計畫面有焦點時）。四層測試全過（第 4 層重跑一次）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0093 HTDESIGNER-95（0.95.0）—— 畫面上拖到另一個 Panel、放開前按 Alt＝換到那個容器（Blend 的 Reparent）；修正「放進表單」會跑進第一個 GroupBox。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0094 HTDESIGNER-96（0.96.0）—— Alt＋點＝一層一層往下選（Blend）；按住 Ctrl+空白鍵點一下＝放大、Ctrl+Alt+空白鍵＝縮小。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0095 HTDESIGNER-97（0.97.0）—— 按住 Shift 拖曳＝拉框選取，從元件上開始也可以（Blend）；Shift＋點照舊加選。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0096 HTDESIGNER-98（0.98.0）—— 顏色可以直接打（#rrggbb、clRed）、也可以用滴管從畫面上取（WPF 的顏色編輯器）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0097 HTDESIGNER-99（0.99.0）—— 屬性面板分成〔屬性〕〔⚡ 事件〕〔</> 程式碼〕三頁：屬性頁只剩屬性（像 WPF），接線／程式碼清單移到〔程式碼〕。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0098 HTDESIGNER-100（0.100.0）—— 屬性表改成 WPF 的三欄（名稱｜值｜小方塊），值下面不再塞字，類別是色帶，顏色的 16 色收進 ▾。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0099 HTDESIGNER-101（0.101.0）—— 事件頁整理成 WPF 的樣子（兩張同樣兩欄的表、空的就是空的）；雙擊有函式的事件＝游標直接停在函式本體裡。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0100 HTDESIGNER-102（0.102.0）—— 屬性頁一打開就是乾淨的屬性表：「DFM 屬性」「HTML」預設收起、樣式跟屬性表一致。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0101 HTDESIGNER-103（0.103.0）—— 雙擊屬性名稱＝HTML 開在旁邊、那個值已經選好（改程式更快）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0102 HTDESIGNER-104（0.104.0）—— 屬性面板上方變短（DFM／HTML／複製併到分頁那一行、排列併到搜尋框旁），表格往上兩行。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0103 HTDESIGNER-105（0.105.0）—— 數值欄按 ↑／↓ 加減 1（Shift＝10），連按只寫一次、只算一個 Ctrl+Z。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0104 HTDESIGNER-106（0.106.0）—— 屬性表 Esc＝放棄這次輸入回到原值；正在編輯的那一列整列反白。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0105 HTDESIGNER-107（0.107.0）—— 事件表照名稱 A→Z 排（WPF 的事件分頁），元件事件和網頁事件都一樣。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0106 HTDESIGNER-108（0.108.0）—— 修正：沒有可編輯屬性表的元件，「DFM 屬性」直接打開（屬性頁不會看起來是空的）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0107 HTDESIGNER-109（0.109.0）—— 屬性表每一列都有小方塊（WPF 的 property marker），選單＝重設＋跳到 HTML 原始碼；那一列按右鍵也開同一個選單。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0108 HTDESIGNER-110（0.110.0）—— 屬性表 Tab 只停在值上、Enter 寫入後游標留著（值選好）；網頁事件也有右鍵選單（跳到程式碼／重設）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0109 HTDESIGNER-111（0.111.0）——〔程式碼〕頁同一行不再列兩次；HTML 區點一下值就能改；修正改名後選取偶爾跑到表單。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0110 HTDESIGNER-112（0.112.0）—— 屬性頁一張表不重複：DFM 清單只列上面表格沒有的；小方塊選單可以跳到 .dfm 那一行。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0111 HTDESIGNER-113（0.113.0）—— 新設定 ht9045Designer.zoomWheel：Ctrl＋滾輪（預設）／只用滾輪／Alt＋滾輪縮放（WPF 的 Zoom by using）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0112 HTDESIGNER-114（0.114.0）—— 重開頁面回到上次的縮放比例；新設定 ht9045Designer.defaultZoom（上次／符合全部／100%，WPF 的 Default zoom setting）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0113 HTDESIGNER-115（0.115.0）—— 新設定 ht9045Designer.defaultView：開頁面時只開設計畫面（預設）或旁邊同時開 HTML（WPF 的 Split）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0114 HTDESIGNER-116（0.116.0）—— 新設定 ht9045Designer.snapSpacing：間距吸附的距離（預設 8px，0＝不吸附）。WPF 設定那四項（28～31）都做完。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0115 HTDESIGNER-117（0.117.0）—— 選一個元件時顯示邊距標示（WPF 的 margin adorner：到容器左邊、上邊的線和距離）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0116 HTDESIGNER-118（0.118.0）—— 修正：右下的操作提示不再蓋住左下的工具列（縮放按鈕按得到）。四層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0045 IOWIDGET（web a0e5981）—— 新增 page/ht9045_io_widgets.js：舊版 IO 畫面的 SetCompomentIO／SetCompomentHint／ScanLed（元件用 Alias 對 IO 表、依卡片回讀上色、舊版格式的提示），頁面自己選擇載入；teach 頁載入它＋ht9045_io_do.js，Suck On／Off、Tray X U/D 接上 IO（golden BtnPanelLane1Click）。只做離線測試（假資料，不連 wb_serve、不動機台）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0117 HTDESIGNER-119（0.119.0）—— IO 燈號（TALed 系列）、按鈕面板（TBtnPanel 系列）的特有屬性在屬性表「IO 元件」類別直接改（LEDStyle／Value／Blink／Style／Down／四種顏色），跟 .dfm 比對、改回。程式庫／smoke／面板三層全過；真 VS Code 那層這次沒跑（VS Code 自己在更新、鎖住，要重開 VS Code 才裝完）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0118 HTDESIGNER-120（0.120.0）—— 工具箱多 5 種 IO 元件（MyLedLane／MyLed／ALed／BtnPanelLane／BtnPanel），寫法同網頁產生器、Alias 留空給屬性表挑。程式庫／smoke／面板三層全過；真 VS Code 那層等 VS Code 重開（更新鎖住）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0046 IOWIDGET-2（web 2f86a2f）—— IO 設定頁的燈／按鈕提示前面加上舊版格式「(Lane,IP,Port,Bit) Alias」（golden SetCompomentHint），頁面自己的讀取和上色不動；共用檔的燈號樣式改成跟 IO 頁一字不差（只在框內、沒有外圈光暈）。只做離線測試。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0047 3eab632 F5WAIT —— 按主畫面 Exit 之後馬上按 F5，舊的 wb_serve 還在關站（等軸停穩、煞車、存檔），第一步「檢查舊程式」就直接失敗。
                    改成最多等 30 秒讓它自己關完再繼續；30 秒後還在才失敗。只看不關任何程序。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0119 HTDESIGNER-121（0.121.0）—— 修正 CSV 表格打不進字（EastSun 1001「表格我沒辦法寫入」）：真的滑鼠雙擊沒反應（按下時重畫，雙擊落在整張表上），開著中文輸入法打字沒反應（鍵盤在表格 div 上，輸入法沒輸入框）。改成用第二下按下判斷雙擊、一個看不見的輸入框一直停在目前那一格接按鍵（中文輸入法可用，選字的 Enter 不寫入）。新增真輸入測試（真雙擊、真按鍵、輸入法組字）：新版 4/4 過、舊版 4/4 失敗。三層全過；真 VS Code 那層等 VS Code 重開（更新鎖住）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0048 407ce3f IOWIDGET-3 —— IO 頁按鈕照舊版上色：閒置時要先關安全門才能動的輸出＝橘／黃、黑字（舊版 bIdleNeedCheckSafeDoor，讀法跟舊版一模一樣，1203 超出 [64][32] 的位址也照舊版算法讀）；
                    Index 上有 IC 時按 TestSuck 的 _On／_Off＝照舊版跑那顆吸嘴的 Suck()／Destroy() 流程，不是直接切線圈。離線測試 17/17、機台檔案 MD5 不變；這台 HT9050 的 IO 表目前沒有任何按鈕需要先關門。要 F5 重建才生效。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0047 IOWIDGET-3（web b76649b）—— 配合 cpp 0048：要先關安全門的 IO 按鈕畫成舊版的橘／黃、黑字，提示多一行說明；其他接上的按鈕＝舊版的藍／淺藍、白字（只改元素、不改原始碼）。TestSuck 走 Suck()／Destroy() 時，訊息寫「照 golden 走 Suck() 流程、吸嘴：…」。teach 頁、IO 設定頁離線測試都對。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0120 HTDESIGNER-122（0.122.0）—— 修正：在設計工具新增 C++ 事件會讓 F5 建置失敗（EastSun 1001；08:46 main.html Exit 鈕 OnMouseDown：六個檔都「沒存檔」、產生檔還是沒存的新分頁，只存到四個，CMakeLists.txt 指到不存在的 HtdEvents.gen.cpp）。現在建置要用的三個檔直接寫磁碟、順序是產生檔 → CMake → wb_serve.cpp 分支（逐位元組插入，其他位元組不動），中途停在哪都還能建置；產生檔每列用編譯時判斷「類別有那個函式才呼叫」，表單 .h／.cpp 沒存也編得過。兩個編譯器對真的樹檢查 0 錯誤、假表單實跑 8/8；三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0121 HTDESIGNER-123（0.123.0）—— CSV 表格照 Excel 官方說明重新對過：修掉會不知不覺改錯的（貼一個值到整欄只改第一格、Ctrl+－ 選一格就刪整列、Shift+空白鍵打進空白、打字後 ←→ 不換格、F2 全選、Ctrl+X 馬上清空、Ctrl+R 開最近的檔案、打字中 Ctrl+Z 復原整個檔），加上 Ctrl+Enter／Ctrl+D／Ctrl+R 填滿、範圍內 Enter／Tab、Ctrl+H 取代、最後一列下新增、狀態列顯示是哪一軸和加總。真按鍵 13 項全過、三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0122 HTDESIGNER-124（0.124.0）—— 屬性表：多選時按某一列的「重設」以前會把主要選取的 DFM 值寫到全部，現在每一個改回自己的 DFM 值（只那一項、一次 Ctrl+Z）；雙擊 True／False 的值只切換一次（BCB6 屬性表的習慣，以前等於切兩次）。三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0123 HTDESIGNER-125（0.125.0）—— 設計畫面照 VS／Blend／WinForms 說明再對一次：按住 Alt 拖曳（不吸附）放開時不再被搬進 Panel（拖到一半才按 Alt 才換容器）；滾輪縮放以滑鼠為中心；左／上控制點拉過頭不再滑走；拖曳中 Esc＝取消；工具箱放置會吸格線；Ctrl+A 不選隱藏、鎖定的。三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0049 f0008c6 + web 0048 f059d13 MERGE-e240a3f4 —— 機台套到筆電第 99 包（GitHub main 736814b／GitLab main e240a3f4），第 86～99 包一次合：
                    335 檔＝相同 119、新檔 45、機台舊版 129（換筆電版）、兩邊都改 42。兩邊都改的 18 檔三方合併（10 檔自動、8 檔 20 處人工）：
                    TOKEN-OFF（W906_WEB_TOKEN_ENFORCE 0、cfg.enforceControlToken）照舊留機台的；IOWIDGET-3 的 TestSuck Suck()／Destroy() 留機台的；
                    其餘收筆電的（notifyAck、A01 自動登出、STREAM、MinGW 相容寫法、新測試）。wb_serve.cpp 筆電那段 HMI-KEEP／STOPBTN 跟機台原有的
                    一字不差，只收 notifyAck 本體，不重複。WebMotorAccessLive.cpp 自動合併多出一行 g_W906BrakeNote，已刪成一份（掃過 18 檔沒有別處）。
                    tools/vscode-htdesigner 的 24 檔不收（設計外掛工作階段的，機台版比較新）。web：新 2、舊版 17、合併 3。
                    沒套的：第 86 包要機台改 Mot_Table M35／M36／M38～M40 Enable=0（機台設定檔，EastSun 決定）。
                    o2 建置 0 錯誤、PE 13/13 完整。
  cpp 0050 b3c612a HT9050-ORG —— HT9050（9050GPIB）＋1203 分支：1203 軸「在原點」＝1203 回來的原點訊號 LOW（EastSun 1001「home 是 high
                    代表沒偵測到、low 代表有偵測到」）。Teach 頁 IsCanQuickJogMove 的 Z 軸原點檢查（以前一律「不明＝不在原點」，跳「Please let InArm Z
                    at home position first!!」）與 Motor Test／Teach 的 HOME 燈都改用這條；只讀，不寫卡片／驅動器。原本計畫的「SensorType 開機寫
                    CFG_AxOrgLogic」取消（會把訊號意思反過來）。⚠ 給筆電：Motor/myEthercatmotor.cpp 的 ScanMotorStatus／GetHomeIO（筆電擁有）
                    仍把 ORG=1 當在原點，WB_ENGINE_MOTOR_1203 武裝時要照這條規則。ctest WebMotorAccess 通過（多 3 項）、production_audit 0 變更。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（0049 的 16 處命中都是 bNeedPassword 之類的變數名與註解）；沒有機台設定檔。
  tools 0124 HTDESIGNER-126（0.126.0）—— 屬性視窗用真的滑鼠、按鍵測出兩個會不知不覺改值的問題並修好：數值欄有焦點時滾輪會改值（Left 54→52）、下拉選單按 ↑↓ 會改值；現在滾輪只捲動、↑↓＝換列（Alt+↓ 打開清單）。切到事件分頁時預設事件有焦點；網頁事件空白 Enter／雙擊＝預設名稱。方向鍵規則照 EastSun 裁決維持 XAML 風格。三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0125 HTDESIGNER-127（0.127.0）—— 屬性視窗變成一張表、每個類別只出現一次（WPF）：以前下面另外一區「DFM 其他屬性」有自己的版面／外觀，現在那些 BCB6 .dfm 屬性是同一張表裡的灰色唯讀列，一套類別；照名稱、照 DFM 順序都是一整串。三層測試全過、截圖確認。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0126 HTDESIGNER-128（0.128.0）—— 設計畫面右鍵「選取這裡的元件…」（Blend 的 Set Current Selection：列出滑鼠底下疊著的元件，挑一個就選它）；多選時「順序」整組一起移、彼此前後不變、一個 Ctrl+Z（以前只移主要選取）。三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0051 6b70166 HT9050-ORG-ENG —— 引擎這一半：HT9050＋1203 軸「馬達在原點」一律＝1203 原點訊號 LOW（EastSun 1001「所有動作如果有馬達判定
                    home的 都要用9050 1203來判定」）。原因：1203 馬達路徑沒開時，myEthercatmotor.cpp 的 ScanMotorStatus 把 1203 軸的 HOME 燈一律設
                    false，引擎所有「Z 要在原點」的判斷都看成不在。改在 TMyMotor::ScanMotorStatus 最後套規則（全引擎讀 Led[iHomeLed] 都經過它），
                    以及兩個直接問 Motor->HomeFlag() 的回原點流程（mymotor.cpp 第 20 步、uhome.cpp InArmX）；判斷本體跟 Teach 同一支函式。
                    讀不到訊號＝不在原點（照樣擋）；不是 HT9050 或不是 1203 軸＝golden 原樣。
                    ⚠ 給筆電：Motor/myEthercatmotor.cpp（ScanMotorStatus／GetHomeIO）與 EtherCAT/Pci1203GaliRouteCore.cpp（Index Z1）都把 ORG=1 當在原點，
                    WB_ENGINE_MOTOR_1203／WB_ENGINE_INDEXZ_1203 武裝時要照「LOW＝在原點」；Mot_Table SensorType 機台 19 軸全部 0（跟這條規則一致，
                    1 會讓路由開機寫卡時把 ORG 反過來）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0127 HTDESIGNER-129（0.129.0）—— 補上 WinForms／BCB6 格式選單的指令：對齊格線、大小對齊格線、水平／垂直間距加大／縮小／移除（主要選取不動），元件樹「…」全部鎖定（跟全部解除鎖定成對）。三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0128 HTDESIGNER-130（0.130.0）—— 多選 IO 燈／按鈕時，值不一樣的屬性（LEDStyle、Value、顏色…）留白標「（不同）」；選了元件再新增，新的放在同一個容器的最上層，不會被蓋住。三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
MD5 清單在 MANIFEST_MD5.tsv。
