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
  tools 0129 HTDESIGNER-131（0.131.0）—— 多選時的「⚡ 事件」頁（BCB6／WinForms）：只列選取的元件都有的事件、函式不同的留白；打名稱＝全部接到同一個函式，重設＝全部拿掉。三層測試全過。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0130 HTDESIGNER-132（0.132.0）—— CSV 表格的右鍵選單（Excel 的儲存格選單）：剪下／複製／貼上、插入／刪除列、清除內容、選整列／整欄、尋找／取代，每項附快捷鍵；Shift+F10 也能開。三層測試全過（含真的滑鼠右鍵）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0131 HTDESIGNER-133（0.133.0）—— CSV 表格的名稱方塊（Ctrl+G 打 C12、B2:D5、列號＝跳過去）與雙擊欄的邊線＝自動調整欄寬（Excel）。三層測試全過（含真的按鍵、真的雙擊）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0132 HTDESIGNER-133b —— 交接文件 HANDOVER.md（流程、規則、分工、EastSun 的裁決、待辦、踩過的坑、檔案地圖）＋ dev\ 開發小工具（語法檢查、截圖、推送）。EastSun 20261001：詳細紀錄也要推、要讓別的帳號能接手；之後每一輪都更新它。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0052 29db258 VC8-IP —— 第一版：IO_Table 新增 IOType=Vacuum 列（InArm／OutArm／Index）指定 ECAT-VC8 站號。EastSun 當天改主意，
                    由 0053 整個撤掉（0052＋0053 合起來，程式碼淨變動只有測試檔）。
  cpp 0053 eef5797 VC8-IP-2 —— 撤掉 0052（EastSun 1001「Vacuum 那三列刪了不要用了，照舊用 Sucker 列」）。真空模組的新站號照 golden 放在
                    吸嘴列，機台 IO_Table（機台設定檔，不進 git）：InArmSuck* IP 32→160（0xA0）、OutArmSuck* 48→161（0xA1）、
                    FTestSuck AA–AD／BA–BD 64→162（0xA2），Sucker／_On／_Off 共 72 列，這 72 列 Enable 0→1（EastSun 1001）；其他格一格都沒動。
                    程式不用改：VacuUnitType=1 時 golden 的 SetIOTableByECAT_VC8_Sucker 把吸嘴列的站號／Port 帶給真空頁，InitSucker 用同一列綁吸嘴，
                    吸嘴保護在新站號登記。ctest VacuumVc8 [10] 補 160／161／162 的檢查（128/128）；IO 表相關 ctest 全過；production_audit 0 變更。
                    ⚠ 給筆電：machines/HT9050/IO_Table.csv 還是舊站號（32／48／64）、Enable 0。OutArmSuck 的 DO（_On／_Off 的 Port）跟感測（Port 128+VC）
                    差 4 個 VC（例：OutArmSuckA 感測 135＝VC7、DO 22／23＝VC3 那一對），InArm／FTest 是一致的——這是 0923 的表原本就這樣，要對配線。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0054 fb32cb9 MERGE-985c44be —— 筆電第 100、101 包（GitHub main bdb4665、GitLab main 985c44be）併進機台樹。第 100 包：加熱器保險絲上限
                    TempFuseLimitType 開機照 golden 設（250／200／170，依客戶、機種、溫度上限；以前是 0＝41 秒後斷電、Temp_Set 存不進去的原因）、
                    Motor Test 非 1203 軸的 HOME 跑 golden 單軸回原點（1203 軸不受影響）、St02 MR !22。第 101 包：普查 129 照 golden 補（三溫機安全門 6
                    鎖、急停通知 ATC、BinCount.txt、一輪結束放開 Auto 盤氣缸、[I41] 空 socket 檢查、Initial／RT Start 時上料氣缸預推——氣缸會動）。
                    17 檔：新 3、舊版 9（照收筆電的）、兩邊都改 5（WebMotorAccess.cpp／.h／Live.cpp、test_web_motor_access.cpp、wb_serve.cpp）
                    三方合併 0 衝突、0 重複列，機台的 HT9050-ORG 原點程式碼全部保留。o2 建置 0 錯誤、PE 24/24 完整；8 個相關 ctest 全過；
                    production_audit 0 變更。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0133 HTDESIGNER-133c —— HANDOVER.md 更新分工：筆電包 100–101 已由機台整合工作階段整合完（沒有動到外掛）；整合的工作階段名稱會變，用 ListAgents 找。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0055 17016ef MERGE-0b3a0735 —— 筆電第 102～104 包（GitHub main 309a6d3、GitLab main 0b3a0735）併進機台樹。第 102 包：安全門鎖照 golden 跟著
                    運轉狀態（運轉中鎖、停止放開；Magazine 門也一樣——IO 會動）、大風扇跟著 FAN 鈕、SetLotState 通知 TCP/IP tester／GPIB。
                    第 103 包只有文件。第 104 包：fMain->Pause 照 golden 真的暫停機台（狀態機約 40 處、RemoteControl／RCMD:PAUSE）、SECS
                    REMOTE_START、回原點後寫 config.ini（[I06]）、Jerry 的 CheckSocketSensor。20 檔：新 7、舊版 13（照收筆電的）、兩邊都改 0。
                    o2 建置 0 錯誤、PE o2 28/28＋F5 目錄 178/178 完整；8 個 ctest 通過。⚠ WebMotorAccess、TempFuseLimit 這次在機台上沒跑：
                    Windows 拒絕啟動那兩支 exe（Access is denied，檔案可讀、權限正常，疑似 Trend Micro 行為監控擋下）；它們的原始碼不在這三包裡，
                    第 100～101 包那輪兩支都通過。production_audit 0 變更。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0134 HTDESIGNER-133d —— HANDOVER.md 更新：筆電包 102–104 已整合（沒有動到外掛）；筆電包 93 帶的外掛跟機台 0.111.0 逐檔相同（只差換行）；防毒會擋新建的測試 exe。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0056 c964ea1 WORKLOG —— 新增 docs/WORKLOG_MACHINE.md：機台端的總紀錄（EastSun 1001「請你把做的詳細紀錄都記錄起來」）。東西放在哪裡、
                    機台設定檔每一次改動（不進 git）與備份位置、09-25～10-01 每天做了什麼（對到這裡的 patch 編號）、筆電包整合紀錄、
                    還在等 EastSun 的事、機台端做事的規矩。每顆 patch 的長說明仍以本檔（README.txt）為準。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0049 28bd17e VACUNIT-OPENFAIL —— Vacuum Unit 開頁被 C++ 拒絕時（例：登入 Operator，golden 規定 Tools 要 Engineer 以上），頁面照實寫原因、鎖鈕，
                    不再一直停在「等 C++ 的即時快照」（EastSun 1001 14:49 截圖「這是在鎖啥?」）。重開視窗就生效，不用重開 wb_serve。
  cpp 0057 8556530 WORKLOG —— 工作紀錄補 10-01 第 10～12 項：上面這個修正，以及唯讀確認 0xA0／0xA1／0xA2 三站都在 ring 上，
                    但卡片對應表只有 DO 通道 16～23，沒有 DO 24～31、沒有 DI 128～135（跟 golden VC8 版面不同，等模組名稱）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0135 HTDESIGNER-138（0.138.0）—— 筆電包 106 合進機台：ES02（EastSun 筆電）在機台 133d 上接著做的 0.134–0.138（元件樹裡的分頁、工具箱放到分頁上、Alt 拖進分頁、在檔案中尋找、方案總管、新增事件前的型別檢查）。46 個檔逐檔比過，沒有少掉機台的東西。機台測試：探針 7/7＋分頁樹 7/7、假 VS Code 207/207、面板全過；程式庫 173/174（那 1 項是 IO_Table.csv 的標記列 #NEW_FROM_9050_DRAWING_20260923，不是程式）；e2e_build.ps1 沒跑（會啟動 wb_serve）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0050 6a1bccc IOCLEANPANEL-2 —— IO 頁 Clean Panel（Vacuum > In Arm，HT9050）的 C_CleanPanelOff_On 燈＋標籤拿掉（EastSun 1001「圖片上的sensor幫我刪掉」）。
                    它在 IO_Table 跟 C_CleanPanel_Off 同一個位址 I17.0；IO_Table 沒動。
  cpp 0058 15562d0 MERGE-003a758d —— 筆電第 105 包（GitHub main 372dcd1、GitLab main 003a758d）：出貨組態的 TCP tester 真的連線、GPIB 遠端 START／STOP
                    照 golden、安全門開關紀錄與 [O18] 最後開門時間、冷卻風扇（只在 SIM 組態）。21 檔：新 3、舊版 18、兩邊都改 0。
                    ⚠ 這一包的檔案是 LF 行尾（之前每一包與機台樹都是 CRLF），已轉回 CRLF 再 commit，內容逐字相同。o2 建置 0 錯誤、PE 32/32、
                    8 個相關 ctest 通過、production_audit 0 變更。（第 106 包只有設計外掛，由設計外掛工作階段處理，見 tools 系列。）
  cpp 0059 4a6fdf8 WORKLOG —— 工作紀錄補 10-01 第 13～14 項（上面兩件）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0136 HTDESIGNER-138a（0.138.0，版本號不變）—— Alias 下拉清單不再列出 IO 表的分段標記列（#NEW_FROM_9050_DRAWING_20260923）；並更正 tools 0135 說明：那 1 項失敗的原因是 0.138 把測試改嚴了，不是 IO 表 15:05 被改（15:05 是 EastSun 對調 C_CleanPanel_On／Off）。三層全過（程式庫 175/175）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0060 d70025d WORKLOG —— 工作紀錄補 10-01 第 15 項：0xA0 系列真空模組實際是 ECAT-VC4-ODM1（4 通道，模組自己回報），不是 VC8；
                    第 3 節補第 106 包（設計外掛工作階段整合）；第 1 節記 EastSun 15:05 自己改 IO_Table（C_CleanPanel_On／_Off 對調）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0137 HTDESIGNER-138b —— ⚠ 給筆電（EastSun 20261001 16:3x：「叫筆電 給我新版」）：機台要 HTML 視覺設計工具（外掛）的最新版。
                    請把 ES02（EastSun 筆電）在 GitLab v906/es02-htdesigner 上 0.138.0 之後的外掛進度，做成「只帶 tools/vscode-htdesigner」的筆電包推到 GitHub main（跟第 106 包一樣）；
                    一起收機台的 tools 0136（aaae678，Alias 下拉清單跳過 IO 表的 # 分段標記列，版本號不變，還是 0.138.0），版本號請接在 ES02 最後一版後面。
                    機台現在套到第 106 包（第 105 包＝cpp 0058，第 106 包＝tools 0135）；外掛是 0.138.0＋tools 0136。
                    機台每 30 分鐘自動查 GitHub main，看到帶外掛的新包就逐檔比對、測試後合進來（不跑 e2e_build.ps1：它會啟動 wb_serve）。
                    這份交接也記在 tools/vscode-htdesigner/HANDOVER.md §5。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0051 fd75fc8 TEACH-TRAYZ —— Teach 頁照 golden FormShow 顯示 Tray Z 校正點（Loader／Empty／Auto1～3…，Tray Arm 頁「Tray Z Motor」群組＋Axle Control 的 Tray Z jog）：
                    每次開 Teach 視窗讀 Gerneral.ini [TrayZ] *_Z_USE_MOTOR（唯讀），只顯示有勾的；讀不到＝全部隱藏並寫原因。今天九個旗標都是 0，
                    要 EastSun 在 HandlerSys「Tray Z Use Motor」勾了才會出現。新檔 page/ht9045_teach_trayz_c.js＋HW.teach.html 一行 include。離線測試三種情況都對。
  cpp 0061 c50e99a WORKLOG —— 工作紀錄補 10-01 第 16～17 項：Teach 校正點位功能檢查結果（Set／Save／GO 的接線對、但從沒真的校正過；
                    手推教導重新激磁前沒有照 golden 同步命令位置等 6 項問題），以及 Tray Z 校正畫面；第 4 節補等 EastSun 的事。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0052 74ef8c6 MENU15-CRISP —— Tools／Config／Debug 選單照最後大小直接畫，不再是「舊值×1.5」放大（EastSun 1001「不要直接用放大的 解析度太低太醜」）：
                    原因是 14px 的 SVG 圖示被拉成 21px、尺寸帶小數像素。改成圖示 24px 整數格、字 17px、框 2px；整體大小不變，項目／順序／灰字規則不變。
  cpp 0062 c016e87 WORKLOG —— 工作紀錄補 10-01 第 18 項（上面這件）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0063 9eabe4f MERGE-c723bfdb —— 筆電第 107 包（GitHub main 5378c8c、GitLab main c723bfdb）的 C++ 部分：開機照 golden 對非 1203 軸下 Servo On（這台全是 1203
                    軸，不受影響）、開機 GetHotPlateYHalfPos、fMotorTest 空指標保護、St02 測試機通訊共用面板／W58（只在模擬組態）、ELA_Ftp 連結修正。
                    C++ 70 檔（新 10、舊版照收、兩邊都改 2 檔：mymotor.cpp 保留 HT9050 原點程式碼、wb_serve.cpp）；三方合併 0 衝突、0 重複列。
                    包又是 LF 行尾，已照每個檔在機台樹原本的行尾存（新檔 CRLF），diff 只有內容。o2 建置 0 錯誤、PE 38/38、10 個相關 ctest 通過、
                    production_audit 0 變更。設計外掛的 10 檔（0.143）不在這裡，由設計外掛工作階段整合。
  web 0053 0287f8e MERGE-c723bfdb (web) —— 同一包的 7 個網頁檔（網頁操作權杖：最新的畫面優先；測試機通訊共用面板），照收筆電的版本。
  cpp 0064 d534e18 WORKLOG —— 工作紀錄補 10-01 第 19 項（第 107 包）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0138 HTDESIGNER-143（0.143.0）—— 筆電包 107 的外掛部分合進機台：ES02 的 0.139–0.143（頁面清單併進方案總管、點頁面＝上設計畫面／下原始碼、切到 C++ 自動關 HTML 分頁、C++ 類別／成員導覽、狀態列執行列〔▶＝F5，機台選出貨組態就是真的開機台軟體〕、側欄與資料夾圖示）。三方合併（基準＝包 106）：7 個只有 ES02 改、3 個兩邊都改都沒有衝突；機台的 tools 0136 Alias 補丁保留。C++ 部分由整合工作階段做（cpp 0063）。機台測試：程式庫 176/176、探針 7/7＋分頁樹 7/7、假 VS Code 210/210、面板全過；e2e_build.ps1 沒跑（會啟動 wb_serve）。
  ⚠ 給筆電：機台的 tools 0136（aaae678，lib/aliasedit.js：Alias 清單跳過 IO 表 # 分段標記列，含 lib 測試）還沒進 GitLab，請一起收；機台現在套到第 107 包。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0065 608de7e WORKLOG —— 工作紀錄第 3 節補第 107 包的設計外掛部分（tools 0138，設計外掛工作階段整合，0.143.0）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0066 7eb2008 LOGIN-HONPREC —— MachineType.h 新開關 W906_LOGIN_HONPREC_ALWAYS（打開）：出貨組態開機就登入 HonPrec、A01 閒置不切回 Operator
                    （EastSun 1001 現場測試「權限 一開始直接調成 最高」）。⚠ 交機前要註解掉。
  cpp 0067 5b766a4 CLOSE-BUSY —— 關站：伺服 OFF／警報、命令速度 0 的 1203 軸，就算卡片讀成 BUSY、停止命令回錯也視為已停（1001 19:11 斷電後關不了）。
                    伺服 ON 有速度、回原點中、讀不到的照樣擋；強制關閉的等級規則沒動。
  cpp 0068 518faa2 TEACH-ZDISABLED —— Teach 的 Z 軸在原點檢查跳過「非 1203 而且 Mot_Table Enable=0」的軸（MTestZ2 不再擋 Shuttle）。
  cpp 0069 3e8ceff VC4 —— ECAT-VC4-ODM1 真空模組獨立分支（4 通道；EastSun 實測開真空在偶數通道 16+2n、破真空 17+2n）；VC8 不變；
                    VC4 閥值 SDO 寫入預設關（W906_VC4_SDO_WRITE）。⚠ 給筆電：IO_Table 的吸嘴列要改成 VC4 格式才會動（方案在工作紀錄）。
  cpp 0070 e7c5e06 WORKLOG —— 工作紀錄補 10-01 第 20～23 項。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0054 d049405 MENU-MINGLIU —— Tools／Config／Debug 選單項目改細明體、不要粗體（EastSun 1001「圖片上的我不要粗體 我要細明體」）；只改這三個選單。
  cpp 0071 02598f6 PKG-108 —— 筆電第 108 包（GitHub main 5de3e3d）的 C++ 部分：St02 主計時器／ESD 計時器、加熱器鏈、Config 的 Tray Plate（S98）、
                    TA5 捲軸、D028 每日 Jam 紀錄、SECS 目錄／通知確認。42 檔（新 14、照收 28、兩邊都改 2：MainClose.cpp 保留機台 CLOSE-BUSY、wb_serve.cpp）；
                    0 衝突、0 重複列；LF 行尾已照原本行尾存。o2 建置 0 錯誤、PE 48/48、12 個相關 ctest 通過、production_audit 0 變更。
                    設計外掛的 6 檔（tools/vscode-htdesigner）不在這裡，由設計外掛工作階段整合。
  web 0055 5b1544c MERGE-108 (web) —— 同一包的 3 個網頁檔（Config 的 Tray Plate 頁、Tray Assign 事件），照收筆電的版本。
  web 0056 59c6cb5 VACUNIT-NOCONFIRM／VACUNIT-FIT —— Vacuum Unit 按了就送（原版沒有確認框，拿掉我加的 4 個）；InArm／Index／OutArm 外框跟著面板撐大，
                    第二排不再被捲軸切掉（EastSun 1001「我按下不要有提醒視窗」「我畫面被截掉了」）。
  cpp 0072 dedea08 WORKLOG —— 工作紀錄補 10-01 第 24～28 項（含機台 IO_Table 吸／破真空 Port 對調，設定檔不推）。
  ⓘ 機台樹另有 8979fea（TEACH-3AXES）、e696967（1203-LINKLOST）已 commit、還在建置測試，下一次推。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0139 HTDESIGNER-145（0.145.0）—— 筆電包 108 的外掛部分合進機台：ES02 的 0.144–0.145（尋找框 Ctrl+Shift+F：一個框、選範圍〔這個檔／專案／整個方案／C++／網頁／BCB6，Big5 照讀〕；Ctrl+F 也開這個框，預設「這個檔案」，設定 ht9045Designer.ctrlFFind 可關）。三方合併（基準＝包 107）：4 個只有 ES02 改、2 個兩邊都改都沒有衝突；機台的 tools 0136 Alias 補丁保留。包說明寫 0.144，package.json 是 0.145。C++ 部分＝整合工作階段的 cpp 0071。機台測試：程式庫 176/176、探針 7/7＋分頁樹 7/7、假 VS Code 211/211、面板全過；e2e_build.ps1 沒跑。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0073 8979fea TEACH-3AXES —— MOutShuttle1／2 左右、MCCDY 檢測／校正的教導點（W906 擴充，原版沒有；EastSun 1001「三軸都幫我加 在合適的地方」）：
                    產生器加 6 列（只在 IO_CARD_TYPE==PCI1203_IO）、teach.ini 鍵＝欄位名；Teach 互鎖照原版 In Shuttle／Index Y 寫法
                    （出料飛梭：MTestZ1／Z2＋所有入出料手臂 Z 在原點；CCD Y：MTestZ1／Z2）。ctest TeachButtonsGen、WebMotorAccess、GaliRouteEngine 通過。
  cpp 0074 e696967 1203-LINKLOST —— 1203 斷線 10 秒跳原版 WAR16152（EastSun 1001「1203 如果斷線10秒 要跳出異常」）：看過 OP 的站連續 10 秒不在 OP／讀失敗、
                    或卡片不回／沒開 → 一次 WAR16152（照原版停馬達、記事件），恢復後重新計時；開機還沒連上過不跳。只用輪詢已經在讀的資料，沒有新的 SDK 呼叫。
                    ctest Pci1203LinkWatch 43/43；pci1203_readonly_gate PASS。⚠ 實機拔線測試還沒做。
  web 0057 530fd74 TEACH-HIDEAXIS／TEACH-SPEEDBAR／TEACH-3AXES (web) —— Teach 頁：馬達表沒有或 Enable=0 的軸全部藏（整框藏光就藏框）；
                    Speed Adjust 照原版橫捲軸能用（1～100，跟 Now Speed 互相同步，下一個命令帶給 C++）；三軸教導點的畫面。無頭瀏覽器實測。
  cpp 0075 2bae95f WORKLOG —— 工作紀錄補 10-01 第 28～32 項。
  ⓘ 測試前後 production_audit 0 變更（第一次的變動是 EastSun 20:46 關站寫的，重拍快照再跑一次 0 變更）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0076 3f4835f PKG-109 —— 筆電第 109 包（GitHub main bd20f84、GitLab c4ddcada）的 C++ 部分：主畫面 Timer2 加熱段照原版每秒跑（SetTemp 掛勾還沒裝）、
                    RotateKit 取料失敗重試先把 Z 移到安全高度（Jimmy 裁決）＋入料臂 CheckInArmZ 接回、HT9050 Shuttle 流程（包在 Type_HT9050 裡，今天行為不變）、
                    G-031 格式差異文件。22 檔（新 5、照收 16、合併 1：tests/CMakeLists.txt 檔尾兩邊的新測試都留）；0 重複列；LF 行尾已照原本行尾存。
                    o2 建置 0 錯誤、PE 52/52、7 個相關 ctest 通過（含 RotateKitRetry、Flow9050_Shuttle）、production_audit 0 變更。
                    設計外掛 8 檔（0.147）不在這裡，由設計外掛工作階段整合。
  cpp 0077 a4ffaaa VC4-SDO —— 打開 ECAT-VC4 的閥值 SDO 寫入（EastSun 1001「不要擋了」「VC4 沒有的 用VC8 去推論」）：VC4 照 VC8 手冊的物件表
                    （80n0:02 閥值模式、80n0:13 閥值），只開 VC0～3。ctest VacuumVc8、Pci1203LinkWatch 通過；pci1203_readonly_gate PASS。
  cpp 0078 879da37 WORKLOG —— 工作紀錄補 10-01 第 33～36 項（含機台 IO_Table 的 VC4 吸嘴感測列 64+VC，設定檔不推）；第 3 節補 108（tools 0139）、109。
  ⚠ 給筆電：機台的 IO_Table 吸嘴列已改成 VC4 格式（站 160～162：_On 偶數＝吸、_Off 奇數＝破、感測 64+VC，VC＝DO 所在的 VC）；
     B／D／F／H 與 FTest AC／BC／AD／BD 在 VC4 上沒有通道。machines\HT9050\IO_Table.csv 要不要跟著改請 Jimmy 決定。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0140 HTDESIGNER-147（0.147.0）—— 筆電包 109 的外掛部分合進機台：ES02 的 0.146–0.147（方案總管上方的搜尋框，打字篩選檔案／頁面／元件；只剩一行輸入框；側欄每一區的標題列有底色和框線，像 Visual Studio 的工具視窗）。三方合併（基準＝包 108）：6 個只有 ES02 改、2 個兩邊都改都沒有衝突；機台的 tools 0136 Alias 補丁保留。C++ 部分＝整合工作階段的 cpp 0076。機台測試：程式庫 176/176、探針 7/7＋分頁樹 7/7、假 VS Code 213/213、面板全過；e2e_build.ps1 沒跑。
  ⚠ 給筆電：機台的 tools 0136（lib/aliasedit.js：Alias 清單跳過 IO 表的 # 分段標記列，含 lib 測試）到第 109 包還沒進 GitLab，請收；機台現在套到第 109 包，外掛 0.147.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0079 108b8e3 MT-SPDLIVE —— Motor Test 改速度直接生效（EastSun 1001「當我百分比速度有變動時 請要直接改速度 不然每次我按第二次 JOG 速度都不一樣」）：
                    參數表 InitialSpeed／JogHigh／JogLow／Acc／Dec、Jog High／Low 鈕、Copy From 一改，就用目前捲軸的 % 重算並寫進卡片（jog＋點位），
                    回覆框寫出新速度；HOME 進行中不改。跟原版不同（原版只改記憶體，要拉捲軸才生效）。ctest WebMotorAccess、GaliRouteEngine 通過。
  web 0058 e509570 MT-SPDLIVE (web) —— Speed 欄打 %＝把捲軸拉到那一格（捲軸跟著移、JOG 也用這個 %）。
  cpp 0080 cd0cb5c WORKLOG —— 工作紀錄補 10-01 第 37 項；第 3 節 109 補設計外掛部分（tools 0140，0.147.0）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0081 68ff728 PKG-110 —— 筆電第 110 包（GitHub main 0fa9db2、GitLab e6d4da65）的 C++ 部分：開機照原版設 RunInfo.Factory 與 Observer labFactory（E-BOOT-005）、
                    St02 S-14 主畫面 Timer3。14 檔（新 4、照收 8、合併 2：tests/CMakeLists.txt、tools/wb_serve.cpp，0 衝突、0 重複列）；
                    o2 建置 0 錯誤、PE 54/54、7 個相關 ctest 通過、production_audit 0 變更。設計外掛 11 檔（0.148）由設計外掛工作階段整合。
  cpp 0082 7b91d08 VC4-ODD —— ECAT-VC4 其實是奇數吸（17+2VC）、偶數破（16+2VC），跟 VC8 一樣（EastSun 1001 21:3x 實機：Vacuum Unit 按 v＝DO 19 在吸）。
                    先前「偶數吸」的寫法撤回；VC4 其他差異不變。ctest VacuumVc8、Pci1203Pure、Pci1203LinkWatch 通過；pci1203_readonly_gate PASS。
  cpp 0083 895e7ad WORKLOG —— 工作紀錄補 10-01 第 38～41 項（含機台 IO_Table：吸／破對調回來、VC4 沒有通道的 12 顆吸嘴 Enable=0，設定檔不推）。
  ⚠ 給筆電：機台 IO_Table 站 160～162 的吸嘴列＝_On 奇數、_Off 偶數、感測 64+VC（VC＝DO 所在的 VC），B／D／F／H 與 FTest AC／BC／AD／BD Enable=0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0141 HTDESIGNER-148（0.148.0）—— 筆電包 110 的外掛部分合進機台：ES02 的 0.148（方案總管重做成一格：搜尋框在標題下面、下面是樹，打字只留下符合的；不再列出 BCB6 原始碼，BCB6 照樣用「尋找」找得到）。三方合併（基準＝包 109）：5 個只有 ES02 改＋3 個新檔原樣複製（panels_render.ps1 保留 BOM）、3 個兩邊都改都沒有衝突；機台的 tools 0136 Alias 補丁與測試保留。C++ 部分＝整合工作階段的 cpp 0081。機台測試：程式庫 177/177、探針 7/7＋分頁樹 7/7、假 VS Code 213/213、面板全過；e2e_build.ps1 沒跑。
  ⚠ 給筆電：機台的 tools 0136（lib/aliasedit.js：Alias 清單跳過 IO 表的 # 分段標記列，含 lib 測試）到第 110 包還沒進 GitLab，請收；機台現在套到第 110 包，外掛 0.148.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0059 b9f3566 MT-SAVEMOT-2 (web) —— Motor Test「回寫 Mot_Table」寫成功不再跳結果視窗，結果寫在狀態列（EastSun 1001「這視窗不要出現了」）；沒寫進去照舊跳視窗。
  cpp 0084 93794ab WORKLOG —— 工作紀錄補 10-01 第 42～43 項；第 3 節 110 補設計外掛部分（tools 0141，0.148.0）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0085 fa7e281 TEACH-SVON —— Teach 的 Set 預設改成激磁教導（伺服保持開、直接讀位置），勾「手動教導」才關伺服用手推（EastSun 1001「我這邊預設值是 激磁教導」）；
                    跟原版不同（原版 Set 一律手動教導）。ctest WebMotorAccess（新增激磁教導段）、GaliRouteEngine、TeachButtonsGen、WebTeachLeave 通過。
  web 0060 f56a47b TEACH-SVON／TEACH-HOMEOK／WIREBAR-THRU (web) —— Teach 右邊加「手動教導」勾選與第 11 顆燈「Home OK」（回原點完成）；
                    所有頁面上方的「讀取完成」狀態條改成滑鼠點穿（theme.css）。無頭瀏覽器量過不重疊。
  cpp 0086 58a2141 WORKLOG —— 工作紀錄補 10-01 第 44～47 項（含馬達頁全面檢查的結果）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0087 cdd98ed TEACH-SPD-AXIS —— Teach 的速度「有沒有變」改成每一軸各自記：換軸後 JOG 速度一定照畫面顯示的寫進去（Teach 檢查：21:55:38 MInArmY 畫面 1%
                    實際用 Motor Test 留下的 100%）。ctest WebMotorAccess（新增換軸段）、GaliRouteEngine、TeachButtonsGen、WebTeachLeave 通過。
  cpp 0088 c93a3bf PKG-111 —— 筆電第 111 包（GitHub main e8c4536、GitLab bdc0f1ba）的 C++ 部分：St02 G-023 TesterTCP Open／Short 報告、E-T1-022 第二型 Bin 編號面板。
                    14 檔（新 4、照收 9、合併 1，0 衝突、0 重複列）；o2 建置 0 錯誤、PE 56/56、6 個相關 ctest 通過、production_audit 0 變更。
                    設計外掛 16 檔（0.150）不在這裡，由設計外掛工作階段整合。
  cpp 0089 938605c WORKLOG —— 工作紀錄補 10-01 第 48～51 項：21:58 撞機調查（Loader Go 照原版 X、Y 同時走直線；SHT1 畫面上的值沒存）、Teach 換軸速度、
                    Teach／馬達頁全面檢查（含 1203 頁可在伺服關時放開垂直軸煞車）、第 111 包。
  ⚠ 給筆電：撞機調查與全面檢查的結果在 docs/WORKLOG_MACHINE.md 第 48～50 項，修正要等 EastSun 決定（Go 要不要走安全路徑、1203 頁煞車互鎖）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0142 HTDESIGNER-150（0.150.0）—— 筆電包 111 的外掛部分合進機台：ES02 的 0.149–0.150（執行列綠色 ▶＝建置並啟動 wb_serve、☑ 模擬／☑ Debug、建置進度條與取消、⏸ 中斷到除錯器；側欄藍灰外框；沒選元件時屬性面板顯示按鈕的範例並反灰；方案總管搜尋字上色）。三方合併（基準＝包 110）：12 個只有 ES02 改＋2 個新檔原樣複製、2 個兩邊都改都沒有衝突；機台的 tools 0136 Alias 補丁與測試保留。C++ 部分＝整合工作階段的 cpp 0088。⚠ 機台上綠色 ▶ 會真的建置並開機台軟體（不勾模擬＝出貨組態；勾了模擬 1203 卡照樣開）。機台測試：程式庫 177/177、探針 7/7＋分頁樹 7/7、假 VS Code 214/214、面板全過；e2e_build.ps1 沒跑。
  ⚠ 給筆電：(1) 機台的 tools 0136（lib/aliasedit.js：Alias 清單跳過 IO 表的 # 分段標記列，含 lib 測試）到第 111 包還沒進 GitLab，請收；(2) media/htd_build.ps1 檔頭寫只用 ASCII，但第 1、2 行註解有中文（只在註解，不影響執行）。機台現在套到第 111 包，外掛 0.150.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0061 345184d TEACH-GOHINT／SIDECMD (web) —— 撞機防呆：Teach 的 Set／Go 一停上去（或按下）就把它會用的那幾格橘框標出來、狀態列寫「Go → Loader：MInArmX=…」
                    （21:58 撞機：Input Arm 機構圖 Loader 的 Go 畫在數值表 SHT1／SHT2 欄正上方）；開程式後 30 秒按鈕被丟掉的問題改走背景通道
                    （motor-access.js sendSide：Teach 開頁查詢、Motor Test formShow／formClose）。無頭瀏覽器實測。
  cpp 0090 cd86928 WORKLOG —— 工作紀錄補 10-01 第 52 項；第 3 節 111 補設計外掛部分（tools 0142，0.150.0）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0062 34ee63e MOTORVIEW-LED (web) —— Motor View 的 11 顆燈照原版（Ready＝HomeFlag 綠燈，其他 10 顆＝MOT[].Led[] 紅燈），讀不到畫斜線，
                    數字欄讀不到顯示「---」（不再假裝是 0）。
  web 0063 d0b4ddb TEACH-HIDEPT／TABS／UNWIRED-2／TYPED (web) —— Teach 1491 個元件逐一檢查（不抽樣）：這台沒有或 Enable=0 的軸的 Set／Go 藏起來
                    （515 顆、欄位 219 格、空群組一起收）；原版在這台會藏的 5 個分頁照 Gerneral.ini 藏；沒接功能的按鈕變灰＋按了說原因；
                    打過字的格子一定照送（原版 GoButton020Click 讀畫面上的字）。
  cpp 0091 0d1d592 WORKLOG —— 工作紀錄補 10-01 第 53～54 項（撞機：不是原版沒寫入也不是沒合好——按到 Loader 的 Go；Teach 逐一檢查結果）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0092 ae813b6 PKG-112 —— 整合筆電第 112 包（GitHub main 2787258、GitLab 65330849）：Jerry J-7 TestSocket 起始位置、J-10 DoCheckSocketHasIC 解開閘門
                    （只在生產流程 DoTestY case 20 呼叫，非模擬機台上會動 Index）、St02 H-013 第 1 部分、sync_web.py。設計外掛部分＝tools 0143（設計外掛工作階段）。
  cpp 0093 77a1741 TEACH-ZALLUP —— Teach 的 In/Out Z All Up（golden uteach.cpp:4466／:4480）與 Index 分頁四顆 Servo（:4253／:4270／:2919）照原版接上。
  cpp 0094 37117e3 HOMEMON —— Home Monitor 的 C++ 半邊：JsonBridge/ChanHome（home.* 標籤）＋ act.home.abort＝golden sbAbortHomeClick（只在 fHome->fShow 時）。
  cpp 0095 cc8183f WORKLOG —— 工作紀錄補 10-01 第 55～58 項；第 3 節 112；第 4 節兩件待決定。
  web 0064 5d2800f TEACH-ZALLUP (web) —— 那 6 顆按鈕的網頁半邊（ht9045_teach_zallup_c.js、motor-access.json 6 列）。
  web 0065 d758607 HOMEMON (web) —— Home Monitor 不再是假資料（列、燈、位置、紀錄來自 C++）；Abort Home 沒有確認視窗；視窗改成不擋其他視窗。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0096 af53dc4 TEACH-FORMSHOW —— Teach 照原版 FormShow 顯示／隱藏：C++ 用原版條件式、機台真正的設定算出約 150 個元件的 Visible／分頁／標題／顏色／Enabled／開在哪一頁`n                    （FileRW/TeachFormShow_File.cpp，第二型 bridge，只顯示）；FormState 加顏色與依名字切分頁；ctest TeachFormShowBridge 28/28。`n  cpp 0097 826d04b WORKLOG —— 工作紀錄補 10-01 第 59 項。`n  web 0066 e34f7df TEACH-FORMSHOW／ALLCOMP (web) —— 開頁與每次開窗套用 C++ 算好的 FormShow（無頭瀏覽器 186 項全部套上）；TTL 勾選框灰掉＋說明。`n  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。`n
  web 0067 9d49661 CLIPFIX (web) —— 畫面截掉：80 頁、每個分頁量過，249 個截斷修掉 196 個、6 個變小，剩 54 個都有原因（golden 本來就捲動、等決定的視窗大小…）；`n                    群組框標題被切掉 4px（25 頁）、Setup 勾選框跑到畫面外、Offset 照原版寬度、Observer 的 Exit 被捲軸蓋住等；放大倍率時視窗會留在畫面內。`n  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。`n
  tools 0143 HTDESIGNER-154（0.154.0）—— 筆電包 112 的外掛部分合進機台：ES02 的 0.151–0.154（方案總管上方的執行工具列〔綠色 ▶＝建置並啟動 wb_serve〕、偏黑的工具視窗樣式；屬性表能改底線／刪除線／換行／唯讀／字數上限／打勾／Tab 順序；工具箱補齊 Button、BitBtn、單選、Memo、單選群組、分頁、圖片、色塊、分隔線並分類；改名時網頁程式一起改）。三方合併（基準＝包 111）：17 個只有 ES02 改、3 個兩邊都改都沒有衝突；機台的 tools 0136 Alias 補丁與測試保留。C++ 部分＝整合工作階段的 cpp 0092。
                    機台補丁（版本號不變）：C++ 索引、尋找、方案總管不再掃樹裡的 .claude 資料夾（別的工作階段的幫手在 .claude\worktrees\ 開了整份原始碼的複本，每個函式找到兩次、屬性面板要等 30 秒）；smoke「檢查所有頁面」改成自己數同一批頁面（HW.home.html 被修好後寫死的 290 誤判）。機台測試：程式庫 178/178、探針 7/7＋分頁樹 7/7、假 VS Code 216/216、面板全過；e2e_build.ps1 沒跑。
  ⚠ 給筆電：請收機台的 (1) tools 0136（lib/aliasedit.js＋test/run_tests.js：Alias 清單跳過 # 分段標記列）、(2) tools 0143 的 .claude 跳過（lib/cppindex.js、lib/projectsearch.js 的 SKIP_DIR、run_tests.js 的測試）與 smoke 的頁面檢查改法。機台現在套到第 112 包，外掛 0.154.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0098 e5551f2 WORKLOG —— 工作紀錄補 10-01 第 60 項（畫面截掉全站修正）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0068 d50fd77 TEACH-LOCK (web) —— Teach 照原版顯示鎖定：選取的那一軸在動時「*Lock by … moveing」、pnlStop 變黃、MoveN／MoveP／Home／MoveTo 鎖住（每軸獨立，同 Motor Test）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0069 0f8a3ef TEACH-ALLCOMP (web) —— Teach：點「Active Motor」標籤切到 Axle Control 分頁（原版 Label2Click）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0099 3d0dfee WORKLOG —— 工作紀錄補 10-01 第 61 項（Teach 全元件收尾）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0100 28ddd34 PKG-113-114 —— 整合筆電第 113～114 包（GitHub main e9f0fc7）：St01 D-026 警報 Note 密碼（dialog.auth）、St02 DIO 頁 Delete 鈕、cMyDB、TcpCmdServer 測試；
                    3 檔衝突全部兩邊都留（wb_serve.cpp:671、WebBridgeServer.cpp:1448、tests/CMakeLists.txt）。設計外掛部分＝設計外掛工作階段。
  cpp 0101 fb2db79 WORKLOG —— 工作紀錄補 10-02 第 62 項；第 3 節 113～114。
  web 0070 840bed7 PKG-113-114 (web) —— DIO 頁 Delete 鈕（ht9045_dio_delete.js）、警報 Note 密碼（ht9045_dialog_host.js）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0144 HTDESIGNER-157（0.157.0）—— 筆電包 113 的外掛部分合進機台：ES02 的 0.155–0.157（設計畫面的機台螢幕大小框線、用點的設定 Tab 順序、ComboBox／RadioGroup／Memo 的項目清單編輯器）；包 114 沒有外掛。三方合併（基準＝包 112）：9 個只有 ES02 改＋1 個新檔原樣複製、4 個兩邊都改都沒有衝突；機台自己的 tools 0136（Alias）與 tools 0143（跳過 .claude、頁面檢查自己數）都保留。C++ 部分＝整合工作階段的 cpp 0100。機台測試：程式庫 179/179、探針 7/7＋分頁樹 7/7、假 VS Code 219/219、面板全過；e2e_build.ps1 沒跑。
  ⚠ 給筆電：機台的 tools 0136 與 tools 0143 的外掛補丁還沒進 GitLab，請收（見 tools 0143 那段）。機台現在套到第 114 包，外掛 0.157.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0102 e4e61ee WORKLOG —— 第 3 節 113～114 補上設計外掛部分＝tools 0144（0.157.0）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0103 ba6e984 PKG-115-117 —— 整合筆電第 115～117 包（GitHub main 1dc9241）：Lot Info Tester TCP Show 照原版、St02 計時器修正、TesterIF 小鍵盤上下限照原版、G023 測試；
                    沒有設計外掛檔。
  cpp 0104 243df4c WORKLOG —— 工作紀錄補 10-02 第 63 項；第 3 節 115～117。
  web 0071 d2b310c PKG-115-117 (web) —— Lot Info TCP Show（ht9045_lotinfo_testertcp.js；Data.LotInfo.html 與機台的截斷修正合併）、TesterIF 小鍵盤。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0105 4570e45 COMPK-DATAS —— 資料／權限頁全元件檢查的 C++ 半邊：Observer 照原版藏分頁＋Time Data、Counter Selection 存檔後的 NeedRef、StartCondition 警報上限；ctest CompK_DataS。
  cpp 0106 745b8a8 WORKLOG —— 工作紀錄補 10-02 第 64 項；第 4 節資料頁要決定的事。
  web 0072 5e4a5bd COMPK-DATAS (web) —— 13 頁 3,392 個元件逐一判定：修 46 個、要決定 220 個、做不到 9 個；原版沒有確認框的存檔不再多問。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0107 2a06d6e SETUPA-N04／KB —— 設定頁全元件檢查的 C++ 半邊：Configuration 的 Host／IP（原版建構子）、Contact 小鍵盤執行期上下限與輸入後修正；ctest SetupA_KbExtraN04。
  cpp 0108 3defd66 WORKLOG —— 工作紀錄補 10-02 第 65 項；第 4 節設定頁要決定的事。
  web 0073 fa6aea5 COMPK-SETUPA (web) —— 12 頁 5,730 個元件逐一判定：Configuration 又存得進去（原版 Exit 的那一題）、ContactForce 接上、小鍵盤照原版、反灰欄位不開鍵盤。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0109 ec6a4e1 WORKLOG —— 工作紀錄補 10-02 第 66 項；第 4 節：警報 Note 的安全問題（網頁按鍵沒照原版的 IC 在座鎖、PTI 的 START）等。
  web 0074 812a16b COMPK-MOTORB (web) —— 29 頁 4,270 個元件逐一判定、修 457 個：小鍵盤照原版上下限、Speed 的 47 個微調鈕、ShowMessage 不再顯示假數字、testercomm Run Mode 照原版只在模擬時出現。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0110 58f1886 IOSV-FORMSHOW／SHOWALL／HSYS-EVT —— IO 頁照原版 FormShow（第二型 bridge FileRW/IoSetViewFormShow_File.cpp）、chkShowIndexAll、HandlerSys 點擊邏輯在 C++ 照做；ctest IoSetViewFormShowBridge。
  cpp 0111 ab47c0a WORKLOG —— 工作紀錄補 10-02 第 67 項；第 4 節 IO 頁要決定的事。
  web 0075 fd1d4e1 COMPK-IOS (web) —— 7 頁 4,411 個元件逐一判定：IO 頁開窗套用 C++ 算好的 FormShow、IO Table 篩選照原版、HandlerSys／LtcSensor、沒移植的硬體表單反灰＋原因。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0076 6510023 COMPK-KB-MERGE (web) —— 兩支原版小鍵盤表合成一支（ht9045_golden_kb_unwired.js），原本兩支都攔同一個事件、互相蓋掉；7 頁改載入合併後那支，舊檔移除。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0112 6be18de NOTE-KEYGATE —— ⚠ 警報 Note 畫面上的選擇鍵照原版上鎖（IC 掉進測試座要先開 Index 門按 Z1、接觸過壓、Auto Clean 門、換清潔墊、Safe Lock、Index Jam、Auto Retest Jam）；
                    原版 BtnSkipClick＋UpdateButtonStatus（note.cpp:2764-2870）；面板實體鍵本來就有，網頁路徑以前沒有。ctest NoteWebKeyGate。
  cpp 0113 c3df6a2 WORKLOG —— 工作紀錄補 10-02 第 68 項。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0114 fb75cde PKG-118-119 —— 整合筆電第 118～119 包（GitHub main 58f3487）：VTEST 機台 TesterIF 最長測試時間 0～36000；St01 Lot Start 檢查函式（還沒接，行為不變）；小鍵盤第一分支稽核文件。
  web 0077 5968853 PKG-118-119 —— 同上（ht9045_testerif_c_wire.js）。
  cpp 0115 54115e4 WORKLOG —— 工作紀錄補 10-02 第 69 項、§3 一列、§4 Teach 頁教導值上下限。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0116 8dd3860 TEACH-KB-AUDIT —— ⚠ Teach 小鍵盤範圍在這台的實測（唯讀，沒改程式）：照 golden 補會把 11 個真實教導位置改掉（這台解成 9046LS，但尺寸像 HT9045）；
                    523 欄全部列舉，全文 docs/TEACH_KB_RANGE_HT9050_20261002.md，要 EastSun 選 A／B／C。工作紀錄第 70 項。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0117 f507fd5 TEACH-KB —— ⚠ Teach 每一欄的小鍵盤照原版（EastSun 裁決 C，這台走 9046LS 臂）：C++ 照原版 40 個處理函式算 523 欄範圍（FileRW/TeachKb.cpp），
                    開頁 extra 送給網頁；OK／Abort 都照原版夾；那 11 個在用的位置點開就會被夾。ctest TeachKb。
  web 0078 f2523ed TEACH-KB —— 網頁那一半（ht9045_teach_kb_c.js）。
  cpp 0118 ce40df1 WORKLOG —— 工作紀錄第 71 項。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0119 b2e2720 NOTE-SCREENSTART／IO-FORMSHOW-OUT —— ①警報 Note 畫面上的 START 照原版不做事（這台 PTI；面板實體 START 鍵照常啟動）；
                    ②IO 視窗打開時 C++ 照原版抱住 Index 煞車（SwFMotorBreaker／SwBMotorBreaker Off）並記錄 Index 四軸位置，運轉中不做。ctest IoFormShow。
  cpp 0120 befb016 WORKLOG —— 工作紀錄第 72～73 項；§4 記 EastSun「剩下照 BCB」。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0121 a87f2a5 F5-PROGRESS／HMI-SECOND —— F5 的等待畫面顯示編譯進度（tools/build_with_status.ps1 寫進度、tasks.json 三個編譯改走它）；
                    HMI 視窗程式第二次啟動時把網址交給已開的視窗（舊視窗直接換到等待畫面）。
  web 0079 fa21814 F5-PROGRESS —— boot_wait.html 每秒讀進度：編譯 NN%／目前檔案／完成／失敗＋第一個錯誤。
  cpp 0122 0b4c97d WORKLOG —— 工作紀錄第 74 項。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0123 637123a NONSTOP-SEM —— 測試：C++ 的 ShowErrorMessage 告警一律走停機頁（ctest NonStopRoute）。
  web 0080 006f2b2 NONSTOP-SEM —— 不停機小窗不再把 C++ 的停機告警（WAR1676 權限不足、WAR1677、WAR1681）畫成「機台未停機，仍在運轉」。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0124 a4c5673 PKG-120-123 —— 整合筆電第 120～123 包（GitHub main 15895cf）；Teach 頁筆電的反灰清單排除機台已接好的 6 顆。
  web 0081 ac1225f PKG-120-123 —— 同上（19 支接線檔範圍、qwerty 小數、Teach 頁、CommView、ht9045_kb_generic.js）。
  cpp 0125 b651217 WORKLOG —— 工作紀錄第 75～76 項、§3 一列。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0126 0192f29 F5-PROGRESS 復原 —— F5 的三個編譯工作改回直接呼叫 cmake（09:33 F5 編譯沒有啟動；tasks.json 回到早上之前的內容）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0127 8b266d6 F5-EXTCON —— F5（不接除錯器）改成在獨立的程式視窗跑 wb_serve，不再放在 VS Code 的終端機裡（今天 F5 啟動的三次都在 10～20 秒內被結束）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0082 b1f0d95 TAGS-RECONNECT —— 主框架的機台狀態連線斷了會每 2 秒自動重連（之前 wb_serve 重開後畫面一直以為「機台運轉中／訊號不可知」，Teaching 等頁打不開）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0128 3b2dfcf OBS-TEMPSERIES —— 修正 wb_serve 開機 10～20 秒後自己中止（筆電第 116 包的每分鐘溫度紀錄去畫 Observer 溫度圖，圖上一條曲線都沒建，取第 0 條就丟例外）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0129 09ba9e9 WORKLOG —— 工作紀錄第 77 項：Teach 打不開的原因與修正（EastSun 1002 按 F5 實測，Teach 可以進入）。
  ⚠⚠ 給筆電（EastSun 1002：「通知main更新」）：**請盡快把 cpp 0128（OBS-TEMPSERIES）收進 GitLab main**。從第 116 包起，只要 Observer 視窗算開著，
     每分鐘的溫度紀錄（MainTimer3.cpp RecordTemp）就會呼叫 TfObserver::UpdateTempChart，但 TempChart 在靜態初始化時沒有曲線（cObserver.cpp 建構子的
     AddSeries 迴圈因 INIFileGeneral==0 跳過），Series[0] 丟 std::out_of_range、沒人接 → wb_serve 開機 10～20 秒就 terminate（WER 0x40000015），
     網頁斷線、Teach 等頁被擋。修法只動 cObserver.cpp（第一次用到時照原版補建曲線與下拉選項、ItemIndex-1 超出範圍就不畫）與 forms/fObserver.h（SeriesCount()），
     不移動行號。也請一起收 web 0082（TAGS-RECONNECT：background.html 的 tag 連線斷了每 2 秒重連）。cpp 0127（F5-EXTCON）只改 .vscode/launch.json，main 不用收。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0083 34fcbbd HSYS-ENTRY —— 機況監視列的「⚙ Handler System」入口重新看得到（0930 LAYOUT110 把每頁底部的程式對應說明列一律藏起來，入口剛好放在那一列裡，被一起藏掉）；搬到 OCR 那一行的右邊，點了一樣問密碼再開 Handler System。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  tools 0145 HTDESIGNER-157a（0.157.0，版本號不變，機台補丁）—— 第一次用真的 VS Code 跑第 4 層（0.79 之後一直被 VS Code 更新擋住）抓到：0.140 的版面在開 C++ 檔時會關掉頁面，之後從屬性面板「跳到程式碼／HTML」會讀到已關掉的設計畫面而整個中斷（Webview is disposed），已修；第 4 層的時限 180 秒太短改 600、測試用 design 版面跑原本的檢查：159 項 156 過，失敗的 3 項是 0.139 已經併進方案總管的「頁面」清單檢查（測試過期）。另外 head 裡第一句就 location.replace 的頁面認得是轉址頁（Setup.Configuration／Setup.DIOInterFaceCFG）。三層全過（程式庫 179、假 VS Code 219）。
  ⚠ 給筆電：(1) 請收機台的 tools 0136、0143、0145；(2) 第 4 層要照方案總管重寫「頁面」清單那 3 項，並替 0.140 的 wpf 版面（上下兩格、開 C++ 會關頁面）加檢查。機台現在外掛 0.157.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  web 0084 9253691 GRIDKB —— Motor Database 與 IO Table 表格的小鍵盤照原版：名稱欄只能英數字、GearRatio／Acc／Dec 小數、其他整數（以前每一格都開完整英文鍵盤，可以打出 0.0.071425 這種數字；M37 MColorZ 的 GearRatio 現在就是這個值，程式讀成 0）。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0130 7f05533 PKG-125-126 —— 機台整合筆電第 125～126 包（GitHub main 2efc840）。TEACH-KB 筆電暫不收，機台照舊保留自己的（CMakeLists 的 FileRW/TeachKb.cpp、HW.teach.html 那一行、tests/CMakeLists.txt 的 TeachKb 測試段）。
                    tests/CMakeLists.txt：第 125 包的基底比機台舊，三方合併會把機台的測試段重複一次（109 行），所以改用筆電版＋機台的 TeachKb 段。設計外掛 13 檔沒碰（ES02 的；機台端已停手）。
                    o2 編過；ctest 11/11；網頁 E021_ObserverPage 30/0、KbMachineSetting 138/138、Stream2e 27/0、NonStopRoute 8/8；PE 92/92。
  web 0085 365189d PKG-125-126 (web) —— 同一包的 5 個網頁檔（Data.Observer 保養分頁、TrayForm、kb_generic、observer_wire；新檔 ht9045_observer_ev.js）。
  cpp 0131 0c87d87 WORKLOG —— 工作紀錄第 78～79 項。
  ⚠ 給筆電（第 125 包請機台回答的兩題，只讀，沒有改任何設定）：
     ① W-07  D:\HT9045\system\Gerneral.ini：[Ground_Man] USE_GROUND_MAN=0、Ground_Man_COM_PORT=COM18（Ground_Man_ScanPoint=0、Ground_Man_AlarmOhm=0）；USE_OTD=0。
     ② W-13  D:\HT9045\config\config.ini（09/28 11:28）：
          [In/Out Arm] bE33InOutArmZOffsetSameOne=1、bE34InOutArmPitchZOffsetSameOne=0、bAutoCleanUseHotplate(E43)=0、bE43_1_AutoCleanCountSaveFolder=0、
          bE46_LoaderUse2Offset=0、bE47_ShuttleUse4Offset=0；bE30InArmUseDifferentScale／bE30_1…_Hot／bE30_2…_Cold、bE75_InArmHeightFollow7000(E88)：檔裡沒有這幾個鍵（用程式預設）；
          [Tray] P06_LoaderUseCarrierTray=0、bE74_InspectArmPosition：檔裡沒有；[Function] bA27EnableLightScale=0。
          bUseTrayBlockMode、bHotPlateMove1CM：不是 config.ini 的鍵，由 CosFunction.cpp 依客戶碼設定（不在檔裡）。
        teach.ini（機台實際用的是 D:\HT9045\_integ_ioweb\runcfg\system\teach.ini，10/02 14:06）：
          In  X：Loader 21301、HP2 1892、InSht1 32441、AutoClean 11284（InPick 20903）；Y：Loader -55884、HP2 -6723、InSht1 -37718、AutoClean -85342（InPick -57992）；
              Z：[MInArmZE] SetEditPickLoader=-1600、SetEditAutoClean=0，[InArm] AutoCleanPick=-1220；HP2／InSht1 的 Z 鍵檔裡沒有。
          Out X：Auto1 -53595、Auto2 -35078、Auto3 -16538、Fix1 -35657、Fix2 -21371、Fix3 -7061、OutSht1 -48846（OutPick -53548）；
              Y：Auto1 -55618、Auto2 -55633、Auto3 -55601、Fix1 -10234、Fix2 -10219、Fix3 -10233、OutSht1 -37595（OutPick -57881）；Z：只有 [MOutArmZE] SetEditPlaceFix2=0。
          [InArmZSub] Picker Aa=50、[OutArmZSub] Picker Aa=30。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔（上面只是抄數值）。
  tools 0146 HTDESIGNER-161（0.161.0）—— 筆電包 126 的外掛部分合進機台：ES02 的 0.158–0.161（圖片元件可以換 Picture、方案總管外框到底、「群組到…」可選 Panel／GroupBox、選取時四邊都標邊距）。三方合併（基準＝包 125）：9 個只有 ES02 改、4 個兩邊都改都沒有衝突；機台的 tools 0136／0143／0145 補丁都保留。C++ 部分＝整合工作階段的 cpp 0130。機台測試：程式庫 179/179、探針 7/7＋分頁樹 7/7、假 VS Code 221/221、面板全過；真的 VS Code 156/159（已知 3 項「頁面」清單檢查過期）；e2e_build.ps1 沒跑。包 127 沒有外掛。
  ⚠ 給筆電：機台的 tools 0136、0143、0145 外掛補丁還沒進 GitLab，請收。機台現在外掛 0.161.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0132 8a5f75b BOOT-INITMOTOR —— 開機開卡、馬達送電 1 秒後，照 golden 對 M35 MLoaderZ 跑一次 InitMotor（清錯、設定表含 SensorType→ORG 極性與 In1Logic→ALM 極性、最大速度、Servo ON、座標歸零），結果寫在 oplog。EastSun 1002 要求，先只做 MLoaderZ。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0133 bf0b457 PKG-127 —— 機台整合筆電第 127 包（GitHub main bce78fd、GitLab 025fdabe）：批次 41（提示框按確認照原版暫停、會擋的警報按 PAUSE 送 DoPause＋ESD 停止）、Ifor !106（I-03 溫控器序列埠）。web 兩檔與機台的 0083／0084 相同，沒有 web patch。
                    兩個機台本地改過的檔（tests/CMakeLists.txt、tools/wb_serve.cpp）用包內的 base_8f3cdc53（第 126 包）三方合併，0 衝突。o2 編過；ctest ThermoComm／NoticeAck 2/2；PE 94/94。
                    第 128 包（HT9050-ORG-ST，要先把 Mot_Table 16 列改 SensorType=1）這次沒套：EastSun 1002 決定先不套。
  ★ 新增：machine_params\ 與 workorder\（EastSun 1002：給同仁測試、確認用）。
     machine_params\ = 機台參數快照：D_HT9045_system（D:\HT9045\system）、D_HT9045_config（D:\HT9045\config）、runcfg（SetUp.inf、teach.ini、config\；不含 logs）。
     workorder\ = 目前工單：SetUp.inf 指定的配方資料夾（現在是 IOWEB_TEST_R003）＋ LastSet.ini。
     之後機台每次推送都會重拍（鏡像），git 歷史就是設定的歷史；說明在各資料夾的 README_PARAMS.txt／README_WORKORDER.txt。
     ⚠ 照原樣放、沒有遮任何值（含密碼檔），EastSun 裁決。是 HT9050 這台的設定，別台不要整包覆蓋。
     推送工具也放進來了：_tools\push_stage.ps1。
  ⚠ 給筆電：第 128 包機台還沒套（EastSun 先不套）；機台現在 = GitHub main bce78fd（第 127 包）＋機台修改。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0134 0a7f2de WORKLOG —— 工作紀錄第 80 項（第 127 包、machine_params／workorder 快照、推送規矩改成「編譯過就推、附參數與工單」）。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0135 7508a5a PKG-128 —— 機台整合筆電第 128 包（GitHub main 5c3a4a8、GitLab 116a0809）：NB2 !119 HT9050-ORG-ST（1203 HOME 燈、Teach「Z 在原點」、全機 HOME 原點確認改成逐軸照 Mot_Table SensorType：1＝ORG low 在原點、0＝high）、Ifor !109 SetTempSave、St02 !108 測試。
                    EastSun 1002 實測「沒遮到 home sensor 時 1203 讀 0」（10-01 的「0＝在原點」讓每顆 HOME 燈都亮），選第 128 包的規則。
                    ⚠ 包裡要求先把機台 Mot_Table 改成 SensorType=1——**沒做**：那等於維持「0＝在原點」＝剛量到的錯誤方向。16 列 SensorType=0 維持（high＝在原點，符合實測）；MInArmX／Y／ZA（SensorType=1）待遮 sensor 確認。Mot_Table 沒動。
                    五個機台改過的檔用包內 base_025fdabe 三方合併 0 衝突。o2 編過；ctest 8/8；PE 95/95。
  cpp 0136 5472554 ENGHOME —— 主畫面全軸 HOME 送到 1203 卡：打開 WB_ENGINE_MOTOR_1203＋WB_PUMP_1203_START_RING（EastSun 1002「兩個都開」；開卡時 ring 0 下 Acm_MasStartRing；引擎的馬達動作——HOME、之後 START——都會真的送到卡）。閘門腳本期望值同步。F5（★★）改回 release（不接除錯器）；DEBUG 設定拿掉「1203 唯讀」字樣、加檢查舊程式。
  ⚠ 給筆電：機台已套第 127～128 包；第 128 包的 Mot_Table SensorType=1 指示機台沒照做（理由見 cpp 0135），machines/HT9050/Mot_Table.csv 那 10 列改 1 對這台是反的，請確認。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0137 5825146 ORG-INV —— EastSun 1002「分支1203時 teach 和 mottest 頁面home燈號都反向」：MachineType.h 新增 #define W906_HT9050_ORG_INVERT，Motor Test／Teach 的 HOME 燈、Teach「Z 在原點」互鎖、全機 HOME 的原點確認一起反向（只有畫面反會跟互鎖說相反的話）。理由：19 軸 SensorType=0、沒遮感測器時 1203 每軸讀 1、燈全亮（20:2x 同樣情況讀 0，位元意義待查）。註解掉＝第 128 包原規則。o2 編過；ctest WebMotorAccess、MotorPoints_HT9050 過。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0138 71e7d7b ZHOME-200 —— EastSun 1002「Z軸歸原點 … 往回 200 就好」：所有軸照舊由驅動器回原點（124／128，驅動器設原點）；名稱以 Z 或 Z＋一個字結尾的 1203 軸（MLoaderZ、MEmptyZ、MAuto1～3Z、MInArmZA、MOutArmZA、MTestZ1）回完後用 HomeLowSpeed 朝回原點的同一方向再走 200 pulse，走完才算回原點完成（arm Z 之後照舊去 ZSafePos）。Motor Test 單軸與主畫面全機 HOME 都有。開關 MachineType.h W906_Z_HOME_BACKOFF_PULSE（負數＝反方向＝原版 SYNTEK 的 iHomePitch 方向）。o2 編過；ctest WebMotorAccess、Pci1203MotorRoute 過。
  ⚠ 給筆電：fc2fe83 的 SensorType=1 更正機台沒照做——EastSun 1002 實測沒遮到時 1203 讀值會變，機台現在是 SensorType 全 0＋ORG-INV（cpp 0137）。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0139 b0c82ac WORKLOG —— 工作紀錄第 81～85 項。★ 機台設定檔：D:\GPIB9045\system\general.ini 的 [Version] Model 9045GPIB→9050GPIB（EastSun 1002）。0930 已是 9050GPIB，10-02 21:46:58 被一支 GPIB 程式整檔寫回 9045GPIB（同秒 LastFile=…\2026-10-02 21 46 58.txt），HT9050 原點規則整段沒啟動、燈號＝1203 原始位元——這就是「反向了燈還亮」的原因。1203 原始 ORG 位元：沒遮到＝1。
  ★ 快照新增 machine_params\D_GPIB9045_system（只收 *.ini／*.dat）。
  ⚠ 給筆電：哪一支 GPIB 程式會把 Model 寫成 9045GPIB？HT9050 機台要 9050GPIB，被蓋掉 HT9050 分支就失效。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0140 f2d5c3f Revert ZHOME-200 —— EastSun 1002「你幫我把往回跑200流程刪掉好了」：cpp 0138 整顆反做。實測 MInArmZA：+200 往下走，接著原版 case 500 移到 ZSafePos=50 又往上；改成「往上 200 就停」的版本只在 o2 編過、沒 commit，之後 EastSun 決定整段刪掉。Z 軸回原點＝驅動器回原點 → 手臂 Z 去 ZSafePos → HomeFlag=1（原版）。o2 乾淨重編；ctest WebMotorAccess、Pci1203MotorRoute 過。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  tools 0147 HTDESIGNER-161a（0.161.0，版本號不變，機台補丁）—— 按 F5 編譯時 VS Code 右下角有進度條（EastSun 1002：「編譯進度可以有個進度條嗎?」）：讀 CMake 自己在建置資料夾的 CMakeFiles\Progress（跟終端機 [ 45%] 同一個數），F5 的工作一行都沒改（早上包一層的做法讓 F5 不動、已退回）。編完說「建置完成」，沒編完跳訊息。程式庫 181/181、假 VS Code 223/223、探針與分頁樹 7/7、面板全過。
  ⚠ 給筆電：請收機台的 tools 0136、0143、0145、0147 外掛補丁。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0141 43580b0 PKG-129 —— 機台整合筆電第 129 包（GitHub main aab3911、GitLab 13a57384）：Teach「Arm Cell」分頁、Motion View 9050、DTME08（未啟用）、St02 ADAM-6024（連線開關關）、LI-9 FTP、Jerry J-12／J-14、St01 q59、MES16441。設計外掛 8 檔沒碰（設計外掛工作階段 tools 0148）。
                    11 個兩邊都改的檔：7 個乾淨三方合併；4 個兩邊都在檔尾各接一段（MachineType.h、WebMotorAccess.cpp/.h、WebMotorAccessLive.cpp），兩段都留、機台在前。機台的 SensorType 全 0＋W906_HT9050_ORG_INVERT、全機 HOME 開關、HonPrec 都保留；Mot_Table 沒動。
                    o2 全部編過；挑 35 支測試 30 過。
  web 0086 80cccd7 PKG-129 (web) —— 同一包的 17 個網頁檔（HW.teach.html 三方合併乾淨）。
  ⚠ 給筆電：(1) 4 支 *_HT9050 測試讀 repo 的 machines/HT9050，機台這份還是 9/24 的；改餵 D:\HT9045\system 正本 3 支過，MachineMotors 93/94（它要某軸 SensorType=1，EastSun 1002 已全改 0）——請把 test_machine_motors 的 SensorType 期望改成 0。(2) 新的 Adam6024_Pressure 在機台失敗：KpaTransferKG 4q 78.36 vs 76.85、MultiTransferKG 5e／5f 3533.27 vs 34.98、9t；ADAM 連線開關關著所以不影響執行，請查是不是機台多了哪個開關。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  tools 0148 HTDESIGNER-162（0.162.0）—— 筆電包 129 的外掛部分合進機台：ES02 的 0.162（刪除事件先跳提醒視窗，列出會刪的網頁那一行、分派表那一列、C++ 處理函式，一起刪；別處還在用的函式自動保留並寫原因）。三方合併（基準＝包 128）：4 個只有 ES02 改、4 個兩邊都改都沒有衝突；機台的 tools 0136／0143／0145／0147 補丁都保留。C++ 部分＝整合工作階段的 cpp 0141。機台測試：程式庫 181/181、探針 7/7＋分頁樹 7/7、假 VS Code 224/224、面板全過；真的 VS Code 156/159（已知 3 項）。
  ⚠ 給筆電：請收機台的 tools 0136、0143、0145、0147 外掛補丁。機台現在外掛 0.162.0。
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆；沒有機台設定檔。
  cpp 0142 3e51a46 WORKLOG —— 工作紀錄第 86～87 項（Z 軸往回 200 刪掉、第 129 包；設計外掛部分＝tools 0148，0.162.0）。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0143 741a0bd PKG-130 —— 機台整合筆電第 130 包（GitHub main 75c49fd、GitLab 577c41c5）：INDEXZ2（WB_ENGINE_INDEXZ_1203 打開：Index Z1 在 START／HOME 時經 1203 移動，第一次 EastSun 在旁）、St01 review6 E-021～E-029（Contact 頁／自動 Offset 的 START 照原版啟動機台）、Ifor TP-1b、筆電收了機台 cpp 0131～0140。
                    筆電已經帶機台的修改，所以 8 個 LOCAL 檔先列「機台比筆電多的行」：WebMotorAccess 三檔＋閘門腳本沒有 → 用筆電版；CMakeLists／tests／wb_serve 有機台的 TeachKb 與權杖關閉 → 三方合併；MachineType.h＝筆電版＋機台的 W906_WEB_TOKEN_ENFORCE 0 段落。Mot_Table 沒動。o2 全編過；挑 51 支 47 過，4 支 *_HT9050 改餵機台正本全過。
  web 0087 f4dae6e PKG-130 (web) —— 同一包的 23 個網頁檔。
  ⚠ 給筆電：(1) 機台的 W906_WEB_TOKEN_ENFORCE 0（TOKEN-OFF，EastSun 0928）main 沒有，請收；(2) 機台 repo 的 machines/HT9050 不在包裡，*_HT9050 測試在機台仍讀 9/24 的表——要不要把 machines/HT9050 也放進包？
  掃描：見下方結果。
  （推送掃描命中 10 處，逐一看過都是註解／鍵盤 PASSWORD 旗標／測試假值 pw-test-1 的誤報。另外 cTemperFrom_E023.cpp 的 E023T_PasswordRefused 含原版寫死的 Handler System 密碼——掃描沒抓到；它早已在 main 的 HSys.cpp／cObserver.cpp／fLotInfo.cpp／Config.json 等公開，這次沒有多公開。）
  web 0088 8ea4d5b WIREBAR-CORNER —— EastSun 1003「你這視窗都擋到我看的位子了 你可以把這視窗移到不會擋到的地方嗎」：開頁的「✓ 讀取完成（C 路…）」狀態條（#ht9045WireBar，44 頁共用）從上方整條改成右下角小框（theme.css，!important 蓋過引擎與 7 支補件的 inline 樣式；避開 Teach 右下小標籤），乾淨讀取 6 秒自動收（❌／⚠／存檔結果照舊留到按 ×）。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  web 0089 289fc48 VACBTN-TOGGLE —— EastSun 1003「往上的箭頭觸發是真空ON 關掉是OFF，往下箭頭觸發是破真空ON 沒觸發是OFF」＋選「按一下 ON、再按一下 OFF」，通道確認正確（往上＝吸）。oplog：每次按下 0.44～0.53 秒後又來一次點擊（連點／觸控觸發兩次），把輸出馬上切回 OFF；程式本身每次點擊只送一次、本來就是切換。改：同一顆箭頭 0.7 秒內的第二次點擊不算（連點的第二下也不算）；ON 時按鈕畫成凹下去（原本只變淺藍，看起來像沒反應）。只改網頁，C++ 不動。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  10-03 09:45 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20260929.txt：1 個檔變動
     machine_log/oplog_20260930.txt：1 個檔變動
     machine_log/oplog_20261001.txt：1 個檔變動
     machine_log/oplog_20261002.txt：1 個檔變動
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20260929.txt：新增 9982 行
     操作紀錄 oplog_20260930.txt：新增 4681 行
     操作紀錄 oplog_20261001.txt：新增 7559 行
     操作紀錄 oplog_20261002.txt：新增 9171 行
     操作紀錄 oplog_20261003.txt：新增 1328 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  cpp 0144 4e1569e TEACH-ROTATE —— EastSun 1003「這個畫面幫我加入rotate 的馬達按鈕 讓我可以操控 你應該可以看到MOT_TABLE 哪顆是enable」：Teach 的 Axle Control 本來就有「Rotate」框（In X＝MotorInRotateKit＝M41 MInRotate、Out X＝MotorOutRotateKit＝M42 MOutRotate，網頁用編號對應），原版依 USE_ROTATE_KIT（這台 0）藏起來；HT9050（9050GPIB）改依 Mot_Table Enable 顯示，不改 USE_ROTATE_KIT（打開會讓生產流程啟用旋轉模組）。o2 編過；ctest TeachFormShowBridge／TeachButtonsGen 過。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  10-03 10:18 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 930 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_095732
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  cpp 0145 f64409b PKG-131 —— 機台整合筆電第 131 包（GitHub main 19b954b、GitLab fd1b71c5）：Motor Test Gear Ratio 分頁（存檔會改 Mot_Table GearRatio／teach.ini）、Arm Cell 補充、St02 C9 G2／C14／C10、前面板 PAUSE 關提示框；⚠ 開機 Bin 顯示器開 COM14（NUMBER_PANEL_TYPE=3），沒接面板每 60 秒跳提示——機台設定沒動，等 EastSun 決定要不要設 0。
                    wb_serve.cpp 衝突（第 130 包合併時 JAM-STOP 說明位置不同）：機台與基準只差 3 行網頁權杖 → 用筆電版補回那 3 行。其餘 5 檔三方合併乾淨。o2 全編過；34 支 30 過，4 支 *_HT9050 改餵正本 3 過、MachineMotors 差 MTestZ1 速度（EastSun 10:36 從 Motor Test 回寫 Mot_Table）。
  web 0090 f8d179c PKG-131 (web) —— 同一包的 7 個網頁檔。
  掃描：見下方。
  10-03 10:48 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 367 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_104247
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 11:18 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 831 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  cpp 0146 9868761 VACDIAG —— EastSun 1003「圖片上幫我檢查一下 我G的數值 都不會變動 其他是會的 檢查一下軟體讀取位置有沒有錯誤」「不然幫我可讀取的位置數值 都列出來顯示在上面」。查程式：InArm A／C／E／G＝VC0～3，壓力讀這個 ECAT-VC4（ring 1 站 160）輸入資料的位元組 0/1、2/3、4/5、6/7（MyLaneIo.cpp:808-818），G 的 6/7 在 VC4 有效範圍內、算法與其他三格相同，IO_Table 四格設定一致——位址沒有錯。G 畫面是 OK（不是錯誤卡住），所以比較可能是模組 VC3 的壓力不在 6/7 或那一路硬體。vacuum.get 每個站多回：原始位元組 0～15、每兩個位元組換算的 kPa（VC n）、OK 位元 64～71／128～135。o2 編過；ctest I115B_Vacuum、VacuumVc8 過。
  web 0091 bd4cb03 VACDIAG (web) —— Vacuum Unit 頁上方的診斷列（點一下收起），標出每個 VC 對應哪一格。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  cpp 0147 ec0e5f7 WORKLOG —— ★ 機台設定檔：D:\HT9045\system\IO_Table.csv 的 OutArm（站 161）吸嘴 Bit 改成跟 VC4 Port 一致（EastSun 1003「為什麼inarm 和outarm 吸嘴位置不一樣」→「照建議改」）。Bit＝原版 VaccumCopyFormSuck 當 VC 編號用的欄位；10-01 改 VC4 Port 時 OutArm 的 Bit 沒跟著改（仍是 VC8 反排 A=7…H=0），所以啟用的 A/C/E/G 顯示 Error5、停用的 B/D/F/H 反而顯示別格的壓力。改成 A/C/E/G＝3/2/1/0、B/D/F/H＝7/6/5/4，每格感測／吸／破三列共 24 列，其他欄不動；備份 D:\HT9045\_BACKUP_20261003_iotable_outarm_bit。新表在 machine_params\D_HT9045_system\IO_Table.csv。
  ⚠ 給筆電：machines/HT9050/IO_Table.csv 的 OutArm Bit 也要照這個改（MachineSuckers_HT9050 測試的期望值會跟著變）。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  web 0092 e4261d4 VACDIAG-OFF —— EastSun 1003「檔到畫面了啦 快點改回來 嚴禁檔到畫面」：Vacuum Unit 頁上方的診斷列蓋住面板，改成頁面上不顯示（只有用 ?vudiag=1 另開分頁才顯示）。C++ 回傳的原始資料不變。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  10-03 11:49 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 6 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 12:19 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 602 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  cpp 0148 00eeacc JOG-MAXVEL —— EastSun 1003「M35~M40 幫我檢查一下 這幾顆是步進馬達 跟其他不同 我設定JOG 速度都沒有效過」。卡片的 JOG 速度不能超過 CFG_AxMaxVel，而這個上限只有 InitMotor 會寫（原版 CFG_AxMaxVel＝PJogHighSpeed），這台開機只對 MLoaderZ 跑 InitMotor；之後改 JogHigh，上限還是舊的——13:15 M35 改速度卡片回 0x80000087。改：送 JOG 速度前，監看器讀到的上限比這次要的速度／加減速低，就先把上限設回原版 InitMotor 的值（不低於這次的值）；讀不到或夠高就不送（同以前）。程式對步進／伺服沒有分支，問題是上限沒更新。
                    另外：M35～M40 的 Acc／Dec 只有 40000，JOG 加速度不跟百分比縮放，要加到 40 萬要 10 秒，按幾秒看起來會像速度沒變——這是 Mot_Table 資料，沒改。o2 編過；ctest WebMotorAccess 過（含新檢查）。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  10-03 13:28 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 38 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  cpp 0149 71a45c0／web 0093 d444bf0 MT-COPYALL —— EastSun 1003「我複製M35的參數但是為啥沒作用」：原版 Copy From（BitBtn1Click :1392）只接受 M00～M24 當來源，選 M35 網頁照原版不做、只在狀態列提示，命令沒送到 C++（13:25 之後 oplog 沒有 copyFrom）。HT9050 有 M35～M42，改成依完整編號（M35＝35），只要那顆馬達存在就照抄。複製的項目維持原版 6 項（JogHigh／JogLow／HomeHigh／HomeLow／SoftLimitP／SoftLimitN；InitialSpeed／Acc／Dec／Range 原版就不抄）。o2 編過；ctest WebMotorAccess 過。
  掃描（patch 與 README）：權杖／私鑰／7z 密碼／部署金鑰 0 筆。
  10-03 13:49 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  cpp 0150 77bb222／web 0094 ba0b8e2 PKG-132＋MT-ACCLIVE —— 機台整合筆電第 132 包（GitHub main a071554、GitLab db9f0430）：加熱器監控每 20 ms（HEATER_CTRL_TYPE=4，加熱測試前要設 3——機台設定沒動）、Teach Out Z All Down 會動 Out Z、提示框會停 Motor Test HOME／Loop、Gear Ratio R171、St02 c912→906／Bin 狀態面板／ADAM A3、Bin 顯示 NUMBER_PANEL_TYPE=4＋COM14 檢查。
                    10 個 LOCAL：9 個三方合併乾淨（機台的 JOG-MAXVEL／MT-COPYALL／TEACH-ROTATE／開關全保留），HW.teach.html 一處衝突＝筆電的 ZALLUP 行＋機台的 TEACH-KB 行。筆電刪的 SafePlcIOInstall.cpp／SecsAlarmForm.cpp/.h 已無引用，刪掉。test_gear_teach_save 補連 FileRW/TeachKb.cpp（機台 Teach.cpp 用到）。
                    MT-ACCLIVE：EastSun 1003「我把加速度上調也沒用」——原版表格改 Acc／Dec 只改資料庫值，執行中的 dAcc 要到下次 SetADCRate(100)（HOME）才跟上；13:54 存了 400000，之後 JOG／設速度仍送 40000。改成表格改 Acc／Dec 立即 SetAcc／SetDec；速度上限（CFG_AxMax*）提高也擴到 PTP（Move＋／Move－／Go）。
                    o2 全編過；30 支 29 過，GearTeachSave 讀筆電 repo 的 machines/HT9050/snapshot（這台沒有）。
  ⚠ 給筆電：GearTeachSave 與 4 支 *_HT9050 測試讀 machines/HT9050（含 snapshot\），這些不在包裡，機台跑不到——要不要把 machines/HT9050 也放進包？
  掃描：見下方。
  10-03 15:13 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：8 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 965 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_143853、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_143931、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_144027
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:12 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：6 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/IOWEB_TEST_R003：5 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 1521 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/IO_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/runcfg/config/config.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:13 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 72 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_161249
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:15 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 42 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:25 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 127 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:35 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 137 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:41 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 160 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:45 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 298 行
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 16:50 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 275 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 17:03 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 226 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 17:04 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 17:18 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 569 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 17:19 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0149 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- One stop stops everything (EastSun 1003): the status bar's and the Solution Explorer toolbar's stop button
  both cancel the designer's build, stop every debug session, and end this tree's wb_serve / wb_publish /
  wb_gateway still running (other folders untouched). VS Code's own stop ending one of this tree's
  sessions stops the tree's other sessions too (F5's wb_publish + wb_gateway pair).
