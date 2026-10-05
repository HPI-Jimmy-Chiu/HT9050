// ===========================================================================
//  JsonBridge/actions/MainStateRecord.cpp
//
//  AI(W906-STATEREC) 20260924.  act.main.stateRecord —— 守衛、呼叫、回報。說明在 .h 檔頭。
//  golden 的按鈕處理器：main.cpp:26209-26212（sbStateRecordClick -> DoStateRecord(0, true)）。
//
//  ⚠ 本檔**只**碰 forms 層的符號（fMain、W906_StateRecordBody、W906_StateRecordWorkerBusy），
//    不碰 cStateRecord.cpp —— 所以編進 test_sjson_chan 也不會把背景複製／7z 那一整包連進測試。
// ===========================================================================
#include "JsonBridge/actions/MainStateRecord.h"

#include <string>

#include "JsonBridge/EventLog.h"   // LogAppend（SKILL §4.7 規則 1：每個 act.* 在本體前記一筆）
#include "WebBridge/JsonWriter.h"
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"           // SOFT_SIMULTE
#include "forms/fMain.h"           // fMain、W906_StateRecordBody、W906_StateRecordWorkerBusy

namespace ht9045 {
namespace sjson {

namespace {

const char* const kGolden     = "main.cpp:26209-26212 sbStateRecordClick -> DoStateRecord(0, true)";
const char* const kGoldenBody = "main.cpp:26340-26678 DoStateRecord";

void WriteSyncFiles(webbridge::JsonWriter& w)
{
    // cStateRecord.cpp 同步段（tick 執行緒，呼叫當下）寫進 NewPath 的檔 —— 都是變數快照。
    w.Key("syncFiles").BeginArray();
    w.String("Task_ListWithTime.csv");       // SaveTaskList（golden main.cpp:6742）
    w.String("Task_ListWithTime2.csv");      // SaveTaskList（golden main.cpp:6748）
    w.String("DecisionVariables.csv");       // SaveDecisionVariables（golden main.cpp:6951）
    w.String("MainFormSnapshot.txt");        // DumpMainFormSnapshot（golden main.cpp:26222）
    w.String("Ver.txt");                     // fObserver->Memo1（golden main.cpp:26672）
    w.EndArray();
}

void WriteBackgroundFiles(webbridge::JsonWriter& w)
{
    // 背景執行緒依 golden 順序複製進 NewPath 的東西（條件照 golden）。
    w.Key("backgroundCopies").BeginArray();
    w.String("EventLogTxt\\ (作用中檔案 + robocopy D:\\HT9045_Log\\EventLogTxt 近 2 天)");
    w.String("HT9045.elf (D:\\HT9045\\EXE\\HT9045.elf，存在才複製)");
    w.String("HT9045\\setup.inf");
    w.String("HT9045\\IniData\\Data\\<目前工單> (SHFileOperation)");
    w.String("HT9045\\system\\ (XCOPY D:\\HT9045\\system)");
    w.String("HT9045\\config\\ (XCOPY D:\\HT9045\\config)");
    w.String("GPIB9045\\system\\ (XCOPY D:\\GPIB9045\\system)");
    w.String("GPIBLOG\\ | TCP_LOG\\ | TTL_LOG\\+RS232_LOG\\ (依 TestIF_File.iTestType，近 2 天)");
    w.String("Galil_LOG\\ (asGalilCmdPath 近 2 天)");
    w.String("MNetLog\\ (robocopy D:\\HT9045_Log\\MNetLog 近 2 天)");
    w.String("Automation\\ (CosFunction.bOLPFunction && IniConfig.bN08_1SaveOLPLog)");
    w.String("CleanPad_Log\\ (bRunAutoClean && TestIF_File.iAutoClean_Function)");
    w.String("SECS_GEM_LOGS\\ + ATKDataTxt\\ (fAGV->IsATK_AMR())");
    w.EndArray();
    w.Key("thenZip").String("d:\\HT9045\\7z.exe a -tzip <folder>.zip <folder>；成功就 Del_Tree(<folder>)"
                            "（golden main.cpp:26194-26201）⇒ 完成後只剩 .zip");
}

void WriteOutsideFolder(webbridge::JsonWriter& w)
{
    // golden 本來就會寫、而且不在 NewPath 底下的檔。列出來是因為 D:\HT9045 這台是真機台。
    w.Key("writesOutsideFolder").BeginArray();
    w.String("D:\\HT9045\\system\\machinerecord.dat -- SaveMachineRecord()（同步；golden main.cpp:26347 -> cinitial.cpp WriteData）");
    w.String("D:\\HT9045\\system\\PickHPRec.json -- SaveMachineRecord() 尾段 PickFromHPList->SaveFile(sHPPickRec)（同步）");
    w.String("D:\\UnloaderInfo\\... -- SaveMachineRecord() 裡每個有用的 tray 呼叫 SaveUnloaderInfo（同步）");
    w.String("D:\\HT9045\\system\\1.bat -- 複製批次檔（背景；golden main.cpp:26615，Steven 20210308 把位置改到 system）");
    w.String("d:\\HT9045\\7z.exe -- 只有不存在時才從 C:\\Program Files\\7-Zip 複製（背景；golden main.cpp:26393-26396）");
    w.String("D:\\HT9045_Log\\... -- LogIndexMaxMinPos／RecordIndexPosition／SaveRecordCleanPad／OLP SaveRecord 的既有 log");
    w.EndArray();
}

//AI(W906-FLOWDIAG) 20260927: payload 有沒有 "taskListOnly": true（只認這個寫法；冒號前後容許空白）。
bool WantsTaskListOnly(const std::string& p)
{
    const std::string k = "\"taskListOnly\"";
    std::string::size_type i = p.find(k);
    if (i == std::string::npos) return false;
    i = p.find_first_not_of(" \t\r\n", i + k.size());
    if (i == std::string::npos || p[i] != ':') return false;
    i = p.find_first_not_of(" \t\r\n", i + 1);
    return i != std::string::npos && p.compare(i, 4, "true") == 0;
}

}  // namespace

bool StateRecordBodyInstalled()
{
    return W906_StateRecordBody != 0;
}

std::string DoStateRecordAction(const std::string& payloadJson)
{
    (void)payloadJson;                          // golden 的按鈕沒有參數
    webbridge::JsonWriter w;

    //AI(W906-FLOWDIAG) 20260927: {"taskListOnly":true} —— 動作流程對照（INBOX 第 49 列）專用，不是 golden 的按鈕；
    //  網頁的 State Record 鈕不帶它（行為不變）。只寫 Task_ListWithTime*.csv＋DecisionVariables.csv（cStateRecord.cpp 檔尾），
    //  模擬組態也寫（golden 的錄製在模擬下整段跳過，比對工具才拿不到）。不受「背景複製還在跑」守衛影響：它不碰背景那一段。
    if (WantsTaskListOnly(payloadJson)) {
        w.BeginObject();
        if (fMain == 0 || W906_TaskListOnlyBody == 0) {
            w.Key("executed").Bool(false);
            w.Key("guard").String("tasklist-body-not-installed");
            w.Key("detail").String("W906_InstallStateRecordBody() 沒被呼叫（cStateRecord.cpp 檔尾）-- 這個執行檔拿不到 task 紀錄");
        } else {
            LogAppend(kLogProcess, "act.main.stateRecord taskListOnly", "", "", "act");
            const AnsiString dir = W906_TaskListOnlyBody(fMain);
            w.Key("executed").Bool(true);
            w.Key("guard").String("");
            w.Key("taskListOnly").Bool(true);
            w.Key("folder").String(std::string(dir.c_str()));
            w.Key("files").BeginArray();
            w.String("Task_ListWithTime.csv");
            w.String("Task_ListWithTime2.csv");
            w.String("DecisionVariables.csv");
            w.EndArray();
            w.Key("note").String("不是 golden 的功能：動作流程對照用（INBOX 第 49 列）；只寫這三個檔，不錄製、不動 NewPath");
        }
        w.EndObject();
        return w.Str();
    }

    // --- 守衛 1：本體有沒有裝上（移植樹自己的事實，不是 golden） ---------
    if (fMain == 0 || !StateRecordBodyInstalled()) {
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("guard").String("body-not-installed");
        w.Key("detail").String("W906_InstallStateRecordBody() 沒被呼叫 -- "
                               "TfMain::DoStateRecord 仍是 no-op（forms/fMain.cpp:400）");
        w.EndObject();
        return w.Str();
    }

    // --- 守衛 2：上一筆的背景複製／壓縮還在跑（移植樹自己的，理由見 .h） --
    if (W906_StateRecordWorkerBusy.load()) {
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("guard").String("busy");
        w.Key("detail").String("上一筆 State Record 的背景複製／7z 還沒結束；結束後再按");
        w.Key("folder").String(std::string(fMain->NewPath.c_str()));
        w.EndObject();
        return w.Str();
    }

    // --- 留痕（SKILL §4.7 規則 1）：golden 的本體第一行自己會 RecordProcess("State Record.")，
    //     這一筆記的是「是網頁按的」。
    LogAppend(kLogProcess, "act.main.stateRecord pressed", "", "", "act");

    fMain->sbStateRecordClick(0);               // golden main.cpp:26209-26212

    const std::string folder(fMain->NewPath.c_str());
    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("guard").String("");
    w.Key("golden").String(kGolden);
#ifdef SOFT_SIMULTE
    // golden 把整段錄製包在 #ifdef SOFT_SIMULTE #else ... #endif（main.cpp:26343-26674），照翻。
    w.Key("softSimulte").Bool(true);
    w.Key("recorded").Bool(false);
    w.Key("note").String(std::string("SOFT_SIMULTE 組態：golden ") + kGoldenBody +
                         " 在模擬下不錄製（:26343-26674 整段跳過），只跑 RecordProcess／"
                         "LogIndexMaxMinPos／DumpMainFormSnapshot(NewPath)。NewPath 在此組態從未被設過，"
                         "所以 MainFormSnapshot.txt 落在目前磁碟機根目錄（golden 同形）。"
                         "要真的錄製請用出貨組態建置（-DW906_NO_SOFT_SIMULTE=ON）。");
    w.Key("folder").String(folder);
    w.Key("snapshot").String(folder + "\\MainFormSnapshot.txt");
#else
    w.Key("softSimulte").Bool(false);
    w.Key("recorded").Bool(true);
    w.Key("folder").String(folder);
    w.Key("zip").String(folder + ".zip");
    w.Key("backgroundRunning").Bool(W906_StateRecordWorkerBusy.load());
    WriteSyncFiles(w);
    WriteBackgroundFiles(w);
    WriteOutsideFolder(w);
    w.Key("sideEffectsSkipped").BeginArray();
    w.String("螢幕截圖 MainForm/MotorView/MotionView/GPIB.bmp -- wb_serve 沒有 VCL 視窗（cStateRecord.cpp GATE G2）");
    w.String("另存對話框 -- 沒有 SaveDialog1，等於按取消、走預設資料夾（GATE G4）");
    w.String("Motor.xls／Task.xls／Task_List.xls／AutoClean.xls／HP*.xls -- SGDToXLS 在移植樹是 no-op（SgdToXLS.cpp:88-94）");
    w.String("作用中 MNetLog 檔的立即複製 -- slMNetLog 不存在（GATE G5）；robocopy 近 2 天那條照做");
    w.EndArray();
#endif
    w.EndObject();
    return w.Str();
}

void WriteStateRecordActionSchema(webbridge::JsonWriter& w)
{
    w.BeginObject();
    w.Key("cmd").String("act.main.stateRecord");
    w.Key("golden").String(kGolden);
    w.Key("goldenBody").String(kGoldenBody);
    w.Key("portBody").String("cStateRecord.cpp（安裝座 forms/fMain.h 檔尾 W906_StateRecordBody）");
    w.Key("installed").Bool(StateRecordBodyInstalled());
    w.Key("args").BeginObject();
    w.Key("taskListOnly").String("bool，選填（AI(W906-FLOWDIAG) 20260927，不是 golden）：true = 只寫 Task_ListWithTime*.csv＋DecisionVariables.csv 到 "
                                 "D:\\HT9045_StateRecord\\<時間>_tasklist\\，給動作流程對照用；模擬組態也寫");
    w.EndObject();
    w.Key("guards").BeginArray();
    w.String("body-not-installed -- 移植樹自己的狀態，不是 golden 的守衛");
    w.String("busy -- 上一筆的背景複製／7z 還在跑（移植樹自己的，golden 不在背景做事）");
    w.EndArray();
    w.Key("output").String("D:\\HT9045_StateRecord\\YYYY-MM-DD HH_MM_SS\\ -> 同名 .zip（golden main.cpp:26354／:26379／:26197）");
#ifdef SOFT_SIMULTE
    w.Key("softSimulte").Bool(true);
#else
    w.Key("softSimulte").Bool(false);
#endif
    w.EndObject();
}

}  // namespace sjson
}  // namespace ht9045
