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

MD5 清單在 MANIFEST_MD5.tsv。