- Ctrl+F (EastSun 1003: VS Code's own find box made it look like the search ran there): Ctrl+F on the designer
  page opens the designer's find box; opening it closes VS Code's leftover find widget.
- Auto-wiring: form.event's own after-ack call (W906_FormEventRunAfterAck, new in wb_serve.cpp 1003) is not
  copied into the htd.event branch; another unknown FormEvent call = refused, not guessed.
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 (see their sections).
  10-03 17:38 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 268 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 17:42 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0150 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- Find is its own window (EastSun 1003: not VS Code's built-in box, and the earlier searches visible):
  Ctrl+F / Ctrl+Shift+F open a floating window "Find" (keyword, scope, Aa / whole word / regex), the searches
  made before on the left (newest first; a click searches again, x deletes one, kept across restarts), the
  results on the right (hit marked; a click opens the line in the editor group the user came from).
  Settings: ht9045Designer.find.newWindow off = a tab beside; ht9045Designer.find.window off = the old box.
- Tests: run_all 3/3 layers (the window also drawn in headless Edge); real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 / 0150.
  10-03 17:49 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
10-03 派工給筆電（EastSun 交辦；Jimmy 說 EastSun 需要協助調查與修正）——機台端沒有動程式，請筆電端調查、修正後出包：
  ⚠ 給筆電（派工 1）：查證 Index Auto Height（Index 自動測高）在 1203 上能不能正常動作。原版這段走 Galil（Index Z＝MTestZ1，機台現在經 Pci1203GaliRoute 轉到 1203），
     裡面會讀「力矩」相關參數；1203（DS402 驅動器）的力矩讀值來源、單位（DS402 是額定力矩的 0.1%，Galil 是電壓）、正負號、上限寫入可能都跟原版不同。
     請確認：讀值換算是否正確、判斷門檻會不會永遠不觸發（⚠ 安全：門檻不觸發會讓 Z 一直往下壓到 socket）、各 Galil 命令在 1203 路由裡是支援／假值／拒絕；
     需要機台實測才能定的值，請列出要量的項目（量的時候 EastSun 在旁邊）。
  ⚠ 給筆電（派工 2）：所有頁面在這台機台電腦啟動後，哪些視窗會被遮住或被截斷——請逐一列出（每個視窗：位置、大小、內容是否超出、有沒有被別的視窗或浮動提示蓋住）並全部修正。
     環境：HMI 外殼（build_hmi_shell\ht9045_hmi.exe，WebView2）開 background.html，主畫面固定 925x720，畫面縮放約 110%（main.html 的 🔍）。
     EastSun 的規則：任何提示／浮動框都不能擋到畫面內容。機台端 10-03 已修：Temp Set 頁（灰框蓋數值、欄位標題空白、狀態列蓋表格，web 0098）、
     主畫面軟體面板鍵被公司 Logo 的透明框蓋住點不到（web 0101）。
  10-03 18:19 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
10-03 派工給筆電（EastSun 交辦給 Jimmy）——機台端沒有動程式，請筆電端修正後出包：
  ⚠ 給筆電（派工 3）：HT9050 自動流程的吸嘴對 IC 方式要改。機台現在跑的是「單顆 IC」：HT9050 選取單顆 IC 動作時，是 VC4 的 4 個吸嘴一起吸一顆 IC；
     舊版（原版 9 系列）都是一個吸嘴吸一顆。所以自動流程裡跟「一個吸嘴＝一顆 IC」有關的部分都要跟著改：哪幾個吸嘴要一起開／破真空、真空到位的判斷
     （4 個一起算一顆）、IC 有無／掉料判斷、吸嘴對應的 Tray／Socket 格位與 pitch、計數。
     參考：機台 IO_Table 吸嘴列在 VC4／VC8 真空模組（InArm 站 160、OutArm 站 161、Index 站 162；OutArm 的 Bit 已在 10-03 改成跟 VC4 Port 一致，
     見 WORKLOG_MACHINE §1），真空開／破＝VC 的 DO、到位＝DI 64+VC（VacuumUnit/Vc8Route、TestIF_File_VacuumUnit）。
     請列出要改的流程點（In Arm／Out Arm／Index 各自的吸、放、檢查），修正後出包；需要機台確認的地方（哪 4 個吸嘴是一組）請列出來問 EastSun。
10-03 派工給筆電（EastSun 交辦給 Jimmy）——機台端改了多次沒修好，紀錄檔一起放在 dispatch\20261003_F5_close_rebuild\：
  ⚠ 給筆電（派工 4）：F5 之後關掉 HMI 視窗，程式沒有跟著結束——「關掉視窗後它才又開始編譯／啟動」，之後視窗又自己跳出來。EastSun：「你已經修正多次沒有修好」。
     現象（hmi_shell.log 最後幾行）：18:20:48 F5 開出等待畫面 boot_wait.html（此時還在編譯）；18:21:32 EastSun 按關閉 → 「close: page answered "none"」→ 只有視窗結束，
     F5 的 preLaunchTask 編譯照跑、wb_serve 編完照樣啟動，wb_serve 的 keeper（W906_HmiKeeperTick）發現沒有頁面又把 HMI 視窗開回來。
     關鍵：boot_wait.html 沒有定義 HT9045ShellCloseRequest；hmi_shell 收到 "none" 時只關自己，沒有通知 F5 的編譯／啟動停下，也沒有通知 wb_serve。
     機台端已試過的修正（都沒解決這個情況）：EXIT-ORDER a4f4c05（關站順序＋15 秒強制結束＋oplog EXIT 紀錄，只管已經在跑的 wb_serve）、
     F5WAIT 3eab632、F5-PROGRESS／HMI-SECOND a87f2a5、F5-EXTCON 8b266d6、HMI-SHELL 1954b25、CLOSE-BUSY 5b766a4 …（完整清單 fix_history_exit_hmi_f5.txt）。
     機台端草擬過的修法（沒套）：boot_wait 定義 HT9045ShellCloseRequest 回 'boot'；hmi_shell 在 'boot'／沒有頁面時寫 operator_close 旗標檔並對 wb_serve 送
     Local\HT9045_wb_serve_quit_<pid>；wb_serve 的 W906_ExternalQuitDue 認旗標檔走正常關站，keeper 看到旗標就不再開視窗；F5 開頭清旗標。
     ⚠ 另外要決定：關視窗時 F5 的「編譯」要不要也一起取消（VS Code 的 preLaunchTask 不在 hmi_shell 管轄內）。
     附檔：hmi_shell.log（HMI 視窗紀錄）、bootsample_20261003_*.txt（每次開機的取樣）、launch.json／tasks.json（機台 F5 設定）、fix_history_exit_hmi_f5.txt；
     操作紀錄 machine_log\oplog_20261003.txt（每輪開機、EXIT、關站都有時間戳）。
=== tools 0151 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- The keyword search reads 32 files at once (EastSun 1003: "search feels slow"): 10-11 s -> about 3 s here
  (4,364 files / 217 MB; the time was the reads, not the matching). Same hits, same order as before.
  No in-memory cache (this PC: 7.7 GB, 1.5 GB free).
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 / 0150 / 0151.
  10-03 18:26 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 18:34 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 313 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/Pci1203Modules.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 19:05 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 244 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
10-03 派工給筆電（EastSun 交辦給 Jimmy）——機台端沒有改建置設定，請筆電端修正後出包：
  ⚠ 給筆電（派工 5）：機台上編譯太慢，F5（-O0）跟 release（-O2）都慢，EastSun 要求加快。
     機台端實測（檔案時間戳）：F5 只建 wb_serve，改一個 .cpp 要 ~61 秒，其中編譯只有 3 秒，其餘是重新打包 libht9045_sm.a（25 秒）、
     重做 wb_serve 的 objects.a（22 秒）、連結（11 秒）；改到 MachineType.h／cmydef.h 要重編 400～600 個檔（-O0 245 秒、-O2 352 秒，
     wb_serve 有 78% 的 obj 相依 MachineType.h）；連測試一起建（ALL，-O2）要 ~21 分鐘。
     機台：i7-14700 28 執行緒、RAM 只有 7.7 GB（所以 F5 用 -j 6）、原始碼和所有 build 目錄都在 D:（5400 轉傳統硬碟），C: 是 NVMe。
     機台端猜的加速方向（請筆電端評估、實作後出包）：build 目錄放 C:／防毒排除、改 Ninja 或不要每次重做 objects.a、ht9045_sm 改 OBJECT library 或 thin archive、
     PCH／ccache、測試共用 test_bootstrap 並移出 ALL；拆 MachineType.h 開關要先問 EastSun（開關放 MachineType.h 是他的裁決）。
     限制：oracle 線 g++ 6.3.0 不能換、build\ 的 CMAKE_BUILD_TYPE 不能動；請先在新的 build 目錄比時間。
     附檔 dispatch\20261003_build_speed\：build_timing_20261003.txt（時間表＋機台規格）、o2_ALL_build_20261003_1858-1904.log（今天整包 -O2 編譯輸出）、
     兩條線的 CMakeCache、ht9045_sm／wb_serve 的 link.txt、機台的 launch.json／tasks.json。
  10-03 19:20 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 137 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0152 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- F5's build progress bar always shows (EastSun 1003): the extension now starts on F5 ("onDebug"; before it only
  started with a designer page / view, so F5 right after opening VS Code had no bar). The status bar also shows
  "build ######## 75% (30/40)". Not onStartupFinished (it slowed other parts in the real-VS-Code test).
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 / 0150 / 0151 / 0152.
  10-03 19:31 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 208 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
10-03 派工給筆電（EastSun 交辦給 Jimmy）——機台端只查證、沒有改這部分程式：
  ⚠ 給筆電（派工 6）：EastSun：「為啥軟體開起來，好像先激磁又放激磁」。請查明並修正（要照原版的地方請標明）。
     機台端查到的（今天 29 次開機逐次看 oplog）：開機時軟體沒有送 Servo ON 也沒有送 Servo OFF（1203 監看器 svo 一次 1->0 都沒有；開機看到的 svo=1 是上一輪留下的）；
     原版開機 InitMotor 會對每一軸 Servo ON，移植樹這一步沒有碰到卡（路由還沒裝好）。
     最可能被看成「激磁又放開」的是：(A) 按 HOME 後約 2.2 秒照原版切馬達電源繼電器約 4 秒再接回（uhome case 2/3，golden uhome.cpp:2207/:2330），
     (B) 開機時煞車先放開、DoMotorPowerOn 又把 Index／Magazine／Cassette 煞車鎖住、約 4 秒後再放開（csystem.cpp:16140、:14415-14418）。
     不確定的：繼電器 DO 開關卡時實體會不會掉（oplog 的 relay 是軟體值，不是卡片讀回）、svo 位元代表命令還是驅動器真的激磁。
     附檔 dispatch\20261003_servo_on_off_at_start\：findings_machine_side.txt（逐點 file:line）、oplog_20261003.txt（今天 29 次開機）、oplog_20261002.txt、
     三次 HOME 的 bootsample。另：機台端 cpp 0168-0171 是今天回原點的修正（激磁檢查等電源、未認領的 1203 軸補認領、HOME／JAM／ROUTE 紀錄）。
  10-03 19:34 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0153 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- A jump to code (from an event or elsewhere) marks where it went in its own background colour: the whole
  function (header to closing brace) or else that line; a scroll bar mark too; the next jump moves it.
  Colour: ht9045Designer.jumpTargetBackground (Settings > colours).
- Back / forward to code places: two big status bar buttons with words ("back to previous place",
  "next place") = Alt+Left / Alt+Right; arrows on the editor title bar when they can go. (EastSun 1003)
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0153.
  10-03 19:43 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 345 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 19:50 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 291 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 20:01 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 271 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0154 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- Find window filters (EastSun 1003): three checkboxes -- assigned (= += ++ --), used as a condition
  (if / while / for / switch ( ), == != < > && || ! ?), a function (name followed by "("). Any checked = those
  uses only, comments and strings excluded; each result tagged. Also: the options row no longer wraps, file
  names no longer stack while scrolling.
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0154.
  10-03 20:08 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 147 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 20:15 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 276 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 20:23 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 546 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0155 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- One toolbar on the editor title bar (EastSun 1003): back, forward, build+start, continue, pause, stop all,
  restart, step over / into / out, simulation on/off, Debug on/off -- greyed when they cannot act. The status
  bar (blue) shows information only now.
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0155.
  10-03 20:32 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0156 (20261003, machine) -- HTML designer (tools/vscode-htdesigner): notes only ===
- Tabs and buttons on two rows (EastSun 1003): this machine's VS Code user setting
  "workbench.editor.editorActionsLocation": "titleBar" -- the editor buttons (the designer toolbar) move to the
  window title bar. Not in the repo; set it on another PC for the same layout.
  10-03 20:40 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 678 行
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0157 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- Claude's tabs on a row of their own (EastSun 1003): a Claude tab is pinned when it opens; with the user setting
  workbench.editor.pinnedTabsOnSeparateRow = true the rows are buttons / Claude tabs / file tabs.
  Setting ht9045Designer.pinClaudeTabs turns it off.
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0157.
  10-03 20:54 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 328 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 21:02 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 21:08 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 21:15 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 341 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0158 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- Stop goes grey when the program is closed (EastSun 1003): only this tree's wb_* count (another folder's did
  before), the state is re-checked every 3 s, the stop tooltip says why it is lit.
