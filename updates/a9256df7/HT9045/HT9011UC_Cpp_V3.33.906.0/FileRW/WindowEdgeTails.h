// ===========================================================================
//  FileRW/WindowEdgeTails.h -- 設定視窗「真的開／關」那一下要跑的 golden 程式（事件批次 B10 part a）。
//
//  //AI(W906-EVB10A) 20260929 [W906] St01 新檔（手寫，不是產生檔）。
//  派工：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B10、第四節 B10 表
//    （YM-3、SU-9、OS-6、OS-7、CT-3b、CC-E10；TS-10 與 SU-9 的 Exit 鈕走 form.event，不在本表）。
//  裁決：Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」；
//    20260929「照 BCB 的邏輯」。頁面表（Q51，WebPageTable.cpp）告訴 C++ 網頁視窗什麼時候真的開／關。
//
//  白話：BCB 的設定視窗關掉時一定跑 FormClose（按 Exit 或按視窗的 ✕ 都一樣：VCL Close() → OnClose）；網頁版以前
//    按 Exit／✕ 只是外框把視窗藏起來，C++ 什麼都沒跑。現在頁面表每 0.5 秒看一次網頁送來的「哪些視窗開著」，某個視窗
//    從開變關（或從關變開）就呼叫 W906_EvB10A_WindowEdge(golden 表單名, 開或關)（tools/wb_serve.cpp:4389 頁面表的
//    邊緣掛勾那一行；在 C 路清「開過了」之前先跑），這裡照下表找到那個表單的 golden 程式來跑。
//
//  表（kEvB10ARows）：
//    fYieldMonitoring  關 ⇒ golden btnOkClick（uYieldMonitoring.cpp:3190-3195：ReadFile、fTemperFrom->ShowYieldFuntion、Close→FormClose）
//    fSetup            關 ⇒ golden FormClose（cSetUp.cpp:3363）；Exit 鈕已經經 form.event 跑過 sbtExitClick（含 FormClose）就不再跑
//    fOffSet           關 ⇒ golden FormClose（cOffSet.cpp:835：Z 教導沒做完 ⇒ fAllMotorHome=false、iHome=1）
//                      （開 ⇒ OS-7 的 SECS EnterOffset 在 C 路開頁 FileRW_Offset_Page 裡送，跟 "Enter Offset" 同一次）
//    fContact          關 ⇒ golden FormClose（cContact.cpp:1842：SystemStart=false、收掉 Contact 模式）
//    fConfiguration    關 ⇒ golden sbExitClick 前三句（cConfiguration.cpp:6383-6389：ShowSpeed、PE 鈕、[C12] 關掉時自動關 PE）
//
//  規則：
//    * 「這一次開窗 golden FormShow 有跑過」才跑關窗程式（各結構自己查：C 路 PageDesc 頁用 filerw::PageShownNow，
//      Offset／Configuration 用各自的開頁旗標）。例：開窗閘拒絕（等級不足）⇒ golden 視窗根本沒開 ⇒ 關掉時不跑 FormClose。
//    * skipWhileRunning＝true 的列：運轉中（SystemStart||SoftStart）看到的關窗不跑、只印一行（同 R86：golden 運轉中打不開這幾個
//      模態視窗 —— V912 main.cpp:3970-3978 運轉中把 palSetup／palConfig 藏起來；Yield／Setup／Configuration 都是 ShowModal）。
//      Offset（非模態，golden sbOffsetClick 沒有 SystemStart 守衛）與 Contact（FormClose 自己就是「停機」SystemStart=false）照跑。
//    * 每一個邊緣都印一行 `[EVB10A] <表單> opened|closed -> ...`，golden 的訊息／待辦（filerw::ELMessage／ELTodo）另外逐行印出
//      （不在任何 WS 指令裡，沒有回覆可以帶；同 WebTeachLeave.h 的 W906_WindowEdgeRegister 說明）。
//
//  本檔只有宣告、表、純邏輯（tests/test_evb10a_edges.cpp 直接 include，不連任何機台碼）。
//
//  //AI(W906-EVB10C) 20260929 [W906] 事件批次 B10 part c（St01；ST01-E 20260929 派工與逐頁放行）：同一張表多 14 列（第 6～19 列），
//    其他 C 路設定頁按 Exit／✕ 關掉時也照 BCB 跑 golden FormClose（VCL Close → OnClose）。golden 一律 V912：
//    fSpeed           cSpeed.cpp:1272            fShow=false、ReadFile、DoIniDataToForm                          ShowModal main.cpp:28695
//    fLd_ULd          cLd_ULd.cpp:170            ReadFile、（rbTemp->SetFocus＝頁面焦點，產生檔已閘）、fShow=false       ShowModal :28453
//    fTrayForm        cTrayForm.cpp:559          ReadFile、DoIniDataToForm、fShow=false（結構 UserDefForm_File）       ShowModal :28409
//    fTrayAssignment  cTrayAssignment.cpp:1325   rgLoaderTrayMode->Enabled=true、ReadFile、DoIniDataToForm（結構 TrayForm）ShowModal :28433
//    fTemp_Set        uTemp_Set.cpp:4196         fShow=false、rgTemperatureMode->Enabled=true                    main ShowModal :28352，Contact 頁 Show（cContact.cpp:15256／:17477）⇒ 運轉中照跑
//    fDIOFrom         DIOInterFaceCFG.cpp:264    fShow=false、DoIniDataToForm（結構 TTLCfg）                      ShowModal :28669
//    fQAMode          QAMode.cpp:171             fShow=false、DoIniDataToForm                                    Show :29925
//    fBarCode         BarCode\BarCode.cpp:531    bShow=false、tmr1 停、DoIniDataToForm                            Show :29916
//    fVacuumUnit      VacuumUnit\VacuumUnit.cpp:304 ReadFile、fShow=false、DoIniDataToForm                         Show :35560
//    fBinSel          cBinSel.cpp:2137           bShow=false、ReadFile(false,false,"")、fShowBinSelect->InitShowBinDigital()  ShowModal :28324
//    HandlerSystem    HandlerSys.cpp:1215        tsCustomerCode->TabVisible=false（myLog.Do_Log 照 FormShow :174 閘掉，GATE H22-1）ShowModal cTemperFrom.cpp:1778
//    fCounterSel      cCounterSel.cpp:56         IniConfig.bShow*、ProcessLastSetIni_Visible(bWriteFile) 寫 config.ini [Visible]  ShowModal :28565
//    fStartCondition  cStartCondition.cpp:619    sbSaveClick（WriteLastDataFile）、LastSet.iStartMode、fLotInfo 站號…    ShowModal :28553
//    fCleaning        AutoClean\uCleaning.cpp:2106 SetWorkParameter()、rgAutoCleanOnOff->Enabled=false、fShow=false   ShowModal :29674
//    規則同上，另加一條「不跑第二次」：這一次開窗裡存檔流程／form.event 的 golden Close() 已經跑過 FormClose（"closed"，或存檔本身就是
//    FormClose 的 CounterSel）⇒ 關窗不再跑 —— 存檔後引擎一定重讀（PageShownNow 又變 true），所以另有記號 filerw::PageFormCloseRan
//    （FileRW/_EditPage.cpp；關窗邊緣 PageWindowClosed 清）。各結構入口用 filerw::PageCloseEdgeRefused(tag) 一次問兩件事。
//    不在表上（ST01-E 20260929 裁決，交件列理由）：fTeach（Jimmy 的 5(b) 關窗取消）、fShuttleMove（原生頁）、fGroundMan（RS232 重開）、
//    fContactForce（ADAM EP 輸出）、FTestIF（TestIF_File_TesterIF.* 不歸 St01）、fHotPlate（A 路，沒有「這一次開窗」可問）。
// ===========================================================================
#ifndef FileRW_WindowEdgeTailsH
#define FileRW_WindowEdgeTailsH

