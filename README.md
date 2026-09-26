# HT9050 機台更新用 repo

> **這不是主版本庫。** 主版本庫是公司 GitLab（`gitlab.honprec.com/.../ht9045`，分支 `main`）。
> 這個 repo 只為了讓連不到公司 git 的 HT9050 機台可以用 `git pull` 拿到更新；每一顆 commit ＝ GitLab main 某一版的**更新包快照**，不帶 GitLab 的歷史。

## 裡面有什麼

| 路徑 | 內容 |
|---|---|
| `HT9045/` | 從機台上次合進去的版本（`BASE`，見下表）到這一版（`REV`）之間**有變動的檔案**，路徑照 `D:\HT9045` 的結構（`HT9011UC_Cpp_V3.33.906.0/`、`web/`…） |
| `_machine_ai/` | 給機台端 Claude 的套用工具：`check_and_copy.ps1`（檢查／三方合併）、`package_manifest.tsv`、`known_versions.tsv`、`base_<BASE>/`（三方合併的底稿）、`README_MACHINE_AI.md`（這一版的內容與步驟） |

| 版本 | 值 |
|---|---|
| BASE（機台上次合進去的筆電版本） | 見 `_machine_ai/README_MACHINE_AI.md` |
| REV（這一包對應的 GitLab main） | 見 `_machine_ai/package_manifest.tsv` 第一行 |

## 更新包清單（照順序套，一包套完、commit 之後才套下一包）