- Machine setting (notes): VS Code's own floating debug toolbar hidden (debug.toolBarLocation = hidden).
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0158.
  10-03 21:21 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：7 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_212048、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_212059
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 21:29 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：8 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 848 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_212310、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261003_212650
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
10-03 機台端回覆筆電（St01 審查 cpp 0160～0170、NB2-1 派工 6）——EastSun 裁決「照BCB裡面方法修改」：
  ✅ M-a：照 golden 改（cpp 0187 HOME-POWERDROP）——回原點中電源穩定過之後才掉（急停／開門／安全迴路），停止回原點（StopAllMotor＋SystemStart=false＋訊息框），不再自動重新激磁；HOME 自己 case 2/3 的斷電再上電照舊。
  ✅ M-b：照 golden 改（cpp 0187 HOME-1RETRY）——golden 只 reset 一次：302 檢查最多再清警報＋激磁 1 次，仍異常就停止回原點（原本 20 次）。
  ⏸ M-c：還沒改。golden 回完要看原點燈，但 HT9050 的伺服用 DS402 方式 24／28 回原點會停在原點開關旁邊，照 golden 檢查 10-03 下午 8 支伺服全部被判失敗、全機回原點卡住。之後機台端已加：回完位置必須是 0（cpp 0176／0179），不是 0＝中斷＝失敗。要不要再加 statusword homing attained，等 EastSun。
  ✅ 派工 6 (B)：照建議 A＝照 golden（cpp 0187 BRAKE-BOOT）——放煞車加回 bMotorPowerState && MotorPowerOnDelay==0（golden ckernel.cpp:2910／:2976）。
  ✅ 派工 6 (1)：照 golden（cpp 0187 BOOT-SERVOALL）——開機 InitMotor 改成所有 1203 軸（原本只有 MLoaderZ）；MTestZ1 照 golden 走 Galil 路徑。
  ❓ EMG 題（放開急停後馬達電源會不會自己回來）、派工 6 時間點（開機前幾秒或按 HOME 之後）、派工 1／6 的量測：還沒回，等 EastSun 在機台旁。
  另外今晚機台端 cpp 0180～0186：HOME-BRAKE（302 後放煞車組）、HOME-TIMEOUT 後撤回（EastSun 要求）、雙軸一次一軸關掉（EastSun 要試雙軸同時）、HOME-STEPLEAVE（SW3D 步進壓在原點上先往 HomeDirection 移 1000，最多 3 次，失敗報警，單軸與全機共用一支函式）、ALARM-WHY（錯誤框顯示原因）、HOME-MAXVEL-SINGLE（單軸回原點先拉高 CFG_AxMaxVel：M30 0x80000081）。
