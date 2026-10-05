// ===========================================================================
//  FileRW/MainClickTail.h -- golden TfMain 主畫面設定鈕 sbXxxClick 在「設定視窗 ShowModal 回來（視窗關掉）之後」的尾段。
//
//  AI(W906-FRW-S158) 20260927 [W906]: 新檔。NOT in golden（golden 的尾段直接寫在 TfMain::sbXxxClick 裡）。
//    Q41 盤點第一節第 3 項「關窗尾段」；方案 docs/Q41_CLOSETAIL_PLAN_20260927.md（§3 逐頁表、§4 掛法）。
//    本體在 FileRW/MainClick.cpp 檔尾（跟 SetUp 頁的 W906_Main_sbSetupClickTail 同一處，S100-a）。只給 wb_serve 的 FileRW 入口 cpp 用。
//    裁決：RULINGS_20260926 S107-1（「存檔後就跑，不改成等 Exit」，docs/RULINGS_20260926.md:284）；decisions-pending
//    R84（Contact 照 golden 在開頁跑）、R85（其餘頁照 S107-1）、R86（每支開頭再查 SystemStart||SoftStart）—— Steven 沒反對就照 St01 建議 A。
//
//  為什麼另開一個標頭：呼叫端都在匿名 namespace 裡，區塊內的前置宣告會變成另一支沒有本體的函式；呼叫端的 gen.inc 又有一大堆
//    #define（ReadFile、MyBinPanel…），所以本檔只放宣告、不 include 任何標頭。
//
//  回傳：nullptr＝跑了；非 nullptr＝沒跑的原因（呼叫端立刻交給 filerw::ELTodo，進 ack.session.todo）。目前唯一的原因是運轉中（R86）。
//
//  第一批（c8028b21；第二批 TrayForm、HotPlate、TrayAssignment、Yield、Configuration 見檔尾 —— AI(W906-FRW-S158) 20260927 同一行改寫）：
//    函式                          golden V912 main.cpp                          呼叫端
//    W906_Main_sbLdUldClickTail    sbLdUldClick   :28448-28455，尾段 :28454       FileRW/Ld_UldDelayTime.cpp SaveFlow
//    W906_Main_sbSpeedClickTail    sbSpeedClick   :28677-28700，尾段 :28696-28699 FileRW/ArmSpeed_File.cpp SaveFlow
//    W906_Main_sbBinClickTail      sbBinClick     :28319-28328，尾段 :28325-28327 FileRW/BinSelect.cpp BinCloseTail
//    W906_Main_sbContactClickOpen  sbContactClick :28302-28317，:28315-28316（:28314 Show() 之後，開窗當下）
//                                                                                 FileRW/DeviceForm_File.cpp FormShowAndSnap
// ===========================================================================
#pragma once