#include <cstddef>

// ---- 各結構入口（本體在各自的 FileRW/<結構>.cpp 檔尾；呼叫端 W906_EvB10A_WindowEdge 持 FormLock、已 SessionBegin）-------
//  回傳一行英文說明（跑了什麼／為什麼沒跑；各檔自己的靜態緩衝，下一次呼叫前有效）。
const char* FileRW_YieldMonitoring_WindowEdge(bool open);   // FileRW/TestIF_File_YieldMonitoring.cpp
const char* FileRW_Setup_WindowEdge(bool open);             // FileRW/TestIF_File_SetUp.cpp
const char* FileRW_Offset_WindowEdge(bool open);            // FileRW/Offset_File.cpp
const char* FileRW_Contact_WindowEdge(bool open);           // FileRW/DeviceForm_File.cpp
const char* FileRW_IniConfig_WindowEdge(bool open);         // FileRW/IniConfig.cpp
// CT-3b：golden TfContact::FormClose（cContact.cpp:1842）＋C 路收尾；關窗邊緣與 A02 存檔（golden spbSaveClick :14190 Close()）共用
const char* FileRW_Contact_EvB10AFormClose(const char* why);   // FileRW/DeviceForm_File.cpp
// AI(W906-EVB10C) 20260929 [W906]：B10c 的 14 個入口（本體在各結構檔尾；名稱照各檔的 FileRW_<名>_Boot）
const char* FileRW_Speed_WindowEdge(bool open);             // FileRW/ArmSpeed_File.cpp            fSpeed
const char* FileRW_LdUld_WindowEdge(bool open);             // FileRW/Ld_UldDelayTime.cpp          fLd_ULd
const char* FileRW_TrayForm_WindowEdge(bool open);          // FileRW/UserDefForm_File.cpp         fTrayForm
const char* FileRW_TrayAssignment_WindowEdge(bool open);    // FileRW/TrayForm.cpp                 fTrayAssignment
const char* FileRW_Temperature_WindowEdge(bool open);       // FileRW/Temperature.cpp              fTemp_Set
const char* FileRW_TTLCfg_WindowEdge(bool open);            // FileRW/TTLCfg.cpp                   fDIOFrom
const char* FileRW_QAMode_WindowEdge(bool open);            // FileRW/TestIF_File_QAMode.cpp       fQAMode
const char* FileRW_BarCode_WindowEdge(bool open);           // FileRW/TestIF_File_BarCode.cpp      fBarCode
const char* FileRW_VacuumUnit_WindowEdge(bool open);        // FileRW/TestIF_File_VacuumUnit.cpp   fVacuumUnit
const char* FileRW_BinSelect_WindowEdge(bool open);         // FileRW/BinSelect.cpp                fBinSel
const char* FileRW_HSys_WindowEdge(bool open);              // FileRW/HSys.cpp                     HandlerSystem
const char* FileRW_CounterSel_WindowEdge(bool open);        // FileRW/IniConfig_CounterSel.cpp     fCounterSel
const char* FileRW_StartCondition_WindowEdge(bool open);    // FileRW/StartCondition.cpp           fStartCondition
const char* FileRW_Cleaning_WindowEdge(bool open);          const char* FileRW_Observer_WindowEdge(bool open);   /*AI(W906-E021-OB1) 20261002 [W906] (St01): todo E-021 OB-1, row 20 fObserver -- body at the end of cObserver.cpp (ht9045_sm); same line*/   // FileRW/TestIF_File_Cleaning.cpp     fCleaning