10-03 機台端回覆筆電（St01 審查 cpp 0160～0170、NB2-1 派工 6）——EastSun 裁決「照BCB裡面方法修改」：
  ✅ M-a：照 golden 改（cpp 0187 HOME-POWERDROP）——回原點中電源穩定過之後才掉（急停／開門／安全迴路），停止回原點（StopAllMotor＋SystemStart=false＋訊息框），不再自動重新激磁；HOME 自己 case 2/3 的斷電再上電照舊。
  ✅ M-b：照 golden 改（cpp 0187 HOME-1RETRY）——golden 只 reset 一次：302 檢查最多再清警報＋激磁 1 次，仍異常就停止回原點（原本 20 次）。
  ⏸ M-c：還沒改。golden 回完要看原點燈，但 HT9050 的伺服用 DS402 方式 24／28 回原點會停在原點開關旁邊，照 golden 檢查 10-03 下午 8 支伺服全部被判失敗、全機回原點卡住。之後機台端已加：回完位置必須是 0（cpp 0176／0179），不是 0＝中斷＝失敗。要不要再加 statusword homing attained，等 EastSun。
  ✅ 派工 6 (B)：照建議 A＝照 golden（cpp 0187 BRAKE-BOOT）——放煞車加回 bMotorPowerState && MotorPowerOnDelay==0（golden ckernel.cpp:2910／:2976）。
  ✅ 派工 6 (1)：照 golden（cpp 0187 BOOT-SERVOALL）——開機 InitMotor 改成所有 1203 軸（原本只有 MLoaderZ）；MTestZ1 照 golden 走 Galil 路徑。
  ❓ EMG 題（放開急停後馬達電源會不會自己回來）、派工 6 時間點（開機前幾秒或按 HOME 之後）、派工 1／6 的量測：還沒回，等 EastSun 在機台旁。
  另外今晚機台端 cpp 0180～0186：HOME-BRAKE（302 後放煞車組）、HOME-TIMEOUT 後撤回（EastSun 要求）、雙軸一次一軸關掉（EastSun 要試雙軸同時）、HOME-STEPLEAVE（SW3D 步進壓在原點上先往 HomeDirection 移 1000，最多 3 次，失敗報警，單軸與全機共用一支函式）、ALARM-WHY（錯誤框顯示原因）、HOME-MAXVEL-SINGLE（單軸回原點先拉高 CFG_AxMaxVel：M30 0x80000081）。
