# State Record 機台→分析端轉送（AI(W906-SRRELAY) 20261005）

機台出問題時先按 State Record（RULINGS_20261004 第 1 條），再用這裡的兩支工具把紀錄送到分析端：
機台推上 GitHub，NB2-1（或其他人）抓下來分析。使用者 1005 裁決 A／A／A（RULINGS_20261005 第 2 條）：
推 wb_serve 產出的 `.zip` 原檔；推送腳本由筆電寫、隨 GitHub 更新包送到機台；NB2-1 每一輪檢查有沒有新紀錄。
兩支都**不是 golden 的功能**，也不進建置。

| | 機台端 | 分析端（NB2-1／筆電／同事） |
|---|---|---|
| 工具 | `push_staterecord.ps1`（Windows PowerShell 5.1，純 ASCII） | `fetch_staterecord.py`（Python 3） |
| 做什麼 | 取最新一份紀錄（`D:\HT9045_StateRecord\<yyyy-MM-dd HH_mm_ss>.zip`），連同說明 `REQUEST.md` 推到 GitHub `machine/integ-ioweb` 的 `dispatch/<yyyyMMdd>_staterecord_<HHmmss>/` | 抓 `dispatch/` 裡還沒解開過的紀錄，核對 MD5 後解到 `D:\HT9045_Staterecord\from_machine\<資料夾>\` |
| 需要 | 機台現有的推送資料夾 `D:\HT9045\_push_github_20260926`（deploy key 已設好） | git、能連 GitHub（repo 公開，不用帳密） |

## 機台端：按完 State Record 之後

```
powershell -NoProfile -ExecutionPolicy Bypass -File HT9011UC_Cpp_V3.33.906.0\tools\staterecord\push_staterecord.ps1 -Note "HOME 卡在 step 1520，10:15 按 Abort 之前先按了 State Record"
```

- `-Note` 一定要寫：**發生什麼、幾點、按之前做了什麼**（分析的人看不到現場，也沒有截圖）。中文從 cmd／Git Bash 呼叫可能變亂碼，改用 `-NoteFile <UTF-8 文字檔>`。
- 預設推「最新的一份」。wb_serve 的背景工作還在複製或壓縮時，腳本會等（最多 `-WaitSec 900` 秒，等資料夾被刪、只剩 zip）；
  資料夾放超過 10 分鐘沒人寫，視為壓縮失敗的殘留，腳本自己壓（有 7-Zip 用 7-Zip，沒有就用 .NET），並在 REQUEST.md 註明。
- 指定某一份：`-Record "D:\HT9045_StateRecord\2026-10-05 10_15_30.zip"`。只看不推：`-Preview`。
- 不會挑 `_tasklist` 資料夾（那是動作流程對照用的，不是紀錄）。同一份推第二次會被拒絕。超過 95 MB 拒絕（GitHub 單檔上限 100 MB）。
- 推送前推送資料夾必須是乾淨的（其他推送腳本也用它）；遠端有新 commit 會先 fast-forward；推送瞬間被別人搶先，會 rebase 後再推一次（只新增一個資料夾，不會衝突）。推完用 `ls-remote` 核對遠端 HEAD。

## 分析端：NB2-1 每一輪

```
python HT9011UC_Cpp_V3.33.906.0/tools/staterecord/fetch_staterecord.py          # 抓新的、解開
python HT9011UC_Cpp_V3.33.906.0/tools/staterecord/fetch_staterecord.py --list   # 全部清單（x＝這台已解開）
python HT9011UC_Cpp_V3.33.906.0/tools/staterecord/fetch_staterecord.py --record 20261005_staterecord_101530   # 重解一份
```

- 印出 `NEW …` 就是有新紀錄：照 `ht9045-state-record-analysis` 技能（SOP：`references/analysis-sop.md`）分析，**連 `oplog_*.txt` 一起看**；
  報告寫 `docs/nb2_assist/RD5軟體_SR_<資料夾>_<YYYYMMDD_HHMMSS>.md`，README 那一輪記一行。要機台端做的事寫在報告的「給機台」一節，筆電轉 GitHub README／TO_ES02。
- NB2-1 不在時，其他人（St01／St02）可以接，**先在自己的頻道認領**再分析，避免兩邊重工。
- 也認得人工推的：`dispatch/` 底下任何資料夾，只要有「檔名以按下時間開頭」的 `.zip`／`.7z`，或資料夾名含 `staterecord` 且有 `.zip`／`.7z`。沒有 REQUEST.md 時會註明「去問機台端發生什麼」。
- git 快取是 blob-less clone（`<dest>\_gitcache`），只下載紀錄本身；同一分支上 100 MB 以上的機台安裝包不會被抓。
- 檔名：7-Zip 在 cp950 機台上把 Big5 檔名寫在標頭、UTF-8 檔名寫在 Unicode Path 附加欄位（1005 量 1001 參考軌跡的 `2DFTPPara - 複製.ini`）；工具優先用 UTF-8 那份，Windows 不允許的字元換成 `_`。

## 這份紀錄看得到什麼、看不到什麼（1005 評估，main `966965c0`）

詳細逐檔對照是 St02 的 S-24 缺口分析（`v906/steven-handoff` 的 `docs/handoff/ST02_S24_GAP_ANALYSIS_20261004.md`）。重點：

| 內容 | 現在 | 說明 |
|---|---|---|
| IO（全部 DI／DO、氣缸、吸嘴） | **沒有** | `cStateRecord.cpp` 0 處讀 `Sen[]`／`SW[]`；golden 靠 `MotionView.bmp` 等截圖，移植版不截圖（GATE G2） |
| 馬達 | **部分** | `Task_ListWithTime.csv` 的 MOT 段只列 `Motor!=NULL && Enable` 的軸（`cStateRecord.cpp:350`），只有 fCanMove*／CMD／Encoder／Speed；沒有目標位置（`Motor.xls` 沒翻）、Servo／Alarm／原點／極限燈、HomeFlag、1203 statusword；停用或沒馬達的軸整列不見 |
| HOME 狀態機 | 部分 | `HomeStep` 只在 `SystemStart` 為真時取樣；`ProcessMotorHome` 的 flag1～18 是 static，讀不到 |
| 電源／煞車 | 沒有快照 | 只有 oplog 的切換紀錄 |
| 對話框／卡死 | 沒有 | tick 卡死或阻塞框開著時按鈕按不出來 |
| 變化歷史 | 只有 oplog | `oplog_*.txt` 記指令、各軸變化、Motor Test 鎖／電源（需設 `W906_OPLOG_DIR`，機台有設） |

補強：St02 S-24 的 `W906_*` 檔（Motor1203／Home／PowerBrake／Dialogs／Health／OpLogTail＋卡死路徑＋停擺 60 秒自動錄）
程式已寫好，1004 14:43 起停在「6 個共用檔的掛勾」被 St02 那邊的權限檢查擋住。**它的設計沒有完整的 IO 表**，見 RULINGS_20261005 第 2 條的待決題。
