# NB2 F5、Bin 分類與 Motion View 修正（20261006）

交付：原本 D:/HT9045/Obj/V906/build_dbg/wb_serve.exe；SIM / Ninja / MinGW 6.3。原始碼與執行檔同步，不另設修正版啟動位置。

## 原因與修正
- NB2 手動測試分支 IO 執行緒整合漏掉 Impl.ioStaged/ioPub 宣告，以及 wb_serve 的 W906_IoThreadBoot 本體。補齊宣告；boot 本體從原始 430155e4a 還原，開關維持原定義。
- 舊程序未關時，Windows 鎖住相同 exe，連結 Permission denied。關掉同一路徑舊 SIM 程序後編譯成功。
- WAR07356：20:59:02 ZIP 使用舊 exe ef73a459...；OFF_LINE、正常 FT 模式。Bin 1 在一般分類表未對應盤，而 GPIB 離線預設回 Bin 1。F5 明確設 W906_SIM_TEST_BIN=2，只在 SOFT_SIMULTE 有效；允許 1..14，無設定或不合法維持預設 1。不改機台配方；Bin 2 在目前工單對應 Auto2。實機 Bin 1 出料目的地仍須依工單需求決定。
- HT9050 FinePitch SIM 冷啟動／HOME：主步驟 1 時先 InitTestYFPTask。
- Motion View9050 示意圖尚無即時位置投影；軸讀數表已接 motor/runtime。增加資料更新、馬達位置變化、斷線提示。
- State Record 增加 iTestRunMode、iTestBinCount、各 Bin 對應盤與盤型、每個 TestSocket Item / testerBin。

## 驗證
- 原本 F5 build.bat serve、等待畫面 -DryRun 成功；Pci1203_IoThread、FP9050_Index 通過。
- TesterComm_GPIB seed=2 與未設定 seed 各通過，核對引擎生命週期真正發布的 Bin 選擇。
- JavaScript 語法檢查通過。
- 執行期來源：GitHub machine/integ-ioweb e57a0a843，機台拍照 2026-10-06 20:15；32 關鍵檔 SYNCED。8093 測試結束後，各次新備份還原且 MD5 驗證成功。
- 21:17:50.zip：新 SIM SHA256 a5fb7a4d8de3e16c3768378da82bb574359ad4212e484d3315f0c034a618bd46；testerBin=2、OutArmTask=300、AutoSHT2Task=10，已通過原分類檢查並進入出料流程。mainProc.callCount=358、silentSec=0.145、alive10s=1。
- 不宣稱完整連續多盤循環已驗收。Jimmy 接著用原本 F5 手動測試。

## 判讀
頂部更新次數增加代表資料持續更新；馬達位置變化代表讀數確有改變。位置不變可能停止或等候，不能單憑示意圖判定 hang。卡住時錄 State Record，核對 W906_Health、Task_ListWithTime 和新 Bin routing 欄位。

先前 20:30/20:36/20:43 machine_sync 備份保留，其後 Jimmy 已操作及新增資料，不直接覆蓋新狀態。本次 21:06:29、21:16:45 備份已各自還原。