=== tools 0159 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- F5's build in a window of its own, big (EastSun 1003): an 80px %, a thick bar, steps / total, the time;
  done = green and it closes itself, not finished = red and it stays. The status bar build item is gone.
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0159.
  10-03 21:52 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：2 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/runcfg/system/teach.ini、machine_params/runcfg/system/teach.ini.bak_20261003_homepos0
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
10-03 給筆電（Jimmy 想知道，EastSun 交辦）：驅動器讀回來的扭力值＝**馬達額定扭力的 %，解析度 0.1 %（小數一位）**。讀 6077h（雙軸 B 軸 6877h），INT16；% = raw × 2704h:1 ÷ 2704h:2，9/15 實測每軸 2704h＝1/10 → raw÷10；Motor Test 顯示小數一位剛好等於原始解析度。細節（含 file:line、N·m 換算、範圍、注意事項）與手冊（安川 SGDXW／SGDXS EtherCAT 手冊、SGDXW 系列、SW3D 步進 EVER DS402）在 dispatch\20261003_torque_units_manuals\。
  10-03 22:02 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 827 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 22:05 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 22:14 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 505 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 22:28 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 405 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0160 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- Stop works during F5's build (ends that build's process tree, deletes half-written exes); the find window
  closes when the focus leaves it (Ctrl+F brings it back with the last results); F5 / build+start first end the
  same program still running (no need to press stop); save / save all on the toolbar; the extension starts with
  VS Code. (EastSun 1003)
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0160.
  10-03 22:43 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