const char* W906_Main_sbLdUldClickTail();     // golden V912 main.cpp:28454
const char* W906_Main_sbSpeedClickTail();     // golden V912 main.cpp:28696-28699
const char* W906_Main_sbBinClickTail();       // golden V912 main.cpp:28325-28327
const char* W906_Main_sbContactClickOpen();   // golden V912 main.cpp:28315-28316（開窗當下，R84）

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: 第二批（方案 §5.3、§5.6-§5.9；本體 FileRW/MainClick.cpp 檔尾「第二批」那一段）：
//    函式                                golden V912 main.cpp                                       呼叫端
//    W906_Main_sbTrayFormClickTail       sbTrayFormClick      :28404-28414，尾段 :28411-28413       FileRW/UserDefForm_File.cpp SaveFlow
//    W906_Main_sbPlateFormClickTail      sbPlateFormClick     :28416-28426，尾段 :28422-28425       FileRW/HotPlateForm_File.cpp SaveFlow
//                                                                                                    （產生檔；tools/formbridge/TfHotPlate.py 的 saveFlowAfter）
//    W906_Main_sbTrayAssignClickTail     sbTrayAssignClick    :28428-28438，尾段 :28434-28437       FileRW/TrayForm.cpp SaveFlow（Tray Assignment 頁）
//    W906_Main_sbYieldClickTail          sbYieldClick         :28500-28513，尾段 :28510-28512       FileRW/TestIF_File_YieldMonitoring.cpp SaveFlow
//    W906_Main_sbConfigurationClickHead  sbConfigurationClick :28615-28622（開窗前記 6 個舊值）     FileRW/IniConfig.cpp IniConfigPageJson（IC_FormShow 之前）
//    W906_Main_sbConfigurationClickTail  sbConfigurationClick :28626-28657（關窗後，不看存沒存）    FileRW/IniConfig.cpp FileRW_IniConfig_Save（IC_FormClose 之後）
//  HP-3 的呼叫端在產生檔的 namespace ht9045::formbridge 裡 —— 所以一定要從這個全域標頭宣告，呼叫時寫 ::W906_Main_sbPlateFormClickTail()，
//    沒跑的原因交給 J.Todo（A 形狀的 ack.todo），不是 filerw::ELTodo。
// ---------------------------------------------------------------------------
const char* W906_Main_sbTrayFormClickTail();          // golden V912 main.cpp:28411-28413
const char* W906_Main_sbPlateFormClickTail();         // golden V912 main.cpp:28422-28425
const char* W906_Main_sbTrayAssignClickTail();        // golden V912 main.cpp:28434-28437
const char* W906_Main_sbYieldClickTail();             // golden V912 main.cpp:28510-28512
void        W906_Main_sbConfigurationClickHead();     // golden V912 main.cpp:28615-28622（開窗前）
const char* W906_Main_sbConfigurationClickTail();     // golden V912 main.cpp:28626-28657（關窗後）

// ---------------------------------------------------------------------------
//  AI(W906-EVB6) 20260928 [W906]: 批次 B6／B9（本體 FileRW/MainClick.cpp 檔尾「批次 B6＋B9」那一段）。
//    R107：HP-3 尾段裡 golden 的 ShowMyMessage（ELMessage）與 ELTodo 收下來，由 HotPlate 的 SaveFlow（產生檔 FileRW/HotPlateForm_File.cpp，
//      設定 tools/formbridge/TfHotPlate.py saveFlowAfter）在 ::W906_Main_sbPlateFormClickTail() 之後取走、放進 J.Message／J.Todo。
//      回 true＝取走一筆；*en／*zh／*todo 指到的字串在下一次呼叫同一支之前有效（本檔不帶型別、不 include，理由見檔頭）。
//    S169（B9）給 Jimmy 的取用點：主畫面控制鈕／Site 格「按了」的事件（C++ 只登記，golden 本體由 Jimmy 在 tick 裡取走後跑）。
//      一下一筆；按下後 3 秒（kEvTtlMs）沒被取走就丟掉 —— golden 按下當場就跑，不會晚跑（接上取用端時不會有舊的一下突然執行）。
//      button：reset／oneCycle／trayFeed／alarmReset（HOME＝WS main.home、CLEAN OUT＝act.main.cleanOut，兩顆已照 golden 接上，不走這裡）。
//      Site 格座標是網頁 8×4（x＝col，arm＝y/2＋1，row＝y%2；tag site.arm{a}.s{n} 的契約），不是 golden mtDutOnOff 的 X／Y。
// ---------------------------------------------------------------------------
bool W906_Main_Hp3TailTakeMessage(const char** en, const char** zh);   // R107
bool W906_Main_Hp3TailTakeTodo(const char** todo);                     // R107
bool W906_Main_TakeCtlButtonEvent(const char* button);                 // golden V912 main.cpp:7655／:4469／:14467／:22867（S169）
bool W906_Main_TakeSiteClickEvent(int* x, int* y);                     // golden V912 main.cpp:29932 mtDutOnOffMouseUp（S169）
