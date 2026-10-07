# Tray Arm 教導保護與 State Record 補強

Jimmy 20261006：可自主維護 State Record 分析資料；真機教導由 Jimmy 到機台處理，筆電先填模擬值；最後指示「編譯成功就讓我手動測試」。

## 真機端核對項目

1. 核對 MTrayX Loader / Empty / Color 的實際位置與座標方向。目前記錄為 Loader=-4150、Empty=0、Color=0；Empty 與 Color 同為 0 是否符合 HT9050 的實體配置，須現場確認。
2. 核對取盤偏移與 Offset。目標不是單純教導值：本次有效 Loader=2650、Empty=6800、Color=6800；教導值加上取盤偏移與各站 Offset 才是 Prod 目標。
3. 核對保護公式的適用性。HT9050 一般分支沿用 LS：(Empty教導+Color教導)/2+6500。本次為 6500，因此 Empty 目標 6800 被攔。Loader/Empty 目標須不大於該界線；Color/Auto 目標須不小於該界線。若 HT9050 實機 Empty/Color 確實共用位置，應裁決機型適用的保護公式，不能為了過判斷而教導虛假座標。
4. 核對 Auto1/2/3 教導值（記錄為 79080/97430/115960）與實際目標、行程範圍、In/Out Arm Z 上位互鎖。Auto4/5/6 在本組資料未教導為 0，不能把本次有限範圍模擬說成全站驗證。

## 證據與定位

- 原始資料：D:/HT9045_StateRecord/2026-10-06 19_04_29.zip。
- 執行檔 SHA256 A45011BB1488E6433E7421E81FF741F7C144536BC2834E80E7602B5B87EF1426；W906_ReadMe build=SIM、exeTime=18:57:06。Ver.txt 空白，並非無其他版本資訊。
- HOME 19:04:02.686 完成，HomeStep 到 1600 再回 1。19:04:03.938 自動搬盤 DoCatchTray case 10 呼叫 TrayArmMotorMove(Prod.iXTrayEmpty) 才拒絕 6800。
- MainProc LastEnter=19:04:29.333、SaveTime=19:04:29.554，差 0.221 秒；SilentSec=0.263、Alive=Y。Task_ListWithTime2 最新活動=19:04:03.911。屬警報等待，不是執行緒掛死。
- 先比機台再結論：同步 GitHub machine/integ-ioweb ef3658569（機台 18:59 快照）後，START→HOME→自動搬盤仍重現同一警報。其後驗證新版用 cced7eac2（機台 19:14 快照），只另外覆寫已授權的模擬教導檔。

## 筆電模擬資料

- 獨立檔：D:/HT9045/config/simulation/teach.ini。來源教導檔不改，只將 [MTrayX] setEditTrayColorX=20000，檔頭明示 SIMULATION ONLY。
- 工具：tools/prepare_ht9050_sim_teach.py；保留其他所有教導值，既有輸出先備份，不允許來源與輸出相同。
- 新界線 16500；實測 Loader目標=2650、Empty目標=6800、Color目標=26800，Auto1/2/3目標=85880/104230/122760。
- 僅在本機 F5 的模擬項目加入 W906_TEACH_INI_PATH 指向此檔；本機 launch.json 舊檔備份 D:/AI_TempFile/tray6800-before-launch.json。其他啟動項目與機台快照不改，模擬副本不推回機台。
- 實測約 80 秒，沒有新 modal，CatchTrayTask 已過原本第10步，循環 400→600→1000→1100；MTrayX 快照已到122760。這是通過本次堵點的證據，尚未宣稱全自動生產各種工單都驗證完成。

## State Record 變更

- 舊資料夾殘留的兩個含 U+00A6 的檔名，CP950 經 ANSI API 轉成 `|` 後不存在；Del_Tree 刪不掉。兩份原 ZIP CRC 完整，殘留檔逐位元組都已在 ZIP，原始證據保留。
- StateRecordArchive.h：僅清理已成功壓縮的時間資料夾，Unicode API 貫穿，拒絕非時間名稱、不跟隨 reparse point，唯讀檔可清除，刪除失敗記錄 Win32 錯誤。
- W906_Executable.txt：在背景執行緒記錄實際執行檔 SHA256、該記錄模組編譯時間與 SIM/SHIP；原 W906_ReadMe 仍提供路徑和 exe 檔案時間。
- DecisionVariables.csv 補齊機種、教導來源、教導/Prod值、MTrayX位置快取；W906_EffectiveTeach.ini 保存實際使用的副本。
- Motor/mymotor.cpp 只增加拒絕瞬間的 RecordProcess 診斷，不改保護條件：target、safe、station、機種、教導值、Prod值、位置快取。snapshot 值與事件瞬間值有明確區別。
- 分析技能同步更新入口與此案判讀。

## 驗證與交付範圍

- 新增 StateRecordArchive 測試：Unicode特殊檔名、唯讀、目錄名稱邊界、鎖檔錯誤保留、SHA256 abc/空檔已知向量、不存在檔案；通過。
- 既有 St02_StateRecordDiag 通過。兩支 ctest 2/2。
- 19:18:38 新版實錄 ZIP CRC 正常，原資料夾不存在；有效 teach Color=20000，SHA256 與實際執行檔一致，新增 DecisionVariables 欄位有真實值。
- 模擬／出貨完整建置途中，依使用者「編譯成功就讓我手動測試」停止，改只完成 wb_serve 模擬目標。沒有宣稱完整 gate 或出貨組態驗證完成；不部署真機。
- 實機同步與測試造成的設定/工單變更已以 machine_sync restore 和 realfile_guard restore/check 還原。只有獨立模擬教導檔與本機 F5 覆寫設定為本次刻意保留的配置。