10-03 給筆電（EastSun 交辦給 Jimmy，請建成 SKILL）：dispatch\20261003_homing_1203_skill\HOMING_1203_FOR_SKILL.md（開頭已附建議的 SKILL frontmatter name: ht9050-1203-homing）。重點：以前 SMC 是卡片回原點；PCI-1203 EtherCAT 不是 1203 回原點，是我們下命令給 1203、1203 再下命令給驅動器，由驅動器回原點（DS402 method 24/28），所以不同驅動器回原點可能不一樣（HT9050 實測：Yaskawa 伺服壓在原點上會先退開，SW3D-680 步進會一直往前找）。內容含：卡片式 vs 驅動器式、樹怎麼選路、今天找到的驅動器差異與修法、golden 對不上的地方、新驅動器上線檢查清單、待決定與要在教導後移除的 HT9050 暫時分支。
  10-03 22:58 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 577 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 23:04 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 401 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0161 (20261003, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- Find window: replace (one / all listed; documents only, not saved; golden never). A log file per day, cleaned
  (14 days / 50 MB). Solution Explorer's "building xx%" shows F5's progress in colour. The build window is off.
  (EastSun 1003)
- Tests: run_all 3/3 layers; real VS Code 153/159 (3 known stale + the BtnPanelLane3 selection set, see HANDOVER).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0161.
  10-03 23:18 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 394 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 23:23 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 381 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 23:28 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 375 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 23:33 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 554 行
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-03 23:37 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-04 20:56 機台端今天的修改（程式＋快照）：
     cpp 0202＝13eca55  PKG-134：筆電第 134 包（Gear Ratio 手推量測、Tester Pause 延遲響鈴 912 等）
     cpp 0203＝ecfa1a4  PKG-135..140：筆電第 135～140 包（BRAKE-SERVOFIRST、Servo ON 寫回命令位置、Index 成對指令在 HT9050 報警、TRAYSAFE-3 等）；o2 全建＋ctest 343/361，18 個失敗都是環境／表格內容／既有
     cpp 0204＝0540c57  WORKLOG：135～140 包 ctest 失敗清單
     cpp 0205＝e820eea  HT9050-TEACH-KB-LOADERY：Teach 的 Loader Y 小鍵盤在 HT9050 不夾上下限
     cpp 0206＝3800d02  HT9050-TEACH-KB-ALL：Teach 全部欄位小鍵盤在 HT9050 不夾上下限（開關 W906_HT9050_TEACH_KB_CHECK，預設 0）
     cpp 0207＝fa29db7  WORKLOG 10-04：§2 第 120～123 項、§4 空跑前必處理清單、HT9050 Out Shuttle 規則（EastSun 第 7～10 點）、Frank 待回
     web 0103＝94b238e  PKG-134（web）
     web 0104＝f6f923a  PKG-135..140（web）
     web 0105＝9a0cc07  HT9050-TEACH-KB-ALL（web 半邊）
     web 0106＝1554472  TEACH-1PICKZ：Teach 的 In/Out Arm Pick Up／Place Z 點在單吸嘴機台重新顯示（原版 ZE→ZA 換算套到「沒這軸就藏」）
     web 0107＝1e95562  TEACH-1PICKZ-2：同上，在 HMI 外框裡也顯示（等整頁載完再判斷吸嘴數）
     掃描：18 筆「password = 值」都在 0202／0203／0104 的程式碼裡（原版變數名與 golden 寫死的 FTP 密碼），同一個 repo 的 main 早已公開；照 EastSun 1002「照原樣、不遮」裁決推送，沒有權杖／私鑰
     machine_log/oplog_20261003.txt：1 個檔變動
     machine_log/oplog_20261004.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261003.txt：新增 417 行
     操作紀錄 oplog_20261004.txt：新增 14854 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/runcfg/system/teach.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-04 22:14 整合筆電第 141～144 包（main 到 369f553）＋機台快照：
     程式：cpp 0208（33880da，141～144 包：Index Z 扭力讀 1203、開機表格稽核 T1～T12、St02 N07 警報橫幅、F5 關視窗停止啟動、Teach CHECK_RANGE 重讀修正等；
           WebBridge/WebBridgeServer.cpp 沒套＝筆電把 act.observerSG.state 加進免權杖，等 EastSun 決定）、cpp 0209（WORKLOG）、web 0108（af09ed6，網頁那一半）。
           o2 全建 exit 0、ctest 348/368（失敗清單見 WORKLOG §3 141～144 列）、production_audit 設定檔 0 變更。
     掃描擋下 5 行（DoPassword／bNeedPassWord 的註解、網頁自測 'PASSWORD:' 說明文字），都來自筆電包、公開的 main 本來就有，照 1002「照原樣」裁決放行。
     machine_log/oplog_20261004.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/IOWEB_TEST_R003：3 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261004.txt：新增 943 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 08:10 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261004.txt：1 個檔變動
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/IOWEB_TEST_R003：5 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261004.txt：新增 655 行
     操作紀錄 oplog_20261005.txt：新增 6599 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/runcfg/system/teach.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0162 (20261005, machine) -- HTML designer (tools/vscode-htdesigner), version stays 0.162.0 ===
- F5 starts at once when the program is already built and no source is newer (the wait page still opens);
  otherwise it builds as before. Solution Explorer's start button spins orange during F5's build again. (EastSun 1005)
- Tests: run_all 3/3 layers; real VS Code 156/159 (3 known stale checks).
- ! For the laptop: still not taken -- machine patches tools 0136 / 0143 / 0145 / 0147 / 0149 - 0162.
  10-05 09:48 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 2207 行
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 10:44 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 365 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0163 (20261005, machine) -- HTML designer (tools/vscode-htdesigner) 0.163.0 ===
- Laptop package 134 (ES02 0.163.0) merged: same code as the machine's tools 0150; version 0.163.0 and its notes
  taken; the machine's tools 0151-0162 kept. The layout (buttons on the title bar, Claude tabs on their own row,
  no floating debug toolbar) is now set by the extension itself on any PC (once, only settings not set by the user).
