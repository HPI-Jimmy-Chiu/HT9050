# 派工給 Jimmy：沒有 AI 的環境下，HT9050 機台軟體怎麼安裝、怎麼更新（請實測）

- **發出**：機台端（EastSun 1009「請派工給 jimmy 需要測試 在沒有AI 的環境下 安裝與更新 需要怎麼做」）。
- **目的**：今天機台的安裝、套包、建置、驗證全部靠機台上的 Claude。客戶端或沒有 AI 的電腦要能**只靠文件和腳本**完成同樣的事。請筆電端整理出這套流程，並在一台**沒有 Claude 的電腦**上從頭實測一次。

## 交付物
1. **安裝手冊**：一份人照著做就能完成的步驟。
2. **更新手冊＋腳本**：從第 N 包更新到第 N+k 包。
3. **實測紀錄**：用哪台電腦、從哪一包到哪一包、每一步花多久、卡在哪裡、怎麼解。

## 一、全新安裝：需要寫清楚的內容
- **前置軟體**（版本、安裝位置、下載來源）：
  - WinLibs i686 g++ 16 工具鏈（`%LOCALAPPDATA%\Programs\ht9045-nonoracle-toolchain\mingw32`）
  - CMake 4.4.2
  - Advantech Common Motion 驅動（PCIE-1203，ADVMOT.dll）
  - 選用：VS Code＋HTML 設計外掛
- **目錄配置**：
  - 程式樹 `D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`
  - 網頁 `...\web`
  - 執行設定 `runcfg`
  - 機台設定 `D:\HT9045\system`
  - 紀錄 `D:\HT9045_Log`
- **兩種安裝方式都要寫**：
  - (a) 直接放**預先編好的 wb_serve.exe**，不需要編譯環境，這最可能是客戶端的做法。
  - (b) 從原始碼編。編譯指令請寫死 **`-j 2`**：機台只有 7.7 GB RAM，10/08 用 -j4 整包重編時當機（Kernel-Power 41），還留下 NUL 填滿的 .obj／.a 檔。
- **機台設定檔永遠不覆蓋**：IO_Table.csv、Mot_Table.csv、Gerneral.ini、Pci1203Modules.ini、teach.ini、lastdata.dat。安裝程式要先檢查這些檔案是否已經存在，存在就跳過。
- **啟動方式**：不用 F5，要一個 .bat 或捷徑，而且要寫清楚環境變數，例如 `W906_GENERAL_INI_PATH`、`W906_IOTABLE_PATH`。

## 二、更新（套包）：需要寫清楚的內容
1. **先備份**：程式樹（建一個 git 分支）、web、`D:\HT9045\system`、`lastdata.dat`。
2. **分類**：照現在的 `_machine_ai/check_and_copy.ps1`，把每個檔分成 NEW／OLD／SAME／LOCAL。
   - **OLD 不能直接蓋**：每個 OLD 檔要先掃機台獨有的 `AI(W906-...)` 標記和 `W906_` 識別字，掃到會消失的就改走三方合併。實例：直接換 153～157 包會把軟體急停和 TOKEN-OFF 刪掉。
   - **LOCAL（兩邊都改）**：沒有 AI 時要有明確規則，例如 `git merge-file` 沒衝突就收；有衝突就停下來，列出檔名請人處理，不能自動猜。
   - **機台固定保留的清單**要寫在腳本裡，不要靠人記：IOTHREAD、TEMP-*、軟體急停／SOFTKEY、TOKEN-OFF、.vscode/tasks.json、WORKLOG_MACHINE.md、設計外掛資料夾。
3. **套用**：保留機台原本的行尾（CRLF／LF）。新的網頁檔一律用 CRLF。
4. **建置**：
   - 用 `cmake --build ... -j 2`。
   - 套用時檔案時間要更新。注意 Copy-Item 還原備份會帶回舊的修改時間，make 會以為不用重編。
   - 被中斷或當機的建置，要先檢查產物裡有沒有 NUL 填滿的檔案。
5. **驗證**：
   - ctest，失敗清單要跟「已知失敗清單」比對，請把清單做成檔案放進包裡。
   - 啟動 wb_serve，看到 oplog 的「1203 連線正常」和「模組檢查通過」。
   - IO 頁要能看到**數值真的在變**，不能只看狀態欄位。
6. **失敗時退回**：切回備份分支、還原設定檔、重新編譯，要寫成一個指令。

## 三、現場常見狀況（請寫進手冊的疑難排解）
| 現象 | 原因／處理 |
|---|---|
| 馬達命令全部失敗 `0x800000D3 GetAuthorityFailed` | Common Motion Utility 開著，或驅動器在警報。先關 Utility，再 Alarm Reset |
| 開卡失敗 `0x8301000E ECAT_ReconnectError_R0` | ring 0 驅動器的 EtherCAT 狀態異常。依序試：等待重試 → 裝置管理員停用／啟用「PCIE1203 Series Motion Device」（需要系統管理員權限）→ ring 0 驅動器控制電斷電再送電 |
| 驅動器 A.A12（EtherCAT Output Data Synchronization Error） | 斷電或主站中途消失後留下的。開機時其他 18 軸會自動清；MTestZ1 從 cpp 0348 起也會自動清 |
| 開機 30～90 秒畫面沒反應 | 開卡時主迴圈在等，屬於正常 |
| 實體按鈕沒反應 | 驅動器警報時，主流程一直重送停止命令，拖慢主迴圈。先 Alarm Reset |

## 四、實測要求
- 一台**乾淨、沒有 Claude** 的 Windows 電腦，模擬組態就可以；有實機更好。
- 情境 1：照安裝手冊從零安裝，啟動成功。
- 情境 2：從某一包更新到最新一包，其中至少要有一個 LOCAL 衝突，看手冊怎麼處理。
- 情境 3：刻意讓建置或更新失敗，照手冊退回。
- 請回報：全部步驟、每一步的時間、遇到的問題，以及手冊需要修改的地方。

（機台端目前在第 218 包，machine/integ-ioweb 最新到 cpp 0348。）