// ---- 分派（本體 FileRW/MainClick.cpp 檔尾，只在 wb_serve；tools/wb_serve.cpp:4389 頁面表邊緣掛勾那一行呼叫）------------
void W906_EvB10A_WindowEdge(const char* goldenForm, bool open);

// ---- CC-E10 用的主畫面 PE 鈕（本體 FileRW/MainClick.cpp 檔尾；呼叫端 FileRW/IniConfig.cpp 檔尾）---------------------------------
void        W906_Main_SetPEModelVisible(bool bVisible);   // golden fMain->sbPEModel->Visible = …（cConfiguration.cpp:6386）
const char* W906_Main_PEModelClickFromConfig();           // golden fMain->sbPEModelClick(this)（cConfiguration.cpp:6388）

namespace evb10a {

struct Row {
    const char* form;              // golden 表單名（＝D:\HT9045\web\background.html WINDOWS 的 form:；頁面表 PageTableFind 的鍵）
    const char* golden;            // 跑的 golden 程式（V912）
    bool        onOpen;            // 開的那一下要不要呼叫
    bool        onClose;           // 關的那一下要不要呼叫
    bool        skipWhileRunning;  // 運轉中（SystemStart||SoftStart）看到的「關」不呼叫（見檔頭規則）
};

// 順序＝上面檔頭的表。改這張表要同步改 FileRW/MainClick.cpp W906_EvB10A_WindowEdge 的函式對照（ctest EvB10A_Edges 會比對個數）。
const Row kRows[] = {
    {"fYieldMonitoring", "uYieldMonitoring.cpp:3190-3195 TfYieldMonitoring::btnOkClick (+ FormClose :3288)", false, true,  true },
    {"fSetup",           "cSetUp.cpp:3363 TfSetup::FormClose",                                            false, true,  true },
    {"fOffSet",          "cOffSet.cpp:835 TfOffSet::FormClose",                                           false, true,  false},
    {"fContact",         "cContact.cpp:1842 TfContact::FormClose",                                        false, true,  false},
    {"fConfiguration",   "cConfiguration.cpp:6383-6389 TfConfiguration::sbExitClick (head, before Close)", false, true,  true },
    // ---- AI(W906-EVB10C) 20260929 [W906]：B10c（檔頭第二張表；skipWhileRunning＝golden ShowModal）----
    {"fSpeed",           "cSpeed.cpp:1272 TfSpeed::FormClose",                                            false, true,  true },
    {"fLd_ULd",          "cLd_ULd.cpp:170 TfLd_ULd::FormClose",                                           false, true,  true },
    {"fTrayForm",        "cTrayForm.cpp:559 TfTrayForm::FormClose",                                       false, true,  true },
    {"fTrayAssignment",  "cTrayAssignment.cpp:1325 TfTrayAssignment::FormClose",                          false, true,  true },
    {"fTemp_Set",        "uTemp_Set.cpp:4196 TfTemp_Set::FormClose",                                      false, true,  false},
    {"fDIOFrom",         "DIOInterFaceCFG.cpp:264 TfDIOFrom::FormClose",                                  false, true,  true },
    {"fQAMode",          "QAMode.cpp:171 TfQAMode::FormClose",                                            false, true,  false},
    {"fBarCode",         "BarCode\\BarCode.cpp:531 TfBarCode::FormClose",                                 false, true,  false},
    {"fVacuumUnit",      "VacuumUnit\\VacuumUnit.cpp:304 TfVacuumUnit::FormClose",                        false, true,  false},
    {"fBinSel",          "cBinSel.cpp:2137 TfBinSel::FormClose",                                          false, true,  true },
    {"HandlerSystem",    "HandlerSys.cpp:1215 THandlerSystem::FormClose",                                 false, true,  true },
    {"fCounterSel",      "cCounterSel.cpp:56 TfCounterSel::FormClose",                                    false, true,  true },
    {"fStartCondition",  "cStartCondition.cpp:619 TfStartCondition::FormClose",                           false, true,  true },
    {"fCleaning",        "AutoClean\\uCleaning.cpp:2106 TfCleaning::FormClose",                           false, true,  true },   {"fObserver",        "cObserver.cpp:654-674 TfObserver::FormClose (906 = V912 same lines; + SavePrecautionParameter 906 :4154-4183, V912 :4385-4414)", false, true,  false},   //AI(W906-E021-OB1) 20261002 [W906] (St01): todo E-021 OB-1 -- closing the Observer runs golden FormClose (906 = V912 same lines, AI(W906-E030-CITE) 20261003; St01 body W906_E021_FormClose at the end of cObserver.cpp) only when this open's FormShow ran (bShow); skipWhileRunning=false: golden opens it with sbMessageClick (906 0618 main.cpp:27927-27939, V912 :28959-28971, AI(W906-E032) 20261003; ShowModal, level 5) while running too -- the main toolbar, not palSetup / palConfig (main.cpp:3970-3978). Same line, no line moves
};
const std::size_t kRowCount = sizeof(kRows) / sizeof(kRows[0]);

inline int Find(const char* form)
{
    if (!form) return -1;
    for (std::size_t i = 0; i < kRowCount; ++i) {
        const char* a = kRows[i].form;
        const char* b = form;
        while (*a && *a == *b) { ++a; ++b; }
        if (*a == 0 && *b == 0) return (int)i;
    }
    return -1;
}

enum Verdict {
    kNotListed     = 0,   // 表上沒有這個表單（或這一邊不管）⇒ 什麼都不做、不印
    kRun           = 1,   // 呼叫那個結構的入口
    kSkipRunning   = 2    // 運轉中看到的「關」、這一列 skipWhileRunning ⇒ 不呼叫、印一行（之後也不補呼叫，同 S122 R82）
};

// 純邏輯：這一個邊緣要不要跑。running＝SystemStart||SoftStart。
inline Verdict Decide(const char* form, bool open, bool running)
{
    const int i = Find(form);
    if (i < 0) return kNotListed;
    const Row& r = kRows[i];
    if (open ? !r.onOpen : !r.onClose) return kNotListed;
    if (!open && r.skipWhileRunning && running) return kSkipRunning;
    return kRun;
}

// 「這一次開窗 golden FormShow 跑過」的門閂（Offset／Configuration 自己的入口用；C 路 PageDesc 頁用 filerw::PageShownNow）。
//   Shown()：開頁（golden FormShow）跑過；Closed()：golden FormClose 已經跑過（例：存檔時 golden 自己 Close()）；
//   TakeForClose()：關窗邊緣問「要不要跑 FormClose」，問完就清（下一次開窗重新算）。
struct OpenLatch {
    bool shown = false;
    void Shown()  { shown = true; }
    void Closed() { shown = false; }
    bool TakeForClose() { const bool s = shown; shown = false; return s; }
};

// ---- AI(W906-EVB10C) 20260929 [W906]：B2 守衛（ST01-E 20260929「guard B2」）—— Start Condition 開著時跑過生產 ----------------------
//   golden TfStartCondition 是 ShowModal（main.cpp:28553）：表單開著時點不到 START，「開窗 → START → 生產 → 停機 → 關窗／存檔」在 golden 走不到。
//   網頁非模態走得到，這時頁面／伺服器端的格子還是開窗那一刻的計數（sgContactCount → LastSet.iContactCT、Socket 頁 → LastSet.iSocketContactCount、
//   sgSocketCount → IniConfig.SocketContactCount…），golden 的關窗 FormClose（sbSaveClick）與各存檔鈕會把生產中加上去的計數拉回開窗值。
//   規則：這一次開窗（golden FormShow＝editlist.get 之後）只要看到 SystemStart||SoftStart 為真，就記「生產跑過」；
//     關窗邊緣不跑 golden FormClose、這一頁的每一顆存檔鈕都拒存（要頁面重新讀＝重新 FormShow 才清）。結果＝golden 的結果（golden 根本不會有這個狀態）。
//   本體：FileRW/StartCondition.cpp 檔尾（取樣 FileRW_StartCondition_EvB10CSample，由 FileRW/MainRecord.cpp W906_MainRecordTimer1Tick 每拍呼叫）。
struct RanDuringOpen {
    bool ran = false;
    void FormShown() { ran = false; }                                    // golden FormShow（頁面開窗／重讀）⇒ 格子是新的
    // 每拍取樣：running＝SystemStart||SoftStart，shown＝這一頁現在開著（filerw::PageShownNow）。回 true＝這一拍剛記上（印一行用）
    bool Sample(bool running, bool shown) { if (running && shown && !ran) { ran = true; return true; } return false; }
    bool Ran() const { return ran; }
};
// 關窗邊緣（forSave=false）／存檔（forSave=true）要不要擋；回 nullptr＝不擋，否則回理由（關窗：主控台一行；存檔：英文，中文另給）。
inline const char* StartConditionProductionGuard(const RanDuringOpen& g, bool forSave)
{
    if (!g.Ran()) return nullptr;
    return forSave
        ? "Production ran while this page was open; the counters have changed. Close and reopen this page before saving (nothing was saved)."
        : "[EVB10C] fStartCondition closed after production ran during this open -- golden FormClose (save) not run, to avoid rolling back LastSet.iContactCT";
}
const char* const kStartConditionProductionGuardZh = "生產中開著這一頁，計數已經變了；請關掉重開這一頁再存";

}  // namespace evb10a

#endif  // FileRW_WindowEdgeTailsH