| 順序 | 位置 | 對應 GitLab main | 相對（底稿） | 內容 |
|---|---|---|---|---|
| 1 | 根目錄 `HT9045/`＋`_machine_ai/` | `56bbf785` | 機台上次合進去的 `66cb14e0`（`_machine_ai/base_66cb14e0/`） | 合併包（取代 USB 第二、三包），277 檔 |
| ~~2~~ | `updates/410d27d9/` | `410d27d9` | `56bbf785`（`updates/410d27d9/_machine_ai/base_56bbf785/`） | 網頁權杖卡住（Motor Test 拿了不還，IO 頁按 Output 被擋 10 分鐘），4 檔（**被第 3 包取代**：漏帶了在它之前的 HAlarm） |
| 3 | `updates/db6736c5/` | `db6736c5` | `56bbf785`（`updates/db6736c5/_machine_ai/base_56bbf785/`） | **取代第 2 包**：HAlarm（氣缸逾時照 golden 停機跳 JAM）、權杖修正、第 10～13 條、HT9050＝HT9046_LS＋1203（扭力不再開 COM11）、Steven 的 widget，58 檔。**Jimmy 說可以再套** |
| 4 | `updates/0b506e82/` | `0b506e82` | `db6736c5`（`updates/0b506e82/_machine_ai/base_db6736c5/`） | Steven 的 cMyDB CSV 版（sqlite 退役＋AlarmCode 目錄）與 Tester 通訊 P0 骨架（未接 wb_serve），24 檔。**先套第 3 包**，Jimmy 說可以再套 |
| 5 | `updates/b5fb53be/` | `b5fb53be` | `0b506e82`（`updates/b5fb53be/_machine_ai/base_0b506e82/`） | **你們（EastSun）的修改已合進 main**（`20494aea`，TEMP-DOORS 不含），這包大部分是你們自己的檔（Check 會顯示已相同）；新的是：警報框一次只跳一個（HAlarm FormClose）、Light Scale 持有權杖＋keepAlive 補閒置計時、G03 註解、Sync.h。**請把你們的 `machines/HT9050/IO_Table.csv` 現場表推上來**（README_MACHINE_AI）。94 檔。**先套第 3、4 包** |
| 6 | `updates/1e15c4f9/` | `1e15c4f9` | `b5fb53be`（`updates/1e15c4f9/_machine_ai/base_b5fb53be/`） | 第 9 條：阻塞框在等的時候塔燈／蜂鳴器／面板鍵燈照 golden 動、30 秒沒有網頁自動開瀏覽器、是／否框面板 Alarm Reset 消音；**等待中也會 Poll 1203 監看器**（README_MACHINE_AI 有理由）。附 Jimmy 第 27 條裁決（R66-GALI＝A 你們做、D13 維持 0）。11 檔。**先套第 3～5 包** |
| 7 | `updates/bc5add9e/` | `bc5add9e` | `1e15c4f9`（`updates/bc5add9e/_machine_ai/base_1e15c4f9/`） | 安全 PLC 閘照 golden 打開（**SafePlcIO=0 行為不變**；SafePlcIO 還不要改 1，見 README_MACHINE_AI）；網頁權杖：motor.access 被拒不自動重送、按住 jog 時不還權杖。10 檔。**先套第 3～6 包** |
| 8 | `updates/a6b553ef/` | `a6b553ef` | `bc5add9e`（`updates/a6b553ef/_machine_ai/base_bc5add9e/`） | Steven02 的測試機通訊（GPIB／RS232Standard 引擎，`TesterComm/` 42 個新檔）第一次接上 wb_serve —— **開機會自動啟動引擎執行緒，不要就設 `HT9045_TESTERCOMM=0`**；共用標頭有新增行，要全量重編。59 檔。**先套第 3～7 包** |
| 9 | `updates/378fbb77/` | `378fbb77` | `a6b553ef`（`updates/378fbb77/_machine_ai/base_a6b553ef/`） | 教導頁 HOME 的權杖：HOME 進行中不還、做完照 golden 彈起按鈕、每 60 秒續權杖。1 檔（網頁，不用重建 C++）。**先套第 3～8 包** |
| 10 | `updates/4c067b5f/` | `4c067b5f` | `378fbb77`（`updates/4c067b5f/_machine_ai/base_378fbb77/`） | Steven02 測試機通訊第二批：atester 四段活的翻譯、On-Line／Off-Line 切換本體（Off-Line 一律走 GPIB 模擬；SECS 遠端切換現在會真的切）、主畫面 Tester 鈕的 C++ 動作（網頁還沒有按鈕）。16 檔，**要全量重編**。**先套第 3～9 包** |
| 11 | `updates/10936285/` | `10936285` | `4c067b5f`（`updates/10936285/_machine_ai/base_4c067b5f/`） | IO 輸出命令快取越界修掉（14 組 1203 輸出共用快取位元，RULINGS_20260925 第 18／29 條）：快取放大、MotionNet 規則不動、1203 輸出超出快取回 2。8 檔，**要全量重編**。**先套第 3～10 包** |
| 12 | `updates/dfe09efb/` | `dfe09efb` | `10936285`（`updates/dfe09efb/_machine_ai/base_10936285/`） | ctest 不再寫到機台的 system\lastdata*.dat（只有測試執行檔轉進自己的沙盒，正式程式不變）；新 ctest LastDataSandbox。6 檔。**先套第 3～11 包** |
| 13 | `updates/19844f8e/` | `19844f8e` | `dfe09efb`（`updates/19844f8e/_machine_ai/base_dfe09efb/`） | 多分頁的視窗總表不再互相蓋掉（WebCommand.connId 以前永遠是 0）；ctest 不寫真實 config.ini（正式程式不變）。7 檔，**要全量重編**。**先套第 3～12 包** |
| 14 | `updates/21323505/` | `21323505` | `19844f8e`（`updates/21323505/_machine_ai/base_19844f8e/`） | ⚠ **UpdateMainOperateMode 整支照 golden 翻**（0922 裁決）：wb_serve 開機／讀配方／切模式時會切加熱器繼電器、送 ATC 命令、寫 lastdata 與 config.ini（照 golden）。10 檔，**要全量重編**，套之前確認機台周圍有沒有人。**先套第 3～13 包** |
| 15 | `updates/c65ddd85/` | `c65ddd85` | `21323505`（`updates/c65ddd85/_machine_ai/base_21323505/`） | NB2 R70 覆核第 9 條的四件已修：**警報框開著時網頁 IO 輸出不再被執行**（以前約一半會動）、關框後 START／PAUSE 燈不再卡住、bAlarmReset 照 golden 清、是否框收 sim.di.set。3 檔。**先套第 3～14 包** |
| 16 | `updates/39bd8f1e/` | `39bd8f1e` | `c65ddd85`（`updates/39bd8f1e/_machine_ai/base_c65ddd85/`） | ChangeATCSiteUse 接上另外兩處：三溫機 TriTemp 的 5 個呼叫點（原本是空樁）、Home（經 hook）；HT9050 不是三溫機、沒有 ATC 連線，實際效果很小。7 檔。**先套第 3～15 包** |
| 17 | `updates/d4897dbd/` | `d4897dbd` | `39bd8f1e`（`updates/d4897dbd/_machine_ai/base_39bd8f1e/`） | ⚠ 警報框開著時照 golden 檢查 Index 吸嘴的 IC 掉落（該有 IC 卻沒真空、INDEX_SUCKER_TYPE==1 時會動到真空輸出）；面板 Alarm Reset 照 golden 清兩個 SECS 旗標。3 檔。**先套第 3～16 包** |
| 18 | `updates/5459651f/` | `5459651f` | `d4897dbd`（`updates/5459651f/_machine_ai/base_d4897dbd/`） | ⚠ 警報框照 golden 設暫停標記：用 START 答掉警報後，恢復運轉時手臂先回 Z 安全位、tester 逾時重算（NB2 R71 C1；筆電沒實跑，請機台驗）。3 檔。**先套第 3～17 包** |
| 19 | `updates/04c72d84/` | `04c72d84` | `5459651f`（`updates/04c72d84/_machine_ai/base_5459651f/`） | 測試通訊（St02）：機台停著、測區沒有 IC 時，On/Off-Line、工單、GPIB 位址、bin 數的變化照 golden 同步給測試機橋接程式（P2f，以前從來沒送）；TCP/IP OS 測試機換工單送 WORKFILE；測試通訊頁 400 ms 內同一顆鈕只算一次（P7）。不會讓馬達動。9 檔。**先套第 3～18 包** |
| 20 | `updates/df8d1f69/` | `df8d1f69` | `04c72d84`（`updates/df8d1f69/_machine_ai/base_04c72d84/`） | 🔴 **`USE_ATC_MODE=4` 的機台換配方／登入／HOME 會當，這一包修掉**（開機照 golden 先 InitialATC）；Jam 次數、UPH 表、one cycle 存 `system\Arm*.dat`、Index 時間平均、測試秒數照 golden 開始有數字。不會讓馬達多動；套之前加備 `system\Arm*.dat`／`ArmHis*`／`ArmByLot*`、`ATC.ini`。15 檔。**先套第 3～19 包** |
| 21 | `updates/71eda9b5/` | `71eda9b5` | `df8d1f69`（`updates/71eda9b5/_machine_ai/base_df8d1f69/`） | St02 第三批：開機照 golden 建 log 物件、**開始寫 `D:\HT9045_Log\`**；On-Line 測試不再一送 SOT 就逾時；Qorvo 的 Tester Pause 等 MaxTestTime 才響；觀察頁 bin 歷史與 bin 顏色照 golden。不會讓馬達多動；套之前加備 `D:\HT9045_Log\`。32 檔。**先套第 3～20 包** |
| 22 | `updates/d1bd26ad/` | `d1bd26ad` | `71eda9b5`（`updates/d1bd26ad/_machine_ai/base_71eda9b5/`） | 開機紀錄 `D:\HT9045\Error\BootLog.txt` 照 golden 接上（開機當掉時看停在哪一行）；其餘只改註解。不會讓馬達動、不改 IO。6 檔。**先套第 3～21 包** |
| 23 | `updates/661cc68c/` | `661cc68c` | `d1bd26ad`（`updates/661cc68c/_machine_ai/base_d1bd26ad/`） | St02 第四批：GPIB 程式的額外 RS232 port 跟配方走（P6 Q2(a)）、TTL 重送條件改讀網頁視窗總表、開機讀／建 `D:\HT9045\Error\AlarmCodeList.txt`（cMyDB P3）、testercomm 頁。不會讓馬達動、不改 IO；套之前加備 `D:\HT9045\Error\` 與 GPIB 配方。25 檔。**先套第 3～22 包** |
| 24 | `updates/ef83be05/` | `ef83be05` | `661cc68c`（`updates/ef83be05/_machine_ai/base_661cc68c/`） | 面板 Alarm Reset 照 golden 送 SECS 事件 30（SECS 開著的機台 host 會收到）。沒有 IO、沒有動作。3 檔。**先套第 3～23 包** |
| 25 | `updates/44ddf2c6/` | `44ddf2c6` | `ef83be05`（`updates/44ddf2c6/_machine_ai/base_ef83be05/`） | 觀察頁的測試／Index 時間表照 golden 每次測完更新（以前是空的；遠端 IndexTime、SECS TestTime 也有值了）。⚠ `dTestSec` 從此每顆更新（ATC 測試時間補償會用）。不會讓馬達動、不改 IO。9 檔。**先套第 3～24 包** |
| 26 | `updates/dcd37592/` | `dcd37592` | `44ddf2c6`（`updates/dcd37592/_machine_ai/base_44ddf2c6/`） | 只影響 ctest：每一支測試都拿到路徑轉向變數（以前檔尾十幾支會寫真的 `D:\HT9045_Log`／system）；ctest 沙盒 helper 認 `/`。wb_serve 行為不變。**需要 CMake ≥ 3.19**。3 檔。**先套第 3～25 包** |
| 27 | `updates/683fafa1/` | `683fafa1` | `dcd37592`（`updates/683fafa1/_machine_ai/base_dcd37592/`） | St02 第五批：log 根目錄可由環境變數轉向（只給 ctest；機台不要設，沒設＝golden 路徑）、站況 log 轉接函式。wb_serve 行為不變。5 檔。**先套第 3～26 包** |
| 28 | `updates/95c2c26a/` | `95c2c26a` | `683fafa1`（`updates/95c2c26a/_machine_ai/base_683fafa1/`） | 主畫面兩個 log 出口照 golden 翻（memo 仍是替身，機台上沒差別）、觀察頁 Yield 圖帶 dfm 預設值、**wb_serve 開機時印出有設的 W906_* 轉向變數**（F5「IOWEB(這台)」會看到 12 列 `!!`，HT9045_Web.cmd 應該 0 列）、ctest 的 log 根目錄轉向。11 檔。**先套第 3～27 包** |
| 29 | `updates/3f166785/` | `3f166785` | `95c2c26a`（`updates/3f166785/_machine_ai/base_95c2c26a/`） | wb_serve 開機照 golden 呼叫 `InitialMemory()`（開機主控台不再印 `SiteData[] seeded` 那一行）、START 呼叫點普查更正成 34／30／4（活的路徑數沒變）＋新 ctest、觀察頁 mtRow 大小、St02 的 Event Log Analyzer 核心（新 library＋ctest，沒連進 wb_serve）。19 檔。**先套第 3～28 包** |
| 30 | `updates/b635f32d/` | `b635f32d` | `3f166785`（`updates/b635f32d/_machine_ai/base_3f166785/`） | 良率計算兩支放進活的檔（PAT 沒連進 wb_serve，機台上沒差別）、三個解閘標記的註解行號、完成度普查工具修正、InitialMemory 測試多一節。wb_serve 行為不變。11 檔。**先套第 3～29 包** |
| 31 | `updates/fe03a1e7/` | `fe03a1e7` | `b635f32d`（`updates/fe03a1e7/_machine_ai/base_b635f32d/`） | 還原被吃掉的反斜線（原始碼只動 cObserver.cpp 4 行註解，其餘是文件）。wb_serve 行為不變、可不重建。5 檔。**先套第 3～30 包** |
| 32 | `updates/c7dc8145/` | `c7dc8145` | `fe03a1e7`（`updates/c7dc8145/_machine_ai/base_fe03a1e7/`） | RecordErrorLog 照 golden 寫日期 log 檔：開了 Site Use Manager 的機台，運轉時會在 `D:\HT9045_Log\SiteUseMgr\YYYY\MM\` 每小時產生一個檔（跟 BCB6 版一樣）；**要重建 wb_serve 才生效**。另 6 處註解／工具腳本裡被吃掉的反斜線。11 檔。**先套第 3～31 包** |

每一包各自有 `_machine_ai/README_MACHINE_AI.md`（內容與步驟）和自己的 `check_and_copy.ps1`。舊包不會被刪掉，`git pull` 不會讓正在套的那一包消失。

## 機台第一次下載

```
git clone https://github.com/HPI-Jimmy-Chiu/HT9050.git D:\HT9045\_from_github
```
這個 repo 是公開的（使用者 20260926 裁決：否則機台連不進來），clone／pull 不需要帳密。

## 之後每次更新

```
cd /d D:\HT9045\_from_github
git pull
```
接著**照 `_machine_ai/README_MACHINE_AI.md` 的步驟**用 `check_and_copy.ps1` 套到機台整合樹（`D:\HT9045\_integ_ioweb`）：
先 `-Mode Check`（唯讀）→ EastSun 同意 → 備份 → `-Mode Apply`。**不要直接複製覆蓋**，機台有自己的修改，要靠三方合併。

## 安全

- 權杖、7z 密碼都**不可以**寫進這個 repo、commit 訊息或任何檔案。
- 這個 repo 是**公開**的：任何人都能下載。所以只放「更新包內容」，不放 GitLab 的完整歷史；權杖、密碼、客戶資料一律不放。
- 機台端推回來的內容放 orphan 分支 `machine/integ-ioweb`（format-patch），同樣只放機台自己的 commit。