- ! For the laptop (ES02): please take the machine's designer patches tools 0143 and 0151-0163. ES02 0.163.0 has
  0136/0137/0145/0147/0149/0150 only; its run_tests.js / smoke_extension.js dropped 0143's two tests (worktrees skip,
  lint count) -- keep them. Other PCs that install the designer from the laptop package lack: parallel search, the
  find filters / replace, the jump highlight, the title bar toolbar, Claude tab pinning, stop during F5, F5 ending the
  old program / skipping an up-to-date build, the log file, the layout settings.
- Tests: run_all 3/3 layers; real VS Code 153/159 (3 known stale + the BtnPanelLane3 selection set).
  10-05 13:03 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 1177 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 13:35 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 1195 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_131613
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 14:17 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 283 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 14:46 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_HT9045_system：8 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 2273 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_143408、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_143627、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_144039、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_144057、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_144109
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 17:06 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_HT9045_system：16 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 8223 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_145128、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_145337、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_153846、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_153959、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_154020、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_154041、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_154630、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_154710、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261005_155252 …共 16 個
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 18:29 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 1788 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 19:06 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 19:19 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 21:09 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261005.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261005.txt：新增 360 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/runcfg/system/teach.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0164 (20261005, machine) -- HTML designer (tools/vscode-htdesigner) 0.175.0 ===
- Laptop packages 147 / 148 / 150 (ES02 0.165-0.175) merged into the machine: the designer is now the same as
  ES02's 0.175.0 (which already carries all of the machine's patches).
