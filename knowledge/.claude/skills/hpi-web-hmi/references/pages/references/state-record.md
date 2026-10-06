> 保存來源：`.claude/skills/ht9045-html-version/references/state-record.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# State Record（程式快照）— HTML 端設計與協定

> 來源：**BCB6** `main.cpp` `void __fastcall TfMain::DoStateRecord(int iShowAlarm, bool bManual)`
> （Steven 20220716 獨立出來避免被 Alarm 擋住；KenHsieh 20230116 區分手動/自動）。
> 後續 VC++ 版需另行標注，不可與 BCB6 混寫。

## 1. BCB6 行為摘要

| 觸發 | 呼叫 | 說明 |
|---|---|---|
| `sbStateRecord`（tsMotionView/ScrollBox1，120×74 'State Record'）<br>`sbStateRecord2`（tsMain/palSetting/Panel2，43×35 圖示鈕） | `sbStateRecordClick → DoStateRecord(0, true)` | **手動**：跳 `SaveDialog1` 讓使用者選路徑（預設 `D:\HT9045_StateRecord\<yyyy-MM-dd HH_mm_ss>`），結束後 `ShellExecute` 開資料夾 |
| Alarm / Hang 偵測（acatchtray / ainarm2 / asendic / atester* / csystem / uhome） | `DoStateRecord(0|1|2, false)` | **自動**：直接寫到預設路徑；`iShowAlarm` 傳給 `iSaveImgae` 決定抓圖時是否含 Alarm 視窗 |

主要步驟（`state-record.json` 的 `steps[]` 一一對應）：

1. `RecordProcess("State Record.")`、`SaveMachineRecord()`、`UpdateTaskList()`、`QueueGalilCmd.SafeData()`
2. 決定 `NewPath`（手動：SaveDialog；自動：`SDataPath + sFileNameTime`）
3. `MNetLog("Close")`、建立子目錄 `HT9045\{IniData\Data,system,config}`、`GPIB9045\system`、`GPIBLOG`
4. `SendMSG_CMD(MSG_CMD_State_Record)` 通知 GPIB 程式同步存檔
5. 立即複製：目前 EventLog / MNetLog 檔、`HT9045.elf`、`setup.inf`、目前 Recipe 資料夾
6. 產生 `D:\HT9045\system\1.bat`（robocopy `/MAXAGE:2`：EventLogTxt、Galil_LOG、MNetLog；XCOPY：system、config、GPIB system；依 `TestIF_File.iTestType` 複製 GPIB / TCP / TTL+RS232 log；OLP → Automation 7z；AutoClean → CleanPad_Log 7z）→ `ExecZipCommand`
7. `UpdateMotorScreen(true)` → `Motor.xls`、`Task.xls`、`Task_List.xls`、`AutoClean.xls`、`SaveTaskList()`（`Task_ListWithTime*.csv`）、`SaveDecisionVariables()`
8. Hot 模式：HP1/HP2 六組 xls
9. `MainFormSizeToEpson(true)` + `iSaveImageTask=1` → Timer 抓 `MainForm.bmp`，之後 7z 壓縮 `NewPath.zip` 並 `Del_Tree`
10. `Ver.txt`、`RecordIndexPosition(0,3)`、`LogIndexMaxMinPos("StateRecord")`、`DumpMainFormSnapshot(NewPath)`

## 2. HTML 端限制與對策

HTML **不能**寫檔、跑 robocopy/7z、抓螢幕。因此：

| BCB6 動作 | HTML 對策 |
|---|---|
| 實際存檔/壓縮 | 發 **request**（`JSON/state-record.json` → `request`）交給 C++ 端執行 |
| 進度 / 結果 | 輪詢 **`JSON/state-record-ack.json`**（`running` → `done|error`，含 `steps[]`、`zipFile`） |
| SaveDialog1 | `prompt()` 讓使用者改路徑（取消＝放棄；空白＝預設） |
| Task list、旗標等額外資訊 | C++ 端輸出 **`JSON/Task-runtime.json`**、**`JSON/System-runtime.json`**，HTML 只讀 |
| 離線模式（`cppoffline=1`） | 自動 ack，並把 `offlineBundle[]` 列出的 JSON 打包下載 `StateRecord_<時間>.json` |

## 3. 檔案

| 檔 | 角色 | 內容 |
|---|---|---|
| `page/state-record.js` | 模組 `window.HTStateRecord` | `init(opts)`、`run({button,manual,showAlarm,silent})`、`isBusy()`、`current()`、`exportRequest()`、`folderName()` |
| `JSON/state-record.json` | catalog + request | `protocol{ackPollMs,timeoutMs,offlineAutoAckMs}`、`defaults{savePath,folderNameFormat,include{}}`、`offlineBundle[]`、`steps[]`、`request{}` |
| `JSON/state-record-ack.json` | C++ → HTML | `ack{seq,state,message,path,zipFile,startedAt,finishedAt,currentStep,steps[{id,state,at,detail}]}` |
| `JSON/Task-runtime.json` | 額外資訊 | `mainProcMonitor{alive,callCount,lastEnter,silentSec,saveTime,thresholdSec}`、`tasks[{task,current{case,at},history[{at,case}]}]`（對應 `Task_ListWithTime.csv`：`TaskName, time, case, time, case…` 新→舊） |
| `JSON/System-runtime.json` | 額外資訊 | `version{software,machineType,customerCode,model,serialNo}`、`lot{}`、`state{systemStart,pause,alarm,…}`、`flags{}`（DumpMainFormSnapshot）、`decisionVariables{}`、`counters{}` |

產生器：`D:\AI_TempFile\_gen_state_record.py`（跑完再跑 `_gen_json_shim.py`）。

### request 欄位

```json
{ "seq":1, "id":"sr-1", "source":"gbControlBtn", "button":"sbStateRecord", "action":"stateRecord",
  "manual":true, "showAlarm":0,
  "savePath":"D:\\HT9045_StateRecord\\", "folderName":"2026-09-02 14_05_33",
  "newPath":"D:\\HT9045_StateRecord\\2026-09-02 14_05_33",
  "include":{ "...defaults.include" }, "htmlContext":{ "page":"Main.gbControlBtn.html","cppOffline":true },
  "issuedAt":"…", "state":"requested" }
