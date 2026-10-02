# 產生 State Record（程式快照）協定 JSON 與額外資訊 JSON 骨架
#   來源：BCB6 main.cpp  void __fastcall TfMain::DoStateRecord(int iShowAlarm, bool bManual)
#   輸出：JSON/state-record.json（catalog + request）
#         JSON/state-record-ack.json（C++ 端回報）
#         JSON/Task-runtime.json（sgTaskList / Task_ListWithTime.csv / MainProcMonitor）
#         JSON/System-runtime.json（Ver.txt / DumpMainFormSnapshot / SaveDecisionVariables 摘要）
#   跑完要再跑 _gen_json_shim.py
import json, os
from datetime import datetime

OUT = r"D:\HT9045\JSON"
NOW = datetime.now().astimezone().isoformat(timespec="seconds")
SRC = {"toolchain": "BCB6", "cpp": "main.cpp :: TfMain::DoStateRecord(int iShowAlarm, bool bManual)",
       "triggers": ["sbStateRecordClick → DoStateRecord(0,true)",
                    "Alarm/Hang 自動 → DoStateRecord(0|1|2,false)"]}

def dump(name, obj):
    p = os.path.join(OUT, name)
    json.dump(obj, open(p, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print("ok:", p)

# ---------- 1. 協定 / 目錄 ----------
state_record = {
    "schemaVersion": "1.0.0", "generatedAt": NOW, "source": SRC,
    "protocol": {
        "requestFile": "JSON/state-record.json",
        "ackFile": "JSON/state-record-ack.json",
        "extraFiles": ["JSON/Task-runtime.json", "JSON/System-runtime.json"],
        "ackPollMs": 500, "timeoutMs": 180000, "offlineAutoAckMs": 900,
        "mutex": "同時只允許一筆 state=requested/running；HTML 端 pending 未結束前拒絕再次觸發"
    },
    "defaults": {
        "savePath": "D:\\HT9045_StateRecord\\",
        "folderNameFormat": "yyyy-MM-dd HH_mm_ss",
        "include": {
            "machineRecord": True, "taskList": True, "galilLog": True, "eventLog": True, "mnetLog": True,
            "elf": True, "setupInf": True, "recipe": True, "system": True, "config": True, "gpibSystem": True,
            "testerLog": "byTestType", "automationLog": "ifOLP", "cleanPadLog": "ifAutoClean",
            "atkAmrLogs": "ifATK_AMR", "motorXls": True, "taskXls": True, "autoCleanXls": True,
            "decisionVariables": True, "hotPlateXls": "ifHot", "mainFormBmp": True, "verTxt": True,
            "indexPosition": True, "mainFormSnapshot": True, "zip": True
        }
    },
    # 離線模式：HTML 自行打包下載的 JSON（模擬 NewPath 資料夾內容）
    "offlineBundle": ["Motor-config.json", "Motor-runtime.json", "IO-config.json", "IO-runtime.json",
                      "Teach-config.json", "Task-runtime.json", "System-runtime.json", "Sim-scale.json"],
    # C++ 端 DoStateRecord 的步驟順序（HTML 進度顯示用；ack.steps[].id 對應）
    "steps": [
        {"id": "recordProcess", "cpp": "RecordProcess(\"State Record.\")"},
        {"id": "saveMachineRecord", "cpp": "SaveMachineRecord()"},
        {"id": "updateTaskList", "cpp": "UpdateTaskList()"},
        {"id": "galilSafeData", "cpp": "QueueGalilCmd.SafeData()"},
        {"id": "pickPath", "cpp": "bManual ? SaveDialog1 : SDataPath+sFileNameTime"},
        {"id": "mnetLogClose", "cpp": "MNetLog(\"Close\")"},
        {"id": "mkdirs", "cpp": "MyForceDirectories(NewPath\\HT9045\\{IniData\\Data,system,config}, GPIB9045\\system, GPIBLOG)"},
        {"id": "notifyGpib", "cpp": "SendMSG_CMD(MSG_CMD_State_Record)"},
        {"id": "copyEventLog", "cpp": "slEventLog->GetFileName() → EventLogTxt\\"},
        {"id": "copyMNetLog", "cpp": "slMNetLog->GetFileName() → MNetLog\\"},
        {"id": "copyElf", "cpp": "D:\\HT9045\\EXE\\HT9045.elf"},
        {"id": "copySetupInf", "cpp": "D:\\HT9045\\setup.inf"},
        {"id": "copyRecipe", "cpp": "SHFileOperation(DataPath+GetLastOpenFN() → HT9045\\IniData\\Data)"},
        {"id": "batch", "cpp": "TestList → D:\\HT9045\\system\\1.bat (robocopy/XCOPY/7z) → ExecZipCommand"},
        {"id": "motorXls", "cpp": "UpdateMotorScreen(true); SGDToXLS(StringGrid1 → Motor.xls)"},
        {"id": "taskXls", "cpp": "SGDToXLS(StringGrid2 → Task.xls, sgTaskList → Task_List.xls, AutoCleanStringGrid → AutoClean.xls)"},
        {"id": "saveTaskList", "cpp": "SaveTaskList(NewPath) → Task_ListWithTime*.csv"},
        {"id": "decisionVariables", "cpp": "SaveDecisionVariables(NewPath)"},
        {"id": "hotPlate", "cpp": "if(LastSet.iTemperature==Tempture_Hot) HP1/HP2_*.xls ×6"},
        {"id": "mainFormBmp", "cpp": "MainFormSizeToEpson(true); iSaveImageTask=1; iSaveImgae=iShowAlarm → MainForm.bmp"},
        {"id": "verTxt", "cpp": "fObserver->Memo1 → Ver.txt"},
        {"id": "indexPosition", "cpp": "RecordIndexPosition(0,3); LogIndexMaxMinPos(\"StateRecord\")"},
        {"id": "mainFormSnapshot", "cpp": "DumpMainFormSnapshot(NewPath)"},
        {"id": "zip", "cpp": "Timer: 7z.exe a -tzip NewPath.zip → Del_Tree(NewPath)；bManual → ShellExecute(open SDataPath)"}
    ],
    "request": {
        "seq": 0, "id": None, "source": None, "button": None, "action": "stateRecord",
        "manual": None, "showAlarm": 0, "savePath": None, "folderName": None, "newPath": None,
        "include": {}, "htmlContext": {}, "issuedAt": None, "state": "idle"
    }
}
dump("state-record.json", state_record)

# ---------- 2. ack ----------
dump("state-record-ack.json", {
    "schemaVersion": "1.0.0", "generatedAt": NOW, "source": SRC,
    "ack": {
        "seq": 0, "id": None, "state": "idle",          # idle | running | done | error
        "message": None, "path": None, "zipFile": None,
        "startedAt": None, "finishedAt": None,
        "currentStep": None,
        "steps": []                                       # [{id, state: done|skip|error, at, detail}]
    }
})

# ---------- 3. Task-runtime（sgTaskList / Task_ListWithTime.csv）----------
dump("Task-runtime.json", {
    "schemaVersion": "1.0.0", "generatedAt": NOW,
    "source": {"toolchain": "BCB6", "cpp": "main.cpp :: UpdateTaskList() / SaveTaskList(NewPath)",
               "files": ["Task_List.xls", "Task_ListWithTime.csv", "Task_ListWithTime2.csv"]},
    "note": "history 依 Task_ListWithTime.csv：TaskName, time, case, time, case…（新→舊）。"
            "mainProcMonitor 對應 MainProcMonitor 列（Alive/CallCount/LastEnter/SilentSec/SaveTime）。",
    "runtime": {"connected": False, "lastPollAt": None, "pollIntervalMs": 1000, "seq": 0},
    "mainProcMonitor": {"alive": None, "callCount": None, "lastEnter": None, "silentSec": None,
                        "saveTime": None, "thresholdSec": 5},
    "tasks": [
        # 範例骨架；C++ 端輸出時覆寫
        {"task": "AutoSHT1Task", "current": {"case": None, "at": None}, "history": []},
        {"task": "AutoSHT2Task", "current": {"case": None, "at": None}, "history": []},
        {"task": "iArmTask",     "current": {"case": None, "at": None}, "history": []},
        {"task": "OutArmTask",   "current": {"case": None, "at": None}, "history": []},
        {"task": "CatchTrayTask","current": {"case": None, "at": None}, "history": []}
    ]
})

# ---------- 4. System-runtime（Ver.txt / DumpMainFormSnapshot / DecisionVariables 摘要）----------
dump("System-runtime.json", {
    "schemaVersion": "1.0.0", "generatedAt": NOW,
    "source": {"toolchain": "BCB6",
               "cpp": "main.cpp :: DumpMainFormSnapshot(NewPath) / SaveDecisionVariables(NewPath) / fObserver->Memo1 (Ver.txt)"},
    "runtime": {"connected": False, "lastPollAt": None, "pollIntervalMs": 2000, "seq": 0},
    "version": {"software": None, "machineType": None, "customerCode": None, "model": None, "serialNo": None},
    "lot": {"recipe": None, "lotId": None, "lotStatus": None, "testType": None},
    "state": {"systemStart": None, "pause": None, "alarm": None, "alarmCode": None,
              "temperatureMode": None, "artStep": None},
    "flags": {},                 # DumpMainFormSnapshot 的旗標區（key → value）
    "decisionVariables": {},     # SaveDecisionVariables 的變數區
    "counters": {"uph": None, "input": None, "pass": None, "fail": None}
})