- Machine fix: the BCB6 source is found again after the tree moved to C:\HT9045_ssd (a junction left at
  D:\HT9045\_integ_ioweb): lib/roots.js follows a junction pointing at the tree. ES02: please take it (and the same
  lookup in test/vscode_it.ps1).
- Not run on the machine: test/vscode_run_it.ps1 (it builds and starts wb_serve).
- Tests: run_all 3/3 layers; real VS Code 153/159 (3 known stale + the BtnPanelLane3 selection set).
  10-05 22:24 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-05 23:07 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 08:23 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 08:43 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 916 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 09:00 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 183 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 09:14 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 734 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0165 (20261006, machine) -- HTML designer (tools/vscode-htdesigner) 0.175.0 ===
- The toolbar's start button = F5 (EastSun 1006): it starts the configuration picked in Run and Debug with its own
  build task (before: the extension's own build of the folder of the simulation / Debug boxes -- the SIMULATION
  build by default, while F5 builds the shipping one). Setting ht9045Designer.run.likeF5 = false: the old button.
  ES02: please take it.
- Tests: run_all 3/3 layers; real VS Code 153/159 (3 known stale + the BtnPanelLane3 selection set).
  10-06 11:37 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：7 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 1076 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261006_110321、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261006_110404、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261006_110427
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 11:51 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/Gerneral.ini.bak_idx1203_20261006_113804
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 13:40 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 3792 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 13:52 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 14:35 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 2676 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 15:46 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：6 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 758 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/Gerneral.ini.bak_panel_20261006_152525、machine_params/D_HT9045_system/Gerneral.ini.bak_tft_20261006_154508
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 16:52 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 1158 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 17:17 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：6 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 47 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/D_HT9045_system/Gerneral.ini.bak_plcmodel_20261006_171604
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 17:42 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 309 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 17:59 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 749 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0166 (20261006, machine) -- HTML designer (tools/vscode-htdesigner) 0.197.0 ===
- Laptop packages 158-161 (ES02 0.177-0.197) merged; the designer is ES02's 0.197.0 plus the machine fix below.
- Machine fix: CSV table frozen columns scrolled away when the next animation frame did not come (media/csv.js:
  redraw after 40 ms anyway). ES02: please take it.
- Not run on the machine: test/e2e_all.js, test/vscode_run_it.ps1 (they build / start the machine program).
- Tests: run_all 3/3 layers; real VS Code 159/159 (all pass).
=== tools 0167 (20261006, machine) -- HTML designer (tools/vscode-htdesigner) 0.197.0 ===
- The "N items differ from the DFM / reset all to DFM" strip over the properties grid is gone (EastSun 1006: not
  wanted). The per-row DFM values, the reset square and the menu item stay. ES02: please take it.
- Tests: run_all 3/3 layers; real VS Code 159/159.
  10-06 18:31 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/IOWEB_TEST_R003：3 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 351 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 18:38 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/IO_Table.csv、machine_params/D_HT9045_system/machinerecord.dat、machine_params/D_HT9045_system/IO_Table.csv.bak_snallemg_20261006_183730
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 18:59 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：12 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 842 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/IO_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/D_HT9045_system/IO_Table.csv.bak_door5_20261006_184723、machine_params/D_HT9045_system/IO_Table.csv.bak_door6_20261006_185009、machine_params/D_HT9045_system/IO_Table.csv.bak_door7_20261006_185150、machine_params/D_HT9045_system/IO_Table.csv.bak_door8_20261006_185236、machine_params/D_HT9045_system/IO_Table.csv.bak_doors_20261006_184651 …共 13 個
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
=== tools 0168 (20261006, machine) -- HTML designer (tools/vscode-htdesigner) 0.197.0 ===
- The DFM / HTML / copy buttons at the end of the properties panel's tabs line are gone (EastSun 1006: not wanted);
  the jumps stay on a row's double-click / menu and Shift+F7. ES02: please take it.
- Tests: run_all 3/3 layers; real VS Code 159/159.
=== tools 0169 (20261006, machine) -- HTML designer (tools/vscode-htdesigner) 0.197.0 ===
- The Debug / Release button on the Solution Explorer toolbar stands out (EastSun 1006): a bigger icon, the icon
  and the word bright orange (Release) / red (Debug), the button tinted. ES02: please take it.
- Tests: run_all 3/3 layers; real VS Code 159/159.
  10-06 19:14 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini.bak_plcdoor_20261006_191025、machine_params/D_HT9045_system/IO_Table.csv.bak_plcdoor_20261006_191025
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 20:15 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 1960 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 22:09 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261006.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：6 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261006.txt：新增 1447 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/IO_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 22:10 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 22:27 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 22:58 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-06 23:32 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 03:03 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 03:22 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 07:17 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 07:38 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 07:45 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 08:03 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 08:10 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 09:26 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 1976 行
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/runcfg/system/teach.ini、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261007_091146
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 09:32 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 410 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261007_092651、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261007_092933
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 09:38 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 484 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 10:02 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：6 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 762 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/IO_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/IO_Table.csv.bak_dropcyl_20261007_094218
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 10:11 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 10:13 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 10:20 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 10:40 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 10:57 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 10:59 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 712 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 11:14 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：7 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 161 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/IO_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/IO_Table.csv.bak_safedoor_20261007_111239、machine_params/D_HT9045_system/IO_Table_safedoor_off_20261007_111239.txt
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 11:22 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/IO_Table_safedoor_off_20261007_111239.txt、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 12:08 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：6 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 2750 行
     設定檔變動：machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261007_113716、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261007_115853
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 12:11 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 26 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 12:11 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 9 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 12:22 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 130 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 12:25 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 40 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 12:26 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 10 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 13:12 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 562 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 13:19 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 738 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261007_131833
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 13:28 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 313 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 14:11 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 849 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 14:24 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 819 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 15:11 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 5407 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/runcfg/system/teach.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 16:11 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 4101 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/runcfg/system/teach.ini、machine_params/D_HT9045_system/Mot_Table.csv.bak_20261007_151453
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 16:40 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 2692 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/runcfg/system/teach.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 16:49 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 327 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 16:50 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 12 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 16:58 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 112 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 17:06 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 17:31 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
tools 0170 (20261007): HTML designer merged with laptop packages 162/163/167/168 (0.197.0 -> 0.234.0, no conflicts; machine features kept). Build window bar no longer collapses in a short window; panel test pins the Edge window size. Install: tools/vscode-htdesigner pack.ps1 then code --install-extension.
  10-07 17:57 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 18:09 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 18:15 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 264 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 18:29 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：5 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 1049 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Gerneral.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 18:36 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 463 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 19:06 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 2371 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat、machine_params/runcfg/system/teach.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 19:18 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 355 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 19:25 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 415 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 19:53 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     machine_params/runcfg：1 個檔變動
     workorder/IOWEB_TEST_R003：3 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 2198 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/runcfg/system/teach.ini
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 20:11 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 856 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 20:19 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 141 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 20:44 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 516 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 21:14 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/IOWEB_TEST_R003：3 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 617 行
     設定檔變動：machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 21:30 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/IOWEB_TEST_R003：4 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 332 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 21:48 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：3 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 541 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 21:49 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_HT9045_system：2 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 21 行
     設定檔變動：machine_params/D_HT9045_system/Mot_Table.csv、machine_params/D_HT9045_system/Mot_Table.csv.bak_initspeed_20261007_214924
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 22:07 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/D_GPIB9045_system：1 個檔變動
     machine_params/D_HT9045_system：4 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/IOWEB_TEST_R003：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 868 行
     設定檔變動：machine_params/D_GPIB9045_system/general.ini、machine_params/D_HT9045_system/lastdata.dat、machine_params/D_HT9045_system/lastdata_backup.dat、machine_params/D_HT9045_system/lastdata_backup2.dat、machine_params/D_HT9045_system/machinerecord.dat
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
  10-07 22:15 機台快照（EastSun 在機台上做的事，沒有程式修改）：
     machine_log/oplog_20261007.txt：1 個檔變動
     machine_params/README_PARAMS.txt：1 個檔變動
     workorder/README_WORKORDER.txt：1 個檔變動
     操作紀錄 oplog_20261007.txt：新增 145 行
  掃描：權杖／私鑰／7z 密碼／部署金鑰 0 筆（設定檔照原樣、含密碼檔，EastSun 1002 裁決）。
MD5 清單在 MANIFEST_MD5.tsv。