```

`include` 的值：`true/false` 固定；`"byTestType"`、`"ifOLP"`、`"ifAutoClean"`、`"ifATK_AMR"`、`"ifHot"` 表示由 C++ 端依旗標決定（對應 BCB6 的 if 分支）。

## 4. 接線位置

| 頁 | 元件 | 行為 |
|---|---|---|
| `page/Main.gbControlBtn.html` | `sbStateRecord`（「程式快照」區，兩版本皆顯示） | `HTStateRecord.run({button:'sbStateRecord', manual:true})`；`#stateRecInfo` 顯示進度（`onProgress` 讀 `ack.currentStep`） |
| ~~`page/Main.html` 工具列 `sbStateRecord2`~~ | 已移除（2026-09-02） | controller 面板已有 State Record，主畫面不重複提供 |

先 `<script src="state-record.js">` 再綁定；離線旗標取自 `?cppoffline=1|offline=1`。

**離線 JSON 寫入**：`cppoffline=1` 且 background 已授權 JSON 資料夾（json-writer.js）時，`publish()` 會真的寫 `state-record.json`（request）與 `state-record-ack.json`（steps 全 done，`source.toolchain="HTML-offline"`），詳 [naming-release-writer.md](naming-release-writer.md) §4。

## 5. 互斥與時序

- HTML 端 `pending` 存在（requested/running）時再按 → `Busy` 拒絕（BCB6 亦不會重入：Timer 仍在跑 `iSaveImageTask`）。
- ack 逾時預設 180 s（robocopy + 7z 可能較久），輪詢 500 ms。
- 離線：900 ms 後自動 `done`，並下載快照包（驗證 UI 流程用）。

## 6. 給 C++（BCB6/VC++）端的實作要點

1. 監看 `JSON/state-record.json`（或 localStorage `ht9045-state-record-request`）中 `request.state==="requested"` 且 `seq` 增加。
2. 依 `manual`／`newPath` 呼叫 `DoStateRecord(showAlarm, false)` 並以 `newPath` 覆蓋 `NewPath`（不要再跳 SaveDialog）。
3. 每完成一步更新 `state-record-ack.json`：`state:"running"`、`currentStep`、`steps[]` 追加；結束寫 `done` + `zipFile`，失敗寫 `error` + `message`。
4. `UpdateTaskList()` 之後同步輸出 `Task-runtime.json`；`DumpMainFormSnapshot` 之後輸出 `System-runtime.json`。

<!-- preserved-content:end -->
