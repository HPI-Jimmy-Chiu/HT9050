//Steven 20260916
// ----------------------------------------------------------------------
// 新增二進位定長陣列的 struct 投影：GET /api/system/levelset 與 WS 指令
// system.levels.put。Status.Security 的 179 組 radio 存的是
// system\levelset.dat（LAST_LEVEL_SET，256 個小端 int32），
// 不是 config\Security_new.def——後者是 cAuthority.cpp 讀的表單 Enable/Disable 旗標，
// 與那 179 組沒有對應關係。原本的 SysFileTable 只有 ini 與 csv 兩種形狀，
// 且註解明文把二進位排除（「要等 C++ 端依 struct 逐欄位輸出投影」），
// 這次補的就是那個投影。細部見 AI(W906-FW-LEVELSET)。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

//Steven 20260916
// ----------------------------------------------------------------------
// recipe.doc.put 的 ack 補上 {changed, identical, notFound}（原本只傳 perr，
// 計數只 printf 到主控台）。瀏覽器需要 notFound 才能執行「對照表有錯就整頁拒寫」，
// 需要 changed 才能把即將寫入的內容給操作員確認。
// 搭配 WebBridge/WebBridgeServer.cpp 的 AckJson 修正一起看，那邊才是根因。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

//Steven 20260915
// ----------------------------------------------------------------------
// 新增 /api/system/（40 支機台設定檔，可讀寫）、/api/text/（6 個純文字記錄來源，唯讀）、
// WS 指令 system.file.put、--allow-system-write 旗標、ApiRoute() 單一路由分流器。
// 844 -> 1668 行。細部標記見各處 AI(W906-FW-SYSFILE / -TEXT / -DIO)。
// 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
// ----------------------------------------------------------------------

// =============================================================================
//  tools/wb_serve.cpp -- run the web HMI against the REAL handler data layer.
//
//  AI(W906-WebBridge) 20260806.
//
//  This is the WB-2 milestone in runnable form: load the machine config, stand
//  up the bridge, serve D:\HT9045\web, and publish real tag values to whatever
//  browser connects.
//
//      wb_serve.exe                 -> http://127.0.0.1:8045/?src=ws
//      wb_serve.exe 9000            -> different port
//      wb_serve.exe --root <dir>    -> serve a different web root
//      wb_serve.exe --seconds 20    -> exit after N seconds (for scripted runs)
//
//  WHAT IT IS NOT
//  Not the product. The handler proper will own this server on its own UI
//  thread once GA-3 lands the god-stack in HT9045.exe (see LoadMachineConfig in
//  database.h). This exe exists so the whole path can be exercised, and looked
//  at, before that.
//
//  SAFETY POSTURE -- inherited, not re-decided
//    * loopback only, and read-only: WebBridgeConfig defaults both that way
//      because this endpoint can eventually command machine motion
//      (web/docs/ARCHITECTURE.md section 6, questions 2 and 3). This file does
//      not widen either.
//    * LoadMachineConfig() SEEDS missing keys, i.e. it WRITES to asGeneralPath.
//      That is the real system\Gerneral.ini. Running this on a machine is
//      therefore the same class of act as starting the handler -- which is the
//      point, but it is stated here rather than discovered. --dry copies the
//      config to a scratch file first and leaves the real one alone.
//
//  WHAT THE BROWSER WILL SHOW
//  Mostly "---". That is correct, not broken: only ~8 of the tags have a source
//  that the port actually loads today. WebBridgeTags.h carries the measured
//  live/dead inventory and the reason for each.
// =============================================================================
#include "WebBridgeTags.h"
#include <deque>                       // AI(W906-IOWEB-P25) 20260925: g_carry (output-first service). On the old blank line, so no line below moves
#include "WebBridge/WebBridgeServer.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/CommandQueue.h"   // AI(W906-FW-W1) 20260819: cmd channel e2e (--allow-cmd)
#include "wb_dialog_mailbox.h"        // AI(W906-Q30-8) 20260922: 警報對話框的檔案信箱
#include "WebAuth.h"
#include "WebLogin.h"                 //Steven 20260924：主畫面登入（golden cbUserSelectChange／stOperatorClick／btLoginClick）                  // AI(W906-FW-W2) 20260819: auth.login verification core
#include "WebBridgeRecipeDoc.h"        // AI(W906-FW-C1WIRE) 20260911: recipe read/write
#include "Public/cJSON.h"              // AI(W906-FW-C1WIRE) 20260911: parse the PUT payload
#include <ctime>                      // AI(W906-FW-SYSFILE) 20260915: backup timestamp
#include "WebBridge/JsonWriter.h"      // AI(W906-FW-C1WIRE) 20260911: the index response
#include "cmydef.h"                   // AccessLevel, pwPath (golden globals the auth commands drive)
// AI(W906-JSONBRIDGE-S0) 20260923: 開機配置廣播。瀏覽器在畫任何依機型而異的
//   元件之前，必須先知道這顆 exe 是什麼組態建出來的（SOFT_SIMULTE 開不開、
//   裝了哪些硬體選配、容量上限多少）。規格見
//   .claude/skills/ht9045-json-bridge/SKILL.md 4.6。
#include "JsonBridge/MachineDefines.h"
#include "JsonBridge/ChanConfig.h"
// AI(W906-JSONBRIDGE-S1) 20260923: HTML 操作的留痕。每一筆同時進 ring 與
//   golden 的 RecordProcess 族；ring 存在是因為那三個入口在移植樹裡分別
//   落到 stdout／空 body／TU-local no-op 三種不同的地方，瀏覽器看不到自己
//   剛做了什麼。規格 SKILL.md 4.7。
#include "JsonBridge/EventLog.h"
// AI(W906-JSONBRIDGE-S2/S3) 20260923: 依欄位表讀結構實例。
#include "JsonBridge/FieldDesc.h"
// AI(W906-JSONBRIDGE-S6) 20260923: JSON -> 結構的驗證（dryRun）。
#include "JsonBridge/StructApply.h"
#include "JsonBridge/StageThermo.h"          // AI(W906-JSONBRIDGE-S7)
// ===== AI(W906-SJSON-S8S9S10S11) 20260923 BEGIN -- 四條新通道的 include =====
//   實質邏輯**全部**在 JsonBridge/ 底下的新檔裡；本檔只做分派（派工單第 2 條）。
#include "JsonBridge/ChanProduction.h"       // S8  prod.*
#include "JsonBridge/ChanIo.h"               // S9  io.di / io.do（位元打包）
#include "JsonBridge/ChanMotor.h"            // S9  motor.axes
#include "JsonBridge/AlarmChannel.h"         // S10 alarm.* 事件通道
#include "JsonBridge/ChanAction.h"           // S11 act.* 分派
#include "JsonBridge/actions/MainClarnData.h"// S11 Clarn_Data 本體的安裝
// ===== AI(W906-SJSON-S8S9S10S11) 20260923 END =====
// AI(W906-FW-W4) 20260819: first real command family -- counter.clear drives
// the translated TfCounterClear::ClearCount core (test_counterclear_core
// covers it) behind golden's own Security_new.def per-item authorization.
#include "forms/fCounterClear.h"      // fCounterClear global + ClearCount (MachineType.h: eClearType)
#include "forms/fTeach.h"      // AI(W906-TEACH-W1) 20260919: TfTeach / fTeach
#include "cprod.h"                    // AI(W906-LOT-W1) 20260919: RunInfo.bLotStart
#include "forms/fLotInfo.h"           // AI(W906-LOT-W1) 20260919: fLotInfo / SetLotStart
#include "forms/fMain.h"              // fMain->Clarn_Data (golden spbExeClick bracket)
#include "cAuthority.h"               // GetCountClrAuth(), authCounterClr[]
#include "cMyDB.h"                    // MyDBIProcess (recording sim in this tree)
#include "LastSet.h"                  // LastSet (post-clear observable printed to the serve log)
#include "forms/fBinSel.h"            // AI(W906-FW-BIN1) 20260820: fBinSel->ReadFile (BinSelect loader)
#include "forms/fShowBinSelect.h"     // AI(W906-FW-BIN1) 20260820: fShowBinSelect->ShowBinSel (MyBinSel captions)
#include "cinitial.h"
#include "ContactForceLoad.h"   // AI(W906-P2a-CF) 20260919
#include "ContactForce.h"       // ContactForceTables() -- 只為了印筆數                 // AI(W906-FW-BIN1) 20260820: SetTechDataToProd_Yield
#include "Motor/mymotor.h"            // AI(W906-TEACH-W1) 20260919: MOT[] / .Alias
#include "forms/fTemp_Set.h"          // AI(W906-FW-TEMP2) 20260820: fTemp_Set->ReadTempFile (temp.sv/soak/mode loader)
#include "forms/fSetup.h"             // AI(W906-SETUP-READFILE) 20260922: fSetup->Init/ReadFile (HandlerCondition.Data [Configuration])
#include "forms/fTrayForm.h"          // AI(W906-TRAY-READ) 20260923: fTrayForm->Init/ReadFile (Tray.Data -> UserDefForm_File)
int FileRW_IniConfig_Json(const std::string& name, std::string* json);
int FileRW_IniConfig_Save(const std::string& widgetsJson, const std::string& answersJson, std::string* ack, std::string* err);
#include "FileRW/_EditPage.h"   //Steven 20260924 (S12-C)：第二個起的 C 路結構（Ld_UldDelayTime…）依 tag 註冊
int FileRW_Offset_Page(std::string* json);  int FileRW_Offset_Save(const std::string& widgetsJson, const std::string& answersJson, std::string* ack, std::string* err);   /*Steven 20260925 (S12-C Offset_File)：FileRW/Offset_File.cpp，選取式編輯器，widgets＝{offsets,common}*/
int FileRW_BinSelect_Page(std::string* json);  int FileRW_BinSelect_Save(const std::string& valueJson, std::string* ack, std::string* err);   /*Steven 20260925 (S12-C BinSelect)：FileRW/BinSelect.cpp，save 收整包 value*/
int FileRW_IniConfig_Page(std::string* json);  int FileRW_Teach_Page(std::string* json);  int FileRW_Teach_Save(const std::string& widgetsJson, const std::string& answersJson, std::string* ack, std::string* err);   /*AI(W906-W5-TEACH) 20260925: FileRW/Teach.cpp*/   //Steven 20260924 (S12-C，審查 C-A)：WS editlist.get   //Steven 20260924 (S12-C 寫方向)   //Steven 20260924 (S12-C): FileRW/IniConfig.cpp（宣告在檔案層級；放進 anonymous namespace 裡的區域 extern 會綁錯命名空間）
// AI(W906-BU-C4) 20260918: the PCIE-1203 write surface. MachineType.h is named
//   explicitly rather than relied on transitively (cmydef.h pulls it today, and
//   that is exactly the kind of dependency that disappears in a refactor and
//   turns an armed feature silently OFF -- which is the worst direction for a
//   flag whose other setting moves a servo).
#include "MachineType.h"
#ifdef WB_PUMP_1203_CONTROL
#include "EtherCAT/Pci1203Control.h"
#endif
//AI(W906-Q34-1) 20260923: 唯讀監看面。`EtherCAT/Pci1203Monitor.cpp` 早就是本
//  target 的來源檔（CMakeLists.txt wb_serve 區塊），缺的只是呼叫點 —— 與
//  `WebStart.h` 當初的情況同型：編了、連了、零呼叫者。
#ifdef INSTALL_1203_MONITOR
#include "EtherCAT/Pci1203Monitor.h"
#endif
//AI(W906-ST-S3-B4) 20260918: TfMainWeb + StartFromWeb. WebStart.cpp is already
//  one of this target's sources (CMakeLists.txt wb_serve block), it just had no
//  header user -- the class was built and linked with zero callers.
#include "WebStart.h"
#include "WebOlp.h"                 // AI(W906-P3-OLP) 20260919: OLP 設定指令白名單
#include "WebWindowRegistry.h"      // AI(W906-P6-WINREG) 20260920: 視窗狀態總表（取代 golden 的 fShow）
#include "forms/fHome.h"            // AI(W906-P6b-C) 20260921: fHome->iHomeStep（homingActive 的佐證訊號）
#include "SecsTagPublish.h"          // AI(W906-P4-SECS) 20260920: SECS SV 登錄表當 tag 目錄
// AI(W906-FW-W5a) 20260819: ShowMyMessage + its forward hook live in
// canary_support.{h,cpp} -- but that header re-defaults RecordProcess/
// MyDBIProcessNew parameters that common.h (already included above) also
// defaults, which is ill-formed in one TU. Local forward-decls instead,
// the same idiom database.cpp already uses for ShowMyMessage; no defaults
// here, every argument is passed explicitly at the call site.
void ShowMyMessage(AnsiString S1, AnsiString S2, AnsiString S3, bool Ok, bool bServoOff);
extern void (*W906_ShowMyMessage_Hook)(const char* S1, const char* S2);
// AI(W906-FW-W5b) 20260819: the ANSWER-carrying dialog (same local-decl idiom).
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr, AnsiString errPart);
extern int (*W906_ShowErrorMessage_Hook)(const char* Code, int KCode, int Pos);
// AI(W906-P6b-C) 20260921: Bit4_HandlerDiagnostics 的 seam（同一個本地宣告慣例）。
//   ⚠ 不能改成 `#include "canary_support.h"` —— 上面那段註解講的理由今天仍然成立：
//     那個標頭會重新給 RecordProcess / MyDBIProcessNew 預設引數，而 common.h
//     （前面已經 include）也給了，同一個 TU 裡是 ill-formed。
//   宣告必須與 canary_support.h:165 逐字相同，否則是兩個不同的符號、連結會過但
//   安裝到的是另一個變數（這棵樹踩過同名雙宣告的坑）。
extern bool (*W906_DiagnosticsWindowOpen_Hook)(bool systemStart, int contactMode);
// AI(W906-YESNO) 20260925: golden ShowMyMessageBox_YES_NO 與它的應答 hook（同一個本地宣告慣例）。
//   ⚠ 型別必須與 canary_support.h 逐字相同（那邊 S3 帶預設 ""，這裡不帶 —— 預設引數不是型別的一部分）。
int  ShowMyMessageBox_YES_NO(AnsiString S1, AnsiString S2, AnsiString S3);
extern int (*W906_ShowMyMessageBoxYesNo_Hook)(const char* S1, const char* S2, const char* S3);

#include <vector>
#include <map>                        //Steven 20260916: CsvApplyRows (was only reaching us transitively)
#include <set>                        //Steven 20260916: CsvApplyRows
#include "WebCmdGuard.h"              // AI(W906-CMDGUARD) 20260926: S107-3 防連點（W906CmdGuardScope，分派迴圈頭用）。佔用原本的空行，其後行號不動
#include "database.h"
#include "common.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include "forms/fYieldMonitoring.h"   // AI(W906-YM-READFILE) 20260923: fYieldMonitoring->ReadFile (Tester.Data)
#include "forms/fHotPlate.h"        // AI(W906-HP-BOOTWIRE) 20260923: fHotPlate->ReadFile (HotPlate.Data)
#include "cpublic.h"             // Steven 20260925: VerInfo (asHandlerVersion, golden main.cpp:9761)
#include "forms/fOffSet.h"         // AI(W906-OFS-BOOTWIRE) 20260923: fOffSet->ReadFile (Position Offset.Data)
#include "forms/fContact.h"        // AI(W906-CT-BOOTWIRE) 20260923: fContactForm->ReadFile (Contact.Data)
#include "BarCode/BarCode.h"       // AI(W906-BC-BOOTWIRE) 20260923: fBarCode->ReadFile (HandlerCondition.Data [Configuration])
#include "AutoAlignment/AutoAlignment.h"  // AI(W906-AA-BOOTWIRE) 20260923: fAutoAlignment->ReadFile
#include "forms/fTrayMapping.h"    // AI(W906-TM-BOOTWIRE) 20260923: fTrayMappingForm->ReadFile
#include "forms/fQAMode.h"        // AI(W906-QA-BOOTWIRE) 20260923: fQAMode->ReadFile (Tester.Data [QA Mode])
#include "Magazine.h"             // AI(W906-MG-BOOTWIRE) 20260923: fMagazine->ReadFile
#include "forms/fFixAICCD.h"      // AI(W906-FX-BOOTWIRE) 20260923: fFixAICCD->ReadFile
#include "cUnitConvert.h"         // AI(W906-UNITCONV-WIRE) 20260923: DoStructUnitConvert (cUnitConvert.h:118)

// AI(W906-FW-W5a) 20260819: ShowMyMessage -> browser info modal. The hook is a
// raw function pointer (canary_support.h keeps zero includes), so the server
// handle rides a file-scope static. Display-only by design: golden's
// ShowMyMessage returns void, nothing flows back (design doc section 4).
// ===========================================================================
//  AI(W906-WD) 20260923: 停擺看門狗 —— 讓「沒辦法輪詢」自己講出它停在哪。
//
//  ## 為什麼需要
//  wb_serve 只有一條執行緒在推機台狀態：tools/wb_serve.cpp 的 `for(;;)` 迴圈
//  跑 PumpTick()->MainProc()，**而且同一圈才把網頁命令排空並回 ack**。
//  任何一段卡住，會一次造成三個看起來不相干的症狀：
//    (a) MainProc 不再被呼叫（csystem.cpp 的中斷點永遠不中）
//    (b) DoAllProcess / DoInArm_9045 不再被進入
//    (c) 每個網頁命令都回 "no ack within 15000ms"（逾時，不是拒絕）
//  20260923 實測到兩次：一次是 ForwardShowErrorMessage 的 kcode==0 無出口
//  （已修，見 AI(W906-Q30-KZERO)），一次是 golden 自己的無限迴圈
//  AutoCalculateInArmYClosePitch（ainarm9045.cpp，UserDefForm[Ld].YPitch==0 時
//  空轉；該函式檔頭 GOLDEN BUGS PRESERVED (1) 已記載，照 no-fix 規則保留）。
//  兩次都花了數小時才定位，因為**程式自己不講它停在哪**。
//
//  ## 它不是什麼
//  它**不改任何行為**：沒卡住時完全沉默，不動狀態、不動輸出、不影響時序。
//  它只讀 breadcrumb 然後 printf。它也**不會**把卡住的執行緒救回來 ——
//  那需要改行為，而改行為要使用者裁決。
//
//  ## 實作選擇
//  * Win32 CreateThread 而不是 std::thread：避免 MinGW 6.3 的 winpthread 相依，
//    這顆二進位目前的 DLL 相依只有 KERNEL32/msvcrt/PSAPI/USER32/VERSION/WS2_32，
//    不想為了診斷多一個。
//  * CRITICAL_SECTION 而不是 atomic：字串要整份一致地讀走。
//  * 門檻 5 秒起報、之後每 30 秒再報一次。tick 是 500 ms，正常永遠不會觸發。
// ===========================================================================
namespace {

CRITICAL_SECTION   g_wdLock;
bool               g_wdReady = false;
char               g_wdPhase[192] = "boot";
DWORD              g_wdSince = 0;
unsigned long long g_wdSeq = 0;

void WdMark(const char* phase)
{
    if (!g_wdReady) return;
    ::EnterCriticalSection(&g_wdLock);
    std::strncpy(g_wdPhase, phase ? phase : "?", sizeof(g_wdPhase) - 1);
    g_wdPhase[sizeof(g_wdPhase) - 1] = '\0';
    g_wdSince = ::GetTickCount();
    ++g_wdSeq;
    ::LeaveCriticalSection(&g_wdLock);
}

// 兩段式：命令名是外來字串，先夾進固定緩衝再交給 WdMark。
void WdMark2(const char* what, const char* detail)
{
    if (!g_wdReady) return;
    char buf[192];
    std::snprintf(buf, sizeof(buf), "%s%s", what ? what : "?", detail ? detail : "");
    WdMark(buf);
}

DWORD WINAPI WdThread(LPVOID)
{
    unsigned long long lastSeq  = (unsigned long long)-1;
    DWORD              lastWarn = 0;
    for (;;) {
        ::Sleep(1000);
        char  phase[192];
        DWORD since;
        unsigned long long seq;
        ::EnterCriticalSection(&g_wdLock);
        std::strncpy(phase, g_wdPhase, sizeof(phase) - 1);
        phase[sizeof(phase) - 1] = '\0';
        since = g_wdSince;
        seq   = g_wdSeq;
        ::LeaveCriticalSection(&g_wdLock);

        if (seq != lastSeq) { lastSeq = seq; lastWarn = 0; continue; }

        const DWORD stuck = ::GetTickCount() - since;
        if (stuck < 5000) continue;
        if (lastWarn != 0 && (stuck - lastWarn) < 30000) continue;
        lastWarn = stuck;

        std::printf("\n[WATCHDOG] 主執行緒已 %lu 秒沒有前進 -- 停在: %s\n"
                    "           tick 迴圈沒在轉 => MainProc() 不會被呼叫,\n"
                    "           而且每個網頁命令都會 no ack (逾時, 不是拒絕).\n"
                    "           要看確切位置: 用 VS Code 的 Pause(F6) 讀 Call Stack,\n"
                    "           或 gdb -p <pid> 後 bt.\n\n",
                    (unsigned long)(stuck / 1000), phase);
        std::fflush(stdout);
    }
}

void WdStart()
{
    ::InitializeCriticalSection(&g_wdLock);
    g_wdReady = true;
    WdMark("startup");
    DWORD tid = 0;
    ::CreateThread(NULL, 0, &WdThread, NULL, 0, &tid);
}

} // namespace
static webbridge::WebBridgeServer* g_modalServer = 0;
static void ForwardShowMyMessage(const char* s1, const char* s2)
{
    // AI(W906-SMM) 20260925: 顯示搬到檔尾 W906MbShowMyMessage（信箱＋WS modal 觸發＋照 golden ShowModal 等回答）。
    //   這一支留著當舊 hook 的「鏈尾」：WebBuilder／WebSmartDiag 的 MsgCapture 把舊 hook 換成自己再串回這裡，
    //   檔尾用「舊 hook 是不是這一支」判斷現在有沒有頁面動作在收訊息（有就走 ack.messages，不另跳框）。
    (void)s1; (void)s2;
}
bool W906_FormShowing(const char* goldenForm, bool member);   //AI(W906-FSHOW-E3) 20260929 [W906] same decl as W906FormShowing.h (body csystem.cpp:30049); old blank line
// AI(W906-FW-W5b) 20260819: ShowErrorMessage -> browser query, ANSWER flows
// back. Golden blocks its UI thread in a modal loop until the operator picks
// RETRY / SKIP / CLEAN_OUT; the equivalent here is this pump: it blocks the
// tick thread, drains the queue itself, and refuses every command except the
// matching modal.answer with "modal-pending" -- the transport-level rendering
// of VCL modality (everything behind the dialog is inert until it is
// answered). No timeout, faithfully: golden waits forever. Tag publishing
// also freezes while pumping, exactly as golden's blocked UI thread would.
static webbridge::CommandQueue* g_pumpQueue = 0;
static unsigned long long       g_nextQid  = 1;
static std::deque<webbridge::WebCommand> g_carry; static bool g_carryRunnable = false; static bool g_outputsServed = false; static bool g_apiCacheDirty = false; static void W906_TakeCarry(std::vector<webbridge::WebCommand>& out) { for (std::size_t k = 0; k < g_carry.size(); ++k) out.push_back(g_carry[k]); g_carry.clear(); g_carryRunnable = false; }   // AI(W906-IOWEB-P25) 20260925: OUTPUT FIRST. g_carry = commands the output-first service (W906_ServiceOutputs, EOF) took off the queue but did NOT run (it runs outputs only); they keep their arrival order and the next drain -- the main loop's AND the modal wait's below -- takes them ahead of anything newer. g_apiCacheDirty replaces the three unconditional W906_ApiCacheRefresh() calls (20-160 ms each, measured by the click log): rebuilt once per loop, after the drain. On the old blank line, so no line below moves
// AI(W906-Q30-8) 20260922: 信箱目錄。由 main() 在決定 root 之後填。
//   ⚠ 空字串 = 沒接信箱（例如單元測試或還沒 init），此時只走 WebSocket，
//     行為與 20260921 相同 —— 不要讓「沒設定」變成「靜默不發警報」。
static std::string g_dialogMailboxDir;

// AI(W906-Q30-8) 20260922: 每個通道的 seq。契約 transport.ordering 要求
//   「seq must increase monotonically」，而 dialog-bridge.js:402 的條件是
//   `seq > channel.lastSeq`，且 lastSeq 初值 0 ⇒ **seq 必須 >= 1**。
//   樣板檔裡的 0 永遠不會大於 0 ⇒ 用 0 的話框永遠不開，而且不報錯。
//
// AI(W906-Q30-IDLE) 20260923: ⚠ 起點在 main() 改成「開機當下的 Unix 毫秒數」
//   （w906dlg::UnixMillisNow()），不再是 0。
//   理由：dialog-bridge.js 的 channel.lastSeq 是**頁面**的記憶體狀態
//   （dialog-bridge.js:7-10），只有重整頁面才歸 0；而量產是 file: 協定，頁面
//   直接讀檔、**不會因為 wb_serve 重開而重整**。舊寫法每次開機都從 1 數起
//   ⇒ 頁面沒重整、只有 wb_serve 重開時，新告警的 seq（1, 2, …）小於頁面記住的
//   lastSeq，:692 的 `seq > channel.lastSeq` 不成立 ⇒ **框不出來，C++ 在等**。
//   以時間當起點，後一個行程的 seq 一定大於前一個行程發過的任何 seq
//   （前提：時鐘不倒退，且每毫秒不超過一則）。0 仍是這裡的初值，只給
//   「main() 還沒跑到那一行」的極短窗口與單元測試用。
static unsigned long long g_dialogSeq = 0;  static const char* g_w906MotorNoteUnit = 0;  static const char* g_w906MotorNoteMsg = 0;   //AI(W906-JAM-STOP) 20260930: non-null only while ForwardShowMotorErrorMessage (EOF) posts golden ShowMotorErrorMessage's note through ForwardShowErrorMessage's kcode==0 branch -- the note's unit alias / message (golden note.cpp:1102 ErrShowToForm), and "skip the ShowErrorMessage-only stop + record" (the motor body did its own, golden :1054-1092).  Tick thread only
static w906dlg::AlarmSlot g_alarmSlot;   //AI(W906-J5-ACK) 20260930: what the Alarm-dialog-request mailbox holds now (tools/wb_dialog_mailbox.h AlarmSlot: idle / notice / blocking) -- written only by DialogMailboxPostAlarm / DialogMailboxRetire (through w906dlg::AlarmPost / AlarmRetire), read by W906_NoticeAckCommand (EOF).  Tick thread only.  On the old blank line, so no line below moves
// AI(W906-Q30-8) 20260922: 寫一份 ShowErrorMessage 的請求進信箱。
//   欄位逐字對照 web/JSON/Alarm-dialog-request.json（那是最權威的樣本），
//   arguments 五個鍵對上 golden
//   `ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool, AnsiString)`。
//   ⚠ `position` 是 cmydef 的 unit id，不是像素座標。
static bool DialogMailboxPostAlarm(const std::string& requestId,
                                   const char* code, int kcode, int pos)
{
    if (g_dialogMailboxDir.empty()) return false;
    //AI(W906-J5-ACK) 20260930: `++g_dialogSeq`, the JSON and the slot are w906dlg::AlarmPost's now (below); g_dialogAlarmSeq follows g_alarmSlot.seq

    // AI(W906-Q30-8) 20260922: `blocking` **不是常數**。
    //   同事 20260922 早上提出第三種形狀 [P32]（`blocking:false`），
    //   我去 golden 驗過，他是對的而且三棵樹一致：
    //     V899 acatchtray.cpp:4897 / V906 :5382 / V912 :5655
    //     都是 `iUnLoaderCount=8;  // 必須不為0 Handler才不停機`
    //     然後走 `ShowUnloaderTrayMessage()` —— 本體最後是 `Show()` 不是
    //     `ShowModal()`，所以它真的不阻塞、也不 StopAllMotor。
    //   ⇒ `ShowErrorMessage` 這條路恆為 true（golden note.cpp:795-798 兩個
    //     分支都 StopAllMotor，停機不是可選項）；
    //     但 `ShowUnloaderTrayMessage` 接上來的時候必須是 false，
    //     否則 C++ 會去等一個不該等的東西 ⇒ 機台停在那裡不動，
    //     跟 [P32] 要的「不停機」正好相反。
    //   ⇒ 參數化，不要讓下一個人以為它是常數。
    const bool blocking = true;   // ShowErrorMessage 專用；NonStop 那條要傳 false
    //AI(W906-J5-ACK) 20260930: the two snprintf buffers that were here (head[256] / args[192]) became
    //   w906dlg::AlarmRequestJson (tools/wb_dialog_mailbox.h): the same text -- tests/test_notice_ack.cpp [A] compares
    //   it with a verbatim copy of them -- without their silent truncation.  w906dlg::AlarmPost also keeps g_alarmSlot
    //   (kNotice when kcode==0, kBlocking otherwise); WS dialog.notifyAck (EOF W906_NoticeAckCommand) retires a notice only.
    //   ⚠ `buttons` 是**死欄位** —— dialog-page.js 完全不讀它，
    //     按鈕是由 arguments.kCode 的位元驅動的。填了也不影響行為。
    const bool posted = w906dlg::AlarmPost(g_alarmSlot, g_dialogMailboxDir, g_dialogSeq, requestId,
        code ? code : "", kcode, pos,
        g_w906MotorNoteUnit ? g_w906MotorNoteUnit : "",                        //AI(W906-JAM-STOP) 20260930: a motor note carries golden's unit alias (MOT[UnitNo].Alias); every other alarm keeps "" (unchanged)
        g_w906MotorNoteMsg ? g_w906MotorNoteMsg : (code ? code : ""),         //AI(W906-JAM-STOP) 20260930: a motor note carries golden's MyDBIEvent Message (ShowMessageEdit1); every other alarm keeps the code (unchanged)
        blocking);
    { extern unsigned long long g_dialogAlarmSeq; g_dialogAlarmSeq = g_alarmSlot.seq; }   // AI(W906-SMM-IO) 20260925: IO 解除時 Dialog-close-request 要對到這一則（檔尾 AI(W906-SMM-IO)）
    return posted;
}

// AI(W906-J5-ACK) 20260930: THE NOTICE CONTRACT (INBOX 119; St01's web/page/ht9045_dialog_host.js keys on it).
//   Both kinds are written above with the same fields; closePolicy ("acknowledge-only") and buttons ([]) are the SAME
//   for both and identify nothing.  The page tells them apart by arguments.kCode:
//     kCode === 0  a NOTICE (ForwardShowErrorMessage's AI(W906-Q30-KZERO) branch, and every golden ShowMotorErrorMessage
//                  note): no wait loop behind it, dialog.response for it is refused.  Show one 確認 button, which sends
//                    {"type":"cmd","id":<n>,"cmd":"dialog.notifyAck","tag":"<requestId>"}
//                  ok:true  -> the file is idle already; close the box.  ok:false "no-pending-notice" -> already closed;
//                  close the box too.  Any other ok:false (request-mismatch / not-a-notice / golden-refused /
//                  retire-failed / modal-pending / closing: / not-operator) -> keep the box, the operator may press again.
//     kCode !== 0  a BLOCKING alarm: ForwardShowErrorMessage's wait loop answers modal.answer / dialog.response with
//                  tag == requestId, exactly as before; dialog.notifyAck for it answers not-a-notice.
//   Error strings are "<code>" or "<code>:<detail>" (tools/wb_dialog_mailbox.h NotifyAckError).  The ok:true payload is
//   NotifyAckOkJson's, spliced into the ack:
//     {"type":"ack","id":<n>,"ok":true,"notice":"retired","requestId":"<id>","seq":<the idle file's seq>,
//      "pause":"applied"|"already-applied"|"skipped-machine-running"|"no-golden-note","jamCounted":<bool>,"passTime":<s>}
//   What each pause value means and which golden lines ran: forms/fNote_ShowError.cpp EOF (W906_NoteNoticeAckLikeGolden).
//   During another blocking dialog the command is still answered at once: ForwardShowErrorMessage's wait -> not-a-notice,
//   the YES/NO and ShowMyMessage waits -> modal-pending (both: keep the box).







// AI(W906-Q30-8) 20260922: 把請求退役成 idle。
//   ⚠⚠ **這一步不可以省。** dialog-bridge.js 的 lastSeq 是記憶體狀態，
//     瀏覽器一重整就歸 0；留著 state:"pending" 的舊請求會**再彈一次**，
//     而 C++ 會收到第二份同 requestId 的回應。
//     症狀是「答完之後按 F5，同一個警報又跳出來」—— 而且只有重整才看得到。
static bool DialogMailboxRetire()   //AI(W906-J5-ACK) 20260930: returns true = both files are idle (was void; only W906_NoticeAckCommand reads it)
{
    if (g_dialogMailboxDir.empty()) return false;
    //AI(W906-J5-ACK) 20260930: `++g_dialogSeq` and the JSON are w906dlg::AlarmRetire's now (tools/wb_dialog_mailbox.h AlarmIdleJson)
    // ⚠⚠ 20260922 端到端實測抓到：這裡原本是 `char json[256]`，而下面那串
    //   格式化出來約 500 字元 ⇒ `snprintf` **靜默截斷**，寫出一個少了後半段
    //   的壞 JSON。症狀不是「退役失敗」而是
    //   `Invalid control character at column 256` —— 而且只有在**解析**
    //   那個檔的時候才看得到，wb_serve 自己完全不會報錯。
    //   ⇒ 開大到 1024 並在下面斷言沒被截斷。
    //   [AI(W906-J5-ACK) 20260930: now one std::string (w906dlg::AlarmIdleJson) -- no buffer, nothing to truncate; the same
    //    text, byte for byte (tests/test_notice_ack.cpp [A] against a verbatim copy of the 1024-byte snprintf).]
    // ⚠ snprintf 截斷是**靜默**的 —— 它回「本來要寫幾個字元」而不是實際寫的。
    //   不檢查就會寫出半截 JSON，而那只有在對方解析時才炸。   [AI(W906-J5-ACK) 20260930: history -- the check went with the buffer]

    // AI(W906-J5-ACK) 20260930: w906dlg::AlarmRetire also sets g_alarmSlot idle -- only when BOTH files were written, so
    //   a failed retire leaves the request acknowledgeable (a second dialog.notifyAck retries it) instead of reporting
    //   "no-pending-notice" over a box the page still shows.
    const bool ok = w906dlg::AlarmRetire(g_alarmSlot, g_dialogMailboxDir, g_dialogSeq);
    if (!ok)
        std::printf("  ⚠ dialog mailbox retire FAILED -- 下次重整會再彈一次\n");
    return ok;
}
struct W906ModalWaitScope { explicit W906ModalWaitScope(int k, const char* code = 0); ~W906ModalWaitScope(); int kind; };  void W906_ModalWaitTick(int kind, int kcode = 0);  void W906_ModalOutputsRefresh();  int W906_YesNoPreCloseLikeGolden();  extern int* g_w906NoteSelPtr;  void W906_TesterCommPoll(); bool W906_TesterCommHttp(const std::string&, const std::string&, const std::string&, bool, int*, std::string*, std::string*);  bool W906_ElaHttp(const std::string&, const std::string&, const std::string&, bool, int*, std::string*, std::string*);  /* AI(W906-ELA-P3) 20260927: EventLogAnalysis/ElaService.cpp */  bool W906_SimDiCommand(const webbridge::WebCommand& wc);  /* AI(W906-R70) 20260926 YN-3（本體在檔尾，同 :6527）*/  bool CheckIndexAllSuckICFallDown(bool bCheckArm1, bool bCheckArm2);  extern bool SECS_GEM_PPMUSIC_CONTROL_flag;  extern bool SECS_GEM_PPSIGNALTOWER_CONTROL_flag;  /* AI(W906-R70) 20260926 MW-F／YN-4：csystem.h:209、ckernel.cpp:1272／:1275 */  std::size_t W906_StageMotionViewTrays(webbridge::TagSnapshot& snap);  /* AI(W906-S118) 20260928 (St02-E, laptop-approved claim): JsonBridge/ChanMvTrays.cpp, called from PublishExtraTags */   // AI(W906-MODAL-WAKE) 20260926: RULINGS #9 —— 三個阻塞等待迴圈進迴圈前建一個（0 告警、1 是／否、2 ShowMyMessage），任何 return 都經過解構；每一圈呼叫 W906_ModalWaitTick。本體在檔尾
static int ForwardShowErrorMessage(const char* code, int kcode, int pos)
{
    if (!g_w906MotorNoteMsg) { extern void W906_AlarmStopLikeGolden(const char*); W906_AlarmStopLikeGolden(code); }  if (!g_w906MotorNoteMsg) /* AI(W906-JAM-STOP) 20260930: both skipped only for golden ShowMotorErrorMessage's note (EOF ForwardShowMotorErrorMessage) -- golden note.cpp:795-868 are ShowErrorMessage's, the motor body did its own stop + record (golden :1054-1092) */ { extern bool W906_ShowErrorMessageRecordLikeGolden(AnsiString, bool, AnsiString); extern AnsiString W906_ShowErrorMessage_LastErrPart; extern bool W906_ShowErrorMessage_LastDuplicate; W906_ShowErrorMessageRecordLikeGolden(AnsiString(code ? code : ""), W906_ShowErrorMessage_LastDuplicate, W906_ShowErrorMessage_LastErrPart); }  if (!g_modalServer || !g_pumpQueue) return 0;   // unattended -> sim answer  // AI(W906-ALARMSTOP) 20260924: 先照 golden note.cpp:795-801 停機（使用者 20260924：kcode≠0 與 kcode==0 都停；kcode==0 仍依 0923 不阻塞）—— 本體在檔尾  //AI(W906-SHOWERR) 20260929: then golden ShowErrorMessage's alarm record (note.cpp:538 / :794 / :802-818 / :839-868 -- MyDBIEvent -> EventTracker / HANDLER LOG csv, ProductionLog, ErrShowToForm -> fNote->edErrorCode / Edit3 / edUnitName / AlarmType; forms/fNote_ShowError.cpp) with the errPart / bDuplicateErr canary_support.cpp recorded; before the unattended return because golden records every alarm (RULINGS_20260929 section 5 item 7)
    const unsigned long long qid = g_nextQid++;
    char qidStr[24];
    std::snprintf(qidStr, sizeof(qidStr), "%llu", qid);

    // AI(W906-Q30-8) 20260922: ⚠⚠ **kCode 守門，放在最前面。**
    //   畫面只接得住 9 個 K（dialog-page.js:25-28 的 ALARM_BTNS）。
    //   若 kcode 只含 K_FIX(0x100) / K_PAUSE(0x400) / K_START(0x800)，
    //   框會跳出來但**一顆可選鍵都沒有**，而 dialog-page.js:61 的
    //   `if (!selected) return;` 讓 Start/Pause 都變 no-op
    //   ⇒ **關不掉、答不了、機台永久卡住**，而且 log 裡什麼都沒有。
    //   ⇒ 寧可在這裡就大聲講，也不要讓操作員對著一個沒有按鈕的框。
    const int kShown = kcode & (K_RETRY | K_SKIP | K_CLEAN_OUT | K_TRAY_FEED |
                                K_TRAY_END | K_RESET | K_HOME | K_TRAIN | K_ONECYCLE);
    if (kcode != 0 && kShown == 0) {
        std::printf("  ⚠⚠ kcode=%d 不含任何畫面接得住的 K —— 框會沒有按鈕。\n"
                    "     畫面支援的是 golden note.cpp:1235 KeyComp[] 那 9 個；\n"
                    "     K_FIX/K_PAUSE/K_START 不在其中（golden 自己註解掉 K_FIX）。\n"
                    "     這一則改走 WebSocket，不寫信箱。\n", kcode);
    }

    // AI(W906-Q30-KZERO) 20260923: kcode==0 是**通知**，不是問題 —— 不進等待迴圈。
    //
    //   下面那個迴圈唯一的出口是
    //   `if (k != 0 && (k & kcode) != 0)`。kcode==0 時 `(k & 0)` 恆為 0
    //   ⇒ **沒有任何出口，永遠等下去**。
    //
    //   而 wb_serve 是單執行緒：這一支卡住，主迴圈的 `if (pumpBeat) ht9045::PumpTick();`
    //   連帶停擺 ⇒ MainProc 不再被呼叫、DoAllProcess 不再跑，而且**其他網頁
    //   命令拿不到 ack** —— start.run 分支的 `CompleteCommand`
    //   要等 `StartFromWeb()` 返回才發。
    //   （AI(W906-Q34-7-L1) 20260923 夜間：這裡原本寫 `:408`／`:2979`／`:3632` 三個行號，
    //    rebase Q34-7 之後全部漂掉（獨立審查 L1）；改成用程式碼本身當錨點，不再換一組會再漂的數字。）三個症狀、一個根因，而且看起來像
    //   三個不相干的 bug（斷點不中／流程不跑／網頁 no ack within 15000ms）。
    //   光是 StartFromWeb() 裡就有約 13 個 `ShowErrorMessage(code, 0, ...)`
    //   （WebStart.cpp:1486/1914/2064/2418/2638/3000/3336…），任一被打到就整台停。
    //
    //   ## 為什麼回 0 是對的
    //   0 ＝「沒有任何鍵被按」，與本函式最上面 :283 的無人值守出口
    //   `if (!g_modalServer || !g_pumpQueue) return 0;` 同值。而且 kcode==0 的
    //   呼叫點**全部丟棄回傳值**（20260923 逐處實測 WebStart.cpp 那 13 個）。
    //
    //   ## 為什麼只寫信箱、不 PostQuery
    //   PostQuery 會在 g_modalServer 留一個待答查詢，而畫面對 kcode==0 生不出
    //   任何按鈕（上面那段 kShown 警告講的就是這件事）⇒ 操作員關不掉，又沒有
    //   人會去 :417 的 ClearQuery() ⇒ 下一個連上來的瀏覽器收到一個關不掉
    //   的框，它的回應還會被判成 `no query pending`。
    //   信箱是**單槽檔**，而單執行緒下不可能在別人阻塞時輪到我們覆蓋它
    //   （阻塞中的那一支正握著這條執行緒），所以只寫信箱是安全的。
    //
    //   ⚠ 使用者 20260923 裁決：「先這樣做，未來再回頭修改，這些通知雖然忠於
    //     翻譯，但不是最急著處理的」。**這是刻意偏離 golden** —— golden 的
    //     kcode==0 是一則要操作員按鍵消掉的 note，它會擋住機台。要回頭補的是
    //     「通知型對話框的非阻塞確認通道」，不是把這裡改回去等。   [AI(W906-J5-ACK) 20260930: that channel exists now -- WS dialog.notifyAck, tag = this qid (the mailbox requestId): W906_NoticeAckCommand (EOF) retires the notice and applies golden's close, BtnPauseClick's KeyCode==0 arm + FormClose (forms/fNote_ShowError.cpp EOF).  This branch still returns at once]
    if (kcode == 0) {
        const bool noteOnly = DialogMailboxPostAlarm(qidStr, code, kcode, pos);  { extern void W906_NoteNoticeCapture(const char*, bool); W906_NoteNoticeCapture(qidStr, g_w906MotorNoteMsg != 0); }   //AI(W906-J5-ACK) 20260930: keep what golden FormClose reads from fNote (code / AlarmType / iDuplicateError / iEventID) for the ack -- forms/fNote_ShowError.cpp EOF
        // ===== AI(W906-SJSON-S10) 20260923 BEGIN -- 事件通道留一筆 =====
        //   ⚠ blocking=false，而且**馬上補一筆 clear** —— 這一則不進等待迴圈
        //   （AI(W906-Q30-KZERO)），沒有人會回答它，留著 raise 會讓
        //   alarm.active 永遠遞增。退役者是 C++ 自己，所以 action 是空字串。
        //   ⚠ EmitAlarm/ClearAlarm 都是純記憶體 append，不阻塞。
        ht9045::sjson::EmitAlarm(ht9045::sjson::kSrcShowErrorMessage,
                                 code ? code : "", kcode, pos, qidStr, false);
        ht9045::sjson::ClearAlarm(qidStr, "", "", 0);
        // ===== AI(W906-SJSON-S10) 20260923 END =====
        std::printf("query qid=%s code=%s kcode=0 -- NOTE, not blocking (mailbox=%s)\n",
                    qidStr, code ? code : "", noteOnly ? "yes" : "no");
        std::fflush(stdout);
        return 0;
    }

    g_modalServer->PostQuery(qid, code ? code : "", kcode);
    // ===== AI(W906-SJSON-S10) 20260923 BEGIN -- 阻塞式警報的 raise =====
    //   放在 PostQuery 之後、等待迴圈之前：這一刻「警報已經送出去了」是事實。
    //   放在迴圈裡會每 100 ms 記一筆，放在函式最後就只剩解除沒有發生。
    ht9045::sjson::EmitAlarm(ht9045::sjson::kSrcShowErrorMessage,
                             code ? code : "", kcode, pos, qidStr, true);
    // ===== AI(W906-SJSON-S10) 20260923 END =====

    // AI(W906-Q30-8) 20260922: 同一則警報**同時**走兩條路 ——
    //   信箱（同事那套現成的 HMI，會在瀏覽器重開後再彈）與
    //   WebSocket（我們自己的，Q30 第 3 題做的重連補發）。
    //   ⚠ 先到的算數；下面的等待迴圈兩種回應都收。
    const bool posted = (kShown != 0)
                      ? DialogMailboxPostAlarm(qidStr, code, kcode, pos)
                      : false;
    if (!posted && !g_dialogMailboxDir.empty() && (kShown != 0))
        std::printf("  ⚠ dialog mailbox 寫入失敗 —— 只剩 WebSocket 那條路\n");

    std::printf("query qid=%s code=%s kcode=%d -- waiting (mailbox=%s, ws=yes)\n",
                qidStr, code ? code : "", kcode, posted ? "yes" : "no");

    WdMark2("modal wait (blocking, needs a browser answer): ", code ? code : "");  // AI(W906-WD) 20260923
    std::vector<webbridge::WebCommand> local;  W906ModalWaitScope wakeScope(0, code);   // AI(W906-MODAL-WAKE) 20260926: golden TfNote::FormShow／FormClose 的 fShow、蜂鳴器、Alarm Reset 燈（檔尾）
    for (;;) {
        if (g_carry.empty()) g_pumpQueue->waitForPush(100);  { extern void DoAvoidIndexMotorFallDown(); DoAvoidIndexMotorFallDown(); }  W906_ModalWaitTick(0, kcode);  { extern void W906_TesterCommPoll(); W906_TesterCommPoll(); }  /* AI(W906-GB-P3) 20260926: H5 -- golden ShowModal keeps handling WM_COPYDATA from the bridge */   // AI(W906-IOWEB-P25c) 20260925: was Sleep(100) -- an answer or a motor.stop now wakes the wait at once (still bounded to 100 ms; laptop review Q3-3)  //AI(W906-MODAL-SAFETY) 20260925: golden 在 ShowModal 期間 MyMessageBox::Timer1Timer（mymessbox.cpp:538-550／:650）照跑 DoAvoidIndexMotorFallDown（EMG／斷電／Index Z servo off ⇒ 鎖 Index 煞車並停機）；移植樹等待期間 tick 停住，這裡補跑（R-YESNO 審查 high；本體不跳框、SOFT_SIMULTE 下是 no-op）  //AI(W906-MERGE-56bbf785) 20260926: machine P25c wanted the event wait instead of Sleep(100), laptop MODAL-SAFETY wanted DoAvoidIndexMotorFallDown on every pass -- both kept on this one line. The check now also runs on each early wake; golden's Timer1Timer runs it more often than every 100 ms anyway. ⚠ On HT9050 the IO it reads comes from the 1203 monitor's samples, and Poll() does not run inside this wait, so it sees the state from before the wait (same for W906_AlarmIoAnswer below)
        local.clear();
        W906_TakeCarry(local);  g_pumpQueue->drain(local);   // AI(W906-IOWEB-P25) 20260925: carried commands first (a dialog answer the output service moved into g_carry must still reach this wait)
        for (size_t i = 0; i < local.size(); ++i) {
            const webbridge::WebCommand& wc = local[i];
            // AI(W906-Q30-8) 20260922: 兩種回應指令都收。
            //   `modal.answer`    -- 我們自己的 WebSocket 路（20260921）
            //   `dialog.response` -- 同事那套 HMI 經 window.HTDialogHost 送回來的
            //   兩者的 `tag` 都是 qid/requestId，`value` 是動作名。
            //
            //   ⚠ `dialog.response` 的 value 允許 `"<ACTION>:<pressedButton>"`，
            //     例如 `"RETRY:BtnStart"`。理由是**契約層面的**：
            //     `selectedAction.code` **分不出 Start 還是 Pause**
            //     —— 那個 code 由先點的選擇鍵決定，Start/Pause 只是「送出」
            //     （dialog-bridge.js:297 / dialog-page.js:75-76）。
            //     golden 的 fNote 靠 pressedButton 決定後續 SoftStart / SoftStop，
            //     所以這個資訊不能丟。冒號後面沒東西就是 null，與契約一致。
            if ((wc.cmd == "modal.answer" || wc.cmd == "dialog.response")
                && wc.hasTag && wc.tag == qidStr) {
                std::string ans = (wc.hasValue && wc.value.isString())
                                  ? wc.value.asString() : std::string();
                std::string pressed;
                const std::string::size_type colon = ans.find(':');
                if (colon != std::string::npos) {
                    pressed = ans.substr(colon + 1);
                    ans     = ans.substr(0, colon);
                }
                // AI(W906-Q30-KMAP) 20260921: **補滿 9 個** —— 與
                //   `WebBridge/WebBridgeServer.cpp` 的 `PostQuery` 對稱。
                //   兩邊必須同時補：只補送出側，瀏覽器送回一個合法選項會在
                //   這裡被判成 0 而落到下面的 `not an offered option`，
                //   迴圈繼續 —— 症狀與「沒補」一模一樣，但更難查。
                //
                //   清單與 golden `note.cpp:1235` 的 `KeyComp[]` 逐項相同。
                //   ⚠ 不含 `K_FIX`（golden 註解「kevin 20130722 cancel K_FIX」）、
                //     也不含 `K_PAUSE` / `K_START`（不在 `KeyComp[]` 裡）。
                //   ⚠ 這裡用具名常數（`cmydef` 的 `K_*`）而不是字面值 ——
                //     這支 TU 拿得到它們，不必像 WebBridgeServer 那樣硬寫數值。
                int k = 0;
                if      (ans == "SKIP")      k = K_SKIP;
                else if (ans == "RETRY")     k = K_RETRY;
                else if (ans == "TRAY_FEED") k = K_TRAY_FEED;
                else if (ans == "TRAY_END")  k = K_TRAY_END;
                else if (ans == "CLEAN_OUT") k = K_CLEAN_OUT;
                else if (ans == "RESET")     k = K_RESET;
                else if (ans == "HOME")      k = K_HOME;
                else if (ans == "TRAIN")     k = K_TRAIN;
                else if (ans == "ONECYCLE")  k = K_ONECYCLE;
                if (k != 0 && (k & kcode) != 0) {
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, true, std::string());
                    // AI(W906-Q30-REPLAY) 20260921: 問題已經被回答 ⇒ 清掉待答狀態。
                    //   ⚠ 一定要在 `return` 之前。不清的話，下一個連上來的瀏覽器
                    //     會收到這個早就答完的警報框，而且它回的 `modal.answer`
                    //     會被判成 `no query pending` —— 一個關不掉的框。
                    //   ⚠ 這是這個函式**唯一**的正常出口（另一個是最上面
                    //     `!g_modalServer || !g_pumpQueue` 那個 early return，
                    //     而那條路根本沒有 PostQuery 過，沒有東西要清）。
                    g_modalServer->ClearQuery(qid);
                    // AI(W906-Q30-8) 20260922: ⚠⚠ 信箱那一側也要退役。
                    //   不做的話：操作員答完之後按 F5，dialog-bridge 的
                    //   lastSeq 歸 0、request 還是 pending ⇒ **同一個警報再彈一次**。
                    //   而且只有重整才看得到，平常測不出來。
                    DialogMailboxRetire();
                    // ===== AI(W906-SJSON-S10) 20260923 BEGIN -- 解除那一筆 =====
                    //   requestId 與 raise 同一個 qidStr，兩筆才配得起來。
                    //   pressed 可能是空字串（modal.answer 那條路沒有它）——
                    //   空 = 不知道按的是 Start 還是 Pause，序列化成 null。
                    ht9045::sjson::ClearAlarm(qidStr, ans, pressed, k);
                    // ===== AI(W906-SJSON-S10) 20260923 END =====
                    std::printf("query qid=%s answered %s (K=%d) via %s%s%s\n",
                                qidStr, ans.c_str(), k, wc.cmd.c_str(),
                                pressed.empty() ? "" : " pressed=",
                                pressed.c_str());
                    // AI(W906-DLG-PAUSE) 20260923 夜間：告警框上的 Pause 照 golden 讓機台暫停。
                    //   golden note.cpp:3980-3982 TfNote::BtnPauseClick：
                    //       ReturnCode=KeyComp[i]; SoftStop=true; SoftStart=false;
                    //   對照按 START（BtnStartClick → TfNote::Start()，:3665）只做 ReturnCode=KeyComp[i]  [⚠ 20260924 更正：這句不完整 —— golden :3669 接著還有 fMain->Start("fNote::Start 1") 與 :3677 SendCommand_ESD(ESD_SYSTEM_START)，現在照做，見下面 return k; 那行]
                    //   —— SoftStart／SoftStop 那兩行在 golden 本身就被註解掉（:3667-3668），所以 START 這邊不加任何東西。
                    //   在這之前 pressed 只印進 log、沒有人消費（8e7809d 的訊息自己也寫「沒人消費它」），
                    //   於是告警框按 Pause 的效果等於按 START。20260923 夜間 T8 從畫面實測（無頭 Edge 點 iframe 裡的
                    //   #BtnRetry 再點 #BtnPause）：回答送到了（log「answered RETRY ... pressed=BtnPause」、無 [object Object]），
                    //   但按後 3 秒／10 秒 SystemStart 仍是 1。
                    //   ⚠ 只認 "BtnPause"（dialog-page.js 的 fNote 路徑送的就是這個字）；
                    //     pnlPause 是另一條路（送 action PAUSE，不在 KeyComp 裡，這裡本來就會回 not an offered option），不動。
                    //   ⓘ 只是設旗標，跟 pause.run 一樣由 MainProc 在下一拍消化，不會卡住 tick。
                    if (pressed == "BtnPause") {
                        SoftStop  = true;                                       // golden note.cpp:3981
                        SoftStart = false;                                      // golden note.cpp:3982
                        std::printf("query qid=%s: pressed=BtnPause -> SoftStop=1 SoftStart=0 (golden BtnPauseClick)\n", qidStr);
                    }
                    if (i + 1 < local.size()) { g_carry.insert(g_carry.begin(), local.begin() + (i + 1), local.end()); g_carryRunnable = true; }  { extern void W906_NoteJamCountOnClose(int); W906_NoteJamCountOnClose(k); }  /* AI(W906-J2) 20260926: golden FormClose :2528 Jam 計數，在重走 START 之前（forms/fNote_JamCount.cpp） */  if (pressed.empty() || pressed == "BtnStart") { extern bool W906_AlarmAnswerStartLikeGolden(const char*); W906_AlarmAnswerStartLikeGolden(qidStr); }  { extern void W906_NoteFormCloseAlarmClear(); W906_NoteFormCloseAlarmClear(); }  return k;  // AI(W906-HALARM-CLOSE) 20260926: golden TfNote::FormClose note.cpp:2531 Alarm->Clear()（HAlarm.cpp 檔尾）  // AI(W906-ALARMSTOP) 20260924: 按 START（或沒帶按鍵）照 golden TfNote::Start（note.cpp:3665-3678）重走啟動檢查 —— 本體在檔尾  //AI(W906-MERGE-56bbf785) 20260926: the commands drained together with the answer (local[i+1..]) were dropped here -- never run, never acked (the browser saw an ack timeout). They go back to the FRONT of g_carry (machine IOWEB-P25 carry; the main loop wakes on !g_carry.empty() and drains it first), in arrival order. Put back BEFORE the START side effect: StartFromWeb can raise another alarm, and that wait must see these older commands before anything newer. Same line, so no line below moves
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                               "not an offered option");
            } else if (wc.cmd == "ui.windows.put") {
                //AI(W906-MT-FIX1) 20260926: 視窗總表在 modal 掛著時也收（同 cfg.resync 的理由：它是「回報」不碰機台狀態，
                //   WebBridgeServer.cpp:1408-1423 本來就豁免權杖）。原本這裡回 modal-pending：告警框等回答超過 15 秒，
                //   總表的每一條連線都過期、stale 算「開著」（WebWindowRegistry 契約 §6）⇒ fMotorTest／fTeach 讀成開著，
                //   MT-E3b 起 golden MainProc 在它們開著時暫停 ⇒ 答完告警之後生產仍然停著，直到下一次心跳（審查 high 的 C++ 那一半）。
                std::string whyW;
                const std::string frameW = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
                if (!frameW.empty() && ht9045::WebWindowRegistryPut(wc.connId, frameW, whyW))
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"accepted\":true,\"duringModal\":true}");
                else
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                                   "ui.windows.put: " + (frameW.empty() ? std::string("value must be the frame JSON string") : whyW));
            } else if (wc.cmd == "cfg.resync") {
                // AI(W906-JSONBRIDGE-S0) 20260923: 組態查詢在 modal 掛著時也放行。
                //
                //   它不碰機台任何狀態 —— 只把「這顆 exe 是什麼組態建出來的」
                //   再說一次。擋住它反而製造死結：瀏覽器重整之後會收到這個還沒
                //   答的警報框，而它**在渲染那個框之前**需要知道機型與
                //   SOFT_SIMULTE（要不要顯示某些按鈕）。擋住 = 框畫不出來 =
                //   答不了 = modal 永遠不會解除。
                //
                //   ⚠ 這是 modal 期間唯一的例外。要再加別的指令進來以前，先問：
                //     它會不會改到機台狀態？會的話就不該在這裡。
                unsigned long long since = 0;
                // ⚠ TagValue 的存取子是**嚴格**的：asInt() 對非 Int
                //   一律回 fallback，不做型別轉換（WebBridge/TagValue.h 的
                //   Inspection 那節）。而 JSON 的 `1` 在指令通道上會變成 Double，
                //   所以只寫 isNumber()+asInt() 的話 since 永遠是 0 ——
                //   cfg.resync 於是永遠回「有變」並夾帶完整內容，
                //   unchanged 那條捷徑等於不存在。
                //   20260923 由 tools/webprobe/s1_eventlog_probe.py 抓到，不是看出來的。
                if (wc.hasValue) {
                    if (wc.value.isInt())
                        since = (unsigned long long)wc.value.asInt(0);
                    else if (wc.value.isDouble())
                        since = (unsigned long long)wc.value.asDouble(0.0);
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true,
                                               ht9045::sjson::ConfigResyncJson(since));
            } else {
                extern bool W906_IoPageNoGuards(); extern void W906_DispatchIoClick(webbridge::WebBridgeServer&, const webbridge::WebCommand&);   /*AI(W906-IO-NOGUARD) 20260929: EastSun「IO畫面一律不要卡控」-- an IO click is served during this dialog too (JsonBridge/IoBtnPanelClick.cpp W906_IO_PAGE_NO_GUARDS)*/  if (wc.cmd == "io.btnPanelClick" && W906_IoPageNoGuards()) W906_DispatchIoClick(*g_modalServer, wc); else if (wc.cmd == "dialog.notifyAck") { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, std::string("not-a-notice:current=") + qidStr); } else if (wc.cmd == "motor.stop") { extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool); std::string msAck; const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(), (long long)wc.id, msAck, true); g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck); } else { extern bool W906_MsgBoxModelessAnswer(const webbridge::WebCommand&); extern bool W906_SimDiCommand(const webbridge::WebCommand&); if (!W906_MsgBoxModelessAnswer(wc) && !W906_SimDiCommand(wc)) g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "modal-pending"); }   /*AI(W906-SMM) 20260925: 告警框開著時，網頁上不停機的 MyMessageBox 仍按得掉（dialog-bridge.js 不停機那一層在最上層、可單獨按）—— C++ 這側也要收，否則網頁關了、C++ 的 fShow／iUnLoaderCount 還留著（檔尾 W906_MsgBoxModelessAnswer）*/   /*AI(W906-W4D) 20260925: NB2 R23 §2 -- motor.stop answered even under a modal alarm: STOP never starts motion, golden keeps the physical STOP live during a modal, and for 1203 axes opened by the EastSun monitor this is the only stop there is (motor.stop is locked to action=stop)*/   /*AI(W906-J5-ACK) 20260930: dialog.notifyAck while this blocking alarm waits = not-a-notice: this alarm owns the mailbox (a notice under it, kShown==0, is retired by this alarm's DialogMailboxRetire) -- nothing retired, nothing applied, answered at once*/
            }
        }
        // ===== AI(W906-SMM-IO) 20260925 BEGIN -- 框開著時的實體面板鍵（RULINGS_20260925 第 42 條）=====
        //   golden fNote 在 ShowModal 期間由 Timer1Timer（note.cpp:3172 起）→ ScanKey（:2894-3121）掃面板：
        //   K_RETRY／K_SKIP… 先選取，再按 K_PAUSE（BtnPauseClick :3866）或 K_START（Start :3560）確認。
        //   解除時與網頁回答走同一個出口（下面幾行照抄上面 answered 分支），另寫 Dialog-close-request 讓網頁收框。
        {
            extern int W906_AlarmIoAnswer(const char*, int, std::string*, std::string*, std::string*);
            extern void W906_DialogCloseRequest(const char*, const std::string&, unsigned long long, const std::string&, int, const char*);
            extern unsigned long long g_dialogAlarmSeq;
            std::string ioAns, ioPressed, ioInput;
            const int kio = W906_AlarmIoAnswer(qidStr, kcode, &ioAns, &ioPressed, &ioInput);
            if (kio > 0 && (kio & kcode) != 0) {
                g_modalServer->ClearQuery(qid);
                DialogMailboxRetire();
                W906_DialogCloseRequest("show-error-message", qidStr, g_dialogAlarmSeq, ioAns, kio, ioInput.c_str());
                ht9045::sjson::ClearAlarm(qidStr, ioAns, ioPressed, kio);
                std::printf("query qid=%s answered %s (K=%d) via panel IO %s pressed=%s\n",
                            qidStr, ioAns.c_str(), kio, ioInput.c_str(), ioPressed.c_str());
                if (ioPressed == "BtnPause") { SoftStop = true; SoftStart = false; }        // golden note.cpp:3981-3982（同上面網頁那條）
                { extern void W906_NoteJamCountOnClose(int); W906_NoteJamCountOnClose(kio); }  /* AI(W906-J2) 20260926: 同上面網頁那個出口 */  if (ioPressed == "BtnStart") { extern bool W906_AlarmAnswerStartLikeGolden(const char*); W906_AlarmAnswerStartLikeGolden(qidStr); }
                { extern void W906_NoteFormCloseAlarmClear(); W906_NoteFormCloseAlarmClear(); }  return kio;   // AI(W906-HALARM-CLOSE) 20260926: 面板鍵回答也是關框，同上（golden note.cpp:2531）
            }
        }
        // ===== AI(W906-SMM-IO) 20260925 END =====
    }
}

// ===========================================================================
//  AI(W906-YESNO) 20260925：ShowMyMessageBox_YES_NO -> 網頁是／否對話框，答案回流。
//
//  使用者 20260925 裁決（docs/RULINGS_20260925.md 第 10 條，3A）：「YES/NO 對話框被
//  替身自動回答 → 照 golden 跳網頁對話框，等操作員回答」。與同日第 2 條（教導頁的阻塞
//  告警框照 golden 跳框並等待，即使單執行緒 tick 迴圈停住）同方向。
//
//  ## 沒有發明新傳輸 —— 與 ForwardShowErrorMessage 同一條路
//    請求：WebSocket query 訊框（PostQueryOptions，options=["YES","NO"]）
//          ＋ 檔案信箱 Message-dialog-request（同事 dialog-bridge.js 的 show-my-message
//            通道，畫在 page/Alert.MyMessageBox.html —— 那頁本來就有 pnlYes/pnlNo）
//    回應：modal.answer／dialog.response（tag=qid，value="YES"|"NO"；帶 ":<按鍵>" 也接受），
//          WebBridgeServer 的 socket 執行緒把它放進 CommandQueue（權杖豁免，
//          WebBridgeServer.cpp 的 AI(W906-ALARM-ANSWER-TOKEN)），這裡的迴圈自己 drain。
//    ⇒ 瀏覽器端只多了 dialog-page.js 的兩顆鍵（pnlYes/pnlNo）綁定；
//      ht9045_dialog_host.js、dialog-bridge.js、WebBridgeServer 的應答路徑都沒改。
//
//  ## golden 的 YES/NO **會停機** —— 照做
//    golden 本體 mymessbox.cpp:1020 `StopAllMotor();`，ShowModal 觸發的 FormShow
//    :302-310 `if(!iUnLoaderCount){ SystemStart=false; SoftStart=false; ... StopAllMotor(); }`。
//    這一點與告警（fNote）一樣。golden PowerSavingMode.cpp:156-158 的註解也佐證：
//    「不能用ShowErrorMessage，因為retry->Start，機台就跑起來了」—— YES/NO 答完**不會**
//    自己重新啟動，操作員要再按 START。見檔尾 W906_YesNoShowLikeGolden／W906_YesNoCloseLikeGolden。
//
//  ## 等待期間答案進得來嗎（死鎖）
//    與 ForwardShowErrorMessage 完全同構：socket 執行緒獨立於本執行緒收 WebSocket 並
//    push 進 CommandQueue（有鎖），本迴圈每 100 ms drain 一次；modal.answer／dialog.response
//    不需要權杖。其他指令一律回 "modal-pending"（cfg.resync 與 motor.stop 例外，理由同上面那支）。
//    沒有逾時 —— golden 的 ShowModal 也是無限等。
// ===========================================================================

// AI(W906-YESNO) 20260925: 寫一份 YES/NO 請求進 show-my-message 信箱。
//   JSON 由 tools/wb_dialog_mailbox.h 的 w906dlg::YesNoRequestJson 組（欄位與理由寫在那裡；
//   放在標頭是為了讓 tests/test_yesno_dialog.cpp 用 cJSON 驗它是合法 JSON）。
//   S2 以第一個 ';' 切成 lblChineseMsg／lblSubMsg（golden mymessbox.cpp:1032-1047）也在那裡。
static bool DialogMailboxPostYesNo(const std::string& requestId,
                                   const char* s1, const char* s2, const char* s3)
{
    if (g_dialogMailboxDir.empty()) return false;
    const unsigned long long seq = ++g_dialogSeq;
    const std::string json = w906dlg::YesNoRequestJson(seq, requestId,
        s1 ? s1 : "", s2 ? s2 : "", s3 ? s3 : "",
        SystemInitialOK == true,                 // runtime.systemInitialOK（樣本欄位，照填）
        iUnLoaderCount == 0);                    // requestedSideEffects.pauseHandler：golden FormShow :302 `if(!iUnLoaderCount)` 那一臂
    return w906dlg::MailboxPut(g_dialogMailboxDir, "Message-dialog-request", json);
}

// AI(W906-YESNO) 20260925: 把 show-my-message 請求退役成 idle —— 理由同 DialogMailboxRetire
//   （不做的話答完按 F5 會再彈一次）。內容＝版控種子只換掉 seq（w906dlg::MessageIdleJson）。
static void DialogMailboxRetireMessage()
{
    if (g_dialogMailboxDir.empty()) return;
    const std::string json = w906dlg::MessageIdleJson(++g_dialogSeq);
    if (json.empty()) {
        std::printf("  ⚠ Message 信箱退役：種子裡找不到 \"seq\":0 —— 不寫出去，寧可不退役也不要寫壞檔\n");
        return;
    }
    if (!w906dlg::MailboxPut(g_dialogMailboxDir, "Message-dialog-request", json))
        std::printf("  ⚠ Message 信箱退役失敗 -- 下次重整會再彈一次\n");
}

// AI(W906-YESNO) 20260925: 是／否的答案字串 -> golden iValue（pnlYes Tag=1、pnlNo Tag=2，
//   golden mymessbox.dfm:160／:141）。其他一律 0（＝不是這一題提供的選項）。
static int YesNoValueOf(const std::string& ans)
{
    if (ans == "YES") return 1;
    if (ans == "NO")  return 2;
    return 0;
}

static int ForwardShowMyMessageBoxYesNo(const char* s1, const char* s2, const char* s3)
{
    // 沒有網頁可以問（伺服器還沒起來 —— 例如開機序列裡 cinitial.cpp 的四個呼叫 —— 或已關站）：
    //   回 0 ＝「沒有人回答」，canary_support.cpp 改回 SimReturn（預設 0，替身時代的值）。
    //   ⚠ 這條路刻意**不停機**：沒問就不該有 golden「框開著」的副作用，行為與 20260925 前逐位元相同。
    if (!g_modalServer || !g_pumpQueue) return 0;

    // golden mymessbox.cpp:1012-1015：已有 MyMessageBox 開著（且不是不停機那種）就回 3、不再開第二個。
    //   移植樹的等待迴圈佔住唯一的 tick 執行緒，迴圈裡不呼叫任何狀態機碼，所以今天不可能重入；
    //   守衛照翻，萬一日後有人在迴圈裡加了會跳框的呼叫，結果仍與 golden 同。
    //   ⓘ golden :1016-1019（不停機的 MyMessageBox 開著時先 Close 再開）[AI(W906-MODAL-WAKE) 20260926：現在照翻 —— ShowUnloaderTrayMessage 的
    //     非阻塞框佔 fShow（MbFormShow），下一行之後的 W906_YesNoPreCloseLikeGolden 先關它，會停機的那種回 3（檔尾）]
    static bool s_showing = false;
    if (s_showing) return 3;  if (W906_YesNoPreCloseLikeGolden() == 3) return 3;   // AI(W906-MODAL-WAKE) 20260926: golden :1012-1019（見上兩行）
    s_showing = true;

    { extern void W906_YesNoShowLikeGolden(); W906_YesNoShowLikeGolden(); }   // golden :1020 StopAllMotor ＋ FormShow :70／:302-310／:353（:1049-1053 缺相依，見 SAFETY-GATE(W906-YESNO-FTCT)）—— 本體在檔尾

    const unsigned long long qid = g_nextQid++;
    char qidStr[24];
    std::snprintf(qidStr, sizeof(qidStr), "%llu", qid);

    std::vector<std::string> options;
    options.push_back("YES");
    options.push_back("NO");
    g_modalServer->PostQueryOptions(qid, "yes-no", s1 ? s1 : "", options);
    const bool posted = DialogMailboxPostYesNo(qidStr, s1, s2, s3);
    if (!posted && !g_dialogMailboxDir.empty())
        std::printf("  ⚠ Message 信箱寫入失敗 —— 只剩 WebSocket 那條路\n");
    std::printf("yesno qid=%s \"%s\" -- waiting (mailbox=%s, ws=yes)\n",
                qidStr, s1 ? s1 : "", posted ? "yes" : "no");
    std::fflush(stdout);

    WdMark2("yes/no wait (blocking, needs a browser answer): ", s1 ? s1 : "");
    std::vector<webbridge::WebCommand> local;  W906ModalWaitScope wakeScope(1);   // AI(W906-MODAL-WAKE) 20260926: golden TMyMessageBox::FormShow :336／:344、FormClose :408（檔尾）
    for (;;) {
        if (g_carry.empty()) g_pumpQueue->waitForPush(100);  { extern void DoAvoidIndexMotorFallDown(); DoAvoidIndexMotorFallDown(); }  W906_ModalWaitTick(1);  { extern void W906_TesterCommPoll(); W906_TesterCommPoll(); }  /* AI(W906-GB-P3) 20260926: H5 -- golden ShowModal keeps handling WM_COPYDATA from the bridge */   //AI(W906-MODAL-SAFETY) 20260925: golden 在 ShowModal 期間 MyMessageBox::Timer1Timer（mymessbox.cpp:538-550／:650）照跑 DoAvoidIndexMotorFallDown（EMG／斷電／Index Z servo off ⇒ 鎖 Index 煞車並停機）；移植樹等待期間 tick 停住，這裡補跑（R-YESNO 審查 high；本體不跳框、SOFT_SIMULTE 下是 no-op）  //AI(W906-MERGE-56bbf785) 20260926: laptop wrote Sleep(100) here; machine P25c replaced that exact Sleep in ForwardShowErrorMessage with this event wait (an answer / motor.stop wakes it at once). This wait is the same shape, so it gets the same line
        local.clear();
        W906_TakeCarry(local);  g_pumpQueue->drain(local);   //AI(W906-MERGE-56bbf785) 20260926: laptop drained only the queue; machine IOWEB-P25 requires EVERY drain (main loop and each modal wait) to take g_carry first, in arrival order -- otherwise a motor.stop the output-first service parked in g_carry (behind a barrier) would wait until this dialog is answered
        for (size_t i = 0; i < local.size(); ++i) {
            const webbridge::WebCommand& wc = local[i];
            if ((wc.cmd == "modal.answer" || wc.cmd == "dialog.response")
                && wc.hasTag && wc.tag == qidStr) {
                std::string ans = (wc.hasValue && wc.value.isString())
                                  ? wc.value.asString() : std::string();
                std::string pressed;
                const std::string::size_type colon = ans.find(':');   // 今天送來的是純 "YES"/"NO"：dialog-bridge.js responseFor() 只替告警通道帶 pressedButton，訊息通道沒有；"YES:pnlYes" 形式照告警那支一併接受（20260925 無頭 Edge 自測實測）
                if (colon != std::string::npos) {
                    pressed = ans.substr(colon + 1);
                    ans     = ans.substr(0, colon);
                }
                const int v = YesNoValueOf(ans);
                if (v != 0) {
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, true, std::string());
                    g_modalServer->ClearQuery(qid);        // 不清：下一個連上來的瀏覽器會收到早答完的框
                    DialogMailboxRetireMessage();          // 不退役：答完按 F5 同一題再彈一次
                    if (i + 1 < local.size()) { g_carry.insert(g_carry.begin(), local.begin() + (i + 1), local.end()); g_carryRunnable = true; }  { extern void W906_YesNoCloseLikeGolden(const char*, const char*, int); W906_YesNoCloseLikeGolden(s1, s3, v); }   // golden FormClose :385-442 ＋ :1056-1061 MyDBIProcess —— 本體在檔尾  //AI(W906-MERGE-56bbf785) 20260926: same fix as the ShowErrorMessage wait's answer (:627) -- local[i+1..] was dropped at the `return v;` below (never run, never acked); it goes back to the front of g_carry in arrival order, before the golden close side effects. Same line, so no line below moves
                    s_showing = false;
                    std::printf("yesno qid=%s answered %s (iValue=%d) via %s%s%s\n",
                                qidStr, ans.c_str(), v, wc.cmd.c_str(),
                                pressed.empty() ? "" : " pressed=", pressed.c_str());
                    std::fflush(stdout);
                    return v;
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                               "not an offered option");
            } else if (wc.cmd == "cfg.resync") {
                // 同 ForwardShowErrorMessage：純查詢、不碰機台，擋住反而讓重整後的頁面畫不出框。
                unsigned long long since = 0;
                if (wc.hasValue) {
                    if (wc.value.isInt())
                        since = (unsigned long long)wc.value.asInt(0);
                    else if (wc.value.isDouble())
                        since = (unsigned long long)wc.value.asDouble(0.0);
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true,
                                               ht9045::sjson::ConfigResyncJson(since));
            } else if (wc.cmd == "ui.windows.put") { std::string whyW; const std::string frameW = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); if (!frameW.empty() && ht9045::WebWindowRegistryPut(wc.connId, frameW, whyW)) g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"accepted\":true,\"duringModal\":true}"); else g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "ui.windows.put: " + (frameW.empty() ? std::string("value must be the frame JSON string") : whyW));   //AI(W906-MERGE-56bbf785) 20260926: machine MT-FIX1 accepts ui.windows.put during the ShowErrorMessage wait (a wait over 15 s lets the window registry go stale -> fMotorTest/fTeach read "open" -> MainProc stays paused after the answer); laptop's YES/NO wait says its exceptions are "理由同上面那支", so it takes the same one. Same body as that branch, on one line
            } else if (wc.cmd == "motor.stop") {
                // 同 ForwardShowErrorMessage（AI(W906-W4D) 20260925）：STOP 永遠放行。
                extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool);
                std::string msAck;
                const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(),
                                                       (long long)wc.id, msAck, true);
                g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck);
            } else if (W906_SimDiCommand(wc)) { } else if (wc.cmd == "io.btnPanelClick" && []() { extern bool W906_IoPageNoGuards(); return W906_IoPageNoGuards(); }()) { extern void W906_DispatchIoClick(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_DispatchIoClick(*g_modalServer, wc); } else {   /*AI(W906-IO-NOGUARD) 20260929: IO click served during the YesNo dialog too*/   // AI(W906-R70) 20260926 YN-3：是／否框也收模擬 DI（另外兩種框 :671／:6782 都收；同一行寫完，行數不變），SOFT_SIMULTE 下才驗得到第 26 條 Q3 的「面板 Alarm Reset 消音」
                g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "modal-pending");
            }
        }
    }
}

// ===========================================================================
//  AI(W906-P6b-C) 20260921：視窗狀態總表 -> Command.cpp 的 Bit4_HandlerDiagnostics
//
//  `Command.cpp:15027`（golden :7348-7360）原本整段 `#if 0`，現在改走
//  `W906_DiagnosticsWindowOpen_Hook`（宣告在 `canary_support.h`）。
//  這裡是那個 hook 的唯一安裝點 —— 與上面兩個 `W906_*_Hook` 同一個慣例。
//
//  ## 為什麼第三個參數在這裡算，不在 WebWindowRegistry.cpp 裡算
//
//  `WebWindowRegistry.cpp` 必須能只跟 cJSON 一起連結（`tests/CMakeLists.txt:2970`
//  的測試目標就是這樣連的）。在它裡面 `#include "forms/fHome.h"` 會把整個
//  vclcompat 拖進去，那支測試就連不起來了。⇒ 佐證訊號由宿主提供。
//
//  ## `fHome->iHomeStep != 1` 是什麼意思
//
//  `forms/fHome.h:131` 的 `int iHomeStep`：建構子設 1（＝閒置），
//  `uhome.cpp` 有 135 個 `iHomeStep=` 賦值推著它跑完 1→…→1600 的歸零圖。
//  ⇒ `!= 1` ＝「機台正在回原點」。理由（為什麼 `fHome` 需要這個佐證而其他三個
//     不需要）寫在 `WebWindowRegistry.h` 的參數說明，這裡不重複。
//
//  ⚠ `fHome` 由 `forms/fHome.cpp:37` 靜態初始化（`new TfHome()`），非 NULL；
//    仍然守一次，因為這支會在 SIOF 之後很久才被呼叫但守衛是免費的。
// ===========================================================================
static bool ForwardDiagnosticsWindowOpen(bool systemStart, int contactMode)
{
    const bool homingActive = (fHome != 0) && (fHome->iHomeStep != 1);
    extern bool W906_PageDiagnosticsOpen(bool systemStart, int contactMode, bool homingActive);   //AI(W906-PAGETAB-Q51) 20260928 [W906] 設定中位元改頁面表版（WebPageTable.cpp PageDiagnosticsOpen）：
    //   三層清單與順序照舊（WebWindowRegistryTierForms），每個表單改問頁面表規則 1～7；原本 ht9045::WebWindowRegistryDiagnosticsOpen（過期＝開著）。三行換三行，行號不動
    return W906_PageDiagnosticsOpen(systemStart, contactMode, homingActive);
}

// ===========================================================================
//  GET /api/recipe/<doc>   -- one recipe document as the browser's JSON shape
//  GET /api/recipe/        -- the list of documents in the active recipe
//
//  AI(W906-FW-C1WIRE) 20260911.
//
//  WHY A ROUTE AND NOT A TAG. The web author's pages already fetch their data:
//  dialog-bridge.js does fetch("JSON/<name>.json?_=<ts>", {cache:'no-store'}).
//  Serving a document at a URL means that existing pattern works unchanged --
//  no new tag, no 64 KB WS message cap (WebBridgeServer.cpp:249), and no 5 KB
//  string re-diffed into every tag patch.
//
//  ---------------------------------------------------------------------------
//  PATH TRAVERSAL: the document name comes off a URL, so it is never used to
//  build a path. The recipe directory is ENUMERATED and the name is matched
//  against what is actually there. "../../../windows/win.ini" cannot match a
//  directory entry, so the traversal is impossible by construction rather than
//  by a blacklist -- the same reasoning as using TIniFile for the read.
//
//  The name is ALSO charset-checked first, which is belt to that braces: it
//  keeps a hostile name out of the enumeration loop entirely.
//  ---------------------------------------------------------------------------
namespace {

// The web author's document keys are the filename with its first letter
// lowercased and the extension dropped -- verified against all 16 documents he
// published (contact <- Contact.Data, udUld <- UdUld.Data, configByRecipe <-
// configByRecipe.ini). Rather than invert that transform (which would have to
// guess at capitalisation) the match is case-insensitive on the stem.
bool StemFoldEq(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (std::string::size_type i = 0; i < a.size(); ++i) {
        unsigned char x = (unsigned char)a[i], y = (unsigned char)b[i];
        if (x >= 'A' && x <= 'Z') x = (unsigned char)(x - 'A' + 'a');
        if (y >= 'A' && y <= 'Z') y = (unsigned char)(y - 'A' + 'a');
        if (x != y) return false;
    }
    return true;
}

bool SafeDocName(const std::string& n) {
    if (n.empty() || n.size() > 64) return false;
    for (std::string::size_type i = 0; i < n.size(); ++i) {
        const char c = n[i];
        const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                        (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
        if (!ok) return false;
    }
    return n.find("..") == std::string::npos;
}

// AI(W906-WEB-W2b) 20260917 (裁決 A1)：改採 web 同事的單參數版本。
// 資料夾一律是**真實**配方夾，不是 --dry 造出來的 scratch —— 他量到的缺陷是
// 「瀏覽器存檔進 scratch、回報成功、伺服器一停就消失」，而同一頁的 teach.ini
// 卻寫進真實檔。同一頁兩種行為，且「成功」是假的。
// 目錄本身來自 golden 自己的 GetRecipePath()（我們樹上 common.cpp:2466 ->
// DataPath + GetLastOpenFN()），所以「哪一個配方」由 setup.inf 決定，這裡不決定。
static std::string RealRecipeDir();   //Steven 20260916 (R1): defined with the other --dry helpers below

void EnumRecipeDocs(std::vector<std::pair<std::string, std::string> >* out) {
    const std::string base(RealRecipeDir());   //Steven 20260916 (R1): real folder, not the --dry scratch
    static const char* kPats[2] = { "*.Data", "*.ini" };
    for (int p = 0; p < 2; ++p) {
        WIN32_FIND_DATAA fd;
        const std::string pat = base + kPats[p];
        HANDLE h = ::FindFirstFileA(pat.c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) continue;
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            std::string name(fd.cFileName);
            const std::string::size_type dot = name.rfind('.');
            const std::string stem = (dot == std::string::npos) ? name : name.substr(0, dot);
            if (stem.empty()) continue;
            out->push_back(std::make_pair(stem, base + name));
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
}

// A document is capped rather than truncated. A truncated JSON body would make
// the browser drop the whole response and look like a server fault; 413 with a
// readable reason is a fact the operator can act on. The bin-assignment
// documents are ~300 KB of INI and would expand past this -- they are not
// form-editable anyway, and finding that out from a status code beats finding
// it out from a blank screen.
const std::size_t kMaxDocBytes = 2u * 1024u * 1024u;

//AI(W906-WEB-W2a) 20260917: taken verbatim from the web colleague's delivery
// (HT9045_V906_changes_20260916b, his wb_serve.cpp:941-962), because it is
// strictly STRICTER than the one-liner it replaces, in three ways that each
// close a real hole:
//
//   1. A rawValue containing CR or LF would have been written straight into an
//      ini file, where it becomes a new line and therefore a new (bogus) key.
//      Those edits are now dropped and counted as notFound.
//   2. It ALWAYS does the dry pass first, even when about to apply. So the
//      decision to write is made on measured counts, not on hope.
//   3. `notFound > 0` REFUSES the whole apply and sets ok=false. This is the
//      browser's own rule 2 ("a notFound means the wiring table is wrong, so
//      refuse the whole page rather than write part of it") enforced on the
//      SERVER -- and rule 2 had never actually run, because the ack dropped the
//      counts it keys on (the defect WEB-W1 just fixed). His note calls this
//      "refused apply must not look like success", which is exactly the failure
//      class this repo keeps paying for.
//
// `dry` still short-circuits before any write, so --dry is unchanged.
static ht9045::RecipeWriteResult ApplyIniGuarded(const vclcompat::AnsiString& path,
                                                 const std::vector<ht9045::RecipeFieldEdit>& in,
                                                 bool dry) {
    std::vector<ht9045::RecipeFieldEdit> edits;
    int rejected = 0;
    for (std::size_t i = 0; i < in.size(); ++i) {
        const std::string rv(in[i].rawValue.c_str());
        if (rv.find_first_of("\r\n") != std::string::npos) ++rejected;
        else edits.push_back(in[i]);
    }
    ht9045::RecipeWriteResult r;
    r.ok = true; r.changed = r.identical = r.notFound = 0;
    if (!edits.empty()) r = ht9045::RecipeDocApplyEdits(path, edits, ht9045::kRecipeWriteDryRun);
    r.notFound += rejected;
    if (dry || !r.ok || r.changed == 0) return r;
    if (r.notFound > 0) {                          //Steven 20260916 (R2): refused apply must not look like success
        r.ok = false; r.changed = 0;
        r.error = "refused: " + std::to_string(r.notFound) + " key(s) notFound/rejected, nothing written";
        return r;
    }
    return ht9045::RecipeDocApplyEdits(path, edits, ht9045::kRecipeWriteApply);
}

bool RecipeRoute(void* /*user*/, const std::string& method,
                 const std::string& path, const std::string& /*query*/,
                 webbridge::HttpResponse* out)
{
    if (method != "GET" && method != "HEAD") {
        out->status = 405; out->reason = "Method Not Allowed";
        out->extraHeaders["Allow"] = "GET, HEAD";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "405 recipe documents are read-only over HTTP; write with the "
                    "recipe.doc.put command\n";
        return true;
    }

    std::string name;
    if (path.size() > 12 /*strlen("/api/recipe/")*/)
        name = path.substr(12);
    else if (path != "/api/recipe" && path != "/api/recipe/")
        return false;                       // not ours -> fall through to files

    std::vector<std::pair<std::string, std::string> > docs;
    EnumRecipeDocs(&docs);

    if (name.empty()) {
        // The index. Useful to the web author: it names every document the
        // active recipe actually has, so his UI does not have to hardcode 16
        // filenames that vary by machine type.
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("recipe").String(std::string(GetLastOpenFN().c_str()));
        w.Key("path").String(RealRecipeDir());
        w.Key("documents").BeginArray();
        for (std::size_t i = 0; i < docs.size(); ++i) {
            // first letter lowercased -- the web author's own key convention
            std::string k = docs[i].first;
            if (!k.empty() && k[0] >= 'A' && k[0] <= 'Z') k[0] = (char)(k[0] - 'A' + 'a');
            w.String(k);
        }
        w.EndArray();
        w.EndObject();
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = w.Ok() ? w.Str() : std::string("{}");
        out->headOnly = (method == "HEAD");
        if (out->headOnly) {
            out->contentLength = (long long)out->body.size();
            out->body.clear();
        }
        return true;
    }

    if (!SafeDocName(name)) {
        out->status = 400; out->reason = "Bad Request";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "400 document name rejected\n";
        return true;
    }

    for (std::size_t i = 0; i < docs.size(); ++i) {
        if (!StemFoldEq(docs[i].first, name)) continue;
        const std::string json =
            ht9045::RecipeDocToJson(AnsiString(docs[i].second.c_str()));
        if (json.size() > kMaxDocBytes) {
            out->status = 413; out->reason = "Payload Too Large";
            out->contentType = "text/plain; charset=utf-8";
            char msg[192];
            std::snprintf(msg, sizeof(msg),
                          "413 %s renders to %lu bytes of JSON, over the %lu byte "
                          "budget; not truncated on purpose\n",
                          name.c_str(), (unsigned long)json.size(),
                          (unsigned long)kMaxDocBytes);
            out->body = msg;
            return true;
        }
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = json;
        out->headOnly = (method == "HEAD");
        if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
        return true;
    }

    out->status = 404; out->reason = "Not Found";
    out->contentType = "text/plain; charset=utf-8";
    out->body = "404 no such document in the active recipe\n";
    return true;
}


// ---------------------------------------------------------------------------
//  AI(W906-FW-SYSFILE) 20260915: the five machine-config files the browser needs.
//
//      GET  /api/system/            index of the four
//      GET  /api/system/<name>      gerneral | teach  -> the SAME sections/raw
//                                   shape a recipe document returns
//                                   motTable | ioTable -> {columns, keyColumn, rows}
//      WS   system.file.put         write, same dryRun-first contract as
//                                   recipe.doc.put
//
//  WHY A FIXED TABLE AND NOT A DIRECTORY SCAN
//  These are not recipe documents -- `system\` is shared PRODUCTION runtime
//  configuration (AGENTS.md hard boundary). A scan would expose every ini in
//  that directory the moment someone dropped one there. Four named entries
//  cannot grow by accident, and path traversal is impossible by construction.
//
//  PATHS COME FROM GOLDEN'S OWN GLOBALS, never string literals here:
//      asGeneralPath   common.cpp:89     <- --dry repoints this one, so --dry
//                                           keeps working without a special case
//      asTeachPath     common.cpp:239
//      IoTablePath     common.cpp:166
//      MotTablePath    common.cpp:167
//
//  WRITE GATE
//  --allow-cmd alone is NOT enough. system\ writes additionally need
//  --allow-system-write. AGENTS.md: "共用量產執行期參數。預設唯讀。要寫必須先
//  備份，且要人明確同意" -- the extra flag IS that explicit consent, and the
//  backup is taken by the writer itself.
//
//  teach.ini specifically: forms/fTeach.h records that this project has already
//  lost teach data once to a write path that failed SILENTLY. So the ack always
//  carries changed/identical/notFound and the browser must re-GET; a write that
//  matched nothing reports notFound rather than succeeding quietly.
// ---------------------------------------------------------------------------
struct SysFileEntry {
    const char*              key;     // browser-facing name
    const vclcompat::AnsiString* path; // golden global, read at call time
    const char*              suffix;  // non-null -> *path is a DIRECTORY, append this
    bool                     csv;     // false = ini (reuse the recipe machinery)
};

// AI(W906-FW-SYSFILE) 20260915: golden 對 Error\ 沒有路徑全域 —— 它自己就是
// 寫死 "D:\HT9045\Error\AlarmDescription.ini"。沒有全域可取，就只能在這裡
// 放一個常數，並且說清楚它為什麼是例外。
const vclcompat::AnsiString& ErrorDirPath() {
    static const vclcompat::AnsiString kErrDir("D:\\HT9045\\Error\\");
    return kErrDir;
}

const SysFileEntry* SysFileTable(std::size_t* n) {
    // AI(W906-FW-SYSFILE) 20260915: 全部機台設定檔。表格由
    // scratchpad/gen_sysfile_table.py 產生 —— 30+ 筆手打會錯，而且路徑一律要取
    // golden 的全域而不是字面字串。
    //
    // 為什麼仍然是固定表而不是掃目錄：system\ 與 config\ 底下滿是備份變體
    // （Gerneral_old.ini / IO_Table_.csv / config_ORG.ini / Security_new_12.def …）。
    // 掃描會把那些全部暴露出去，而且哪一個是「程式真的在用的」無從判斷。
    // 有專屬路徑全域，就是 golden 自己標記的「用這個」。
    //
    // 二進位一律不列（lastdata.dat / login.dat / levelset.dat / tech.dat /
    // MDB\Handler.db3）：ini/csv 機制碰不到，硬接只會產生看起來對的假資料。
    // 那些要等 C++ 端依 struct 逐欄位輸出投影。
    static const SysFileEntry kTab[40] = {
        { "gerneral",    &asGeneralPath,                 0,                       false },
        { "teach",       &asTeachPath,                   0,                       false },
        { "motTable",    &MotTablePath,                  0,                       true  },
        { "ioTable",     &IoTablePath,                   0,                       true  },
        { "config",      &AuthPath,                      "config.ini",            false },
        { "dio",         0,              0,            false },
        { "lastSet",     &AuthPath,                      "LastSet.ini",           false },
        { "errNote",     &asErrNotePath,                 0,                       false },
        { "description", &ConfigMemoPath,                0,                       false },
        { "setupInf",    &LastDataPath,                  0,                       false },
        { "trayForm",    &TrayTablePath,                 0,                       true  },
        { "plateForm",   &PlateTablePath,                0,                       true  },
        { "trayStepSpeed", &asTrayStepSpeedByMachinePatch, 0,                       false },
        { "machineLife", &asMachineLifePath,             0,                       false },
        { "arms",        &asARSMParaPath,                0,                       false },
        { "secsGem",     &SecsGemPath,                   0,                       false },
        { "contactInfo", &asSystemPath,                  "ContactInfo.ini",       false },
        { "autoTemp",    &asSystemPath,                  "AutoTemperature.ini",   false },
        { "atcSystem",   &asSystemPath,                  "ATC.ini",               false },
        { "barcode",     &asSystemPath,                  "Barcode.ini",           false },
        { "padInterface", &asSystemPath,                  "PadInterfacePara.ini",  false },
        { "eventLogLevel", &asSystemPath,                  "EvenLogLevel.ini",      false },
        // 稽核表 20260909 逐檔查證為 IniData\；system\ 底下另有一份 390 bytes 的同名檔，程式讀的是 IniData 這份。
        { "socketCount", &DefaultPath,                   "SocketCount.ini",       false },
        { "motorTest",   &asSystemPath,                  "MotorTest.ini",         false },
        { "colorSensor", &asSystemPath,                  "ColorSensorType.ini",   false },
        { "mvData",      &asSystemPath,                  "MVData.ini",            false },
        // golden 寫死 "D:\\HT9045\\IniData\\RPDefault.ini"
        { "rpDefault",   &DefaultPath,                   "RPDefault.ini",         false },
        // golden 對 Error 目錄沒有路徑全域，自己寫死字面路徑，故這兩筆用 ErrorDirPath()
        { "alarmDesc",   &ErrorDirPath(),                "AlarmDescription.ini",  false },
        { "alarmCodeList", &ErrorDirPath(),                "AlarmCodeList.txt",     false },
        { "securityNew", &AuthPath,                      "Security_new.def",      false },
        { "criticalPara", &AuthPath,                      "CriticalParaControl.ini", false },
        { "esdConfig",   &AuthPath,                      "ESDconfig.ini",         false },
        { "atcConfig",   &AuthPath,                      "ATC.ini",               false },
        { "pmMonth",     &sPMList_Month,                 0,                       false },
        { "pmQuarter",   &sPMList_Quarter,               0,                       false },
        { "pmYear",      &sPMList_Year,                  0,                       false },
        { "pmTemperature", &sPMList_Temperature,           0,                       false },
        { "pmEsd",       &sPMList_ESD,                   0,                       false },
        { "pmIonFan",    &sPMList_IonFan,                0,                       false },
        { "pmSetting",   &sPMSetting,                    0,                       false }
    };
    *n = 40;
    return kTab;
}

// Resolve an entry to its full path. Kept in one place so the route and the
// command cannot disagree about what "config" means.
// 前置宣告：SysFilePath 需要它，但它的實作要用到後面才定義的輔助函式。
std::string ResolveDioPath();
struct DioResolved;
void EnumDioProfiles(std::vector<std::string>* out);

//Steven 20260916
// --dry swaps asGeneralPath (and DataPath) to a %TEMP% scratch copy so that
// LoadMachineConfig()'s key-seeding cannot rewrite the real file, and deletes the
// scratch on exit. That is right for the LOADER and wrong for the WEB API: with
// the package launcher's default `--dry --allow-cmd`, every browser save of a
// Gerneral.ini field (HW.HandlerSys, 214 of them) went to the scratch copy,
// acked ok/changed=1, re-read "fine" -- and vanished when the server stopped,
// while the same page's teach.ini fields went to the real file. Found by review
// (A1). The web read/write path therefore resolves to the REAL file always;
// the loader keeps its scratch. The write gate (--allow-system-write) is
// unchanged and remains the explicit consent.
static std::string gRealGeneralPath;   // set in main() before the --dry swap
static std::string gRealDataPath;      // same, for the recipe folder

//Steven 20260916 (review round 2, R1)
// The recipe folder the web API works on. golden's GetRecipePath() is
// DataPath + GetLastOpenFN() + "\" and DataPath is the thing --dry swaps to
// %TEMP%\wb_serve_recipe\, so every /api/recipe read and recipe.doc.put went to
// the scratch copy while the startup NOTE claimed otherwise. Same fix shape as
// gerneral: the real folder, captured before the swap.
static std::string RealRecipeDir() {
    // Steven 20260916 (review round 2 close-out): a replace_all had turned this
    // fallback into a call to itself -> stack overflow if gRealDataPath were ever
    // empty. Unreachable today (main() sets it before the server starts) but it
    // is a crash, not an error, so it goes.
    if (gRealDataPath.empty()) return std::string(GetRecipePath().c_str());
    std::string p = gRealDataPath + std::string(GetLastOpenFN().c_str());
    if (p.empty() || p[p.size() - 1] != '\\') p += '\\';
    return p;
}

//Steven 20260916 (review round 2, R6)
// Server-side shape check for a NEW row's key. motTable keys must be "M%02d" --
// golden maps them to motor enums that way (cinitial.cpp:3666); anything else is
// a dead row the machine never reads. The browser checks this too, but a direct
// WebSocket client can skip the browser. Mini pattern: 'M' literal, '#' = digit.
const char* CsvKeyPattern(const SysFileEntry* e) {
    if (e && e->csv && std::string(e->key) == "motTable") return "M##";
    return 0;
}
static bool KeyMatches(const std::string& k, const char* pat) {
    if (!pat) return true;
    const std::string p(pat);
    if (k.size() != p.size()) return false;
    for (std::size_t i = 0; i < p.size(); ++i) {
        if (p[i] == '#') { if (k[i] < '0' || k[i] > '9') return false; }
        else if (k[i] != p[i]) return false;
    }
    return true;
}
static const char* CRouteOwnerDio(const std::string& fullPath);   //AI(W906-FRW-Q3) 20260927: DIO 動態檔名的 C 路擁有者（CRouteOwner 用；本體在檔尾；RULINGS_20260926 S125，todo ★ Q3／F-005）
//Steven 20260924（C 路，檔案擁有者閘）：一個檔只能有一個寫者。
//   C 路（golden 表單橋，FileRW/<結構>.cpp）接手的檔，B 路檔案鏡像（system.file.put／recipe.doc.put）不可再直接改：
//   B 路改檔不會讓記憶體重讀，下一次 golden 存檔（例 SaveLastSetIni）會把舊值寫回去 —— 頁面的修改無聲消失。
//   dryRun（預演）照常允許。回傳擁有者說明；不歸 C 路管回 nullptr。
//   規格：.claude/skills/ht9045-html-json/references/route-c-golden-bridge.md（與 B 路的分工）。
static const char* CRouteOwner(const std::string& fullPath) {
    std::string base = fullPath;
    const std::size_t sl = base.find_last_of("\\/");
    if (sl != std::string::npos) base = base.substr(sl + 1);
    for (std::size_t i = 0; i < base.size(); ++i) base[i] = (char)std::tolower((unsigned char)base[i]);
    static const struct { const char* file; const char* owner; } kOwned[] = {
        { "config.ini",         "FileRW/IniConfig.cpp -- use WS editlist.get / editlist.save tag=IniConfig (Config.Configuration.html); [Visible] also FileRW/IniConfig_CounterSel.cpp tag=IniConfig_CounterSel (Status.CounterSel.html)" },   /*AI(W906-CRT-CounterSel) 20260926（Steven 團隊，S56）：golden TfCounterSel::FormClose 經 ProcessLastSetIni_Visible 寫 [Visible] 11 鍵；兩條 C 路共用同一份 IniConfig 記憶體*/
        { "lastset.ini",        "FileRW/IniConfig.cpp -- use WS editlist.get / editlist.save tag=IniConfig" },
        { "configbyrecipe.ini", "FileRW/IniConfig.cpp -- use WS editlist.get / editlist.save tag=IniConfig" },
        { "hotplate.data",      "FileRW/HotPlateForm_File.cpp -- use GET /api/form + WS form.save (Setup.HotPlate.html)" },
        { "uduld.data",         "FileRW/Ld_UldDelayTime.cpp -- use WS editlist.get / editlist.save tag=Ld_UldDelayTime" },
        { "armcondition.data",  "FileRW/ArmSpeed_File.cpp -- use WS editlist.get / editlist.save tag=ArmSpeed_File (Setup.Speed.html)" },
        { "binasgn.data",         "FileRW/BinSelect.cpp -- use WS editlist.get / editlist.save tag=BinSelect (Setup.BinSel.html)" },
        { "binasgnoff.data",      "FileRW/BinSelect.cpp -- use WS editlist.get / editlist.save tag=BinSelect (Setup.BinSel.html)" },
        { "binasgnoff-line.data", "FileRW/BinSelect.cpp -- use WS editlist.get / editlist.save tag=BinSelect (Setup.BinSel.html)" },
        { "binasgn_art.data",     "FileRW/BinSelect.cpp -- use WS editlist.get / editlist.save tag=BinSelect (Setup.BinSel.html)" },
        { "binasgnoff_art.data",  "FileRW/BinSelect.cpp -- use WS editlist.get / editlist.save tag=BinSelect (Setup.BinSel.html)" },
        { "binasgn_mrt.data",     "FileRW/BinSelect.cpp -- use WS editlist.get / editlist.save tag=BinSelect (Setup.BinSel.html)" },
        { "binasgn_mrt_rt.data",  "FileRW/BinSelect.cpp -- use WS editlist.get / editlist.save tag=BinSelect (Setup.BinSel.html)" },
        { "tray.data",          "FileRW/UserDefForm_File.cpp (Setup.TrayForm.html) / FileRW/TrayForm.cpp (Setup.TrayAssignment.html) -- WS editlist.* tag=UserDefForm_File / TrayForm" },
        { "contact.data",       "FileRW/DeviceForm_File.cpp -- use WS editlist.get tag=DeviceForm_File (Setup.Contact.html)" },
        { "temperature.data",   "FileRW/Temperature.cpp -- use WS editlist.get / editlist.save tag=Temperature (Setup.Temp_Set.html)" },   /*Steven 20260925*/
        { "position offset.data",      "FileRW/Offset_File.cpp -- use WS editlist.get / editlist.save tag=Offset_File (Setup.OffSet.html)" },   /*Steven 20260925*/
        { "position offset hot.data",  "FileRW/Offset_File.cpp -- use WS editlist.get / editlist.save tag=Offset_File (Setup.OffSet.html)" },
        { "position offset cool.data", "FileRW/Offset_File.cpp -- use WS editlist.get / editlist.save tag=Offset_File (Setup.OffSet.html)" },
        { "gerneral.ini",       "FileRW/HSys.cpp -- use WS editlist.get / editlist.save tag=HSys (HW.HandlerSys.html)" },   /*Steven 20260925：golden THandlerSystem::SaveSystemSet*/
        { "autoclean.data",     "FileRW/TestIF_File_Cleaning.cpp -- use WS editlist.get / editlist.save tag=TestIF_File_Cleaning (Setup.Cleaning.html)" },   /*Steven 20260925：IniData\DefineAutoClean（golden SaveAutoCleanData，bUseDefineAutoCleanOffset／CC_TSMC_TAINAN）*/
        { "autocleancount.data", "FileRW/TestIF_File_Cleaning.cpp -- golden ReadWriteAutoCleanCount (IniConfig.bE43_1_AutoCleanCountSaveFolder)" },   /*Steven 20260925*/
        { "handlercondition.data", "FileRW/TestIF_File_SetUp.cpp (Setup.SetUp.html) / FileRW/StartCondition.cpp (Data.StartCondition.html) / FileRW/TestIF_File_Cleaning.cpp (Setup.Cleaning.html) / FileRW/TestIF_File_VacuumUnit.cpp (HW.VacuumUnit.html) / FileRW/TestIF_File_BarCode.cpp (Setup.BarCode.html) / FileRW/ShuttleMove.cpp (HW.ShuttleMove.html, [Shuttle] latch only) -- WS editlist.* tag=TestIF_File_SetUp / StartCondition / TestIF_File_Cleaning / TestIF_File_VacuumUnit / TestIF_File_BarCode / ShuttleMove" },   /*Steven 20260925：兩個 golden 存檔流程（TfSetup::SaveSetupFile、TfStartCondition::ReadWriteStartCondition）；B 路 5 鍵已從 Data.StartCondition 拿掉*/
        { "barcode.ini",        "FileRW/TestIF_File_BarCode.cpp -- use WS editlist.get / editlist.save tag=TestIF_File_BarCode (Setup.BarCode.html)" },   /*Steven 團隊 20260925：system\Barcode.ini，golden TfBarCode::spbSaveClick 在 CC_KYEC_XILINX 寫 [Configuration_Barcode(XILINX)]；查過 web/page 沒有 B 路頁面寫它*/
        { "groundman.ini",      "FileRW/GroundMan.cpp -- use WS editlist.get / editlist.save tag=GroundMan (Status.GroundMan.html)" },  { "contactinfo.ini",    "FileRW/ContactForce.cpp -- use WS editlist.get / editlist.save tag=ContactForce (Setup.ContactForce.html)" },  /*AI(W906-FRW-S57) 20260926（Steven 團隊）：system\ContactInfo.ini，golden TfContactForce::WriteFile／ReadFile；sysfile 表 contactInfo（kTab）目前只有讀者（ht9045_contact_slk.js HT9045System.read），防呆*/   /*AI(W906-CRT-GroundMan) 20260926（Steven 團隊）：system\GroundMan.ini，golden TfGroundMan::spbSaveClick／ReadGroundOffset；kTab 與 recipe.doc 都沒有它（目前沒有 B 路寫者），防呆*/  { "trayform.csv",       "FileRW/CfgTrayPlate.cpp -- golden TfConfiguration sbtReloadTrayClick / sbUpdateTrayClick (Config tsTrayData; no page yet)" },  { "plateform.csv",      "FileRW/CfgTrayPlate.cpp -- golden TfConfiguration sbtReloadHPClick / sbUpdateHPClick (Config tsHPData; no page yet)" },  { "autotemperature.ini", "FileRW/ACTForm.cpp -- golden TACTForm FormShow / btnUpdateClick (no page yet)" },  { "atcwinway.ini",      "FileRW/Winway.cpp -- golden TfWinway ctor (every boot) / btnUpdateClick (no page yet)" },  { "mvdata.ini",         "FileRW/Monitor.cpp -- golden TfMonitor LoadTCPIPParament / SaveTCPIPParament (no page yet)" },  /*AI(W906-FRW-S98/S108/S109/S110) 20260926（Steven 團隊）：System\TrayForm.csv／PlateForm.csv、System\AutoTemperature.ini、config\ATCWinWay.ini、system\MVData.ini 的 C 路擁有者；sysfile 表 kTab 有 trayForm／plateForm／autoTemp／mvData 四筆，查過 web/page 沒有 B 路寫者，防呆*/
        { "socketcount.ini",    "FileRW/StartCondition.cpp -- use WS editlist.get / editlist.save tag=StartCondition (Data.StartCondition.html)" },   /*Steven 20260925：IniConfig.bD05_1SaveSocketCntByHandler*/
        { "teach.ini",          "FileRW/Teach.cpp -- use WS editlist.get / editlist.save tag=Teach" },   /*AI(W906-W5-TEACH) 20260925*/
        { "jam0000.dat",        "WebSecurityJam.cpp -- use WS security.jam (Status.Security.html Jam tab; golden TfSecurity::SaveJamLevel / spbImportClick)" },   /*Steven 團隊 20260926：Error\English\JAM0000.dat；sysfile 表與 recipe.doc 都沒有它，查過沒有 B 路寫者*/
        { "tester.data",        "FileRW/TestIF_File_TesterIF.cpp (Setup.TesterIF.html) / FileRW/TestIF_File_YieldMonitoring.cpp (Setup.YieldMonitoring.html) / FileRW/Temperature.cpp (Setup.Temp_Set.html) / FileRW/TestIF_File_QAMode.cpp (Setup.QAMode.html) -- WS editlist.* tag=TestIF_File_TesterIF / TestIF_File_YieldMonitoring / Temperature / TestIF_File_QAMode" },   /*Steven 20260925：四個 golden 存檔流程（TFTestIF::spbSaveClick、TfYieldMonitoring、TfTemp_Set、TfQAMode::btnApplyClick）；B 路的 testerif／yieldmon／sckart 接線已沒有頁面載入，Setup.SCK_ART 的 fields 是空的*/
    };
    for (std::size_t i = 0; i < sizeof(kOwned) / sizeof(kOwned[0]); ++i)
        if (base == kOwned[i].file) return kOwned[i].owner;
    return CRouteOwnerDio(fullPath);   //AI(W906-FRW-Q3) 20260927: 固定檔名都不是 → 再看是不是 TTLCfg 的 DIO 動態檔名（本體在檔尾）；原本 return nullptr
}

std::string SysFilePath(const SysFileEntry* e) {
    if (!e->path) return ResolveDioPath();    // "dio" -- 依配方與 config 開關解析
    if (e->path == &asGeneralPath && !gRealGeneralPath.empty())   //Steven 20260916 (A1)
        return gRealGeneralPath;
    std::string p(e->path->c_str());
    if (e->suffix) p += e->suffix;
    return p;
}

const SysFileEntry* FindSysFile(const std::string& name) {
    std::size_t n = 0;
    const SysFileEntry* t = SysFileTable(&n);
    for (std::size_t i = 0; i < n; ++i)
        if (StemFoldEq(std::string(t[i].key), name)) return &t[i];
    return 0;
}

bool ReadWholeFile(const std::string& path, std::string* out) {
    FILE* f = ::fopen(path.c_str(), "rb");
    if (!f) return false;
    char buf[8192];
    std::size_t got;
    out->clear();
    while ((got = ::fread(buf, 1, sizeof(buf), f)) > 0) out->append(buf, got);
    ::fclose(f);
    return true;
}

// Split one CSV line. No quote handling on purpose: Mot_Table/IO_Table are
// machine-generated, comma-separated, and carry no quoted fields. Inventing a
// quote parser here would silently change bytes the writer must preserve.
void SplitCsv(const std::string& line, std::vector<std::string>* out) {
    out->clear();
    std::string cur;
    for (std::string::size_type i = 0; i < line.size(); ++i) {
        const char c = line[i];
        //Steven 20260916
        // Skip '\n' as well as '\r'. CsvApplyEdits keeps each line WITH its
        // terminator (so the rewrite is byte-exact) and then calls SplitCsv on
        // the header line as-is -- which made the last column name come out as
        // "In1Logic\n". Every edit addressed to the last column therefore
        // reported notFound (motTable: 44 of 44 rows), and rule 2 in the browser
        // refused the whole page. CsvToJson never hit this because it strips
        // '\n' while splitting lines, so GET showed a column that PUT could not
        // reach. Found by the wire_probe "send every cell back" test.
        if (c == '\r' || c == '\n') continue;
        if (c == ',') { out->push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    out->push_back(cur);
}

//Steven 20260916
// Which column identifies a row, per csv file. Null = column 0 (the old rule).
//
// The old rule was "rowKey is whatever is in column 0" for every csv. That is
// fine for Mot_Table.csv (column 0 = Motorname, M00..M43, unique) and silently
// wrong for IO_Table.csv: column 0 there is IOType, and 8 of its values repeat
// (Sensor x255, Switch x103, Sucker x49, Cylinder_On/Off x41 ...). An edit
// addressed by IOType would land on the FIRST row of that type and CsvApplyEdits
// would report changed=1 -- operator changes one IO point, a different one is
// rewritten, nothing errors. Alias is the identity golden itself uses
// (HSys.IOTable[Tag]->Alias) and is unique across every non-blank row.
//
// Kept as a side lookup rather than a 5th field on SysFileEntry so the 40-row
// table above does not have to change shape for two entries.
const char* CsvKeyColumn(const SysFileEntry* e) {
    if (!e || !e->csv) return 0;
    if (std::string(e->key) == "ioTable")  return "Alias";
    if (std::string(e->key) == "motTable") return "Motorname";
    return 0;
}

// Resolve the key column's index from a header row; -1 if the name is absent.
// With keyCol == 0 the answer is 0 (column 0), so old callers keep old behaviour.
int CsvKeyIndex(const std::vector<std::string>& cols, const char* keyCol) {
    if (!keyCol) return cols.empty() ? -1 : 0;
    for (std::size_t c = 0; c < cols.size(); ++c)
        if (cols[c] == keyCol) return (int)c;
    return -1;
}

std::string CsvToJson(const std::string& path, const char* keyCol /* Steven 20260916 */) {
    std::string text;
    if (!ReadWholeFile(path, &text)) return std::string();
    std::vector<std::string> lines;
    std::string cur;
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') { lines.push_back(cur); cur.clear(); }
        else cur.push_back(text[i]);
    }
    if (!cur.empty()) lines.push_back(cur);

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("path").String(path);
    w.Key("kind").String(std::string("csv"));
    if (lines.empty()) {
        w.Key("columns").BeginArray().EndArray();
        w.Key("rows").BeginArray().EndArray();
        w.EndObject();
        return w.Str();
    }
    std::vector<std::string> cols;
    SplitCsv(lines[0], &cols);
    w.Key("columns").BeginArray();
    for (std::size_t c = 0; c < cols.size(); ++c) w.String(cols[c]);
    w.EndArray();
    //Steven 20260916: the advertised key is the per-file identity column, and a
    // row is skipped when THAT cell is blank (it cannot be addressed for a write,
    // so showing it would invite an edit that has nowhere to go). IO_Table.csv
    // carries 146 such blank placeholder rows; they stay in the file untouched
    // because CsvApplyEdits copies every non-matching line through byte-for-byte.
    const int ki = CsvKeyIndex(cols, keyCol);
    w.Key("keyColumn").String(ki < 0 ? std::string() : cols[(std::size_t)ki]);
    w.Key("rows").BeginArray();
    for (std::size_t li = 1; li < lines.size(); ++li) {
        if (lines[li].empty()) continue;
        std::vector<std::string> f;
        SplitCsv(lines[li], &f);
        if (f.empty()) continue;
        if (ki < 0 || (std::size_t)ki >= f.size() || f[(std::size_t)ki].empty()) continue;
        w.BeginObject();
        for (std::size_t c = 0; c < cols.size(); ++c)
            w.Key(cols[c]).String(c < f.size() ? f[c] : std::string());
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    return w.Str();
}

struct CsvCellEdit {
    std::string rowKey;   // value of the first column
    std::string column;   // header name
    std::string rawValue; // written verbatim
};

// Byte-preserving: only the named cell's text is replaced. Every other byte of
// the line -- and every other line -- is copied through untouched, including
// line endings and any trailing whitespace. Same contract as the recipe writer,
// and for the same reason: this file has columns nothing in the port reads yet.
ht9045::RecipeWriteResult CsvApplyEdits(const std::string& path,
                                        const std::vector<CsvCellEdit>& edits,
                                        ht9045::RecipeWriteMode mode,
                                        const char* keyCol /* Steven 20260916 */) {
    ht9045::RecipeWriteResult r;
    r.ok = true; r.changed = 0; r.identical = 0; r.notFound = 0;

    std::string text;
    if (!ReadWholeFile(path, &text)) { r.ok = false; r.error = "cannot read " + path; return r; }

    // Keep each line WITH its terminator so the rewrite is byte-exact.
    std::vector<std::string> lines;
    std::string cur;
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        cur.push_back(text[i]);
        if (text[i] == '\n') { lines.push_back(cur); cur.clear(); }
    }
    if (!cur.empty()) lines.push_back(cur);
    if (lines.empty()) { r.ok = false; r.error = "empty file"; return r; }

    std::vector<std::string> cols;
    SplitCsv(lines[0], &cols);

    //Steven 20260916
    // Rows are matched on the file's identity column (CsvKeyColumn), not on
    // column 0. Two further refusals, both reported as notFound so the browser's
    // rule 2 ("a notFound means the table is wrong, refuse the page") catches them:
    //   * an empty rowKey -- it would otherwise match the first blank placeholder
    //     row of IO_Table.csv;
    //   * a rowKey that matches MORE than one line -- ambiguous; writing "the
    //     first one" is exactly the silent-misdirect this change exists to stop.
    const int ki = CsvKeyIndex(cols, keyCol);
    if (ki < 0) {
        r.ok = false;
        r.error = std::string("key column '") + (keyCol ? keyCol : "") + "' not in header";
        return r;
    }

    for (std::size_t e = 0; e < edits.size(); ++e) {
        std::size_t ci = (std::size_t)-1;
        for (std::size_t c = 0; c < cols.size(); ++c)
            if (cols[c] == edits[e].column) { ci = c; break; }
        if (ci == (std::size_t)-1) { ++r.notFound; continue; }
        if (edits[e].rowKey.empty()) { ++r.notFound; continue; }
        //Steven 20260916 (review B2): SplitCsv has no quoting. A value carrying
        // ',' would add a column to this line and shift every later cell; '\r' or
        // '\n' would split the line. The on-screen keyboard cannot type these, a
        // direct WebSocket client can. Refuse as notFound so the browser's rule 2
        // rejects the page instead of the file being deformed.
        if (edits[e].rawValue.find_first_of(",\"\r\n") != std::string::npos) { ++r.notFound; continue; }

        // Pass 1: how many lines carry this key. Anything but exactly one -> notFound.
        std::size_t matches = 0, matchLi = 0;
        for (std::size_t li = 1; li < lines.size(); ++li) {
            std::vector<std::string> f;
            SplitCsv(lines[li], &f);
            if ((std::size_t)ki < f.size() && f[(std::size_t)ki] == edits[e].rowKey) {
                ++matches; matchLi = li;
            }
        }
        if (matches != 1) { ++r.notFound; continue; }

        bool hit = false;
        for (std::size_t li = matchLi; li < lines.size(); ++li) {
            std::string body = lines[li];
            std::string eol;
            while (!body.empty() && (body[body.size() - 1] == '\n' || body[body.size() - 1] == '\r')) {
                eol.insert(eol.begin(), body[body.size() - 1]);
                body.erase(body.size() - 1);
            }
            std::vector<std::string> f;
            SplitCsv(body, &f);
            if ((std::size_t)ki >= f.size() || f[(std::size_t)ki] != edits[e].rowKey) continue;
            hit = true;
            if (ci >= f.size()) { ++r.notFound; break; }
            if (f[ci] == edits[e].rawValue) { ++r.identical; break; }
            f[ci] = edits[e].rawValue;
            std::string rebuilt;
            for (std::size_t c = 0; c < f.size(); ++c) {
                if (c) rebuilt.push_back(',');
                rebuilt += f[c];
            }
            lines[li] = rebuilt + eol;
            ++r.changed;
            break;
        }
        if (!hit) ++r.notFound;
    }

    if (mode == ht9045::kRecipeWriteDryRun || r.changed == 0) return r;
    //Steven 20260916 (review B4): apply is all-or-nothing. A notFound means the
    // browser's table is wrong (or the file moved under us between dryRun and
    // apply); writing "the ones that matched" would leave a half-applied save
    // that nothing reports.
    //Steven 20260916 (review round 2, R2): and SAY so in the ack. Returning ok:true
    // with the dry-run's `changed` still set let the browser print "written,
    // changed=N" while the disk was untouched.
    if (r.notFound > 0) {
        r.ok = false; r.changed = 0;
        r.error = "refused: " + std::to_string(r.notFound) + " cell(s) notFound, nothing written";
        return r;
    }

    // Backup first -- AGENTS.md requires it for anything under system\.
    char stamp[32];
    const std::time_t now = std::time(0);
    const std::tm* lt = std::localtime(&now);
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", lt);
    const std::string bak = path + ".bak_" + stamp;
    {
        FILE* b = ::fopen(bak.c_str(), "wb");
        if (!b) { r.ok = false; r.error = "cannot write backup " + bak; return r; }
        ::fwrite(text.data(), 1, text.size(), b);
        ::fclose(b);
    }
    const std::string tmp = path + ".tmp_wb";
    {
        FILE* o = ::fopen(tmp.c_str(), "wb");
        if (!o) { r.ok = false; r.error = "cannot write temp " + tmp; return r; }
        for (std::size_t i = 0; i < lines.size(); ++i)
            ::fwrite(lines[i].data(), 1, lines[i].size(), o);
        ::fclose(o);
    }
    if (!::MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        r.ok = false; r.error = "atomic replace failed";
        ::remove(tmp.c_str());
        return r;
    }
    r.backupPath = bak;
    return r;
}


//Steven 20260916
// ---------------------------------------------------------------------------
//  system.csv.rows -- add / delete WHOLE ROWS of a csv system file.
//
//  CsvApplyEdits only touches existing cells. golden's TStringGrid pages also
//  have btnAddMotor / btnDeleteMotor / btnAddIO / btnDeleteIO (uMotorTest.cpp:2198,
//  iosetview.cpp:3281): "append one blank row" / "drop the selected row". This is
//  the file-level counterpart, same posture as CsvApplyEdits:
//    * rows are identified by CsvKeyColumn (Motorname / Alias);
//    * every untouched line is copied through byte-for-byte;
//    * dryRun first, backup, atomic replace; --allow-system-write gate;
//    * anything ambiguous is a notFound, never a guess.
//
//  Refusals (each counted in notFound, nothing written):
//    add    key cell missing/empty; key already present in the file (or twice in
//           the same request); any cell containing ',' '"' '\r' '\n' -- SplitCsv
//           has no quoting, so such a value could not be read back the same way.
//    delete key matching zero lines or more than one line.
//
//  New lines are appended at the end in header order (unknown column names are
//  ignored, missing ones are ""), using the file's own line ending.
// ---------------------------------------------------------------------------
struct CsvRowsResult {
    bool ok; int added; int deleted; int notFound;
    std::string error, backupPath;
};

CsvRowsResult CsvApplyRows(const std::string& path,
                           const std::vector<std::map<std::string, std::string> >& adds,
                           const std::vector<std::string>& dels,
                           bool dry, const char* keyCol,
                           const char* keyPat /* Steven 20260916 (R6), may be null */) {
    CsvRowsResult r; r.ok = true; r.added = r.deleted = r.notFound = 0;

    std::string text;
    if (!ReadWholeFile(path, &text)) { r.ok = false; r.error = "cannot read " + path; return r; }

    std::vector<std::string> lines;                     // each WITH its terminator
    std::string cur;
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        cur.push_back(text[i]);
        if (text[i] == '\n') { lines.push_back(cur); cur.clear(); }
    }
    if (!cur.empty()) lines.push_back(cur);
    if (lines.empty()) { r.ok = false; r.error = "empty file"; return r; }

    std::vector<std::string> cols;
    SplitCsv(lines[0], &cols);
    const int ki = CsvKeyIndex(cols, keyCol);
    if (ki < 0) { r.ok = false; r.error = std::string("key column '") + (keyCol ? keyCol : "") + "' not in header"; return r; }
    const std::string eol = (text.find("\r\n") != std::string::npos) ? "\r\n" : "\n";

    // key -> line index (only lines whose key cell is non-empty)
    std::map<std::string, std::vector<std::size_t> > where;
    for (std::size_t li = 1; li < lines.size(); ++li) {
        std::vector<std::string> f;
        SplitCsv(lines[li], &f);
        if ((std::size_t)ki < f.size() && !f[(std::size_t)ki].empty()) where[f[(std::size_t)ki]].push_back(li);
    }

    // --- deletes ---
    std::set<std::size_t> drop;
    for (std::size_t d = 0; d < dels.size(); ++d) {
        std::map<std::string, std::vector<std::size_t> >::const_iterator it = where.find(dels[d]);
        if (dels[d].empty() || it == where.end() || it->second.size() != 1 || drop.count(it->second[0])) {
            ++r.notFound; continue;
        }
        drop.insert(it->second[0]);
        ++r.deleted;
    }

    // --- adds ---
    std::vector<std::string> newLines;
    std::set<std::string> seenNew;
    for (std::size_t a = 0; a < adds.size(); ++a) {
        const std::map<std::string, std::string>& row = adds[a];
        std::map<std::string, std::string>::const_iterator kv = row.find(cols[(std::size_t)ki]);
        if (kv == row.end() || kv->second.empty()) { ++r.notFound; continue; }
        const std::string& key = kv->second;
        if (!KeyMatches(key, keyPat)) { ++r.notFound; continue; }   //Steven 20260916 (R6)
        // a key being deleted in the same request does not free it up -- keep it simple
        if (where.count(key) || seenNew.count(key)) { ++r.notFound; continue; }
        bool bad = false;
        for (std::map<std::string, std::string>::const_iterator c = row.begin(); c != row.end(); ++c)
            if (c->second.find_first_of(",\"\r\n") != std::string::npos) { bad = true; break; }
        if (bad) { ++r.notFound; continue; }
        std::string line;
        for (std::size_t c = 0; c < cols.size(); ++c) {
            if (c) line.push_back(',');
            std::map<std::string, std::string>::const_iterator v = row.find(cols[c]);
            if (v != row.end()) line += v->second;
        }
        newLines.push_back(line + eol);
        seenNew.insert(key);
        ++r.added;
    }

    if (dry || (r.added + r.deleted) == 0) return r;
    if (r.notFound > 0) {                          //Steven 20260916 (review E4 + R2): all-or-nothing, and say so
        r.ok = false; r.added = r.deleted = 0;
        r.error = "refused: " + std::to_string(r.notFound) + " row(s) notFound, nothing written";
        return r;
    }

    char stamp[32];
    const std::time_t now = std::time(0);
    const std::tm* lt = std::localtime(&now);
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", lt);
    const std::string bak = path + ".bak_" + stamp;
    {
        FILE* b = ::fopen(bak.c_str(), "wb");
        if (!b) { r.ok = false; r.error = "cannot write backup " + bak; return r; }
        ::fwrite(text.data(), 1, text.size(), b);
        ::fclose(b);
    }
    const std::string tmp = path + ".tmp_wb";
    {
        FILE* o = ::fopen(tmp.c_str(), "wb");
        if (!o) { r.ok = false; r.error = "cannot write temp " + tmp; return r; }
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (drop.count(i)) continue;
            ::fwrite(lines[i].data(), 1, lines[i].size(), o);
        }
        // the file may lack a trailing newline; do not glue the first new row onto the last old one
        if (!newLines.empty() && !lines.empty() && !drop.count(lines.size() - 1)) {
            const std::string& last = lines[lines.size() - 1];
            if (last.empty() || last[last.size() - 1] != '\n') ::fwrite(eol.data(), 1, eol.size(), o);
        }
        for (std::size_t i = 0; i < newLines.size(); ++i)
            ::fwrite(newLines[i].data(), 1, newLines[i].size(), o);
        ::fclose(o);
    }
    if (!::MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        r.ok = false; r.error = "atomic replace failed";
        ::remove(tmp.c_str());
        return r;
    }
    r.backupPath = bak;
    return r;
}


//Steven 20260916 (review C): let dryRun through the write gate.
// dryRun computes and never touches disk, so on a read-only server the browser
// should get counts (and "server is read-only" only when it really tries to
// apply), not "preview failed". Peeked from the raw JSON before the gate.
static bool PeekDryRun(const std::string& json) {
    cJSON* root = cJSON_Parse(json.c_str());
    if (!root) return false;
    const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
    const bool dry = (j && cJSON_IsBool(j) && cJSON_IsTrue(j));
    cJSON_Delete(root);
    return dry;
}

// ---------------------------------------------------------------------------
//  AI(W906-FW-DIO) 20260915: 目前生效的 DIO 介面設定檔。
//
//  page/Config.DIOInterFaceCFG.html 的值會隨「載入哪個工作檔」與「config 的
//  一個開關」而換檔案。這裡複刻 golden 的 GetDIOFileName()
//  （BCB6 DIOInterFaceCFG.cpp:47-70；V906 forms/fDIOFrom.cpp GATE (I-2)）：
//
//      if (bI16TTLSaveInSetupFile)                       // config.ini [Tester]
//          S1 = DataPath + <recipe> + "\" + <TypeName> + ".ini";   // 配方副本
//          if (!FileExists(S1)) CopyFile(DIOCFGPath+..., S1);       // golden 會複製
//      else
//          S1 = DIOCFGPath + <TypeName> + ".ini";                   // 主檔
//
//  ⚠ 兩點與 golden 不同，都是刻意的：
//   1. **不做那個 CopyFile。** 那正是 V906 把 (I-2) gate 起來的理由（"WRITES
//      DISK ... PURE SAFETY GATE"）。配方副本不存在時回報 available:false 並
//      說明原因，不擅自建檔——建了就等於用讀取請求產生副作用。
//   2. golden 取 TypeName 自 `FTestIF->cbDIOType->Text`（活的 UI 狀態）。伺服器
//      沒有 UI，所以改讀 UI 自己寫下去的那筆：作用中配方的 Tester.Data
//      [DIO] TypeName（cTesterIF.cpp:372-373 就是寫在那裡）。
// ---------------------------------------------------------------------------
std::string IniGet(const std::string& path, const std::string& section,
                   const std::string& key) {
    // 自己解析，不呼叫 golden 的 ReadIniData/CheckAndReadIniData —— 後者的實作
    // 會「補上缺少的鍵」，也就是讀取帶寫入副作用。這條路必須純唯讀。
    std::string text;
    if (!ReadWholeFile(path, &text)) return std::string();
    std::string cur, sec;
    std::vector<std::string> lines;
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') { lines.push_back(cur); cur.clear(); }
        else if (text[i] != '\r') cur.push_back(text[i]);
    }
    if (!cur.empty()) lines.push_back(cur);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        std::string t = lines[i];
        while (!t.empty() && (t[0] == ' ' || t[0] == '	')) t.erase(0, 1);
        while (!t.empty() && (t[t.size()-1] == ' ' || t[t.size()-1] == '	')) t.erase(t.size()-1);
        if (t.empty() || t[0] == ';' || t[0] == '#') continue;
        if (t[0] == '[' && t[t.size()-1] == ']') { sec = t.substr(1, t.size()-2); continue; }
        const std::string::size_type eq = t.find('=');
        if (eq == std::string::npos) continue;
        std::string k = t.substr(0, eq);
        while (!k.empty() && (k[k.size()-1] == ' ' || k[k.size()-1] == '	')) k.erase(k.size()-1);
        if (StemFoldEq(sec, section) && StemFoldEq(k, key)) return t.substr(eq + 1);
    }
    return std::string();
}

struct DioResolved {
    std::string path, typeName, source, note;
    bool saveInSetup;
    DioResolved() : saveInSetup(false) {}
};

DioResolved ResolveDio() {
    DioResolved d;
    const std::string cfg = std::string(AuthPath.c_str()) + "config.ini";
    const std::string flag = IniGet(cfg, "Tester", "bI16TTLSaveInSetupFile");
    d.saveInSetup = (!flag.empty() && flag.find('1') != std::string::npos);

    const std::string tester = RealRecipeDir() + "Tester.Data";
    d.typeName = IniGet(tester, "DIO", "TypeName");
    while (!d.typeName.empty() && (d.typeName[0] == ' ' || d.typeName[0] == '	'))
        d.typeName.erase(0, 1);
    while (!d.typeName.empty() &&
           (d.typeName[d.typeName.size()-1] == ' ' || d.typeName[d.typeName.size()-1] == '	'))
        d.typeName.erase(d.typeName.size()-1);
    if (d.typeName.empty()) {
        d.note = "Tester.Data [DIO] TypeName is empty -- nothing selected";
        return d;
    }
    if (d.saveInSetup) {
        d.path = RealRecipeDir() + d.typeName + ".ini";
        d.source = "recipe";
        WIN32_FILE_ATTRIBUTE_DATA fa;
        if (!::GetFileAttributesExA(d.path.c_str(), GetFileExInfoStandard, &fa)) {
            d.note = "bI16TTLSaveInSetupFile=1 but the recipe copy does not exist yet. "
                     "golden would CopyFile it from DioCfg on first use; this server "
                     "deliberately does NOT (a GET must not create files). Open the "
                     "page in the handler once, or copy it by hand.";
        }
    } else {
        d.path = std::string(DIOCFGPath.c_str()) + d.typeName + ".ini";
        d.source = "master";
    }
    return d;
}

std::string ResolveDioPath() { return ResolveDio().path; }

// DioCfg 目錄裡可選的介面型別（下拉選單的來源，golden cTesterIF.cpp:76 也是
// 用 FindFirstFile(DIOCFGPath + "*.ini") 列出來的）。
void EnumDioProfiles(std::vector<std::string>* out) {
    WIN32_FIND_DATAA fd;
    const std::string pat = std::string(DIOCFGPath.c_str()) + "*.ini";
    HANDLE h = ::FindFirstFileA(pat.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        std::string n(fd.cFileName);
        const std::string::size_type dot = n.rfind('.');
        if (dot != std::string::npos) n = n.substr(0, dot);
        if (!n.empty()) out->push_back(n);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// AI(W906-FW-SYSFILE) 20260915: 單一路由的分流器。SetHttpRoute 只存得下一條，
// 所以 /api/recipe 與 /api/system 必須共用一個進入點。
// ---------------------------------------------------------------------------
//  AI(W906-FW-LEVELSET) 20260916: 二進位定長陣列的「依 struct 逐欄位投影」。
//
//      GET  /api/system/levelset     {kind:"i32", count, min, max, values:[...]}
//      WS   system.levels.put        {"values":{"<idx>":<int>,...}, "dryRun":bool}
//
//  為什麼另開一張表而不是加進 SysFileTable
//  SysFileTable 的註解自己寫了「二進位一律不列 …… 那些要等 C++ 端依 struct 逐欄位
//  輸出投影」。這裡就是那個投影。它進不了 SysFileEntry 是因為 SysFileEntry 的
//  兩種形狀（ini 的 sections/raw、csv 的 columns/rows）對這種檔都不成立：
//  levelset.dat 沒有鍵名、沒有表頭、沒有分隔字元，只有 256 個小端 int32。
//  硬塞進去會產生「看起來對的假資料」，那正是那條註解要擋的事。
//
//  為什麼是 Status.Security 需要的東西
//  cSecurity.cpp:1620 GetLevelSet() / :1682 SetLevelSet() 讀寫的就是這個檔，
//  整塊 ReadData/WriteData 一個 LAST_LEVEL_SET（cprod.h:1148，`int
//  AccessLevel[256]`，剛好 1024 bytes）。Status.Security.html 的 179 組 radio
//  是 AccessLevel[NN] 的畫面形狀 —— 不是 config\Security_new.def，那份是
//  cAuthority.cpp 讀的各表單 Enable/Disable 旗標，跟這 179 組沒有對應關係。
//  （20260916 ops daily「後端 securityNew 已在服務中」是把兩個檔搞混了。）
//
//  ⚠ 這條通路只負責檔案的讀與寫。C++ 端 GATE SEC1 / SEC-W1 / SEC-W2 仍然關著
//    （cSecurity.cpp:63 / :1628 / :1687），所以 handler 行程自己不會落盤、也
//    不會在啟動後重讀。也就是說：web 寫進去的值，要等 handler 下次啟動
//    GetLevelSet() 才會被讀到。這是刻意的範圍切分，不是缺陷。
//
//  範圍檢查取 golden 自己的：GetLevelSet() 對每個元素做 CheckRange(x,0,3)，
//  或 CosFunction.bSecurityHave5Level 時 CheckRange(x,0,4)。這裡收 0..4 這個
//  聯集而不是 0..3 —— 因為 5 階是執行期旗標，此處讀不到；收窄會讓 5 階機台的
//  合法值被誤判成非法。超出 0..4 的一律拒寫（不是靜默夾回去）。
// ---------------------------------------------------------------------------
struct SysBinEntry {
    const char*                  key;    // browser-facing name
    const vclcompat::AnsiString* dir;    // golden global，呼叫時才取值
    const char*                  file;   // 檔名，接在 *dir 後面
    int                          count;  // 元素個數；檔案大小必須剛好 count*4
    int                          lo, hi; // 合法值域（含端點）
};

const SysBinEntry* SysBinTable(std::size_t* n) {
    // 路徑一律取 golden 的全域。cSecurity.cpp 自己是寫死
    // "d:\HT9045\system\levelset.dat"；asSystemPath（common.cpp:100）是同一個
    // 目錄的全域，取它而不是再抄一次字面字串。
    static const SysBinEntry kTab[1] = {
        { "levelset", &asSystemPath, "levelset.dat", 256, 0, 4 }
    };
    *n = 1;
    return kTab;
}

const SysBinEntry* FindSysBin(const std::string& name) {
    std::size_t n = 0;
    const SysBinEntry* t = SysBinTable(&n);
    for (std::size_t i = 0; i < n; ++i)
        if (StemFoldEq(std::string(t[i].key), name)) return &t[i];
    return 0;
}

std::string SysBinPath(const SysBinEntry* e) {
    return std::string(e->dir->c_str()) + e->file;
}

// 小端 int32 手動組裝，不 memcpy 到 int 陣列上：memcpy 的正確性依賴本機端序與
// int 寬度，手動組裝把那兩個假設寫成程式碼，換平台時會編不過而不是靜默讀錯。
static int LeI32(const std::string& b, std::size_t i) {
    return (int)((unsigned char)b[i]
               | ((unsigned int)(unsigned char)b[i + 1] << 8)
               | ((unsigned int)(unsigned char)b[i + 2] << 16)
               | ((unsigned int)(unsigned char)b[i + 3] << 24));
}
static void PutLeI32(std::string& b, std::size_t i, int v) {
    const unsigned int u = (unsigned int)v;
    b[i]     = (char)(unsigned char)( u        & 0xFFu);
    b[i + 1] = (char)(unsigned char)((u >>  8) & 0xFFu);
    b[i + 2] = (char)(unsigned char)((u >> 16) & 0xFFu);
    b[i + 3] = (char)(unsigned char)((u >> 24) & 0xFFu);
}

// 讀不到 / 大小不對都回空字串，讓呼叫端回 404 並把實際大小講出來。
// 大小不對絕不補零或截斷：那表示這台機器的 struct 佈局跟這裡假設的不一樣，
// 照寫下去會把整份權限表寫壞。
std::string SysBinToJson(const SysBinEntry* e, std::string* why) {
    const std::string p = SysBinPath(e);
    std::string raw;
    if (!ReadWholeFile(p, &raw)) { if (why) *why = "cannot read " + p; return std::string(); }
    const std::size_t want = (std::size_t)e->count * 4u;
    if (raw.size() != want) {
        if (why) {
            char b[160];
            std::snprintf(b, sizeof(b),
                          "size mismatch: %s is %u bytes, expected %u (%d x int32)",
                          p.c_str(), (unsigned)raw.size(), (unsigned)want, e->count);
            *why = b;
        }
        return std::string();
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("path").String(p);
    w.Key("kind").String(std::string("i32"));
    w.Key("available").Bool(true);
    w.Key("bytes").Number((double)raw.size());
    w.Key("count").Number((wb_int64)e->count);
    w.Key("min").Number((wb_int64)e->lo);
    w.Key("max").Number((wb_int64)e->hi);
    w.Key("values").BeginArray();
    for (int i = 0; i < e->count; ++i) w.Number((wb_int64)LeI32(raw, (std::size_t)i * 4u));
    w.EndArray();
    w.EndObject();
    return w.Str();
}

struct BinWriteResult {
    bool ok; int changed, identical, notFound;
    std::string error, backupPath;
};

// 稀疏寫入：只碰 edits 指名的索引，其餘位元組原樣保留。
// 語意與 system.file.put / system.csv.rows 對齊（20260916 審查第二輪定的）：
// dryRun 先跑；notFound>0 一律 all-or-nothing 拒寫且 ok=false / changed=0，
// 不能只回 notFound 卻假裝成功。
BinWriteResult BinApplyValues(const SysBinEntry* e,
                              const std::map<int, int>& edits, bool dry) {
    BinWriteResult r; r.ok = true; r.changed = r.identical = r.notFound = 0;

    const std::string p = SysBinPath(e);
    std::string raw;
    if (!ReadWholeFile(p, &raw)) { r.ok = false; r.error = "cannot read " + p; return r; }
    const std::size_t want = (std::size_t)e->count * 4u;
    if (raw.size() != want) {
        char b[160];
        std::snprintf(b, sizeof(b), "size mismatch: %u bytes, expected %u; refusing to write",
                      (unsigned)raw.size(), (unsigned)want);
        r.ok = false; r.error = b; return r;
    }

    std::string next = raw;
    for (std::map<int, int>::const_iterator it = edits.begin(); it != edits.end(); ++it) {
        const int idx = it->first, val = it->second;
        if (idx < 0 || idx >= e->count) { ++r.notFound; continue; }   // 索引越界
        if (val < e->lo || val > e->hi) { ++r.notFound; continue; }   // 值域外，拒寫不夾回
        const std::size_t off = (std::size_t)idx * 4u;
        if (LeI32(next, off) == val) { ++r.identical; continue; }
        PutLeI32(next, off, val);
        ++r.changed;
    }

    //Steven 20260916: notFound 的檢查必須排在 changed==0 的早退**前面**。
    // 第一版寫反了，探針 T5/T6 抓到：整批都非法時 changed 是 0，於是先撞上
    // 早退、ack 回成 ok:true / notFound:1 —— 畫面會顯示「寫入完成」但磁碟沒動。
    // 這正是同一天第二輪審查在 csv 那條修過的缺陷（CHANGES_20260916 §「拒寫語意」），
    // 二進位這條新寫的時候又犯了一次，所以把順序的理由寫在這裡。
    //
    // 與 CsvApplyRows 的差異（刻意）：這裡 dryRun 也回 ok:false。dryRun 的語意是
    // 「照這個 payload 走會發生什麼」，既然會被拒，預覽就該說會被拒；回 ok:true
    // 只會讓瀏覽器預覽通過、正式送出時才被打回。
    if (r.notFound > 0) {
        r.ok = false; r.changed = r.identical = 0;
        char b[128];
        std::snprintf(b, sizeof(b), "refused: %d value(s) out of range or index out of bounds, nothing written", r.notFound);
        r.error = b;
        return r;
    }
    if (dry || r.changed == 0) return r;

    char stamp[32];
    const std::time_t now = std::time(0);
    const std::tm* lt = std::localtime(&now);
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", lt);
    const std::string bak = p + ".bak_" + stamp;
    {
        FILE* b = ::fopen(bak.c_str(), "wb");
        if (!b) { r.ok = false; r.error = "cannot write backup " + bak; return r; }
        ::fwrite(raw.data(), 1, raw.size(), b);
        ::fclose(b);
    }
    const std::string tmp = p + ".tmp_wb";
    {
        FILE* o = ::fopen(tmp.c_str(), "wb");
        if (!o) { r.ok = false; r.error = "cannot write temp " + tmp; return r; }
        ::fwrite(next.data(), 1, next.size(), o);
        ::fclose(o);
    }
    if (!::MoveFileExA(tmp.c_str(), p.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        r.ok = false; r.error = "atomic replace failed";
        ::remove(tmp.c_str());
        return r;
    }
    r.backupPath = bak;
    return r;
}
std::string W906_ApiCacheGet(int which); void W906_ApiCacheInit(); void W906_ApiCacheRefresh();   // AI(W906-IOWEB-P6) 20260924: /api/struct/{io,motor}/{schema,runtime} served from a tick-thread cache; defined at EOF in this same anonymous namespace. On the old blank line, so no line below moves.
bool ApiRoute(void* user, const std::string& method,
              const std::string& path, const std::string& query,
              webbridge::HttpResponse* out);
bool TextRoute(void* user, const std::string& method,
               const std::string& path, const std::string& query,
               webbridge::HttpResponse* out);

bool SystemRoute(void* /*user*/, const std::string& method,
                 const std::string& path, const std::string& /*query*/,
                 webbridge::HttpResponse* out)
{
    if (method != "GET" && method != "HEAD") {
        out->status = 405; out->reason = "Method Not Allowed";
        out->extraHeaders["Allow"] = "GET, HEAD";
        out->contentType = "text/plain; charset=utf-8";
        //Steven 20260916: 寫入指令現在有三支，訊息不該只講一支。
        out->body = "405 system files are read-only over HTTP; write with "
                    "system.file.put (ini/csv cells), system.csv.rows (whole csv rows) "
                    "or system.levels.put (i32 projections)\n";
        return true;
    }

    std::string name;
    if (path.size() > 12 /*strlen("/api/system/")*/)
        name = path.substr(12);
    else if (path != "/api/system" && path != "/api/system/")
        return false;

    if (name.empty()) {
        std::size_t n = 0;
        const SysFileEntry* t = SysFileTable(&n);
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("files").BeginArray();
        for (std::size_t i = 0; i < n; ++i) {
            const std::string p = SysFilePath(&t[i]);
            WIN32_FILE_ATTRIBUTE_DATA fa;
            const bool ok = ::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &fa) != 0;
            w.BeginObject();
            w.Key("name").String(std::string(t[i].key));
            w.Key("path").String(p);
            w.Key("kind").String(std::string(t[i].csv ? "csv" : "ini"));
            w.Key("available").Bool(ok);
            w.Key("bytes").Number(ok ? (double)fa.nFileSizeLow : 0.0);
            if (!t[i].path) {
                // AI(W906-FW-DIO) 20260915: dio 的路徑是算出來的，所以把「怎麼算出來的」
                // 一併回報 —— 不然瀏覽器只看到一個路徑，無從判斷它為什麼是那個檔。
                const DioResolved d = ResolveDio();
                w.Key("typeName").String(d.typeName);
                w.Key("saveInSetup").Bool(d.saveInSetup);
                w.Key("source").String(d.source);
                if (!d.note.empty()) w.Key("note").String(d.note);
                std::vector<std::string> profs;
                EnumDioProfiles(&profs);
                w.Key("profiles").BeginArray();
                for (std::size_t k = 0; k < profs.size(); ++k) w.String(profs[k]);
                w.EndArray();
            }
            w.EndObject();
        }
        //Steven 20260916 (W906-FW-LEVELSET): 二進位投影也列進同一份清單，
        // 否則瀏覽器無從得知 levelset 這個名字存在。kind 用 "i32" 與 ini/csv 區分。
        {
            std::size_t bn = 0;
            const SysBinEntry* bt = SysBinTable(&bn);
            for (std::size_t i = 0; i < bn; ++i) {
                const std::string bp = SysBinPath(&bt[i]);
                WIN32_FILE_ATTRIBUTE_DATA bfa;
                const bool bok = ::GetFileAttributesExA(bp.c_str(), GetFileExInfoStandard, &bfa) != 0;
                const bool sized = bok && bfa.nFileSizeHigh == 0 &&
                                   bfa.nFileSizeLow == (DWORD)bt[i].count * 4u;
                w.BeginObject();
                w.Key("name").String(std::string(bt[i].key));
                w.Key("path").String(bp);
                w.Key("kind").String(std::string("i32"));
                // available 要求「讀得到 **而且** 大小對」：大小不對的檔這條通路
                // 一律拒絕讀寫，清單就不該說它 available。
                w.Key("available").Bool(sized);
                w.Key("bytes").Number(bok ? (double)bfa.nFileSizeLow : 0.0);
                w.Key("count").Number((wb_int64)bt[i].count);
                w.Key("min").Number((wb_int64)bt[i].lo);
                w.Key("max").Number((wb_int64)bt[i].hi);
                if (bok && !sized) w.Key("note").String(std::string("size mismatch; read/write refused"));
                w.EndObject();
            }
        }
        w.EndArray();
        w.EndObject();
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = w.Str();
        out->headOnly = (method == "HEAD");
        if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
        return true;
    }

    if (!SafeDocName(name)) {
        out->status = 400; out->reason = "Bad Request";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "400 file name rejected\n";
        return true;
    }
    //Steven 20260916 (W906-FW-LEVELSET): 先查 ini/csv 表，沒有再查二進位投影表。
    // 兩張表的名字不重疊（同一套 StemFoldEq 比對），所以查詢順序不影響結果。
    const SysFileEntry* e = FindSysFile(name);
    if (!e) {
        const SysBinEntry* be = FindSysBin(name);
        if (be) {
            std::string why;
            const std::string bjson = SysBinToJson(be, &why);
            if (bjson.empty()) {
                // 大小不對是最值得講清楚的失敗：靜默補零或截斷會把整份權限表寫壞。
                out->status = 404; out->reason = "Not Found";
                out->contentType = "text/plain; charset=utf-8";
                out->body = "404 " + why + "\n";
                return true;
            }
            out->status = 200; out->reason = "OK";
            out->contentType = "application/json; charset=utf-8";
            out->body = bjson;
            out->headOnly = (method == "HEAD");
            if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
            return true;
        }
        out->status = 404; out->reason = "Not Found";
        out->contentType = "text/plain; charset=utf-8";
        //Steven 20260916: 原文寫死「the four system files」，這張表早就長到 40 筆，
        // 現在再加二進位投影；訊息不該再自稱四個，改成指向清單端點。
        out->body = "404 not a known system file; GET /api/system/ for the list\n";
        return true;
    }

    const std::string p = SysFilePath(e);
    const std::string json = e->csv ? CsvToJson(p, CsvKeyColumn(e))   //Steven 20260916
                                    : ht9045::RecipeDocToJson(vclcompat::AnsiString(p.c_str()));
    if (json.empty()) {
        out->status = 404; out->reason = "Not Found";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "404 file not readable: " + p + "\n";
        return true;
    }
    if (json.size() > kMaxDocBytes) {
        out->status = 413; out->reason = "Payload Too Large";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "413 file too large for the JSON projection\n";
        return true;
    }
    out->status = 200; out->reason = "OK";
    out->contentType = "application/json; charset=utf-8";
    out->body = json;
    out->headOnly = (method == "HEAD");
    if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
    return true;
}


// ---------------------------------------------------------------------------
//  AI(W906-FW-TEXT) 20260915: 整檔純文字讀取（Data.Observer 那一類）。
//
//      GET /api/text/                 可讀的來源清單
//      GET /api/text/<root>           該來源下的項目
//      GET /api/text/<root>/<a>/<b>…  目錄就列舉、檔案就回 {path, bytes, text}
//                                     （Steven 20260916：改成多層，理由見下面
//                                      W906-FW-TEXT2 註解——機台的 log 在
//                                      <root>\YYYY\MM\ 底下，單層版取不到任何檔）
//
//  為什麼另開一條而不是塞進 /api/system/：
//  兩者的需求不同。設定檔是「固定路徑、鍵值結構、要能寫回」；Data.Observer 讀的
//  是「動態路徑、整檔純文字、唯讀」。硬塞進同一張表會讓 sections 形狀對不上，
//  而且日誌檔名依日期產生，固定表列不完。
//
//  唯讀，沒有對應的寫入指令 —— 這些是機台產生的記錄，不是人設定的東西。
//
//  安全：root 是固定表（不可由請求指定任意目錄），**路徑的每一段**都各自走
//  SafeDocName（擋掉 ".." 與路徑分隔字元），再加深度上限。三層擋下來，
//  路徑穿越在結構上不可能。（20260916：原本第三層是「只列單層」，改成
//  「每段都驗＋深度上限」——強度不變，但取得到實際的 log 檔。）
// ---------------------------------------------------------------------------
struct TextRoot {
    const char* key;
    const char* dir;      // 目錄；file 非 0 時忽略
    const char* file;     // 非 0 表示這個 root 就是單一檔案
};

const TextRoot* TextRootTable(std::size_t* n) {
    // 來源全部出自 golden cObserver.cpp 自己讀的路徑，沒有臆測的項目。
    static const TextRoot kTab[6] = {
        { "releaseNote",    0,                                        "D:\\HT9045\\config\\ReleaseNote.txt" },
        { "eventLogTxt",    "D:\\HT9045_Log\\EventLogTxt\\",          0 },
        { "jamCount",       "D:\\HT9045_Log\\EventLogTxt\\SGJamCount\\", 0 },
        { "timeData",       "D:\\HT9045_Log\\TimeData\\",             0 },
        { "indexCycleTime", "D:\\HT9045_Log\\IndexCycleTimeRecord\\",  0 },
        // cObserver.cpp:4097 / :4287 -- 這兩個在 D:\HT9045 之外，是獨立的記錄系統。
        { "precaution",     "D:\\PrecautionRecord\\system\\",         0 }
    };
    *n = 6;
    return kTab;
}

const TextRoot* FindTextRoot(const std::string& name) {
    std::size_t n = 0;
    const TextRoot* t = TextRootTable(&n);
    for (std::size_t i = 0; i < n; ++i)
        if (StemFoldEq(std::string(t[i].key), name)) return &t[i];
    return 0;
}

const std::size_t kMaxTextBytes = 4u * 1024u * 1024u;

bool TextRoute(void* /*user*/, const std::string& method,
               const std::string& path, const std::string& /*query*/,
               webbridge::HttpResponse* out)
{
    if (method != "GET" && method != "HEAD") {
        out->status = 405; out->reason = "Method Not Allowed";
        out->extraHeaders["Allow"] = "GET, HEAD";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "405 these are machine-produced records; they are read-only\n";
        return true;
    }

    std::string rest;
    if (path.size() > 10 /*strlen("/api/text/")*/) rest = path.substr(10);
    else if (path != "/api/text" && path != "/api/text/") return false;

    // --- 來源清單 ---
    if (rest.empty()) {
        std::size_t n = 0;
        const TextRoot* t = TextRootTable(&n);
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("roots").BeginArray();
        for (std::size_t i = 0; i < n; ++i) {
            const std::string p(t[i].file ? t[i].file : t[i].dir);
            const DWORD at = ::GetFileAttributesA(p.c_str());
            w.BeginObject();
            w.Key("name").String(std::string(t[i].key));
            w.Key("path").String(p);
            w.Key("kind").String(std::string(t[i].file ? "file" : "dir"));
            w.Key("available").Bool(at != INVALID_FILE_ATTRIBUTES);
            w.EndObject();
        }
        w.EndArray();
        w.EndObject();
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = w.Str();
        out->headOnly = (method == "HEAD");
        if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
        return true;
    }

    //Steven 20260916 (W906-FW-TEXT2)
    // 改成多層路徑。原本只切一個 '/'，所以 /api/text/<root>/<file> 是唯一的檔案形式，
    // 「只列單層」也寫在上面的註解裡當成安全論證的一環。
    //
    // 實測後這個設計取不到任何一個實際的 log 檔：機台把記錄寫在
    //     D:\HT9045_Log\EventLogTxt\2026\09\EventLogTxt_20260916.csv
    // 也就是 root 底下**再兩層**。舊版列 root 只會看到 "2026" 與 "ByLotID" 兩個目錄，
    // 而 /api/text/eventLogTxt/2026 會回 404（"2026" 被當成檔名去讀）。
    // Data.Observer 之所以一直沒接，不是前端沒做，是後端沒有東西可以取。
    //
    // 安全論證改成（仍然三層，強度不變）：
    //   1. root 一樣是固定表，請求指定不了任意目錄；
    //   2. **每一段**都各自過 SafeDocName —— 它只放行 [A-Za-z0-9_.-]、擋掉 ".."、
    //      也擋掉 '/' 與 '\'，所以任何一段都不可能是 ".." 或含分隔字元，
    //      把它們用 '\' 串起來在結構上無法跳出 root；
    //   3. 深度設上限（kMaxTextDepth），避免有人用超長路徑打列舉。
    // 「只列單層」這條被「每段都驗」取代，不是被放寬。
    std::vector<std::string> seg;
    {
        std::string cur;
        for (std::string::size_type i = 0; i < rest.size(); ++i) {
            if (rest[i] == '/') { if (!cur.empty()) seg.push_back(cur); cur.clear(); }
            else cur.push_back(rest[i]);
        }
        if (!cur.empty()) seg.push_back(cur);
    }
    if (seg.empty()) { out->status = 404; out->reason = "Not Found";
                       out->contentType = "text/plain; charset=utf-8";
                       out->body = "404 no such text source\n"; return true; }
    const std::size_t kMaxTextDepth = 6;   // root ＋ 最多 5 段；實際用到 3（YYYY\MM\檔名）
    if (seg.size() > kMaxTextDepth) {
        out->status = 400; out->reason = "Bad Request";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "400 path too deep\n";
        return true;
    }
    for (std::size_t i = 0; i < seg.size(); ++i) {
        if (!SafeDocName(seg[i])) {
            out->status = 400; out->reason = "Bad Request";
            out->contentType = "text/plain; charset=utf-8";
            out->body = "400 name rejected: " + seg[i] + "\n";
            return true;
        }
    }
    const TextRoot* r = FindTextRoot(seg[0]);
    if (!r) {
        out->status = 404; out->reason = "Not Found";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "404 no such text source\n";
        return true;
    }

    std::string full, subPath;
    if (r->file) {
        if (seg.size() > 1) {
            out->status = 404; out->reason = "Not Found";
            out->contentType = "text/plain; charset=utf-8";
            out->body = "404 this source is a single file; drop the trailing name\n";
            return true;
        }
        full = r->file;
    } else {
        full = r->dir;
        for (std::size_t i = 1; i < seg.size(); ++i) {
            if (!subPath.empty()) subPath += "/";
            subPath += seg[i];
            full += seg[i];
            if (i + 1 < seg.size()) full += "\\";
        }
    }

    // 目錄就列舉，檔案就讀內容 —— 由實際的檔案屬性決定，不是由「有沒有帶檔名」決定。
    // 舊版用後者，於是中間層的目錄名一律被當成檔名，必然 404。
    const DWORD attr = ::GetFileAttributesA(full.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        const std::string dirPath = full + (full.empty() || full[full.size() - 1] == '\\' ? "" : "\\");
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("path").String(dirPath);
        w.Key("root").String(std::string(r->key));
        w.Key("subPath").String(subPath);   // 前端用它接出下一層的 URL，不必自己拼字串
        w.Key("entries").BeginArray();
        WIN32_FIND_DATAA fd;
        HANDLE h = ::FindFirstFileA((dirPath + "*").c_str(), &fd);
        std::size_t listed = 0;
        if (h != INVALID_HANDLE_VALUE) {
            do {
                const std::string nm(fd.cFileName);
                if (nm == "." || nm == "..") continue;
                // 列得出來但取不回來的項目沒有意義，而且會讓人以為是伺服器壞了。
                // 檔名不合 SafeDocName（含空白、中文、超過 64 字）就標出來說明原因。
                const bool isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                w.BeginObject();
                w.Key("name").String(nm);
                w.Key("dir").Bool(isDir);
                w.Key("bytes").Number((double)fd.nFileSizeLow);
                if (!SafeDocName(nm))
                    w.Key("unreachable").String(std::string("name outside [A-Za-z0-9_.-] or over 64 chars"));
                w.EndObject();
                ++listed;
            } while (::FindNextFileA(h, &fd) && listed < 5000);
            ::FindClose(h);
        }
        w.EndArray();
        w.Key("truncated").Bool(listed >= 5000);
        w.EndObject();
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = w.Str();
        out->headOnly = (method == "HEAD");
        if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
        return true;
    }

    std::string text;
    if (!ReadWholeFile(full, &text)) {
        out->status = 404; out->reason = "Not Found";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "404 not readable: " + full + "\n";
        return true;
    }
    if (text.size() > kMaxTextBytes) {
        out->status = 413; out->reason = "Payload Too Large";
        out->contentType = "text/plain; charset=utf-8";
        out->body = "413 file too large to project as JSON\n";
        return true;
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("path").String(full);
    w.Key("bytes").Number((double)text.size());
    w.Key("text").String(text);
    w.EndObject();
    out->status = 200; out->reason = "OK";
    out->contentType = "application/json; charset=utf-8";
    out->body = w.Str();
    out->headOnly = (method == "HEAD");
    if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
    return true;
}

// ---------------------------------------------------------------------------
//  AI(W906-JSONBRIDGE-S0) 20260923
//
//      GET /api/struct/machine.defines          web 那組（61 筆）
//      GET /api/struct/machine.defines?all=1    連 221 個 CC_* 客戶碼常數一起
//
//  唯讀，沒有對應的寫入指令。這是**這顆 exe 的事實**，要改得重新建置 ——
//  和 20260918 的 W906-ZEROARG 裁決同一條：行為由建置決定。
//
//  為什麼不塞進 /api/system/：那條服務的是「機台上的檔案」，有固定路徑、
//  有鍵值結構、要能寫回。編譯期定義一樣都沒有 —— 它不是檔案。
// ---------------------------------------------------------------------------
bool StructRoute(void* /*user*/, const std::string& method,
                 const std::string& path, const std::string& query,
                 webbridge::HttpResponse* out) {
    if (method != "GET" && method != "HEAD") {
        out->status = 405; out->reason = "Method Not Allowed";
        out->contentType = "application/json; charset=utf-8";
        out->body = "{\"error\":\"machine.defines is read-only: "
                    "compile-time facts change by rebuilding, not by a PUT\"}";
        return true;
    }

    if (path == "/api/struct/machine.defines") {
        // ?all=1 / ?all=true。其他一律當 false —— 寧可少送也不要把 221 個
        // 客戶碼常數塞給一個只是打錯查詢字串的瀏覽器。
        // ⚠ 不能用 query.find("all=1")：那是子字串比對，`?install=1`、`?xall=1`、
        //   `?foo=ball=1` 全部都會命中 —— 正好違反這段本來要防的事。
        //   20260923 審查時抓到。改成逐段完整鍵比對。
        bool all = false;
        for (std::string::size_type b = 0; b <= query.size(); ) {
            std::string::size_type e = query.find('&', b);
            if (e == std::string::npos) e = query.size();
            const std::string kv = query.substr(b, e - b);
            if (kv == "all=1" || kv == "all=true") { all = true; break; }
            if (e == query.size()) break;
            b = e + 1;
        }
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = ht9045::sjson::MachineDefinesToJson(all);
        out->headOnly = (method == "HEAD");
        if (out->headOnly) {
            out->contentLength = (long long)out->body.size();
            out->body.clear();
        }
        return true;
    }

    // AI(W906-JSONBRIDGE-S1) 20260923: 留痕的內容。
    //   tag 只送游標（log.seq / log.count / log.dropped / log.db），內容走這裡 ——
    //   200 筆塞進每個 500ms 的 patch 是純浪費，瀏覽器只在 log.seq 變了而且
    //   它正在看 log 的時候才需要內容（SKILL.md 六 的 HTTP 拿全量／WS 送 patch）。
    if (path == "/api/struct/log.tail") {
        unsigned long long since = 0;
        const std::string::size_type sp = query.find("since=");
        if (sp != std::string::npos)
            since = (unsigned long long)std::strtoull(query.c_str() + sp + 6, 0, 10);
        std::size_t limit = 0;                       // 0 = 由 EventLogTailJson 取上限
        const std::string::size_type lp = query.find("limit=");
        if (lp != std::string::npos)
            limit = (std::size_t)std::strtoul(query.c_str() + lp + 6, 0, 10);
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = ht9045::sjson::EventLogTailJson(since, limit);
        out->headOnly = (method == "HEAD");
        if (out->headOnly) {
            out->contentLength = (long long)out->body.size();
            out->body.clear();
        }
        return true;
    }

    // AI(W906-JSONBRIDGE-S7) 20260923: 溫控通道表 ＋ 每欄位的活性宣告。
    //   GET /api/struct/temp.zone/schema
    //
    //   ⚠ 只有 /schema，**沒有**值的端點。值走 tag 串流（temp.zone.<ch>.pv…），
    //     這跟結構綁定不一樣：結構是「一坨設定，整份抓」，溫度是「71 個會變
    //     的量測值」，整份抓沒有意義。
    //
    //   ⚠⚠ 瀏覽器要先讀這個 schema 的 `anyLive` 與 `fields.<f>.live` 再看值。
    //     今天 anyLive=false，71 個通道的每個值都是 null，而那是**正確答案**
    //     不是錯誤 —— 溫控讀取迴圈還沒接進 wb_serve。把 null 畫成 "---"，
    //     不要畫成 0。
    if (path == "/api/struct/temp.zone/schema") {
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        out->body = ht9045::sjson::ThermoSchemaJson();
        out->headOnly = (method == "HEAD");
        if (out->headOnly) {
            out->contentLength = (long long)out->body.size();
            out->body.clear();
        }
        return true;
    }

    // ===== AI(W906-SJSON-S8S9S10S11) 20260923 BEGIN -- 四條新通道的 schema =====
    //   四條都只有 /schema（值走 tag 串流），alarm.tail 另外有內容端點。
    //   ⚠ 值被壓縮過的通道（io 的 base64、motor 的定序陣列）**沒有 schema
    //     就無法解讀**，所以這幾條不是可選的附屬品，是契約本身。
    {
        std::string body;
        if      (path == "/api/struct/prod/schema")  body = ht9045::sjson::ProductionSchemaJson();
        else if (path == "/api/struct/io/schema")    body = W906_ApiCacheGet(0);  else if (path == "/api/struct/io/config") body = ht9045::sjson::IoConfigJson();  else if (path == "/api/struct/io/runtime") body = W906_ApiCacheGet(1);  // AI(W906-IO-POINTS) 20260924: HW.IoSetView 每個 Alias 的設定／即時狀態（JsonBridge/ChanIoPoints.cpp；契約同 IO-config.json／IO-runtime.json）；接在同一行，不移動其後行號  // AI(W906-IOWEB-P6) 20260924: schema/runtime read the 1203 monitor -> served from the tick-thread cache (EOF), never built on this socket thread
        else if (path == "/api/struct/motor/schema") body = W906_ApiCacheGet(2);  else if (path == "/api/struct/motor/config") body = ht9045::sjson::MotorConfigJson();  else if (path == "/api/struct/motor/runtime") body = W906_ApiCacheGet(3);  // AI(W906-MOTOR-POINTS) 20260924  // AI(W906-IOWEB-P6) 20260924: same cache
        else if (path == "/api/struct/act/schema")   body = ht9045::sjson::ActionSchemaJson();
        else if (path == "/api/struct/alarm.tail/schema")
                                                    body = ht9045::sjson::AlarmSchemaJson();
        else if (path == "/api/struct/alarm.tail") {
            // 內容：?since=<seq>&limit=<n>，與 log.tail 同一組參數名。
            unsigned long long since = 0;
            const std::string::size_type sp = query.find("since=");
            if (sp != std::string::npos)
                since = (unsigned long long)std::strtoull(query.c_str() + sp + 6, 0, 10);
            std::size_t limit = 0;
            const std::string::size_type lp = query.find("limit=");
            if (lp != std::string::npos)
                limit = (std::size_t)std::strtoul(query.c_str() + lp + 6, 0, 10);
            body = ht9045::sjson::AlarmTailJson(since, limit);
        }
        if (!body.empty()) {
            out->status = 200; out->reason = "OK";
            out->contentType = "application/json; charset=utf-8";
            out->body = body;
            out->headOnly = (method == "HEAD");
            if (out->headOnly) {
                out->contentLength = (long long)out->body.size();
                out->body.clear();
            }
            return true;
        }
    }
    // ===== AI(W906-SJSON-S8S9S10S11) 20260923 END =====

    // AI(W906-JSONBRIDGE-S2) 20260923: 依 FieldDesc 讀結構實例。
    //   GET /api/struct/<binding>          值
    //   GET /api/struct/<binding>/schema   欄位表（開站抓一次）
    //   綁定表在 JsonBridge/Bindings.cpp（手寫），欄位表在 gen/（產生）。
    {
        const std::string pre = "/api/struct/";
        if (path.size() > pre.size() && path.compare(0, pre.size(), pre) == 0) {
            std::string rest = path.substr(pre.size());
            bool wantSchema = false;
            const std::string sfx = "/schema";
            if (rest.size() > sfx.size() &&
                rest.compare(rest.size() - sfx.size(), sfx.size(), sfx) == 0) {
                wantSchema = true;
                rest = rest.substr(0, rest.size() - sfx.size());
            }
            const ht9045::sjson::Binding* b =
                ht9045::sjson::FindBinding(rest.c_str());
            if (b != 0) {
                out->status = 200; out->reason = "OK";
                out->contentType = "application/json; charset=utf-8";
                out->body = wantSchema ? ht9045::sjson::BindingSchemaJson(*b)
                                       : ht9045::sjson::BindingToJson(*b);
                out->headOnly = (method == "HEAD");
                if (out->headOnly) {
                    out->contentLength = (long long)out->body.size();
                    out->body.clear();
                }
                return true;
            }
            // 不是結構綁定就往下走，讓 machine.defines / log.tail 那兩條處理。
        }
    }

    if (path == "/api/struct" || path == "/api/struct/") {
        // 綁定清單 = 手寫的結構綁定（Bindings.cpp）＋ 兩個非結構的特例。
        out->status = 200; out->reason = "OK";
        out->contentType = "application/json; charset=utf-8";
        {
            const std::string s = ht9045::sjson::BindingIndexJson();
            // 把兩個非結構綁定接進同一個陣列 —— 瀏覽器只想知道「有哪些
            // /api/struct/<x> 可以問」，不該還要記得有兩種不同的清單。
            const std::string extra =
                "{\"name\":\"machine.defines\",\"type\":\"(compile-time)\","
                "\"writable\":false,"
                "\"note\":\"compile-time configuration of this exe\"},"
                "{\"name\":\"log.tail\",\"type\":\"(ring)\",\"writable\":false,"
                "\"note\":\"event log ring; write via WS log.event\"},"
                // ===== AI(W906-SJSON-S8S9S10S11) 20260923 BEGIN =====
                "{\"name\":\"prod\",\"type\":\"(tags)\",\"writable\":false,"
                "\"note\":\"S8 production counters; /schema only, values on the tag feed\"},"
                "{\"name\":\"io\",\"type\":\"(tags)\",\"writable\":false,"
                "\"note\":\"S9 io.di/io.do bit-packed base64; /schema only\"},"
                "{\"name\":\"motor\",\"type\":\"(tags)\",\"writable\":false,"
                "\"note\":\"S9 motor.axes ordered array; /schema only\"},"
                "{\"name\":\"alarm.tail\",\"type\":\"(ring)\",\"writable\":false,"
                "\"note\":\"S10 alarm event ring; answer via WS dialog.response\"},"
                "{\"name\":\"act\",\"type\":\"(commands)\",\"writable\":true,"
                "\"note\":\"S11 action channel; /schema only, invoke via WS act.*\"},"
                // ===== AI(W906-SJSON-S8S9S10S11) 20260923 END =====
                ;
            const std::string::size_type at = s.find("[");
            out->body = (at == std::string::npos)
                      ? s
                      : s.substr(0, at + 1) + extra + s.substr(at + 1);
        }
        out->headOnly = (method == "HEAD");
        if (out->headOnly) {
            out->contentLength = (long long)out->body.size();
            out->body.clear();
        }
        return true;
    }

    out->status = 404; out->reason = "Not Found";
    out->contentType = "application/json; charset=utf-8";
    out->body = "{\"error\":\"unknown struct binding\"}";
    return true;
}

// ---------------------------------------------------------------------------
//  Steven 20260924 (S12)：GET /api/form/、/api/form/<Page>
//
//  第二型 bridge（golden 原檔產生，FormBridge.h）把表單狀態送成 JSON。AI(W906-Q4-S126) 20260927：第一型（DoIniDataToForm＋哨兵）退役。
//  使用者 20260924：「DoIniDataToForm() 就等於是 C++ 發送 JSON 給 HTML」。
//  本體在 JsonBridge/FormJson.cpp；這裡只做 HTTP 的外殼。
//  用宣告而不 include FormJson.h：第一型還在時它帶進 forms/FormWidgets.h 與 ScrollBar.h，
//  本檔已經為了 TWinControl 重複定義避開過一次 header（見 fTrayForm->Init() 的註解）。
// ---------------------------------------------------------------------------
}  // namespace（匿名）—— 暫時關掉：這組宣告必須落在全域的 ht9045，不能是匿名 namespace 裡的另一個 ht9045
namespace ht9045 { namespace formjson {
int FormListJson(std::string* json);
int FormPageJson(const std::string& page, std::string* json);
int FormSave(const std::string& page, const std::string& widgetsJson,
             std::string* ackJson, std::string* err);
void FormLock();
void FormUnlock();
} }
namespace {

bool FormRoute(void* /*user*/, const std::string& method,
               const std::string& path, const std::string& /*query*/,
               webbridge::HttpResponse* out) {
    out->contentType = "application/json; charset=utf-8";
    if (method != "GET" && method != "HEAD") {
        out->status = 405; out->reason = "Method Not Allowed";
        // 審查更正 20260924：已有 form.save（golden bridge 的存檔），不是只能走 recipe.doc.put。
        out->body = "{\"error\":\"/api/form is read-only; save through WS form.save (golden bridge) "
                    "or recipe.doc.put (file mirror)\"}";
        return true;
    }
    int st;
    if (path == "/api/form" || path == "/api/form/") {
        st = ht9045::formjson::FormListJson(&out->body);
    } else {
        st = ht9045::formjson::FormPageJson(path.substr(10), &out->body);   // 10 = strlen("/api/form/")
    }
    out->status = st;
    out->reason = (st == 200) ? "OK" : "Not Found";
    out->headOnly = (method == "HEAD");
    if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
    return true;
}

// ---------------------------------------------------------------------------
//  Steven 20260925：密碼資料不可暴露給瀏覽器。
//  web\JSON 底下的設定快照（General-config.json／.js、Config.json／.js）是 Gerneral.ini／config.ini 的整份鏡像，
//  連 [VENDER] EPuser／EPPass／ConfigPass／ASEK15PassWord、[RMS_Password]、各組 FTP 密碼的值都在裡面，而 wb_serve 把
//  整個 web 目錄當網頁根目錄送出去。Steven 裁決 (A) 檔案裡的值清掉＋(B) 伺服器端也要擋：
//  所有 /JSON/*.json、*.js 由這裡送（不走靜態檔），送出前把「密碼類鍵」的 {"value":…,"raw":…} 字串清成 ""。
//  密碼類鍵＝鍵名以 password／pass／pwd 結尾（不分大小寫）或 EPuser；排除 b／i 開頭的旗標（bContinueFailBySocketPWD）
//  與含 need 的旗標（OutShuttleLoseICNeedPWD）。只動物件形式的值——uiMap 裡「鍵→元件 id」的純字串不動。
//  路徑先 %XX 解碼、\ 換 /、解掉 . 與 ..、比對不分大小寫，避免 /json/、%2D、/page/../JSON/ 繞過；
//  另擋 Windows 開檔的別名：結尾的點與空白、::$DATA 資料流、8.3 短檔名（換成長檔名後才判斷）。
// ---------------------------------------------------------------------------
static std::string g_scrubWebRoot;

static bool W906_CredKey(const std::string& k)
{
    if (k == "EPuser") return true;
    std::string l(k);
    for (std::size_t i = 0; i < l.size(); ++i) l[i] = (char)std::tolower((unsigned char)l[i]);
    auto ends = [&](const char* s) { const std::size_t m = std::strlen(s); return l.size() >= m && l.compare(l.size() - m, m, s) == 0; };
    if (!(ends("password") || ends("pass") || ends("pwd"))) return false;
    if (k.size() >= 2 && (k[0] == 'b' || k[0] == 'i') && (std::isupper((unsigned char)k[1]) || std::isdigit((unsigned char)k[1]))) return false;
    if (l.find("need") != std::string::npos) return false;
    return true;
}

// 回傳清過的文字；*n＝清掉幾個值
std::string W906_ScrubCredentials(const std::string& t, int* n)
{
    const std::size_t N = t.size();
    auto readStr = [&](std::size_t p, std::size_t* end) {   // t[p]=='"'
        std::string s; std::size_t q = p + 1;
        while (q < N && t[q] != '"') { if (t[q] == '\\' && q + 1 < N) { s += t[q + 1]; q += 2; continue; } s += t[q]; ++q; }
        *end = q < N ? q + 1 : N;
        return s;
    };
    auto skipWs = [&](std::size_t p) { while (p < N && std::isspace((unsigned char)t[p])) ++p; return p; };
    std::string o; o.reserve(N);
    std::size_t i = 0;
    while (i < N) {
        if (t[i] != '"') { o += t[i++]; continue; }
        std::size_t e; const std::string key = readStr(i, &e);
        o.append(t, i, e - i); i = e;
        const std::size_t c = skipWs(i);
        if (c >= N || t[c] != ':' || !W906_CredKey(key)) continue;
        const std::size_t b = skipWs(c + 1);
        if (b >= N || t[b] != '{') continue;                 // 只清物件形式 {"value":…,"raw":…}
        o.append(t, i, b + 1 - i); i = b + 1;
        int depth = 1; std::string last;
        while (i < N && depth > 0) {
            if (t[i] == '"') {
                std::size_t e2; const std::string s = readStr(i, &e2);
                const std::size_t m = skipWs(e2);
                if (depth == 1 && m < N && t[m] == ':') { last = s; o.append(t, i, e2 - i); i = e2; continue; }
                if (depth == 1 && (last == "value" || last == "raw") && !s.empty()) { o += "\"\""; if (n) ++*n; }
                else o.append(t, i, e2 - i);
                i = e2; last.clear(); continue;
            }
            if (t[i] == '{') ++depth; else if (t[i] == '}') --depth;
            o += t[i++];
        }
    }
    return o;
}

static bool W906_JsonScrubRoute(const std::string& method, const std::string& path, webbridge::HttpResponse* out)
{
    if (method != "GET" && method != "HEAD") return false;
    std::string d;                                            // %XX 解碼、\ → /
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '%' && i + 2 < path.size() && std::isxdigit((unsigned char)path[i + 1]) && std::isxdigit((unsigned char)path[i + 2])) {
            d += (char)std::strtol(path.substr(i + 1, 2).c_str(), nullptr, 16); i += 2;
        } else d += (path[i] == '\\') ? '/' : path[i];
    }
    std::vector<std::string> seg;                             // 解掉 . 與 ..
    bool colon = false;
    for (std::size_t p = 0; p <= d.size(); ) {
        std::size_t q = d.find('/', p); if (q == std::string::npos) q = d.size();
        std::string s = d.substr(p, q - p);
        if (s != "." && s != "..")                            // Windows 開檔會去掉結尾的點與空白（Config.json. 開到 Config.json）
            while (!s.empty() && (s.back() == '.' || s.back() == ' ')) s.pop_back();
        if (s.find(':') != std::string::npos) colon = true;   // 資料流（Config.json::$DATA）也會開到同一個檔
        if (s == "..") { if (seg.empty()) return false; seg.pop_back(); }
        else if (!s.empty() && s != ".") seg.push_back(s);
        p = q + 1;
    }
    auto lower = [](std::string s) { for (auto& ch : s) ch = (char)std::tolower((unsigned char)ch); return s; };
    if (seg.empty()) return false;
    if (colon && lower(seg[0]) != "api") {                    // 網頁路徑不會有冒號
        out->status = 404; out->reason = "Not Found"; out->contentType = "text/plain"; out->body = "not found";
        return true;
    }
    if (seg.size() < 2) return false;
    // 8.3 短名（JSON~1\GENERA~1.JSO）也會開到同一個檔：換成完整長檔名後再判斷是不是 web\JSON 底下
    auto longPath = [](const std::string& p) {
        char full[4 * MAX_PATH]; DWORD m = GetFullPathNameA(p.c_str(), sizeof full, full, nullptr);
        if (!m || m >= sizeof full) return p;
        char lp[4 * MAX_PATH]; DWORD k = GetLongPathNameA(full, lp, sizeof lp);
        return (k && k < sizeof lp) ? std::string(lp, k) : std::string(full, m);
    };
    std::string fs = g_scrubWebRoot;
    for (const auto& s : seg) { fs += '\\'; fs += s; }
    fs = longPath(fs);
    const std::string root = lower(longPath(g_scrubWebRoot)) + "\\";
    const std::string rel = lower(fs);
    if (rel.compare(0, root.size(), root) != 0) return false;
    if (rel.compare(root.size(), 5, "json\\") != 0) return false;
    const bool isJson = rel.size() > 5 && rel.compare(rel.size() - 5, 5, ".json") == 0;
    const bool isJs = rel.size() > 3 && rel.compare(rel.size() - 3, 3, ".js") == 0;
    if (!(isJson || isJs)) return false;
    // AI(W906-SMM) 20260925: 執行期信箱覆蓋 —— 同 WebBridge/HttpStatic.cpp 的 W906_RuntimeJsonOverlayTarget（AI(W906-MAILBOX-RT) 20260924）：
    //   /JSON/<x> 先找 JSON\runtime\<x>，沒有才用版控樣本。這條路由接走所有 /JSON/* 之後靜態檔那一層的覆蓋就被繞過了 ⇒
    //   /JSON/Message-dialog-request.json（與 Alarm-／Dialog-close-request）送的永遠是版控的 idle 樣本，C++ 寫進 runtime 的
    //   告警／訊息／關框請求**瀏覽器看不到**（20260925 showmymessage_probe.py --mailbox-only 實測：/JSON/… idle、/JSON/runtime/… pending）。
    if (rel.compare(root.size() + 5, 8, "runtime\\") != 0) {
        const std::string ov = longPath(g_scrubWebRoot) + "\\JSON\\runtime\\" + fs.substr(root.size() + 5);
        const DWORD oa = GetFileAttributesA(ov.c_str());
        if (oa != INVALID_FILE_ATTRIBUTES && (oa & FILE_ATTRIBUTE_DIRECTORY) == 0) fs = ov;
    }
    std::string text;
    if (!ReadWholeFile(fs, &text)) return false;             // 不存在 → 交回靜態檔（照舊 404）
    int n = 0;
    out->body = W906_ScrubCredentials(text, &n);
    out->status = 200; out->reason = "OK";
    out->contentType = isJson ? "application/json; charset=utf-8" : "text/javascript; charset=utf-8";
    out->extraHeaders["Cache-Control"] = "no-store";
    if (n) out->extraHeaders["X-W906-Scrubbed"] = std::to_string(n);
    out->headOnly = (method == "HEAD");
    if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); }
    return true;
}
static bool g_tcAllowCmd = false;   //AI(W906-GB-P3) 20260926: set from allowCmd at H2 (:4351); read by H1b (/api/testercomm POST, /api/ela/query). AI 20260927: allowCmd is always true since ZEROARG (:3670), so both POSTs are always allowed here
bool ApiRoute(void* user, const std::string& method,
              const std::string& path, const std::string& query,
              webbridge::HttpResponse* out) {
    // Steven 20260925：/JSON/*.json、*.js 送出前清掉密碼值（見上方 W906_JsonScrubRoute）。路由前綴因此改成 "/"。
    if (W906_JsonScrubRoute(method, path, out)) return true;
    //Steven 20260924 (S12-C)：GET /api/editlist/<elConfig|cbLastSet|elConfig_byRecipe|elUdUld>
    //  —— golden ReadEditTextFromFile＋InitialDataToEdit 之後各筆的值，id＝THTEdit::ControlName（HTML 元件名稱）
    if (path.compare(0, 14, "/api/editlist/") == 0) {
        out->contentType = "application/json; charset=utf-8";
        int st;
        {
            ht9045::formjson::FormLock();
            st = FileRW_IniConfig_Json(path.substr(14), &out->body);
            ht9045::formjson::FormUnlock();
        }
        out->status = st;
        out->reason = st == 200 ? "OK" : st == 405 ? "Method Not Allowed" : st == 409 ? "Conflict" : "Not Found";
        return true;
    }
    if (path.compare(0, 9, "/api/form") == 0 &&
        (path.size() == 9 || path[9] == '/'))
        return FormRoute(user, method, path, query, out);
    if (path.compare(0, 11, "/api/recipe") == 0)
        return RecipeRoute(user, method, path, query, out);
    if (path.compare(0, 11, "/api/system") == 0)
        return SystemRoute(user, method, path, query, out);
    if (path.compare(0, 9, "/api/text") == 0)
        return TextRoute(user, method, path, query, out);
    if (path.compare(0, 11, "/api/struct") == 0)
        return StructRoute(user, method, path, query, out);
    { int st = 200; std::string ct, bd; if (::W906_TesterCommHttp(method, path, query, g_tcAllowCmd, &st, &ct, &bd)) { out->status = st; out->reason = st == 200 ? "OK" : st == 403 ? "Forbidden" : st == 404 ? "Not Found" : st == 405 ? "Method Not Allowed" : "Bad Request"; out->contentType = ct; out->body = bd; out->headOnly = (method == "HEAD"); if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); } return true; } }   /* AI(W906-GB-P3) 20260926: H1b -- GET/POST /api/testercomm/<gpib|rs232|tcpip> (TesterComm/Handler/TesterCommWiring.cpp); every other path falls through */  { int st = 200; std::string ct, bd; if (::W906_ElaHttp(method, path, query, g_tcAllowCmd, &st, &ct, &bd)) { out->status = st; out->reason = st == 200 ? "OK" : st == 202 ? "Accepted" : st == 403 ? "Forbidden" : st == 404 ? "Not Found" : st == 503 ? "Service Unavailable" : "Bad Request"; out->contentType = ct; out->body = bd; out->headOnly = (method == "HEAD"); if (out->headOnly) { out->contentLength = (long long)out->body.size(); out->body.clear(); } return true; } }   /* AI(W906-ELA-P3) 20260927: GET /api/ela, POST /api/ela/query (EventLogAnalysis/ElaService.cpp) */  return false;                     // 不是我們的 -> 交回靜態檔處理
}

// ---------------------------------------------------------------------------
//  AI(W906-JSONBRIDGE-S0) 20260923: 額外 tag 發布者是**單一**函式指標
//  （WebBridgeTags.h 的 ExtraTagPublisher），SECS 已經佔用了它。所以這裡做
//  組合，而不是把 SECS 換掉 —— 換掉的話 secs.sv.* 會無聲消失。
//
//  兩者都在 PublishHandlerTags() 內部、commitPublish() 之前被呼叫，
//  回傳筆數都已計入 stagedTagCount()。
// ---------------------------------------------------------------------------
std::size_t PublishExtraTags(webbridge::TagSnapshot& snap) {
    std::size_t n = ht9045::PublishSecsSvTags(snap);
    n += ht9045::sjson::PublishConfigTags(snap);
    n += ht9045::sjson::PublishEventLogTags(snap);   // AI(W906-JSONBRIDGE-S1)
    // AI(W906-JSONBRIDGE-S7) 20260923: 71 個溫控通道 × 3 個有全域撐著的欄位。
    //   今天全部 null —— 三個資料源都不活（證據在 StageThermo.h 檔頭）。
    //   ⚠ 這 213 個 null **不是**佔位符，它們精確表示「通道存在、值還沒有
    //     來源」。而且 patch 協定只送變動，所以第一次 publish 之後它們
    //     不佔頻寬；等資料源活了，變動會自己流出去。
    n += ht9045::sjson::PublishThermoTags(snap);
    // ===== AI(W906-SJSON-S8S9S10) 20260923 BEGIN -- 四個新發布者 =====
    //   全部是純讀取 + stage，沒有 I/O、沒有鎖、沒有等待（派工單第 3 條）。
    //   ⚠ 一樣必須在 commitPublish() 之前 —— 本函式整個就跑在那個窗口內。
    n += ht9045::sjson::StageProduction(snap);   // S8  prod.*   （含差量）
    n += ht9045::sjson::StageIo(snap);           // S9  io.di / io.do
    n += ht9045::sjson::StageMotor(snap);  n += ::W906_StageMotionViewTrays(snap);        // S9  motor.axes; AI(W906-S118) 20260928 (St02-E, laptop-approved claim): + the main-screen trays motionView.trays.* (34, W54 = A)
    n += ht9045::sjson::PublishAlarmTags(snap);  // S10 alarm.*  （只送游標）
    // ===== AI(W906-SJSON-S8S9S10) 20260923 END =====
    return n;
}

} // namespace

// AI(W906-NOGATE) 20260918：**預設改成 true —— 這不再是一道閘門。**
//   使用者裁決原話：「不用 --allow-system-write，一律都要實際讀寫，不會影響我的運作，
//   我要手動驗證」「沒有實際修改我都無法手動驗證」。
//
//   理由不是「風險變小了」，是**擋住寫入等於讓驗證跑在假路徑上，等於沒驗到**。
//   取代這道閘門的是「備份 → 驗證 → 刪備份」（CLAUDE.md 的 V906 寫入邊界那節）：
//   動真實檔前先備份，驗完比對內容，通過就刪備份，有問題就還原。
//
//   ⚠ `--allow-system-write` 這個旗標**仍然接受**（不然既有的 launch.json 與腳本
//     會撞到 "unknown argument" 然後 return 2），但它現在**什麼都不選** ——
//     與 `--production-recipe` 在裁決 A1.1 之後的處境相同。
//
//   ⚠ 底下三處 `if (!gAllowSystemWrite && ...)`（recipe / system / text 三條寫入路徑）
//     因此**變成不可達**。刻意留著而不刪：它們是這道閘門原本的形狀，
//     萬一裁決被推翻，改回來是一行的事。**不要把它們讀成「還有防護」** ——
//     這棵樹有一條 memory 叫「不可能當掉的 gate 不是 gate」。
namespace { bool gAllowSystemWrite = true; }

// AI(W906-ST-S3-B2) 20260918: the server tick, named once so the pump banner and
//   the loop cannot drift apart. It is ALSO the golden spine's tick now, which is
//   why it gets a name instead of staying a literal in two places.
//   ⚠ 500 ms is a deliberate choice, not a leftover. golden's driver is
//   uruncontrol.cpp's `TRunControl : TThread` at timeBeginPeriod(1): MainProc()
//   every ~1 ms idle, ~2 ms per 3 ticks while running. So this is 500x slower,
//   not 16x. Fine for "press START and watch SystemStart flip"; three orders of
//   magnitude out for actually running a lot. Raising the number is NOT the fix
//   for that -- golden pumps on the UI thread via Synchronize() and this loop has
//   no UI thread, so it is a different execution model and its own decision.
//   Measured and written up in docs/RULINGS_20260917.md B13.
namespace { const int kServeTickMs = 500; }

//AI(W906-Q34-7) 20260923: 1203 IO 讀取的時鐘，與上面的 kServeTickMs **分開**。
//  照翻同事 wb_publish.cpp:355-361（1203CTL-21，使用者原話「IO 讀取燈號 我需要0.2秒的
//  更新率」）：燈號多快跟上感測器，與翻譯過來的引擎多久跑一次，是兩個不同的問題。
//  把 kServeTickMs 改成 200 會讓 MainProc 跟著快 2.5 倍 —— 那是 B13 裁決的範圍，不是這裡。
//  ⓘ 他那邊可以用環境變數 WB_PUBLISH_IO_TICK_MS 覆寫（wb_publish.cpp:377）；A 不取
//    （ZEROARG：零參數 = 全功能），所以是常數，跟 kServeTickMs 一樣只寫一次。
//  ⓘ 只有 INSTALL_1203_MONITOR 編進來時主迴圈才用它（Poll 那段）。
namespace { const int kIoTickMs = 200; }

// AI(W906-ST-S3-B4) 20260918: the TfMainWeb instance fMain is repointed at in
//   main(). Kept as a TYPED pointer as well, because fMain is a TfMain* and
//   StartFromWeb() is not on the base class -- deliberately, so that the ~19
//   fMain->Start() call sites keep getting the empty base body.
namespace { TfMainWeb* g_webMain = 0; }

// ---------------------------------------------------------------------------
//  Steven 20260924 (S12)：存檔後重讀
//
//  使用者 20260924 裁決（decisions.md 二之二）：recipe.doc.put 寫檔成功後，
//  重跑該文件對應的 golden ReadFile() ＋ DoStructUnitConvert()。
//  golden 就是這樣做的：存檔 → ReadFile() → DoIniDataToForm()
//  （cSetUp.cpp:4040-4042、cContact.cpp:1174-1175、cHotPlate.cpp:404-405）。
//  沒有這一步，wb_serve 的結構只在開機讀一次，頁面改讀 /api/form 之後
//  存完會看到舊值，像是沒存到。
//
//  對照表 = 開機讀檔鏈（main() 的 recipe 區塊）裡「讀這個文件的那幾支」，照開機順序。
//  ⚠ ReadFile() 會寫檔（補鍵、鉗制寫回），每次存檔都會發生 —— golden 也是。
//  ⚠ temperature 刻意不重讀：ATC.ini 沒被讀，ReadTempFile() 會把 Chiller Temp
//    夾錯並寫回（porting-gaps.md 十三）。修好之前重讀等於每存一次就寫壞一次。
//  ⚠ binasgn* 沒有列：bin.* 鏈（fBinSel->ReadFile ＋ ShowBinSel）還沒整理成可重入，
//    存了 Bin 頁要重啟 wb_serve 才會反映。
// ---------------------------------------------------------------------------
static bool g_recipeChainsReady = false;   // 開機讀檔鏈跑完才成立

//  Steven 20260924 審查更正（高級審查員 #1，CONFIRMED）：原本只重讀「讀這個文件的那幾支」，
//    但讀檔器之間有依賴 —— golden TfHotPlate::ReadFile 讀 TestIF_File.iTestMode（cHotPlate.cpp:185-263）、
//    TfContact::ReadFile 讀 TestIF_File（cContact.cpp:497-520）、TfTrayAssignment::ReadFile 讀
//    ArmSpeed_File[InArm].bAutoSKIP（cTrayAssignment.cpp:347）、TfSpeed::ReadFile 讀 TrayForm.Loader.iTrayType
//    （cSpeed.cpp:396）。存 HandlerCondition 改 Test Mode 之後只重讀 fSetup，HotPlate／Contact 就停在舊 mode 的鉗制結果。
//    ⇒ 照 file-io-mechanisms.md §D 的建議「走總指揮同一段順序」：任何配方文件存檔後，**整條開機讀檔鏈照開機順序重跑**
//    （main() recipe 區塊，本函式與那裡同序），temperature 與 bin.* 仍排除（理由見上）。
//  Steven 20260924 審查更正（#2）：ReadFile 丟例外（vclcompat StrToFloat 遇到非數字會丟，SysUtils.h:38）時
//    原本鎖不會放、WS 迴圈也沒有 catch → /api/form 永久卡住或整個 wb_serve 掛掉。改 RAII 鎖＋catch(...)，錯誤進 ack。
struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

static std::string ReloadRecipeDocAfterSave(const std::string& doc) {
    if (!g_recipeChainsReady) return "skipped: boot read chain did not run";
    if (StemFoldEq(doc, "temperature"))
        return "skipped: Temperature.Data is C-route owned (FileRW/Temperature.cpp; golden spbSaveClick rereads ReadTempFile(true))";   //Steven 20260925：ATC.ini 已補讀（W906_ReadATCIni）
    if (doc.size() >= 7 && StemFoldEq(doc.substr(0, 7), "binasgn"))
        return "skipped: bin.* chain is not re-entrant yet -- restart wb_serve to pick up Bin changes";
    extern void W906_BootReadHotPlateAndSpeed();
    extern void W906_BootReadTrayAssignment();
    FormLockGuard lock;
    try {
        // 與 main() 開機讀檔鏈同序（golden TfMain::DoReadLastData main.cpp:8898-9007 的順序）
        { extern void FileRW_TrayForm_ReadFile(); FileRW_TrayForm_ReadFile(); }   //Steven 20260924 (S12-C)：golden TfTrayForm::ReadFile
        fSetup->ReadFile();
        fContactForm->ReadFile();
        { extern void FileRW_TesterIF_ReadTestIFFile(); FileRW_TesterIF_ReadTestIFFile(); }   //Steven 團隊 20260925（暫接，整合者定）：golden DoReadLastData main.cpp:9327 FTestIF->ReadTestIFFile() 完整版（FileRW/TestIF_File_TesterIF.cpp），取代 FileRW_TTLCfg_ReadDIOSection（它是 [DIO] 子集）   //Steven 20260925：golden main.cpp:9327 ReadTestIFFile 的 [DIO] 段（只讀；iDioMode／sDioName → TFTestIF cbDIOType）
        fYieldMonitoring->ReadFile();
        fHotPlate->ReadFile();
        fBarCode->ReadFile();
        fOffSet->ReadFile();
        fAutoAlignment->ReadFile();
        fTrayMappingForm->ReadFile();
        /* fQAMode->ReadFile(); */   // AI(W906-RJ03) 20260925: moved to :2643 -- golden main.cpp:8924-8926 reads fTrayAssignment BEFORE fQAMode
        /* fMagazine->ReadFile(); */ // AI(W906-RJ03) 20260925: moved to :2643 with it (golden :8928 follows :8926)
        fFixAICCD->ReadFile();
        DoStructUnitConvert();
        W906_BootReadHotPlateAndSpeed();     // golden :8919 fHotPlate、:8921 fLd_ULd、:8923 fSpeed
        W906_BootReadTrayAssignment();  fQAMode->ReadFile();  { extern void FileRW_QAMode_DoIniDataToForm(); FileRW_QAMode_DoIniDataToForm(); }  fMagazine->ReadFile();  // golden :8925 fTrayAssignment（得在 fSpeed 之後）  // AI(W906-RJ03) 20260925: QAMode/Magazine after TrayAssignment (QAMode seeds "Tray Direct" from TrayForm.Loader.Direction, which fTrayAssignment reads)
        // 高級審查員第二輪（中）：golden 只有 TfSetup（cSetUp.cpp:3643）與 TFTestIF（cTesterIF.cpp:1374）
        // 存檔後叫 SetWorkParameter；HotPlate／TrayForm／Ld_ULd／TrayAssignment 的存檔鈕都沒有。
        // 它會經 ChangeSite() 重算 site map／iInArmType，所以只在那兩個文件存檔時呼叫。
        if (StemFoldEq(doc, "handlerCondition") || StemFoldEq(doc, "tester"))
            SetWorkParameter();
    } catch (const std::exception& e) {
        return std::string("reload FAILED (golden reader threw: ") + e.what() + ") -- structs may be partly reloaded";
    } catch (...) {
        return "reload FAILED (golden reader threw a non-std exception) -- structs may be partly reloaded";
    }
    return std::string("reloaded: full boot read chain (TrayForm..FixAICCD, DoStructUnitConvert, HotPlate/Ld_ULd/Speed, "
                       "TrayAssignment") +
           ((StemFoldEq(doc, "handlerCondition") || StemFoldEq(doc, "tester")) ? ", SetWorkParameter" : "") +
           "); temperature/bin excluded";
}

// =============================================================================
//  AI(W906-RCHG) 20260925（Steven 團隊）：golden TfMain::DoReadLastData（V912 main.cpp:9264-9441）的移植 ——
//  開機（main() 配方區塊，bBoot=true）與換配方（WebRecipeChange.cpp 的 ChangeSetUpFile :25724／:25772，bBoot=false）
//  共用這一份。本段原本是 main() 裡 `if (pathReady) { … }` 的內容，20260925 整段搬出來並照 golden 順序重排
//  （bin 移到 fQAMode／fMagazine 之後、fOffSet 與 fAutoAlignment 移到 golden 的位置、DoStructUnitConvert 移到最後），
//  另補 golden 有、原本沒讀的：ReadLastDataFile、iMachineTempMode、fLaserSensor、第二次 fSetup／ReadTempFile／fMagazine、
//  fSCKART、fTeach、DoIniDataToForm 段、fDIOFrom LoadData、ReadPassword、fStartCondition。移動清單見交件報告。
//  ⚠ 只做一次的建構（Init()、ATC.ini、登入、LastSet.ini）一律包 if (bBoot)；FileRW_*_Boot() 自己有 g_booted，可重入。
//  ⚠ 以後要在這條鏈加東西：照 golden DoReadLastData 的行號插在對應位置；只做一次的放 if (bBoot)。
//  ⚠ ReloadRecipeDocAfterSave()（存檔後重讀）還是自己那一份舊順序，沒有改（範圍外，見交件報告）。
// =============================================================================
void W906_DoReadLastData(bool bBoot, bool& binSelLoaded, bool& tempLoaded)
{
    // ---- golden :9266 `static bool bFirst=false;` —— 移植樹用參數 bBoot（開機那一次＝golden 的第一次呼叫）----
    // golden :9267-9282 jou 2010-01-15 增加加熱offset：Position Offset Hot.Data 不存在就從 Position Offset.Data 複製
    //   ⓘ golden 的路徑前後帶雙引號（"\"%s" …  "\"" ），FileExists 恆為 false、CopyFile 恆失敗 —— 實際是 no-op，照翻不修
    {
        AnsiString str3, asDir="", asHotDir, asAmbientDir;
        str3=fOffSet->GetOffsetPath();
        asDir.sprintf("\"%s", str3.c_str());                                     //Steven 20100927 Start : Offset改資料夾
        asHotDir=asDir+"\\Position Offset Hot.Data\"";
        asAmbientDir=asDir+"\\Position Offset.Data\"";
        if(!FileExists(asHotDir))
            CopyFileA(asAmbientDir.c_str(), asHotDir.c_str(), false);
    }
    {   // golden :9284-9290 Steven 20100812
        bool bRetrun=false;
        bRetrun=ReadLastDataFile();
        if(bRetrun==false)
            ShowErrorMessage("WAR1681", 0, MMSystem, 0, "ReadLastDataFile");
    }
    // ---- golden :9294 fTemp_Set->ReadTempFile(true) ----
    // AI(W906-FW-TEMP2) 20260820: temp.sv/soak/mode, SAME redirected-
    // DataPath window as bin.* directly above -- main-loop ruling
    // supersedes FW-TEMP1's "no wire" verdict (WebBridgeTags.h's
    // AI(W906-FW-TEMP1) block, part (a), now corrected there too):
    // FW-BIN1's scratch-copy above copies the WHOLE recipe folder,
    // and Temperature.Data lives in that SAME folder, so
    // ReadTempFile's MyForceDirectories()/CheckAndReadIniData()/
    // WriteIniData() calls all land in scratch here -- the identical
    // protection bin.* already relies on, not a new exemption.
    // golden boot shape: main.cpp:8868 (TfMain::DoReadLastData)
    // `fTemp_Set->ReadTempFile(true);` -- called TWICE there (the
    // second time is ordering-dependent on fSetup->ReadFile(), which
    // this tool never brings up), so ONE call is the faithful subset;
    // FormShow's own `ReadTempFile(true);` (uTemp_Set.cpp:872) is the
    // same single-call shape.
    //
    // Init() NOT called -- audited, not assumed (20260820): Init()
    // (uTemp_Set.cpp:281-597) does zero file/hardware I/O and zero
    // fMain/other-form dereferences (grepped its whole body for
    // ReadIniData/WriteIniData/CheckAndReadIniData/MyForceDirectories/
    // FileExists/CopyFile/DeleteFile/"fMain->"/"fLotInfo->"/
    // "ATC_InterfaceForm->": 0 hits), so it WOULD be safe to call --
    // but ReadTempFile does not need it: grepping ReadTempFile's own
    // span (uTemp_Set.cpp:2172-3354) for every global Init() populates
    // (myTempPal[]/listNormal/listArm1/listArm2/ATCOffsetEdit[]/
    // ATCPackageOffsetEdit[]/ATCPackageTempEdit[]/ATC_FFCOffset*Edit
    // [][]/ATC_FFCPointUse[][]/ZoneTempUse[]/ZoneTempSetting[]/
    // MultiSensorOffsetUse[]/ATC_MultiSensorOffsetEdit[]) is 0 hits.
    // Every WIDGET ReadTempFile does touch (rgTemperatureMode,
    // edChillerTemp, cbbATC_RecipeFile, rgIndexHeatMode) is NSDMI'd in
    // forms/fTemp_Set.h (`= new T...()` at the member declaration), so
    // each is already a live object the moment `new TfTemp_Set()`
    // below runs -- Init() is not in the dependency chain for these 3
    // tags. Calling it anyway would be pure unused surface, the
    // opposite of the minimal-footprint bin.* precedent set
    // (SetTechDataToProd_Yield() called directly, not the
    // InitialOK-gated SetTechDataToProd orchestrator).
    //
    // fDynamicTemp stays NULL (never `new`'d) -- ReadTempFile's own
    // `if(fDynamicTemp!=NULL)` guard (uTemp_Set.cpp:2266) skips that
    // block entirely, golden-faithful.
    //
    // THIRD write mechanism found, beyond MyForceDirectories/
    // CheckAndReadIniData (both already on record): a bare
    // `WriteIniData(szDir,"ATC","Chiller Temp",...)` at
    // uTemp_Set.cpp:2795, gated behind
    // `ATC_SYSTEM>eATC30 && ATC_SYSTEM==eNewATCSystem` (:2733/:2746).
    // Whether it fires depends on this box's ATC_SYSTEM config; either
    // way it lands in the SAME redirected scratch folder, so it does
    // not change the go/no-go call -- recorded here for completeness.
    //
    // Liveness: golden's OWN "file missing" signal, iSendChangeTempError
    // (uTemp_Set.cpp:2200, `iSendChangeTempError=1; return;` on the
    // unconditional FileExists() check that gates the WHOLE function
    // before bUpdateAll is even tested) -- reset it first so a stale
    // value from an earlier call cannot be mistaken for this one.
    if (fTemp_Set == NULL) fTemp_Set = new TfTemp_Set();   // ctor is fields-only, uTemp_Set.cpp:272-274
    { extern void FileRW_Temperature_Boot(); FileRW_Temperature_Boot(); }   //Steven 20260925 (S12-C Temperature)：DFM 設計期狀態要在 golden 建構子（Init）之前
    //AI(W906-FW-TEMP2) 20260820: Init() IS required before ReadTempFile
    // after all -- the wave's audit grepped ReadTempFile's own span only,
    // but ReadTempFile chains into DoIniDataToForm (uTemp_Set.cpp:3355),
    // which reads myTempPal[] (20 sites) and every other Init()-populated
    // array. Measured: gdb bt on the e2e SEGV lands in DoIniDataToForm;
    // Init() itself is pure in-memory widget wiring (the audit's own
    // 0-hit grep for file/hw/global-form calls stands).
    if (bBoot) fTemp_Set->Init();   // AI(W906-RCHG) 20260925: 只做一次（golden 建構子）
    { extern void FileRW_Temperature_BootPanels(); FileRW_Temperature_BootPanels(); }   //Steven 20260925：myTempPal[] 包裝（Init 建的面板）
    if (bBoot) { extern void W906_ReadATCIni(); W906_ReadATCIni(); }   //Steven 20260925：golden main.cpp:9794-9797 ReadATCModeType（iATC_MODE_TYPE），早於 DoReadLastData→ReadTempFile（porting-gaps 十三）   // AI(W906-RCHG) 20260925: 只在開機（golden FormShow，不在 DoReadLastData）
    iSendChangeTempError = 0;
    fTemp_Set->ReadTempFile(true);
    tempLoaded = (iSendChangeTempError != 1);
    std::printf(tempLoaded
        ? "temp.* chain loaded: ReadTempFile -> fWorkTemperBase/fSoakTime/iMachineTempMode\n"
        : "temp.* chain: Temperature.Data missing in the recipe folder -- stays null\n");

    // golden :9296-9320 iMachineTempMode → LastSet.iTemperature（bLastSetInSetUpFile 時照寫 TestMode.Data）
    if(Temperature.iMachineTempMode==0)
    {
        LastSet.iTemperature=Tempture_Hot;
        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();
        }
    }
    else if(Temperature.iMachineTempMode==1)
    {
        LastSet.iTemperature=Tempture_Ambient;
        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();
        }
    }
    else if(Temperature.iMachineTempMode==3)                                    //kevin 20140918 恆溫控制
    {
        LastSet.iTemperature=Tempture_AmbientHot;
        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();
        }
    }
    // ---- golden :9322 fTrayForm->ReadFile() ----
    // Init() FIRST: golden allocates tSiteMap in TfSetup's CONSTRUCTOR
    // (cSetUp.cpp:185 `tSiteMap=new TStringList();`), so it is always live
    // before any ReadFile.  This port moved that allocation into
    // TfSetup::Init() (cSetUp.cpp), and nothing here called Init(), so
    // ReadFile's two `tSiteMap->Strings[...]` writes (golden :2571/:2589)
    // dereferenced a null pointer -- a hard SIGSEGV during boot, confirmed
    // under gdb.  Init() is golden's ctor body (:123-216): widget setup
    // only, no hardware and no file I/O.
    //
    // AI(W906-SETUP-READFILE) 20260922: HandlerCondition.Data
    // [Configuration] -- golden TfMain::DoReadLastData main.cpp:8900,
    // placed here because golden reads temp first (:8868) then fSetup
    // (:8900).  This is the call the FW-TEMP2 note above says "this tool
    // never brings up" -- it does now, so that note is history.
    //
    // AI(W906-TRAY-READ) 20260923: Tray.Data -> UserDefForm_File[]，照 golden 順序放在 fSetup 之前
    //   （TfMain::DoReadLastData main.cpp:8898 fTrayForm->ReadFile()，:8900 fSetup->ReadFile()）。
    //   在這之前 Tray.Data 從沒被讀過：UserDefForm_File[0].YPitch=0，網頁 START 時
    //   AutoCalculateInArmYClosePitch 以 0 當步進而無窮迴圈（c73a073 的 PITCH0 暫時解擋的就是它）。
    //   elTrayForm（golden 在 TfMain::TfMain 的 main.cpp:1487 建立）由 fTrayForm->Init() 在第一次呼叫時建立 ——
    //   本檔不能 include Public/HTEditList.h（HTEdit.h 與 language.h 各有一份 TWinControl），理由寫在 Init()。
    //   同一個 redirected-DataPath 視窗（ReadFile 用 DataPath + GetLastOpenFN()）。
    //Steven 20260924 (S12-C UserDefForm_File)：golden TfTrayForm 建構子＋ReadFile 改由 FileRW/UserDefForm_File.cpp
    //   （gen_editlist 從 golden 轉，元件有名稱；fTrayForm 的真元件被「收養」成替身，移植樹其他程式照用）
    { extern void FileRW_TrayForm_Boot(); extern void FileRW_TrayForm_ReadFile();
      FileRW_TrayForm_Boot(); FileRW_TrayForm_ReadFile(); }
    std::printf("tray.* chain loaded: Tray.Data -> UserDefForm_File"
                " (Type0 X Pitch=%.3f, Y Pitch=%.3f, X/Y Div=%d/%d)\n",
                UserDefForm_File[0].XPitch, UserDefForm_File[0].YPitch,
                UserDefForm_File[0].XDivision, UserDefForm_File[0].YDivision);

    // ---- golden :9324 fSetup->ReadFile() ----
    // WHY IT MATTERS: TfSetup::ReadFile is the ONLY reader of the
    // [Configuration] section (96 ReadIniData calls, 80 TestIF_File
    // fields).  Until it ran every one sat at its static-init value,
    // including dSiteXPitch / dSiteYPitch (golden :2445-2446), which
    // cUnitConvert feeds to ~2200 site-geometry call sites.  Same
    // redirected-DataPath window as bin.* and temp.* above.
    if (bBoot) fSetup->Init();   // AI(W906-RCHG) 20260925: 只做一次（golden 建構子 cSetUp.cpp:123-216）
    { extern void FileRW_Setup_Boot(); FileRW_Setup_Boot(); }
    { extern void FileRW_Cleaning_Boot(); FileRW_Cleaning_Boot(); }   //Steven 20260925 (S12-C TestIF_File_Cleaning)：golden CreateForm(TfCleaning) HT9045.cpp:226（建構子 :44→:51 LoadAutoCleanData）；要在下面 fSetup->ReadFile（golden DoReadLastData）之前——cSetUp.cpp 的 G-SU-Clean hook 由它設
    { extern void FileRW_StartCondition_Boot(); FileRW_StartCondition_Boot(); }   //Steven 20260925 (S12-C StartCondition)：golden CreateForm(TfStartCondition) HT9045.cpp:205（TfSetup :184 之後）；沒有 HTEditList，只建替身   //Steven 20260925 (S12-C TestIF_File_SetUp)：golden TfSetup 替身，要在 fSetup->Init() 之後（tSiteMap 由 Init 建）
    fSetup->ReadFile();
    std::printf("setup.* chain loaded: HandlerCondition.Data [Configuration]"
                " -> TestIF_File (X Pitch=%.3f, Y Pitch=%.3f)\n",
                TestIF_File.dSiteXPitch, TestIF_File.dSiteYPitch);

    // ---- golden :9326 fContact->ReadFile()、:9328 FTestIF->ReadTestIFFile() ----
    // AI(W906-CT-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8902 -- fContact->ReadFile() runs TWO lines after
    // fSetup->ReadFile() (:8900), so it goes here.
    //
    // ⚠ NOT `fContact`.  In this tree `fContact` is TfContactShim
    // (atester_shims.h:251), which has no ReadFile at all; the real
    // TfContact had no instance anywhere until today.  See the banner
    // on `fContactForm` in forms/fContact.cpp for why a separately
    // named pointer, and what that does and does not buy.
    //
    // Init() first, for the same reason fSetup->Init() is above:
    // golden does this work in the constructor (cContact.cpp:107-276)
    // and this tree moved it to Init() (DEVIATION D-2).
    //
    // ⚠⚠ THIS CALL WRITES, and one of the writes is safety-relevant:
    // golden :404/:408/:420/:421 push DeviceForm_File.IndexContact[0]
    // and [1] -- the INDEX CONTACT HEIGHT -- back into the file through
    // ReadWriteIni(..., write=true), clamped to [1.0, fIndexDownPos].
    // golden's own comment there is "kevin 20180402 0.0 會撞機".
    // Four more plain WriteIniData calls follow (:524/:567/:579/:653).
    // Kept verbatim; the clamp is golden's protection, not ours.
    if (bBoot) fContactForm->Init();   // AI(W906-RCHG) 20260925: 只做一次（golden 建構子 cContact.cpp:107-276）
    fContactForm->ReadFile();
    { extern void FileRW_TesterIF_BootReadTestIFFile(); extern void FileRW_TesterIF_ReadTestIFFile(); if (bBoot) FileRW_TesterIF_BootReadTestIFFile(); else FileRW_TesterIF_ReadTestIFFile(); }   //Steven 團隊 20260925（暫接，整合者定）：golden DoReadLastData main.cpp:9327 FTestIF->ReadTestIFFile() 完整版（fContact 之後、fYieldMonitoring 之前，同 golden），取代 FileRW_TTLCfg_ReadDIOSection（[DIO] 子集）   //Steven 20260925：golden main.cpp:9327 ReadTestIFFile 的 [DIO] 段（只讀；iDioMode／sDioName → TFTestIF cbDIOType）   // AI(W906-RCHG) 20260925: 換配方（bBoot=false）走 FileRW_TesterIF_ReadTestIFFile —— TesterIF 工程師指定（golden 其他流程的 FTestIF->ReadTestIFFile()）
    { extern void FileRW_Contact_Boot(); FileRW_Contact_Boot(); }   //Steven 20260925 (S12-C DeviceForm_File)：golden TfContact 替身（在 ContactForce 表與 fContactForm->Init 之後；CarlibrationTask 在 fContactForm）；存檔照 golden（CalculateTotalAirForce 已移植，存檔前照 golden 事件順序重算衍生欄位）
    std::printf("contact.* chain loaded: Contact.Data -> DeviceForm_File"
                " (IndexContact TestArm1=%.3f TestArm2=%.3f,"
                " ForcePerPinN=%.3f)\n",
                DeviceForm_File.IndexContact[0], DeviceForm_File.IndexContact[1],
                DeviceForm_File.ForcePerPinN);

    // ---- golden :9330 fYieldMonitoring->ReadFile() ----
    // AI(W906-YM-READFILE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8906 -- fYieldMonitoring->ReadFile() sits FOUR lines after
    // the fSetup->ReadFile() above it (:8900) in golden's own boot order.
    //
    // WHY IT IS HERE AND NOT LEFT TO RunStartMode.  The port's only other
    // call site is RunStartMode.cpp:809 (SetRunStartMode), which nothing
    // in wb_serve's boot reaches -- it fires on an operator run-mode
    // switch.  golden has FOUR call sites; the other three live in
    // TfMain::DoReadLastData (:8906), TfMain::sbConfigurationClick
    // (:27635) and KYECFTP/FTPClient.cpp:3314, none of which is
    // translated yet.  So without this line the 806-line body would be
    // compiled, linked, and never executed.
    //
    // ⚠ FAITHFUL ORDER, ONE STEP STILL MISSING: golden runs
    // fContact->ReadFile() (:8902) and FTestIF->ReadTestIFFile()
    // (:8904) between fSetup and fYieldMonitoring.  fContact IS wired
    // now (AI(W906-CT-BOOTWIRE), above); FTestIF is not translated, so
    // one step of golden's order is still absent from the middle.
    // missing from the middle.
    fYieldMonitoring->ReadFile();
    std::printf("yield.* chain loaded: Tester.Data -> TestIF_File"
                " (LowYieldLimit=%d, LowYieldCount=%d)\n",
                TestIF_File.iLowYieldLimit, TestIF_File.iLowYieldCount);

    // ---- AI(W906-RCHG) 20260925：golden :9334 fLaserSensor->ReadFile()、:9336 fSetup->ReadFile()（第二次）、
    //      :9337-9338 第一次且 SingleSite 時再讀 fContact、:9340 fTemp_Set->ReadTempFile(true)（第二次，「得在fSetup後面」）
    //      —— 這四行移植樹開機鏈原本都沒有 ----
    { extern void W906_RC_LaserInitEdtListOnce(); extern void W906_RC_LaserReadFile();   // WebReadChainCalls.cpp
      if (bBoot) W906_RC_LaserInitEdtListOnce();   // golden TfLaserSensor 建構子 InitLaserEdtList（elLaser 由 FileRW_IniConfig_Boot 建好；LaserSensor.cpp 的 GA-3 HAND-OFF）
      W906_RC_LaserReadFile(); }                   // golden :9334
    fSetup->ReadFile();                                                         // golden :9336
    if(bBoot && TestIF_File.iTestMode==SingleSite)                              // golden :9337 kevin 20180409 JerryYang 20171215 (Steven) Single site只支援浮動頭1對1或是2對1（bFirst==false ⇔ bBoot）
        fContactForm->ReadFile();
    iSendChangeTempError = 0;
    fTemp_Set->ReadTempFile(true);                                              // golden :9340 Steven 20110930 : 得在fSetup後面
    tempLoaded = (iSendChangeTempError != 1);
    // ---- golden :9342 fHotPlate、:9344 fLd_ULd、:9346 fSpeed ----
    // AI(W906-HP-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8918 `fHotPlate->ReadFile();` -- golden's own comment on
    // that line is "Steven 20150408 : 發在fSetup後面", i.e. the ordering
    // constraint is *after* fSetup->ReadFile(), which is satisfied here.
    //
    // WHY THIS LINE EXISTS.  TfHotPlate::ReadFile() was translated
    // 20260922 (155 lines, golden cHotPlate.cpp:154-308) but golden has
    // exactly ONE caller for it -- main.cpp:8918 -- and this tree had
    // ZERO.  So the body was compiled, linked, and never executed: every
    // HotPlateForm_File field stayed at its static-init value and
    // HotPlate.Data was never opened.  Only the tail helper
    // SetArmHotPlateYPitch() was reachable, via cSetUp.cpp:1164.
    //
    // No Init() is needed the way fSetup->Init() was: TfHotPlate's
    // widgets are all allocated inline at their declarations
    // (forms/fHotPlate.h:259-270) and `fHotPlate` is a file-scope
    // `new TfHotPlate()` (forms/fHotPlate.cpp:27).
    //
    // WARNING - THIS CALL WRITES.  ReadFile() uses CheckAndReadIniData,
    // which writes the default back into HotPlate.Data when a key is
    // missing (common.cpp:641).  That is golden's behaviour, kept
    // verbatim.
    { extern void W906_BootReadHotPlateAndSpeed(); W906_BootReadHotPlateAndSpeed(); }   // AI(W906-RCHG) 20260925: golden :9342 fHotPlate->ReadFile() → :9344 fLd_ULd->ReadFile() → :9346 fSpeed->ReadFile()（cSpeed.cpp:1716）。原本這裡單獨叫 fHotPlate->ReadFile()，後面 W906_BootReadHotPlateAndSpeed 又讀一次 —— golden 只讀一次，併成這一行
    std::printf("hotplate.* chain loaded: HotPlate.Data -> HotPlateForm_File"
                " (XPitch=%.3f, YPitch=%.3f, PlateSelect=%d)\n",
                HotPlateForm_File.XPitch, HotPlateForm_File.YPitch,
                HotPlateForm_File.iPlateSelect);

    // ---- golden :9348 fTrayAssignment->ReadFile() ----
    { extern void W906_BootReadTrayAssignment(); W906_BootReadTrayAssignment(); }   // golden :9348 fTrayAssignment->ReadFile()（wei 20150317 : 得在fSpeed後面）。AI(W906-RCHG) 20260925: 原本同一行還叫一次 W906_BootReadHotPlateAndSpeed()，已併到上面 :9342 那一行
    // // AI(W906-T4-U37) 20260924: 再加 golden :8925 fTrayAssignment->ReadFile()（本體在 forms/fTrayAssignment.cpp 檔尾）。 // AI(W906-T4-U34U36) 20260924: golden DoReadLastData main.cpp:8919 fHotPlate->ReadFile() ＋ :8923 fSpeed->ReadFile()（本體在 cSpeed.cpp 檔尾）。T4 U34／U36：HotPlate.Data 與 ArmCondition.Data 在這之前從沒讀過。放在空行上，不移動行號  // AI(W906-RJ03) 20260925: fQAMode + fMagazine now run HERE, after fTrayAssignment (golden main.cpp:8924 -> :8926 -> :8928)
    // ---- golden :9350 fQAMode->ReadFile()（Steven 20190326 : 必須在Bin之前）----
    // AI(W906-QA-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8926 -- golden's comment there is "要放在Bin的前面"
    // (must come before Bin), and golden's other two call sites are
    // cBinSel.cpp:1124 and uLotInfo.cpp:11525.  The port had none: the
    // body existed but sat inside GATE (Q-4), retired this wave.
    //
    // ⚠ THIS ONE WRITES, and the gate that used to hold it said so
    // loudly: MyForceDirectories(golden :184) creates directories under
    // DataPath, and eight CheckAndReadIniData calls seed Tester.Data
    // with defaults for any absent key.  Kept verbatim -- same ruling as
    // TfHotPlate (G-1, retired 20260922) and TfYieldMonitoring, which
    // already seeds this same file.
#if 0 // AI(W906-RJ03) 20260925: QAMode read + its log line moved to :3600, after fTrayAssignment (golden main.cpp:8924-8926). QAMode seeds Tester.Data "Tray Direct" from TrayForm.Loader.Direction when the key is absent, and only fTrayAssignment::ReadFile sets that -- read first, it wrote 0 into the real file (NB2 R3 RJ-03)
    std::printf("qamode.* chain loaded: Tester.Data [QA Mode] -> TestIF_File"
                " (Count=%d, RunType=%d, UntestBin=%d, SamplingCnt=%d)\n",
                TestIF_File.iQAModeCount, TestIF_File.iQAModeRunType,
                TestIF_File.iQAModeBin, TestIF_File.iQASamplingCnt);
#endif
    fQAMode->ReadFile(); std::printf("qamode.* chain loaded: Tester.Data [QA Mode] -> TestIF_File (Count=%d, RunType=%d, UntestBin=%d, SamplingCnt=%d)\n", TestIF_File.iQAModeCount, TestIF_File.iQAModeRunType, TestIF_File.iQAModeBin, TestIF_File.iQASamplingCnt);
    // ---- golden :9352 fMagazine->ReadFile()（JerryYang 20230505 : 必須在在Bin之前）----
    // AI(W906-MG-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // calls this TWICE -- main.cpp:8928 (before Bin) and :8934 (after
    // Bin), with golden's own comments saying exactly that.  Only the
    // first is reproduced here, because this boot has no Bin step
    // between them to make the second meaningful; the second is a
    // re-read after fBinSel, which this tool does not run.
    //
    // 4 ReadIniData, zero writes.  Two source files: the recipe's
    // HandlerCondition.Data and AuthPath+config.ini (the 32 Z offsets).
    // AI(W906-MG-BOOTWIRE) 20260923: the RUNTIME PRECONDITION that
    // cinitial.cpp:11368-11375 spells out in capitals.  LoadForm /
    // EmptyForm / ColorForm / AutoForm[] are TRAY_TYPE_PARA POINTERS
    // (cprod.h:1365-1366, defined cprod.cpp:20-21 with no initialiser,
    // i.e. NULL).  golden points them into TrayForm in
    // SetTechDataToProd_Tray (cinitial.cpp:14587-14593 here, golden
    // :8495), which is reached through SetTechDataToProd -- GATE N3-G9,
    // still shut.  That note ends "the first caller must land the
    // AutoForm/LoadForm wiring in the same change"; this is that caller.
    //
    // TfMagazine::ReadFile dereferences LoadForm->YDivision and
    // ->XPitch (golden Magazine.cpp:3636).  Measured without this:
    // 0xC0000005, straight after the qamode line above.
    //
    // The four assignments are golden's own (cinitial.cpp:14587-14593),
    // copied rather than reached, because reaching them means calling
    // SetTechDataToProd_Tray -- which also rewrites a large block of
    // Prod, a much wider change than a null-pointer fix.
    //
    // ⚠ WHAT THEY POINT AT IS STILL EMPTY.  TrayForm is filled by
    // TfTrayForm::ReadFile (golden cTrayForm.cpp:364), which is NOT
    // translated yet.  So LoadForm->YDivision reads 0 and golden's
    // `YDivision<10` branch forces iMagFixTrayType=0.  That is a real
    // data gap, not a crash, and it closes when TfTrayForm lands.
    LoadForm        =&TrayForm.Loader;                                  // golden cinitial.cpp:8495
    EmptyForm       =&TrayForm.Empty;
    ColorForm       =&TrayForm.Color;
    for(int i=0; i<eTrayCount; i++)
        AutoForm[i] =&TrayForm.Auto[i];

#if 0 // AI(W906-RJ03) 20260925: Magazine read + log moved to :3600 with QAMode (golden :8928 follows :8926). The LoadForm/EmptyForm/ColorForm/AutoForm wiring above stays here and still precedes it
    std::printf("magazine.* chain loaded: HandlerCondition.Data + config.ini"
                " -> TestIF_File (FixTrayType=%d, DisplayOrder=%d, TraySource=%d)"
                "  InMagOfs[0]=%d OutAutoOfs[0]=%d\n",
                TestIF_File.iMagFixTrayType, TestIF_File.iMagDisplayOrder,
                TestIF_File.iMagTraySource,
                fMagazine->iInMagOfs[0], fMagazine->iOutAutoOfs[0]);
#endif
    fMagazine->ReadFile(); std::printf("magazine.* chain loaded: HandlerCondition.Data + config.ini -> TestIF_File (FixTrayType=%d, DisplayOrder=%d, TraySource=%d)  InMagOfs[0]=%d OutAutoOfs[0]=%d\n", TestIF_File.iMagFixTrayType, TestIF_File.iMagDisplayOrder, TestIF_File.iMagTraySource, fMagazine->iInMagOfs[0], fMagazine->iOutAutoOfs[0]);
    // ---- golden :9354 fBinSel->ReadParam()、:9356 fBinSel->ReadFile(false, false, "") ----
    //   AI(W906-RCHG) 20260925：原本在整條鏈的最前面（比 FTestIF->ReadTestIFFile 早），bin 用的是預設 iTestBinCount=16，
    //   而且比 golden 要求的 fQAMode／fMagazine（「必須在Bin之前」）早讀 —— 照 golden 順序移到這裡（TesterIF 工程師回報）。
    // golden boot shape main.cpp:1014: fBinSel->ReadFile(false,false,"")
    fBinSel->ReadParam(); fBinSel->ReadFile(false, false, AnsiString(""));  // AI(W906-T4-U40) 20260924: 前面補 golden DoReadLastData main.cpp:8931 的 fBinSel->ReadParam()（Tester.Data [Alarm] 的連續 fail／fail rate 警報啟用）。⚠ bUseContinueFail／bUseFailRate 在移植樹目前沒有任何讀者（git grep 只有 ReadParam 自己），接上是忠實但暫無行為效果；接在同一行，不移動行號
    // battery member called directly, NOT the SetTechDataToProd
    // orchestrator: the orchestrator is InitialOK-gated AND shadowed
    // by ckernel_shims' no-op #define; _Yield itself is neither
    // (recon section 4, both verified there).
    SetTechDataToProd_Yield();
    fShowBinSelect->ShowBinSel();
    binSelLoaded = true;
    std::printf("bin.* chain loaded: BinSelect -> iT6CatData -> MyBinSel captions\n");

    fMagazine->ReadFile();                                                      // golden :9358 JerryYang 20230505 : 必須在在Bin之後（AI(W906-RCHG) 20260925 補上第二次）
    // ---- golden :9360 fBarCode->ReadFile() ----
    // AI(W906-BC-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8936 -- last of the DoReadLastData readers before
    // fOffSet (:8997), and after fHotPlate (:8918), so it goes here.
    //
    // golden has three call sites (main.cpp:8936, main.cpp:28724,
    // SECSGEM/uHGemHT9045.cpp:930).  The SECSGEM one is now live again
    // in this tree too -- GATE [E4] retired this wave -- but nothing in
    // wb_serve's boot reaches it, so this is the line that makes the
    // 379-line body run at start-up.
    //
    // Unlike every other reader in this chain, this one CANNOT write:
    // 86 ReadIniData, zero CheckAndReadIniData / ReadWriteIni /
    // WriteIniData.  Nothing on disk changes.
    { extern void FileRW_BarCode_BootReadFile(); extern void FileRW_BarCode_ReadFile(); if (bBoot) FileRW_BarCode_BootReadFile(); else FileRW_BarCode_ReadFile(); }   //Steven 團隊 20260925 (S12-C TestIF_File_BarCode)：golden DoReadLastData main.cpp:9361（D:\HT9045 V912 行號）fBarCode->ReadFile() 改走 golden 912 版（FileRW/TestIF_File_BarCode.cpp，與 Setup.BarCode.html 開頁／存檔同一份）；移植樹 TfBarCode::ReadFile（V906 翻譯）只差 bEnableBarcodeCSVCompare 兩行
    std::printf("barcode.* chain loaded: HandlerCondition.Data [Configuration]"
                " -> TestIF_File (EnableBarCode=%d, Bottom2D=%d,"
                " MinLen=%d MaxLen=%d, Delay=%d)\n",
                (int)TestIF_File.bEnableBarCode, (int)TestIF_File.bEnableBottom2D,
                TestIF_File.iBarCodeMinLength, TestIF_File.iBarCodeMaxLength,
                TestIF_File.iBarCodeDelay);

    { extern void FileRW_Rotate_ReadFile(); FileRW_Rotate_ReadFile(); }   //AI(W906-FRW-NoPage) 20260926：接上 golden :9361 FrmRotate->fRotate_ReadFile()（FileRW/Rotate.cpp；Rotate.Data → tRotate＋strIn/OutRotateDutAngle，只 ReadIniData）——下面原註解的「未接」已過期   // golden :9362 FrmRotate->fRotate_ReadFile()：移植樹沒有 Rotate 表單（Rotate.Data 沒人讀）—— 未接
    { extern void W906_RC_LaserReadFile(); W906_RC_LaserReadFile(); }             // golden :9364 fLaserSensor->ReadFile()（第二次）
    { extern void FileRW_IniConfig_OCR_ReadFile(); FileRW_IniConfig_OCR_ReadFile(); }   /* AI(W906-FRW-S86) 20260926 (Steven 團隊)：golden DoReadLastData main.cpp:9365 fOCR->fOCR_ReadFile()（D:\HT9045_ref V912 行號）→ FileRW/IniConfig_OCR.cpp：<配方>\AOI.Data [OCR SETTING] 19 鍵 → IniConfig 的 OCR 欄位（只 ReadIniData、不補寫）＋ system\Gerneral.ini [OCR SETTING] "OCR Port"（CheckAndReadIniData，缺鍵照 golden 補寫 24）；INSTALL_OCR!=0 時 fMain->SendMSG_CMD 送 BarCode／Pin1 開關（門面 offline no-op）；開機與換配方都跑 —— 後面原註解的「未接」已過期 */   // golden :9366 fOCR->fOCR_ReadFile()：移植樹沒有 TfOCR —— 未接
    { extern void W906_RC_SCKARTAccessFileRead(); W906_RC_SCKARTAccessFileRead(); }   // golden :9368 fSCKART->AccessFile(true)（移植樹 forms/fSCKART.cpp:69 是空殼）
    // ---- golden :9370 fTrayMapping->ReadFile() ----
    // AI(W906-TM-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8946 -- golden's ONLY call site for this method, and the
    // port had none.  It sits between fBarCode (:8936) and fAutoAlignment
    // (:9007) in golden; placed here, still before DoStructUnitConvert().
    //
    // NOT `fTrayMapping` -- that global is the shim class.  See
    // forms/fTrayMapping.h for the name inversion this tree carries.
    //
    // 23 pure ReadIniData into TestIF_File, zero writes.  The writer in
    // this family, ReadFile_ScanLine, stays gated (it back-writes into
    // system\Gerneral.ini via CheckAndReadIniDataGeneral).
    fTrayMappingForm->ReadFile();
    std::printf("traymap.* chain loaded: HandlerCondition.Data [Configuration]"
                " -> TestIF_File (TrayMap=%d DeviceRemain=%d, StartDelay=%d,"
                " CodeLen=%d..%d)\n",
                (int)TestIF_File.bEnableTrayMap, (int)TestIF_File.bEnableDeviceRemain,
                TestIF_File.iTrayStartDelay,
                TestIF_File.iTrayCodeMinLength, TestIF_File.iTrayCodeMaxLength);

    // ---- golden :9371 fFixAICCD->ReadFile() ----
    // AI(W906-FX-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8947 -- golden's only call site; the port had none.
    //
    // 10 ReadIniData into TestIF_File.  The one write, a
    // CheckAndReadIniDataGeneral seeding [AICCD] RetryConnectTimer into
    // system\Gerneral.ini, does not fire on this machine: the key is
    // already present (Gerneral.ini:660-661).
    fFixAICCD->ReadFile();
    std::printf("fixaiccd.* chain loaded: HandlerCondition.Data [Configuration]"
                " -> TestIF_File (Fix2AICCD=%d, LightScrPos=%d, Thres=%.3f,"
                " RetryConnectTimer=%d)\n",
                (int)TestIF_File.bEnableFix2BGAAICCD, TestIF_File.iBGALightScrPos,
                TestIF_File.dInspectResultThres, fFixAICCD->iRetryConnectTimer);

    if(fTeach!=NULL) fTeach->ReadFile();                                        // golden :9373 kevin 20190427 kevin 20190305 add teach.ini（AI(W906-RCHG) 20260925 補；NULL 守衛同 cinitial.cpp ReadTechData）
    { extern void FileRW_AOISetup_ReadFile(); FileRW_AOISetup_ReadFile(); }   /* AI(W906-FRW-S69) 20260926 (Steven 團隊)：golden DoReadLastData main.cpp:9374 FrmAOI->fAOI_ReadFile()（D:\HT9045_ref V912 行號；FileRW/AOISetup.cpp：AOI.Data → tAOISetup／ScannerAOIIF／bVitroxBGA・PADViewUse／MOT[MMScanAOI].Tray.Data，只 ReadIniData、不寫檔）——後面註解的 FrmAOI「未接」已過期；fAGV 仍未接 */   // golden :9375 FrmAOI->fAOI_ReadFile()、:9377 fAGV->ReadFile()：移植樹沒有這兩個表單 —— 未接
    // ---- golden :9381-9437 DoIniDataToForm 段（AI(W906-RCHG) 20260925 補；移植樹有本體的才接）----
    fHotPlate->DoIniDataToForm();                                               // golden :9381
    // golden :9383 fContact->DoIniDataToForm()：移植樹 TfContact 沒有這一支（C 路頁面開頁時由 FileRW/DeviceForm_File.cpp 跑 golden FormShow）
    fContactForm->DutCount();                                                   // golden :9385 jou 2014-09-06 修正開啟程式的時候EP異常
    // golden :9386 fContact->ShowArmAndDeviceForce()：forms/fContact.h:1498 GATE (X-02)（C 路 DF_ShowArmAndDeviceForce 在開頁時跑）
    { extern void FileRW_TesterIF_DoIniDataToForm(); FileRW_TesterIF_DoIniDataToForm(); }   // golden :9388 FTestIF->DoIniDataToForm()
    // golden :9390 fYieldMonitoring->DoIniDataToForm()、:9395 fSetup->DoIniDataToForm()：沒有獨立本體（A 形狀 FileRW/TestIF_File.cpp 在 /api/form 時跑）
    { extern void W906_RC_LaserDoIniDataToForm(); W906_RC_LaserDoIniDataToForm(); }   // golden :9393
    fTemp_Set->DoIniDataToForm(true);                                           // golden :9397 Steven 20110930 : 得在fSetup後面
    { extern void W906_RC_SpeedDoIniDataToForm(); W906_RC_SpeedDoIniDataToForm(); }   // golden :9400
    { extern void W906_RC_TrayAssignmentDoIniDataToForm(); W906_RC_TrayAssignmentDoIniDataToForm(); }   // golden :9402 wei 20150317 : 得在fSpeed後面
    { extern void FileRW_BarCode_DoIniDataToForm(); FileRW_BarCode_DoIniDataToForm(); }   //Steven 團隊 20260925 (S12-C TestIF_File_BarCode)：golden main.cpp:9405 fBarCode->DoIniDataToForm()（替身；原註解寫 :9404 是 D:\HT9045_ref 行號）
    { extern void FileRW_QAMode_DoIniDataToForm(); FileRW_QAMode_DoIniDataToForm(); } /* Steven 團隊 20260925：golden DoReadLastData main.cpp:9407 fQAMode->DoIniDataToForm()（替身＋fLotInfo->edQAMode） */   // AI(W906-RCHG) 20260925: 從 fQAMode->ReadFile 旁邊移到 golden 的位置 :9406
    { extern void FileRW_Rotate_DoIniDataToForm(); FileRW_Rotate_DoIniDataToForm(); }  { extern void FileRW_FixAICCD_DoIniDataToForm(); FileRW_FixAICCD_DoIniDataToForm(); }   /* AI(W906-FRW-S63) 20260926 (Steven 團隊)：golden :9410 fFixAICCD->DoIniDataToForm()（FileRW/TestIF_File_FixAICCD.cpp；HSys CCD 位址＋TestIF_File 9 欄 → 替身；Gerneral.ini [AICCD] LockNoWaitAOIResult 缺鍵照 golden 補寫）——後面註解的 fFixAICCD「未接」已過期；golden :9409 fTrayMapping->DoIniDataToForm 仍未接（S74） */   //AI(W906-FRW-NoPage) 20260926：golden :9407 FrmRotate->DoIniDataToForm() 接上（FileRW/Rotate.cpp；golden 本體結尾會 SetWorkParameter()）——下面原註解的 FrmRotate「未接」已過期，fTrayMapping／fFixAICCD 仍未接   // golden :9408 FrmRotate->DoIniDataToForm()、:9410-9411 fTrayMapping／fFixAICCD->DoIniDataToForm()：移植樹沒有 —— 未接
    // golden :9413 fCCLink->ReadSetupFile()（CCLink.Data）：移植樹沒有 —— 未接
    { extern void FileRW_TTLCfg_DoReadLastDataLoad(); FileRW_TTLCfg_DoReadLastDataLoad(); }   // golden :9416-9417 S=fDIOFrom->GetDIOFileName(); fDIOFrom->LoadData(S);
    ReadPassword();                                                             // golden :9419（system\login.dat → USER，只在記憶體）
    // ---- golden :9423 fOffSet->ReadFile()（jou 2010-01-15 offset需放在lastset讀取之後）----
    // AI(W906-OFS-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:8997 `fOffSet->ReadFile();`, carrying golden's own
    // ordering comment on the line above it (main.cpp:8996):
    // "jou 2010-01-15 offset需放在lastset讀取之後".  In golden it is the
    // LAST of the DoReadLastData readers, long after fSetup/fContact/
    // FTestIF/fYieldMonitoring/fHotPlate, so last in this chain too.
    //
    // golden has FIVE call sites (main.cpp:8997, main.cpp:21904 inside
    // TfMain::ChangeTempMode, cinitial.cpp:14262, ArmOffsetData.cpp:124,
    // AutoAlignment/SmartSetup.cpp:1691).  None of the other four is
    // translated, so without this line the body would never run -- the
    // same dead-code shape fHotPlate->ReadFile() was in until today.
    //
    // ⚠ THIS CALL WRITES, on two different levels:
    //   (a) MyForceDirectories(szDir) creates the offset folder if it is
    //       absent -- unconditional, golden cOffSet.cpp:2019.
    //   (b) the ReadInvisibleFile() branch (golden :2287-2307) CREATES
    //       D:\HT9045\IniData\DefineOffset\STD_125.Data and STD_25.Data
    //       and back-fills D:\HT9045\data\<recipe>.ini.  It is behind
    //       CosFunction.bUseInvisibleOffset, which is assigned in exactly
    //       one place in this tree -- FUNC_CC_ASE_CL() (CosFunction.cpp:831).
    //       This machine's system\Gerneral.ini says CUSTOMER_CODE=912
    //       (CC_SPIL_CHINA_SUZHOU, MachineType.h:338), so that branch does
    //       not run here and (b) stays inert.  On an ASE-CL machine it
    //       does run, by golden's design.
    // AI(W906-OFS-BOOTWIRE) 20260923: stands in for golden's TfMain
    // constructor, main.cpp:2123-2139.  Without it the first line of
    // ReadFile() dereferences a NULL InArmOffSet_File[0] -- measured,
    // 0xC0000005.  See cOffSet.cpp's banner on the helper.
    EnsureArmOffsetObjects();
    fOffSet->ReadFile();
    { extern void FileRW_Offset_Boot(); FileRW_Offset_Boot(); }   //Steven 20260925 (S12-C Offset_File)：golden TfOffSet 建構子替身（讀 USE_*／CUSTOMER_CODE，需在 EnsureArmOffsetObjects 之後）
    std::printf("offset.* chain loaded: %s -> InArm/OutArm/SortArm OffSet_File"
                " + Offset_File (Loader HandX=%.3f HandY=%.3f,"
                " Preciser Open=%.3f, TestArm1 BusySHHalt=%.3f)\n",
                fOffSet->GetOffsetPath().c_str(),
                InArmOffSet_File[InOfsLoader]->GetX(),
                InArmOffSet_File[InOfsLoader]->GetY(),
                Offset_File.iPreciserOpen, Offset_File.iSHHalft[0]);

    // golden :9425 SetNormalOrPrime()：移植樹 TfMain 門面沒有（cBinSel.cpp:3222-3223）—— 未接
    fMain->UpdateMainOperateMode();                                             // golden :9427（門面計數 stub）
    fMain->LoadRunModePicture();  { extern void FileRW_AutoCalSuckZ_DoIniDataToForm(); FileRW_AutoCalSuckZ_DoIniDataToForm(); }   /*AI(W906-FRW-S70) 20260926：golden :9430 fProductionInfo->DoIniDataToForm()（FileRW/AutoCalSuckZ.cpp；golden 第一行 !CosFunction.bOEEFunction 就 return —— 只有 OEE 機台讀 D:\HT9045\system\ProductionInfo\AutoCalSuckZ.Data；OEE 本體擋掉）*/                                                // golden :9429（門面計數 stub）
    // golden :9431 fProductionInfo->DoIniDataToForm()：移植樹沒有 —— 未接
    // ---- golden :9433 fAutoAlignment->ReadFile()（:9435 DoIniDataToForm 移植樹沒有）----
    // AI(W906-AA-BOOTWIRE) 20260923: golden TfMain::DoReadLastData
    // main.cpp:9007 -- fAutoAlignment->ReadFile() runs after fOffSet
    // (:8997), so it goes here, and it must stay BEFORE the
    // DoStructUnitConvert() below (that one applies everything at once).
    //
    // golden has three call sites (main.cpp:9007, main.cpp:9303, and
    // AutoAlignment's own form logic); the port had none, because the
    // whole AutoAlignment module did not exist until today.
    //
    // Zero writes: 26 ReadIniData, all into the global TestIF_File.
    //
    // NOTE ON THE VALUES BELOW: golden returns early with
    // bEnableAutoAlignment=false when MACHINE_HAS_AUTO_ALIGNMENT_CCD is
    // false (golden :1380-1384), so on a machine without the alignment
    // CCD the printf will show Enable=0 and the point/offset fields keep
    // whatever they had.  That is golden's behaviour, not a failure.
    fAutoAlignment->ReadFile();  { extern void FileRW_AutoAlignment_DoIniDataToForm(); FileRW_AutoAlignment_DoIniDataToForm(); }   //AI(W906-FRW-NoPage) 20260926：golden :9434 fAutoAlignment->DoIniDataToForm()（FileRW/TestIF_File_AutoAlignment.cpp；golden 在這裡把 TrayEvent／ShuttleHotplateEvent OR 入 AfterHome 位元）
    std::printf("autoalign.* chain loaded: HandlerCondition.Data [AutoAlignmrnt]"
                " -> TestIF_File (Enable=%d, PointX=%d PointY=%d,"
                " DecodeTimeOut=%d)\n",
                (int)TestIF_File.bEnableAutoAlignment,
                TestIF_File.iAlignmentPointX, TestIF_File.iAlignmentPointY,
                TestIF_File.iAOA_DecodeTimeOut);

    { extern void FileRW_StartCondition_DoIniDataToForm(); FileRW_StartCondition_DoIniDataToForm(); }   // golden :9437 fStartCondition->DoIniDataToForm()（Steven 20221214 : Add for socket ID）
    // ---- 以下不是 golden DoReadLastData 的內容 ----
    //   AI(W906-RCHG) 20260925：DoStructUnitConvert 原本夾在 fFixAICCD 與 HotPlate/Speed 中間；golden DoReadLastData 裡沒有它
    //   （開機在 SetWorkParameter、換配方在 ChangeSetUpFile :25757），移到所有讀檔之後，換算到的是這一輪全部讀進來的值。
    // AI(W906-UNITCONV-WIRE) 20260923: golden runs DoStructUnitConvert()
    // after EVERY read and EVERY save -- 24 call sites tree-wide.  This
    // tool ran it ZERO times, so everything the six ReadFile calls above
    // just loaded stopped at the *_File layer, and the structs the engine
    // actually runs on stayed at their static-init values.
    //
    // MEASURED BEFORE WIRING (probe output, same boot, same recipe):
    //     TestIF_File.dSiteXPitch         =  40.000 -> TestIF.dSiteXPitch         = 0.000
    //     DeviceForm_File.IndexContact[0] =-113.650 -> DeviceForm.IndexContact[0] = 0.000
    // Steven 20260923 12:49 predicted exactly this and named this exact
    // pair as the check.  He was right.
    //
    // WARNING - RUNTIME-CONFIGURATION CHANGE WITH WIDE REACH.  One call
    // rewrites TestIF, DeviceForm, HotPlateForm, ArmSpeed/SHSpeed/MGSpeed,
    // the InArm/OutArm/SortArm offsets, Offset and UserDefForm -- from
    // all-zero to the recipe's real values.  It is what golden does; it is
    // still a behaviour change on a tool that has been running on zeros.
    // It contains NO motion and NO IO: the body (cUnitConvert.cpp:675-692)
    // is six Do*Convert calls plus one ArmSpeed[OutArm].dWaitOnSH addition.
    //
    // ORDER MATTERS: DoArmOffsetConvert() dereferences the six ARM_OFFSET
    // arrays, so this MUST stay after EnsureArmOffsetObjects() above.
    // Before today those arrays were all NULL and this call would have
    // been an access violation, not a no-op.
    //
    // NOT un-gating cinitial.cpp:7175 (GATE N3-G8).  That gate's stated
    // reason -- "cUnitConvert.h does not declare it ... no body anywhere"
    // -- is STALE (cUnitConvert.h:112-118 marks it SUPERSEDED since PT-W8,
    // 20260811), but that gate is paired with N3-G7/G9 and retiring it is
    // a cinitial-boot decision, not this tool's.  Wiring it here reaches
    // golden's behaviour for wb_serve without touching that pairing.
    DoStructUnitConvert();
    std::printf("unit-convert applied: TestIF.dSiteXPitch=%.3f (file %.3f x100)"
                "  DeviceForm.IndexContact[0]=%.3f (file %.3f)\n",
                TestIF.dSiteXPitch, TestIF_File.dSiteXPitch,
                DeviceForm.IndexContact[0], DeviceForm_File.IndexContact[0]);
    // AI(W906-TRAY-READ) 20260923: 讀檔**之後**再跑一次 SetWorkParameter()，讓 N3-G8 剛解開的
    //   DoStructUnitConvert() 換算到**剛讀進來**的值（UserDefForm_File -> UserDefForm、TestIF_File -> TestIF，×100）。
    //   golden 開機順序：DoReadLastData()（main.cpp:9562，讀檔）→ … → SetStartModeData()（:9577）
    //   → 每個分支都呼叫 SetRunStartMode()（:24215-24232）→ SetWorkParameter()（:1090）→ DoStructUnitConvert()。
    //   ⇒ golden 是先讀、後換算。本檔上面 `const bool okWP = SetWorkParameter();` 那一次（:2801）在讀檔**之前**（為了 Tech），換算到的是 0。
    //   SetStartModeData／SetRunStartMode 整條鏈沒有在 wb_serve 翻（那會改 run mode 等更多狀態，
    //   超出 T5 的範圍，已列晨報）；這裡只補上那條鏈裡負責換算的那一段。
    //   SetWorkParameter 在 golden 被呼叫幾十次（存檔後重新載入，main.cpp:25044 / :27326 …），重複呼叫是 golden 的常態。
    if (bBoot) {   // AI(W906-RCHG) 20260925: 只在開機 —— golden FormShow main.cpp:9564 ReadLastSetIni()＋之後 SetStartModeData 鏈的換算；換配方由 ChangeSetUpFile :25757-25758 自己做 DoStructUnitConvert／SetWorkParameter
        ReadLastSetIni(); const bool okWP2 = SetWorkParameter();   // AI(W906-A4-3) 20260924: golden main.cpp:9564 ReadLastSetIni()（「必須在 Initial 之後」）—— InitialHandler＋DoReadLastData 之後再讀一次 LastSet／config.ini，接著才是 SetStartModeData 那條鏈負責的換算
        std::printf("SetWorkParameter (after reads) -> %s   (UserDefForm[0].YPitch=%d, TestIF.dSiteXPitch=%.0f)\n",
                    okWP2 ? "true" : "false",
                    (int)UserDefForm[0].YPitch, (double)TestIF.dSiteXPitch);   fMain->SetStartModeData();   // AI(W906-SSMD) 20260927: golden FormShow main.cpp:9577（ReadLastSetIni :9563 之後）—— 填起動模式清單並把 LastSet.iRunStartMode 種回下拉（WebStart.cpp:2945 的缺口）
    }
    if (bBoot) { extern void W906_BootInitDIOStstus(); W906_BootInitDIOStstus(); }   // AI(W906-DIO) 20260925: golden TfMain::FormShow main.cpp:10106-10108 `#ifndef SOFT_SIMULTE InitDIOStstus(false)`（TTLCfg → Prod.DIOCfg、TTL 輸出設回初始、SwClear2／6 關）—— golden 在 DoReadLastData :9562／ReadLastSetIni :9564 之後、szSupervisor :10566（下一行 WebLogin_Boot）之前。SOFT_SIMULTE 閘與本體在 cDIOStatus.cpp。接在同一行，不移動行號
    if (bBoot) WebLogin_Boot();   //Steven 20260924：golden TfMain 建構子 :2057 InitialSuperVisorPassword、FormShow :10566 szSupervisor、:10837-10879 pwPath／登入模式、:11062-11069 SOFT_SIMULTE／DEBUG 預設 HonPrec   // AI(W906-RCHG) 20260925: 只在開機
    if (bBoot) ReadTasterInfo();   //Steven 20260924 (S12-C)：golden TfMain::FormShow main.cpp:10883（開機序列尾段）；沒讀的話 IniConfig.N06_TasterList* 是空字串，TfConfiguration 存檔（SaveTasterInfo）會把 config.ini [Taster] 路徑清空（round-trip 實測）   // AI(W906-RCHG) 20260925: 只在開機（golden FormShow）
    g_recipeChainsReady = true;   // Steven 20260924 (S12)：ReloadRecipeDocAfterSave() 從這裡起才可用
}
bool W906_RecipeChainsReady() { return g_recipeChainsReady; }   // AI(W906-RCHG) 20260925: WebRecipeChange.cpp 用（開機讀檔鏈跑完才可換配方）

int main(int argc, char** argv)
{
    // AI(W906-BA-ERRMODE) 20260911: FIRST statement, before anything can fault.
    // Same call, same reason, as tools/wb_publish.cpp:86 -- this file simply
    // never got it. All three sidecars are declared in the ROOT CMakeLists, so
    // none receives tests/'s directory-scoped ht9045_test_bootstrap and its
    // modal-dialog suppression. Sitting for hours unattended without this means
    // an access violation raises a Windows Error Reporting box that blocks
    // forever with NOTHING in the log -- a failure mode this project has
    // already paid for once. docs/SCOPE.md:177 lists the absence as one of the
    // rules the merged single-process shape has to hand-carry.
    ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
                   SEM_NOOPENFILEERRORBOX);  { extern void RotateBootLogIfNeeded(); extern void WriteBootLog(AnsiString); RotateBootLogIfNeeded(); WriteBootLog("WinMain Enter"); }   // AI(W906-BOOTLOG) 20260926: golden WinMain HT9045.cpp:130-131 (AI(ht9045-v899) 20260423: BootLog for crash diagnosis when 24V/hardware not ready) -- D:\HT9045\Error\BootLog.txt, rotated at 512 KB (Public/cBootLog.cpp). Right after ERRMODE, which the port keeps as the very first statement. golden :161 "ExePath OK" / :164 "Application Initialize Done" have no counterpart here (no ExePath check, no VCL Application); :292 / :298 are WinMain's catch blocks, and main() has none

    std::setvbuf(stdout, 0, _IONBF, 0);

    unsigned short port = 8045;
    //AI(W906-F5WebRoot) 20260922: 預設 root 一度從 D:\HT9045\web 下移一層到
    //  D:\HT9045\web\web，因為當時 HMI 實際在那裡。**已還原**，見本段末。
    //
    //  量測（20260922，零參數啟動 build_dbg\wb_serve.exe）：
    //      GET /background.html      -> 404
    //      GET /web/background.html  -> 200, 48,136 bytes
    //  那 48,136 正是 launch.json:上方註解在 20260911 量到的同一個數字，所以
    //  頁面本身沒壞 —— 是 20260914 那份「HT9045 Web Client 交付包」被放進
    //  D:\HT9045\web，把原本在該層的 HMI 擠到了 web\web\。
    //
    //  為什麼改 root 而不是改 launch.json 的 uriFormat：
    //  tools/webprobe/f5_contract_probe.cjs 在 :91 / :295 / :340 三處硬性斷言
    //  uriFormat 必須是 'http://127.0.0.1:%s/background.html'。改網址會同時
    //  弄壞那三條斷言，而那支 probe 存在的理由就是抓 F5 契約漂移。
    //  改 root 則 uriFormat 與「零參數」契約都原封不動。
    //
    //  ⚠ root 必須等於 HMI 所在目錄：本檔把 JSON 發布到 <root>\JSON，而頁面讀
    //  的是自己目錄下的相對 JSON/。兩者不一致時會在 <root>\JSON\js 生出空目錄。
    //
    //AI(W906-XFER) 20260922 晚：**還原成 D:\HT9045\web**，照上一則註解自己寫的
    //  條件（「若日後把 HMI 搬回頂層，這一行要跟著改回去」）。
    //  新機佈署把 transfor 包的 HMI 解回 D:\HT9045\web 頂層
    //  （722 檔 / 96,023,588 bytes，與 03_清單\MANIFEST.tsv 位元組相符），
    //  web\web\ 不存在；實測 D:\HT9045\web\background.html 存在、78,433 bytes。
    //  擠占該層的交付包已改放 D:\HT9045_Client（樹裡多處寫死該路徑，
    //  例 tools/merge_docs.py:27、docs/client_package/TESTING.md:183），
    //  所以上面那個「下移一層」的前提已經消失。
    std::string    root = "D:\\HT9045\\web";
    int            seconds = 0;      // 0 = run until Ctrl-C
    // ⚠⚠ AI(W906-ZEROARG) 20260918：**零參數 = 全功能。** 使用者裁決：
    //   「我不要任何參數且全部都要執行」「軟體模擬僅有開啟 SOFT_SIMULTE，
    //     否則就是機台上能跑，就這兩種」。
    //
    //   ⇒ 這支的行為只由**建置**決定（SOFT_SIMULTE 開 = 模擬；關 = 機台上能跑），
    //     不由執行期旗標的組合決定。BCB6 的 HT9045.exe 就是這個形狀：它不吃參數。
    //   ⇒ 旗標全部保留成「接受但不改變預設行為」，只是為了讓既有的 launch.json、
    //     腳本與 f5_contract_probe 不會撞到 "unknown argument" 而 return 2。
    //
    //   dry：預設 **false**（原本 true）。`--dry` 仍可手動開，因為測試要它；
    //        但預設不再隔離啟動載入器 —— 保護改成「備份 → 驗證 → 刪備份」
    //        （CLAUDE.md 的 V906 寫入邊界那節）。
    /* bool dry -- AI(W906-NODRY) 20260924: 行程層級的 dry 旗標移除。--dry 完全退場（使用者 20260923「--dry 不再使用」、20260924「全面用 #define SOFT_SIMULTE 來卡控模擬或實際機台」）；模擬／真機只由建置期 SOFT_SIMULTE 決定 */
    // AI(W906-WEB-W2b) 20260917 (裁決 A1.1)：這個旗標已經沒有選擇作用了 ——
    // 真實配方夾變成 API 的唯一行為。它**不能刪**，因為 .vscode/launch.json 會傳它、
    // tools/webprobe/f5_contract_probe.cjs 會 deepEqual 斷言它。所以保留、接受，
    // 但在主控台明講它已是預設，不要讓它靜靜變成 no-op。
    bool           productionRecipe = false;
    // AI(W906-FW-W1) 20260819: write-path channel opt-in. 原本預設 READ-ONLY。
    // ⚠ AI(W906-ZEROARG) 20260918：預設改成 **true** —— 見上方。指令通道是
    //   「全部都要執行」的一部分；沒有它，瀏覽器按鈕一個都送不出去。
    //   `--allow-cmd` 仍接受（既有 launch.json 會傳），但已不再是它開啟的。
    bool           allowCmd = true;

    //AI(W906-FW1) 20260817: same two safety reversals as wb_publish, paid for
    // the same evening (see tools/wb_publish.cpp): unknown arguments refuse
    // instead of falling through to atoi-as-port, and touching the REAL
    // system\Gerneral.ini needs an explicit --real -- a config load can WRITE
    // the file it reads, and the TIniFile flush destroys comments and layout.
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--root") == 0 && i + 1 < argc) {
            root = argv[++i];
        } else if (std::strcmp(argv[i], "--seconds") == 0 && i + 1 < argc) {
            seconds = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--dry") == 0) {
            std::printf("--dry 已退場（使用者 20260923／20260924 裁決）：模擬與真機只由建置期 #define SOFT_SIMULTE 決定，不接受任何執行期旗標。請拿掉 --dry 再啟動。\n"); return 2;   // AI(W906-NODRY) 20260924: 傳了就拒絕，不再默默進入暫存模式（舊失敗模式：存檔 ack ok、重讀正常、伺服器一停整批蒸發）
        } else if (std::strcmp(argv[i], "--real") == 0) {
            /* --real：舊參數，真實讀寫現在是唯一模式，接受但不做事 */   // AI(W906-NODRY) 20260924
        } else if (std::strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = static_cast<unsigned short>(std::atoi(argv[++i]));
        } else if (std::strcmp(argv[i], "--allow-system-write") == 0) {
            // AI(W906-FW-SYSFILE) 20260915: system\ is shared PRODUCTION
            // configuration. --allow-cmd opens the command channel; THIS flag
            // is the separate, explicit consent AGENTS.md requires before any
            // of it is rewritten. Reads never need it.
            gAllowSystemWrite = true;
        } else if (std::strcmp(argv[i], "--allow-cmd") == 0) {
            allowCmd = true;
        } else if (std::strcmp(argv[i], "--production-recipe") == 0) {
            productionRecipe = true;   // AI(W906-WEB-W2b) 20260917: 已是預設，見下方公告
        } else {
            std::printf("wb_serve: unknown argument '%s'\n"
                        "usage: wb_serve            <-- NO ARGUMENTS. That is the intended way.\n"
                        "\n"
                        "  AI(W906-ZEROARG) 20260918: full function is the DEFAULT. Reads and\n"
                        "  writes go to the REAL machine files, the command channel is open,\n"
                        "  and the real recipe folder is the only one the API knows.\n"
                        "  What this binary can do is decided at BUILD time by SOFT_SIMULTE,\n"
                        "  not by a combination of runtime flags -- same shape as the BCB6\n"
                        "  HT9045.exe, which takes no arguments either.\n"
                        "\n"
                        "  Accepted, but none of them is needed any more:\n"
                        "    --port N --root DIR --seconds N   still do what they say\n"
                        "    --allow-cmd                       accepted; already the default\n"
                        "    --allow-system-write              accepted; selects nothing\n"
                        "    --production-recipe               accepted; selects nothing (ruling A1)\n"
                        "    --real                            accepted; already the default\n"
                        "    --dry                             REMOVED (20260923/0924): rejected, exits 2\n"
                        "                                      sim vs real = build-time SOFT_SIMULTE only\n"
                        "\n"
                        "  Protection is BACKUP -> VERIFY -> DELETE BACKUP, not a flag.\n",
                        argv[i]);
            return 2;
        }
    }
    { extern void W906_ModalWakeConfigure(int); W906_ModalWakeConfigure(seconds); }   // AI(W906-MODAL-WAKE) 20260926: --seconds N 的有限期執行不自動開瀏覽器（檔尾）
    // AI(W906-WEB-W2b) 20260917 (裁決 A1.1)：旗標留著但已無作用 —— 明講，不要靜默。
    if (productionRecipe) {
        std::printf("--production-recipe: now the DEFAULT (ruling A1, 20260917);"
                    " flag accepted, selects nothing\n");
    }

    // AI(W906-NOGATE) 20260918: 改成**無條件**印。
    //   20260917 時這段掛在 `if (gAllowSystemWrite)` 底下，因為那時它真的是一道閘門。
    //   裁決之後預設就是開的（見 :1701 的說明），所以「有條件地警告」會變成
    //   永遠成立的 if —— 那既沒有鑑別力，也會讓人以為關掉旗標就安全。
    //   ⇒ 無條件講清楚現在是什麼狀態，不要讓操作員從旗標去推論。
    std::printf("\n"
                "  **************************************************************\n"
                "  * WRITES GO TO THE REAL MACHINE FILES. ALWAYS. (20260918)    *\n"
                "  *   system\\Gerneral.ini, the active recipe's teach.ini,      *\n"
                "  *   Mot_Table.csv / IO_Table.csv, config\\config.ini.         *\n"
                "  *                                                            *\n"
                "  * There is no flag that turns this off any more:             *\n"
                "  *   --allow-system-write  accepted, selects nothing          *\n"
                "  *   --dry                 REMOVED -- rejected, exits 2       *\n"
                "  *                         (rulings 20260923 / 20260924)      *\n"
                "  *                                                            *\n"
                "  * The protection is BACKUP -> VERIFY -> DELETE BACKUP,       *\n"
                "  * not a gate. Back up before you test, compare file content  *\n"
                "  * afterwards, then remove the backup.                        *\n"
                "  **************************************************************\n\n");  { extern void W906_PrintDataRedirects(); W906_PrintDataRedirects(); }   // AI(W906-ENV-BANNER) 20260927: NB2 R79 -- list the other 18 W906_* redirects in effect (body at EOF); prints only, see there. NOT on the recipeRedirect line below: f5_contract_probe.cjs:173 splices that statement into a standalone program

    // AI(W906-WEB-W2b) 20260917 (裁決 A1.2)：**改成無條件**。裁決之後 web API 一律
    // 解析到真實檔，所以測試重導根目錄會讓「真實」這個詞失去意義 —— 與其服務一個
    // 我們說不出名字的東西，不如拒絕。原本它掛在 --production-recipe 底下，
    // 而那個旗標現在什麼都不選，掛著等於這道檢查永遠不會生效。
    const char* recipeRedirect = std::getenv("W906_INIDATA_ROOT");
    if (recipeRedirect && *recipeRedirect) {
        std::printf("web API: W906_INIDATA_ROOT is redirected; refusing to serve\n");
        return 2;
    }

    // --- 0.9 repoint fMain at the web impl ----------------------------------
    //
    // AI(W906-ST-S3-B4) 20260918: this is the S3 repoint that WebStart.h:84-93
    // specifies, and it is the ONLY way a browser START can change anything.
    //
    //   WHY REPOINT INSTEAD OF JUST CONSTRUCTING ONE. golden's Start() body is a
    //   TfMain MEMBER FUNCTION: it writes `spbUserName->Caption = "Operator"`,
    //   `iStartIn = 0`, `SoftStart = true` and ~200 other members directly, i.e.
    //   on whatever object it is running on. If TfMainWeb were a SECOND object,
    //   every one of those writes would land on a copy nobody reads, and the
    //   whole thing would look like it worked. WebStart.h:86 says exactly this.
    //
    //   ⚠ WHY THIS IS SAFE HERE, measured 20260918 rather than assumed. Three
    //   greps, all empty:
    //     1. no file-scope variable captures fMain's value
    //        (`^[A-Za-z_]... = fMain;` -> 0 hits outside tests/)
    //     2. nothing anywhere reassigns fMain (`fMain = ` -> only its own
    //        definition at forms/fMain.cpp:416, plus one comment)
    //     3. nothing writes fMain's members at file scope
    //        (`^[A-Za-z_].*fMain->x =` -> 0 hits)
    //   So the object the 124 files reach through `fMain->` is whatever this
    //   line leaves there, and no state written before now is lost.
    //
    //   ⚠ WHAT THIS DOES NOT DO: it does NOT override TfMain::Start(). The ~19
    //   `fMain->Start()` call sites in the tree (SECS/GEM remote start,
    //   clean-out auto restart, InArm pick-error restart) still reach the base
    //   class's empty body. That asymmetry is the entire design -- overriding
    //   would arm all of them at once. See WebStart.h:22-93.
    //
    //   The old TfMain is deliberately leaked: it is one object, it lives for
    //   the process, and freeing it would invalidate any pointer taken during
    //   static init that the greps above cannot see.
    g_webMain = new TfMainWeb();
    fMain     = g_webMain;
    std::printf("fMain repointed to TfMainWeb (S3): start.run can now reach StartFromWeb()\n");  { W906_RemoteRun.Start = [](AnsiString f) -> bool { extern bool W906_MotorAccessStartBlocked(std::string&); std::string tw; if (!g_webMain || W906_MotorAccessStartBlocked(tw)) { std::printf("TCP HTSET,333 REFUSED: %s\n", tw.c_str()); return false; } g_webMain->StartFromWeb(f); return true; };  W906_RemoteRun.Pause = [](AnsiString f) -> bool { if (!g_webMain) return false; g_webMain->PauseFromWeb(f); return true; }; }   // AI(W906-W10) 20260927 (St02-E): HTSET,333 / 334 (golden Command.cpp:13381 / :13400 fMain->Start / Pause) reach StartFromWeb / PauseFromWeb through this seat (forms/fMain.h end) -- never the base TfMain::Start; the TCP pump runs the command on the tick thread, like start.run; manual teach refuses START as start.run does (:5672).  Same line, no line below moves
    { extern void W906_BootSimLoaderCheckBox(); W906_BootSimLoaderCheckBox(); }  // AI(W906-SENSORSCAN) 20260924: golden TfMain::FormShow main.cpp:10111-10113（SOFT_SIMULTE 時勾上「模擬 Loader 有盤」，ProcessSensorScan 才讓 ALed1 為真；本體在 cSensorScan.cpp 檔尾）。必須在 fMain 改指 g_webMain 之後。佔用原本的空行，不移動行號
    // --- 1. bring the machine data layer up ---------------------------------
    const AnsiString savedGeneralPath = asGeneralPath;
    gRealGeneralPath = savedGeneralPath.c_str();   //Steven 20260916 (A1): web API keeps the real path
    AnsiString scratch;

#if 0 // AI(W906-NODRY) 20260924: --dry 的 Gerneral.ini 暫存導向隨 --dry 退場；下面原本的 else（真實路徑）現在是唯一路徑
        char tmp[MAX_PATH];
        ::GetTempPathA(MAX_PATH, tmp);
        scratch = AnsiString(tmp) + "wb_serve_general.ini";
        if (!::CopyFileA(savedGeneralPath.c_str(), scratch.c_str(), FALSE)) {
            std::printf("--dry: cannot copy %s (err %lu)\n",
                        savedGeneralPath.c_str(), (unsigned long)::GetLastError());
            return 2;
        }
        asGeneralPath = scratch;
        std::printf("--dry: using a scratch copy, real config untouched\n");
        //Steven 20260916 (A1)
        std::printf("--dry: NOTE the web API (/api/system/gerneral, /api/recipe) reads and writes the REAL\n"
                    "       files, not the scratch copy -- only the C++ loader is isolated. Writes still\n"
                    "       need --allow-system-write (system\\) / --allow-cmd (recipes).\n"
                    "       Live tags keep showing the loader's copy until the next start.\n");
#endif // AI(W906-NODRY) 20260924: 下一行的 { 是原本 else 區塊的開頭（保留成一般區塊，零行號位移）
    {   // AI(W906-ZEROARG) 20260918: MEASURED -- the previous text here was wrong in
        // both halves, and it recommended a flag that no longer exists.
        //
        // It said "flushes its in-memory copy on CloseGeneralIniFile() at exit".
        // Two measurements say otherwise:
        //   1. CloseGeneralIniFile() is called ONLY inside `if (dry)` (see the exit   ⚠ AI(W906-NODRY) 20260924: 那個 if (dry) 已隨 --dry 退場（#if 0）
        //      path further down). On this branch it is never called at all.
        //   2. Even if it were, it is a no-op here: vclcompat TIniFile is
        //      write-through (IniFiles.cpp:281 UpdateFile -> `if (!writeThrough_)`),
        //      so there is nothing buffered to flush.
        //   Probe: a key added to the file on disk 8 s into a run survived the exit
        //   byte-for-byte (md5 unchanged across shutdown).
        //
        // AI(W906-T4-INIFMT2) 20260924: THE WRITE HALF OF THIS HAZARD IS FIXED; the read half remains.
        // It used to be: TIniFile::WriteString -> store_.WriteRaw + flush(), flush() =
        // store_.SaveToFile(FileName) -- the WHOLE file rebuilt from the image loaded at
        // OpenGeneralIniFile() time, so ONE write of ONE key silently reverted every on-disk
        // change made since the server started. Now WriteString edits the CURRENT disk file in
        // place exactly like Win32 WritePrivateProfileStringA (vclcompat/IniFiles.cpp EOF;
        // differential test vs the real kernel32: 11,075 cases, 0 mismatches).
        //
        // The browser is a second, independent writer: system.file.put goes through
        // ApplyIniGuarded(path,...) (surgical, keeps a backup) and never touches
        // INIFileGeneral. Its edits now SURVIVE a C++ write (e.g. START -> bInitialCleanCount,
        // WebStart.cpp:1806). AI(W906-A5-INIREAD) 20260924: the read half is fixed too. C++ READS used to come from the startup image
        // (Win32 GetPrivateProfileString re-reads the disk; our store did not); now every INIFileGeneral read re-reads the file,
        // so a browser edit reaches the C++ side at its next read, without a restart. Measured 20260918 (docs/RULINGS_20260917.md);
        // write half fixed 20260924 on branch t4-inifmt2, read half 20260924 (A5, vclcompat/IniFiles.cpp EOF).
        std::printf("NOTE: the loader and the browser are two independent writers of\n"
                    "         %s.\n"
                    "         A C++-side write of a key edits only that line (Win32-style), so\n"
                    "         browser edits are no longer reverted, and every C++ READ re-reads\n"
                    "         the file (Win32-style), so a browser edit is seen at the next read.\n"
                    "         (A value C++ already copied into a variable keeps that copy.)\n",
                    savedGeneralPath.c_str());
    }

    std::printf("loading machine config from %s ...\n", asGeneralPath.c_str());
    WdStart();                                                                  // AI(W906-WD) 20260923
    WdMark("startup: LoadMachineConfig()");  InitialMemory();  W906_InstallChangeLogHooks();   // AI(W906-CHGLOG) 20260927: golden WriteIniData change-log hooks (common_ChangeLog.cpp); they log only once InitialOK is true, like golden. // AI(W906-INITMEM) 20260927: golden database.cpp:47 -- first thing in the HSys ctor, before ReadGeneralIni (= LoadMachineConfig below); cmydef_InitialMemory.cpp. Must stay BEFORE LoadMachineConfig (it zeroes LOAD_Z_USE_MOTOR[] that ReadGeneralIni fills)  // AI(W906-WD) 20260923
    if (!LoadMachineConfig()) {
        std::printf("LoadMachineConfig FAILED -- cannot open the ini\n");
        return 1;
    }
    { extern void FileRW_HSys_ReadMainCtorKeys(); FileRW_HSys_ReadMainCtorKeys(); }  { extern void W906_FRWBoot_JamRateByDayRead(); W906_FRWBoot_JamRateByDayRead(); }  { extern void W906_MainRecordBootLatch(); W906_MainRecordBootLatch(); }  /* AI(W906-PROD-S113) 20260926（Steven 團隊）：golden SYSTEM_MODULAR 建構子 database.cpp:47 HSys.SysTimer.LatchCycleTime(true)（系統時間起點；移植樹該建構子 #if 0）；本體 FileRW/MainRecord.cpp；不套也行（第一拍會補），套了開機這段也算進 PowerOn。接在同一行，不移動行號 */  /* AI(W906-FRW-S91) 20260926 (Steven 團隊)：golden TfMain::FormShow main.cpp:9607（V912 主 repo）RunInfo.ReadJamRateByDay() —— 讀 D:\HT9045_Log\JamRate_Daily 當天的 DailyJamRate txt（只讀；資料夾不在會建）；golden 在 :9604 ReadLastDataFile 之後、:9611 ReadWriteBinCountMode 之前。本體 FileRW/MainBoot.cpp。接在同一行，不移動行號 */  { extern void W906_FRWBoot_BinCountRead(); W906_FRWBoot_BinCountRead(); }   // AI(W906-FRW-Boot) 20260926 (Steven 團隊)：golden TfMain::FormShow main.cpp:9609（V912）ReadWriteBinCountMode(true) —— 讀 system 下的 BinCount.txt [System] Bin／ErrorBin（缺鍵照 golden 補寫）；在 LoadMachineConfig（LastSet、CUSTOMER_CODE）與建構子鍵之後、下面 asHandlerVersion（:9761）之前，同 golden 順序。本體 FileRW/MainBoot.cpp。接在同一行，不移動行號
    {   //Steven 20260925：golden TfMain::FormShow main.cpp:9734-9780（非 HiSilicon、非 ASE_KaohSiung 分支）的 asHandlerVersion。
        //  移植樹一直是 ""（cmydef.cpp:4531），HSys 存檔會把 Gerneral.ini [Version] Ver 寫成空字串。版本來自 exe 版本資源（tools/wb_serve.rc）。
        { extern AnsiString SVNRevision; SVNRevision=VerInfo().GetSVNRev(); }   //Steven 20260925：golden TfMain 建構子 main.cpp:1384（Data.Observer ShowVer :3554 asVer=asHandlerVersion+"."+SVNRevision）
        #ifdef FOR_NVIDIA_2D_SORT
            MainVersion="VisualSort.01.02";
        #else
            MainVersion=VerInfo().GetMainVersion();
        #endif
        if(IniConfig.bKoreaFunction)                  asHandlerVersion="V3.21";     // golden :9748-9751
        else if(CUSTOMER_CODE==CC_SCC)                asHandlerVersion="V3.29";     // golden :9752-9755
        else if(CUSTOMER_CODE==CC_PTI)                asHandlerVersion=VerInfo().GetFileVersion();   // golden :9758-9759
        else                                          asHandlerVersion=VerInfo().GetMainVersion();   // golden :9761
        #ifdef MTK_Version
            asHandlerVersion="MTK_"+asHandlerVersion;
        #endif
        #ifdef FOR_ASECL_L8
            asHandlerVersion=asHandlerVersion+"_L8";
        #endif
        #ifdef BETA_VERSION
            asHandlerVersion=asHandlerVersion+"_BETA";
        #endif
        std::printf("asHandlerVersion=%s (golden main.cpp:9761; exe version resource)\n", asHandlerVersion.c_str());  { extern void W906_FRWBoot_VersionStamp(); W906_FRWBoot_VersionStamp(); }  { extern void W906_FRWBoot_LimitAuth(); W906_FRWBoot_LimitAuth(); }  { extern void W906_FRWBoot_CriticalParaAuth(); W906_FRWBoot_CriticalParaAuth(); }  /* AI(W906-FRW-S91) 20260926 (Steven 團隊)：golden TfMain::FormShow main.cpp:9786 GetLimitAuth()（InputLimit，DoReadLastData 的 ReadTempFile 要用）＋:9787 GetCriticalParaAuth() —— 讀 config\CriticalParaControl.ini [Parameter Control] 26 鍵（缺鍵照 golden 補寫）；golden 在 :9783 版本戳記之後。本體 FileRW/MainBoot.cpp。接在同一行，不移動行號 */   // AI(W906-FRW-Boot) 20260926 (Steven 團隊)：golden TfMain::FormShow main.cpp:9779-9782（V912）WriteIniDataGeneral("Version", "Ver", asHandlerVersion) —— 每次開機寫 Gerneral.ini [Version] Ver（golden 無條件；SIGURD 那個條件 golden 自己註解掉）。必須在上面 asHandlerVersion 算完之後。本體 FileRW/MainBoot.cpp。接在同一行，不移動行號
    }   //Steven 20260925 (S12-C HSys)：golden TfMain 建構子 main.cpp:1772-1815／:2042-2055（EP_Install、INOUT_ARM_PICKER_USE_MOTOR、ION_FAN_TYPE、EP_MAX*）；golden 全域 HSys 建構子（ReadGeneralIni）先跑、TfMain 建構子後跑

    // --- 建構 fTeach（golden 的 CreateForm 對應點）---------------------------
    //
    //AI(W906-TEACH-W1) 20260919: 使用者裁決 A2「同意」。
    //
    // 為什麼需要：`Tech.*`（全部教導值）唯一的填入路徑是
    //   cinitial.cpp SetWorkParameter -> ReadTechData()（:15871）
    //   -> `if(fTeach!=NULL) fTeach->ReadFile()`（:15947）
    // 而 `fTeach` 在 forms/fTeach.cpp:72 是**裸指標，從未建構** —— 那個 `if`
    // 因此永遠是 false，`Tech.*` 永遠是 0，`CompareTechData()`（cinitial.cpp:10242）
    // 第一條就否決，於是 `start.run` 回 accepted:false。
    // 20260919 端到端量到的就是這條鏈。docs/RULINGS_20260917.md §C10.7。
    //
    // ⚠ MUST COME AFTER LoadMachineConfig()：`TfTeach` 的 ctor 會建 440 個
    //   登錄項，每一項都存 `MOT[MotorSelect]` 的索引，而 `MOT[].Alias` 是
    //   `LoadMachineConfig()` 從 Mot_Table.csv 填的。先建會得到一堆空 Alias，
    //   `ReadFromFile()` 就會全部走 `else (*Parameter)=0`。
    //   （這與下面 PumpInit 的 MUST-COME-AFTER 是同一類的順序相依。）
    //
    // ⚠ 為什麼不在靜態初始化期建：forms/fTeach.cpp:67-71 寫明那是**刻意的
    //   SIOF 迴避**，而且 golden 自己也是在 HT9045.cpp 的 CreateForm 才建，
    //   不是在檔案範圍。這裡就是 golden 那個時機的對應點。
    //
    // ⓘ 代價：一次性配置 440 個小物件。沒有其他副作用 —— ctor 只寫自己的欄位
    //   與登錄表，不碰硬體、不讀檔、不寫檔。讀檔是 ReadFile() 的事，
    //   而它要等 SetWorkParameter 被呼叫才跑。
    if (fTeach == 0) {
        fTeach = new TfTeach();
        std::printf("fTeach constructed: %d TechPara + %d TechTwoPara entries\n",
                    (int)fTeach->TechPara.size(), (int)fTeach->TechTwoPara.size());
    }

    //AI(W906-T6-MAINPROC) 20260923: golden TfMain::TfMain（main.cpp:1562-1570）開機時無條件建立的三個 TStringList 全域。
    //   cmydef.cpp:122-124 把它們定義成裸指標，移植樹從來沒有 new 過（SECSGEM/uHGemHT9045.cpp:4321 記載過這個 NULL 曝露）。
    //   T6 讓 MainProc -> CheckContinusStartIsReady -> DoInitialStart 變成活的之後，第一次 Initial Start 就在
    //   DoInitialStart 的 `slDupBundlID->Clear(); slDupBundlID->SaveToFile(asDupBundleID);` 解參考 NULL
    //   （20260923 gdb 實測：DoInitialStart+0xa9c、eax=0；nm: 063fbd94 B slDupBundlID）。
    //   純記憶體容器，建構與 Clear() 沒有副作用；照 golden 的順序與寫法，放在 fTeach（同一個 golden 時機的對應點）後面。
    if (slBundlID == 0)          slBundlID=new TStringList();                                         // golden main.cpp:1562
    if (slDupBundlID == 0)       { slDupBundlID=new TStringList();       slDupBundlID->Clear(); }       // golden main.cpp:1564-1566
    if (slDupUnloadBundlID == 0) { slDupUnloadBundlID=new TStringList(); slDupUnloadBundlID->Clear(); } // golden main.cpp:1568-1570

    // --- 跑一次 SetWorkParameter()（golden 的 bring-up 路徑）-----------------
    //
    //AI(W906-TEACH-W1) 20260919: 建好 fTeach 還不夠 —— 得有人呼叫它。
    //
    // golden 的鏈是：
    //   InitialHandler()（cinitial.cpp:16765，golden 的 bring-up）
    //     -> SetWorkParameter()（:16780）
    //       -> ReadTechData()（:7005，本波解閘）
    //         -> fTeach->ReadFile()（cinitial.cpp:15947，本波解閘）
    //           -> TechPara[i]->ReadFromFile()  <- 教導值真正被讀進 Tech.* 的地方
    //
    // ⚠ 實測 20260919：`wb_serve.cpp` 對 `InitialHandler` 的命中數是 **0**，
    //   也就是這個行程從來沒走過 golden 的 bring-up。所以就算把 fTeach 建起來、
    //   把兩個閘解開，`Tech.*` 仍然全是 0 —— 第一次跑完的實測結果就是這樣：
    //   `start.run` 照樣回 accepted:false，log 照樣印「加熱盤的點位有誤」。
    //
    // 為什麼呼叫 SetWorkParameter() 而不是 InitialHandler()：
    //   `InitialHandler()` 是完整的機台 bring-up（InitHontechHardware、
    //   LoadMachineRecord、ReadPassword、SetMotorSpeed、各種 COM 埠初始化…）。
    //   這個行程要的只是**教導值那一條**。叫整個 bring-up 等於順手啟動一堆
    //   這個行程沒有準備好的東西，而且它們各自有各自的閘。
    //   ⇒ 這是一個**整合接縫**（wb_serve 走 golden bring-up 的哪一段），
    //     不是翻譯偏離 —— `SetWorkParameter()` 本身是逐字翻譯的。
    //   ⓘ 之後如果要讓 wb_serve 走完整 bring-up，這一行會被 InitialHandler()
    //     取代，而不是與它並存（會跑兩次）。
    //
    // ⚠ 會寫真實檔：`ReadFile()` 在機台有 Alignment CCD 且 teach.ini 的
    //   `bAOAMatrix=0` 時會補寫 `bAOAMatrix=1`。使用者 20260919 裁決 A1「可以」。
    //   驗證照 docs/PLAN_START_TO_RUN.md §0.6：tools/realfile_guard.py
    //   snap -> 跑 -> check -> drop。
    // --- 讀 Mot_Table.csv 進 MOT[]（teach 讀取路徑的前置）--------------------
    //
    //AI(W906-TEACH-W1) 20260919: 第二次實測抓到的斷點。
    //
    // 第一次跑完：`SetWorkParameter -> true` 但 `Tech.iInArmPlate1Y=0`。
    // 追下去：`TECH_PARA::ReadFromFile()` 用 `MOT[MotorSelect].Alias` 當 ini
    // 的**區段名**，而 golden 的
    //     `if(MOT[MotorSelect].Alias!="")  ... else (*Parameter)=0;`
    // 在 Alias 是空字串時走 else，把值寫成 0 —— 也就是「讀了，但讀進 0」。
    //
    // `MOT[].Alias` 的唯一寫入點是 `InitialMotorParameter()`
    // （cinitial.cpp:4080 / :4266），而它的唯一呼叫者是
    // `InitHontechHardware()`（cinitial.cpp:10877），再上去是
    // `InitialHandler()`（:16774）—— wb_serve 兩個都沒跑。
    //
    // ⇒ 與 SetWorkParameter 同一個理由：這是**整合接縫**，不是翻譯偏離。
    //   叫最窄的那一支，把 teach 讀取路徑需要的資料補上。
    //
    // ⚠ 這一支不只填 Alias：它還會依 Mot_Table.csv 的 CardModel 掛 vendor 驅動
    //   （`new TMyGALILMotor` / `TMyMN200Motor` / `TMySMCMotor` /
    //   `TMyEtherCatMotor`）並呼叫 `InitMotor()`。在這台筆電上那些是 offline
    //   vendor 層；**在真的有卡的機器上它會去碰卡**。
    //   使用者 20260919 裁決 A3：「不用讓我停下來看，直接執行，我接下來會拿到
    //   機台端測試且用中斷點驗證功能」。依 §0.5 照 golden 的 bring-up 走。
    //
    // ⓘ 前後都印 Alias，讓「是不是這個原因」變成可以直接看的東西，
    //   而不是我在註解裡的主張。
    std::printf("before InitialMotorParameter: MOT[MInArmY].Alias=\"%s\"\n",
                MOT[MInArmY].Alias.c_str());
    /* InitialMotorParameter(); */   // AI(W906-A4-2) 20260924: 使用者裁決 A4「按照舊版本作法」—— 改由下面的 InitialHandler()→InitHontechHardware 呼叫（golden cinitial.cpp:5823）。兩處並存會重複初始化（Galil 物件 delete／new、TMyProductionRecord 洩漏、fAllMotorHome 重設），所以這裡拿掉
    std::printf("(InitialMotorParameter now runs inside InitialHandler below) MOT[MInArmY].Alias=\"%s\"\n",
                MOT[MInArmY].Alias.c_str());
    { extern void W906_BootCreateAlarm(void*); W906_BootCreateAlarm(fMain); }   // AI(W906-HALARM) 20260926: golden TfMain::SetInitialData main.cpp:22472-22473 `Alarm = new HAlarm(this); Alarm->Clear();`（golden 建構子 main.cpp:2120，在下面 :2121 SetMyKitSuckItemAmount 之前）。fMain 此時已指向 g_webMain（:3781）。佔用原本的空行，不移動行號
    // AI(W906-ARMOFS) 20260923: golden TfMain 建構子 main.cpp:2121-2139 —— 吸嘴格數＋手臂偏移物件。
    //   20260923 gdb 實測：按 START、歸零完成進入運轉約 10 秒後 SIGSEGV
    //     MainProc -> DoAllProcess -> DoOutArm -> DoOutArm_9045_1x2_2
    //     -> MoveOutArmToShuttleIncludeZ_9045_1x2_2 (aoutarm9045_1x2_2.cpp:435)
    //     -> GetOutArmPitchY_9045 (aoutarm9045.cpp:269)
    //     -> OutArmOffSet[0]->GetVariableY()  —— this == NULL
    //   ⇒ golden 在 main.cpp:2123-2139 `new` 這些 ARM_OFFSET，移植樹沒有任何地方配置它們
    //     （main.cpp 沒移植；cSocket.cpp 的 ArmDataBootstrap 補了緊接在後的 :2141-2148，
    //      但當年的 nullsweep 抓不到「陣列元素賦值」，這一段就漏了 —— 那段橫幅自己寫了這個限制）。
    //   ⚠ 不能照 ArmDataBootstrap 做成靜態初始化物件：ARM_OFFSET 建構子（cprod.cpp:255）
    //     讀 InArmSuck.iMotRow／iMotCol 決定吸嘴偏移表的格數 —— 那是別的 TU 的全域物件，
    //     而且要等 SetMyKitSuckItemAmount() 設過才是機型的真值。所以照 golden 的順序在執行期做：
    //     golden :2121 SetMyKitSuckItemAmount() 在前，:2123-2139 配置在後。
    //   SetMyKitSuckItemAmount()（cinitial.cpp:10837，活的本體）只設各 TMyKitSuck 的格數／馬達數，
    //   純資料、沒有 IO 也沒有動作。它在 cinitial.cpp 的 `#if 0 // GATE n2-1` 裡另有一個呼叫點，
    //   那個閘寫的理由「0 definitions tree-wide」已過期；那邊不在本段範圍，沒動。
    SetMyKitSuckItemAmount();                                                   // golden main.cpp:2121
    for(int i=0; i<InOfsTotal; i++)                                             // golden main.cpp:2123-2127
    {
        InArmOffSet[i]=new ARM_OFFSET();
        InArmOffSet_File[i]=new ARM_OFFSET();
    }
    for(int i=0; i<OutOfsTotal; i++)                                            // golden main.cpp:2129-2133
    {
        OutArmOffSet[i]=new ARM_OFFSET();
        OutArmOffSet_File[i]=new ARM_OFFSET();
    }
    for(int i=0; i<SortOfsTotal; i++)                                           // golden main.cpp:2135-2139  RogerYang 20250417 for HT9046AU add
    {
        SortArmOffSet[i]=new ARM_OFFSET();
        SortArmOffSet_File[i]=new ARM_OFFSET();
    }
    //   ⚠ **偏離 golden**：把偏移數值歸零。golden 的 ARM_OFFSET 建構子不初始化這些 double，
    //     值是之後由 TfOffSet::ReadFile（cOffSet.cpp）從偏移檔讀進來的；那個讀檔在移植樹
    //     **整族不存在**（cOffSet.cpp／AutoTeach/InOutArmZteach.cpp／ArmOffsetData.cpp 都沒移植，
    //     見 INBOX T4）。不歸零就是未定義的記憶體值，會被當成手臂偏移量去動。
    //     歸零 ＝「偏移檔全部是 0」。Offset 族移植進來之後，讀檔會覆寫這些值，這段就無害。 Steven 20260924：已成立 —— JerryYang ca4e903 移植了 cOffSet.cpp，開機稍後的 offset.* 鏈（本檔 :3272）會覆寫；偏移檔不存在時維持 0。
    {
        ARM_OFFSET** const w906OfsArrays[6] = { InArmOffSet, InArmOffSet_File, OutArmOffSet,
                                                OutArmOffSet_File, SortArmOffSet, SortArmOffSet_File };
        const int w906OfsCounts[6] = { InOfsTotal, InOfsTotal, OutOfsTotal, OutOfsTotal, SortOfsTotal, SortOfsTotal };
        for(int a=0; a<6; a++)
            for(int i=0; i<w906OfsCounts[a]; i++)
            {
                ARM_OFFSET* o = w906OfsArrays[a][i];
                o->bOneByOne=false;
                o->dArmX=0.0;  o->dArmY=0.0;
                o->dArmVariable=0.0;  o->dArmVariableY=0.0;
                o->dArmVariable2=0.0; o->dArmVariable3=0.0; o->dArmVariable4=0.0;
                o->dPickUp=0.0;  o->dPlaceUp=0.0;
                for(int k=0; k<4; k++) o->dXPitch[k]=0.0;
            }
    }
    std::printf("arm offsets allocated (golden main.cpp:2121-2139): In=%d Out=%d Sort=%d, values zeroed"
                " until the offset.* chain below reads the offset files\n", InOfsTotal, OutOfsTotal, SortOfsTotal);
    { extern void FileRW_TrayAssignment_InitProdTrayType(); FileRW_TrayAssignment_InitProdTrayType(); }
    { extern void FileRW_TrayAssignment_InitTrayLayout(); FileRW_TrayAssignment_InitTrayLayout(); }   //Steven 20260925：golden TfMain 建構子 main.cpp:1830-1897 的 Tray 版面全域（iFixRight／iAutoRight／iAutoCnt／iMagAtAuto…，依 AUTO_EMPTY_COLOR）；移植樹從沒設 → iFixRight 停在 cmydef 初值 5，golden ReadFile 讀不到 Fix1..3。全機共用的全域，開機行為照 golden 改變   //Steven 20260925：golden TfMain 建構子 main.cpp:1414-1497 的 Prod.iTrayType[] 預設（移植樹從沒做 → 全部 tNotUse → golden ReadFile 跳過 Auto/Fix 區段）；要在建 fTrayAssignment／第一次 SetWorkParameter 之前
    { extern void W906_BootCreateTrayAssignment(); W906_BootCreateTrayAssignment(); }  // AI(W906-T4-U37b) 20260924: 第一次 SetWorkParameter 之前建 fTrayAssignment，讓解開的 GATE n2-11（DoSetupSystemToProd 結尾重讀 Tray.Data）每次都照 golden 走。放在空行上，不移動行號
    { extern void GetMainAuth(); extern void GetObserAuth(); GetMainAuth(); GetObserAuth(); }   //Steven 20260924 (S12)：golden TfMain::TfMain main.cpp:1738-1739（config\Security_new.def [Main]／[Observer] → authMainForm／authObserver）；移植樹從沒呼叫 → authMainForm 全 false，golden 表單的 tsX->Enabled=(Insufficient(3)&&authMainForm[3]) 全部停用
    { extern void W906_COM2_CreateFormBoot(); W906_COM2_CreateFormBoot(); }   /* AI(W906-R28TORQ) 20260925: golden CreateForm(TCOM2)（HT9045.cpp:174：TfMain 之後、TfTeach／TfConfiguration 之前）的建構子本體 rs232.cpp:95-125 —— 讀 Gerneral.ini [IndexDriver] INDEX_DRIVER_TYPE／USE_HP_COM_CARD（缺鍵時照 golden 補寫回去）；見 rs232.cpp。接在同一行，不移動其後行號 */  { extern void FileRW_IniConfig_Boot(); FileRW_IniConfig_Boot(); }  { extern void W906_CfgTrayPlate_CreateForm(); W906_CfgTrayPlate_CreateForm(); }  /* AI(W906-FRW-S98) 20260926 (Steven 團隊)：golden CreateForm(TfConfiguration)（HT9045.cpp:207）的 Tray／Plate 表那一塊＝建構子尾段 cConfiguration.cpp:226-234 if(bHasTrayCSV) sbtReloadTray->Click()…；兩顆 Load Data 鈕登記成 "TfConfiguration" 具名替身（IniConfig FormShow 的 ->Click() 才讀得到表）。這時 bHasTrayCSV 還是 false（golden 也是：TfMain::FormShow :9987 才設）⇒ 開機不讀檔。本體 FileRW/CfgTrayPlate.cpp。接在同一行，不移動行號 */  { extern void FileRW_Teach_Boot(); FileRW_Teach_Boot(); }   /*AI(W906-W5-TEACH) 20260925: golden TfTeach ctor InitialTeachEditList -> ReadTechData loads Teach.* */
    { extern void W906_SecurityBoot(); W906_SecurityBoot(); }   //Steven 20260924 (S12)：golden CreateForm(TfSecurity)（HT9045.cpp:208，在 TfConfiguration 之後；GetLevelSet 的 CheckRange 依 CosFunction.bSecurityHave5Level，審查第 8 輪 M-5）：iMaxLevelItem＋levelset.dat（見 cSecurity.cpp 檔尾）
    { extern void FileRW_Speed_Boot(); FileRW_Speed_Boot(); }
    { extern void FileRW_YieldMonitoring_Boot(); FileRW_YieldMonitoring_Boot(); }  { extern void FileRW_QAMode_Boot(); FileRW_QAMode_Boot(); }   //Steven 團隊 20260925 (S12-C TestIF_File_QAMode)：golden CreateForm(TfQAMode) HT9045.cpp:228；只建替身，讀檔仍是 DoReadLastData 鏈的 fQAMode->ReadFile()
    { extern void FileRW_TrayAssignment_Boot(); FileRW_TrayAssignment_Boot(); }
    { extern void (*g_W906_TrayAssignmentReadFileHook)(); extern void FileRW_TrayAssignment_ReadFile();
      g_W906_TrayAssignmentReadFileHook = &FileRW_TrayAssignment_ReadFile; }   //Steven 20260925：TfTrayAssignment::ReadFile 全部改走 golden 912 版（Steven 裁決；forms/fTrayAssignment.cpp 的註解）
    { extern void FileRW_TesterIF_Boot(); FileRW_TesterIF_Boot(); }   //Steven 團隊 20260925（暫接，整合者定）：golden CreateForm(TFTestIF) HT9045.cpp:189（TfDIOFrom :196 之前）：TFTestIF 替身＋建構子 InitcbDIOType(false)；不讀 Tester.Data（讀檔在下面 contact 鏈之後）
    { extern void FileRW_TTLCfg_Boot(); FileRW_TTLCfg_Boot(); }   //Steven 20260925 (S12-C TTLCfg)：golden TfDIOFrom 替身（HT9045.cpp:196 CreateForm）；不讀檔
    { extern void FileRW_ShuttleMove_Boot(); FileRW_ShuttleMove_Boot(); }   //Steven 團隊 20260926 (S12-C ShuttleMove)：golden TfShuttleMove 替身（DFM＋建構子 ShuttleMove.cpp:59）；不讀檔
    { extern void FileRW_HSys_Boot(); FileRW_HSys_Boot(); }  { extern void FileRW_ContactForce_Boot(); FileRW_ContactForce_Boot(); }  { extern void FileRW_ACTForm_Boot(); FileRW_ACTForm_Boot(); }  /* AI(W906-FRW-S108) 20260926 (Steven 團隊)：golden CreateForm(TACTForm) HT9045.cpp:221（TfContactForce :220 之後）→ 建構子 AutoTemperature.cpp:289；golden :297 LoadACTData 在 :398 設 FilePath 之前 ⇒ 讀空檔名、ACTData＝預設值，不讀不寫任何檔。要在 LoadMachineConfig 之後（iSocketBaseTempCount）。本體 FileRW/ACTForm.cpp，冪等 */  /* AI(W906-FRW-S57) 20260926 (Steven 團隊)：golden CreateForm(TfContactForce) HT9045.cpp:220（THandlerSystem :210 之後）；只建替身（DFM 設計期狀態＋存檔讀的替身＋容器父子），不讀不寫檔 —— golden 建構子本體延到第一次 editlist.get ContactForce；開機讀檔仍是下面的 LoadContactForceTables()（已補 dIndexZOffset，P4）。冪等 */  /* AI(W906-FRW-S162) 20260927 [W906]：更正前一段 S57 註解 —— R15＝B（Steven，RULINGS_20260926 S162）起 FileRW_ContactForce_Boot() 開機就跑 golden 建構子本體（V912 ContactForce.cpp:421-653，照 CreateForm HT9045.cpp:220 時序；呼叫位置不變）：bUseDynamicKitDiameter 時依 [SLK Type] 在 D:\HT9045\system\ContactInfo.ini 補缺鍵（Ind 段 Diameter_<d>mm_<n>）、沒檔時 WriteFile 建檔；下面的 LoadContactForceTables() 讀的就是補過／建好的檔。細節見 FileRW/ContactForce.cpp FileRW_ContactForce_Boot */   //Steven 20260925 (S12-C HSys)：golden CreateForm(THandlerSystem) HT9045.cpp:210（TfConfiguration 之後）
    { extern void FileRW_BinSelect_Boot(); FileRW_BinSelect_Boot(); }   //Steven 20260925 (S12-C BinSelect)：golden TfBinSel 替身＋收養 fBinSel 真元件；要在下面 fBinSel->ReadFile 之前   //Steven 20260925 (S12-C)：golden TfTrayAssignment 替身（HT9045.cpp:188；建構子讀 bUseAuto2Empty／CosFunction，要在 LoadMachineConfig 之後）；開機讀檔仍是 W906_BootReadTrayAssignment   //Steven 20260925 (S12-C)：golden TfYieldMonitoring 替身（HT9045.cpp:215 CreateForm，在 TfConfiguration 之後；讀檔仍是 fYieldMonitoring->ReadFile）   //Steven 20260924 (S12-C ArmSpeed_File)：golden TfSpeed 的替身（DFM 狀態＋建構子）；讀檔仍是 fSpeed->ReadFile   //Steven 20260924 (S12-C)：golden main.cpp:1532-1543 new 各 HTEditList ＋ TfConfiguration 建構子的註冊（InitConfigEdtList），要在 InitialHandler（golden FormShow）之前；見 FileRW/IniConfig.cpp
    { extern void FileRW_BarCode_Boot(); FileRW_BarCode_Boot(); }  { extern void FileRW_Monitor_Boot(); FileRW_Monitor_Boot(); }  /* AI(W906-FRW-S110) 20260926 (Steven 團隊)：golden CreateForm(TfMonitor) HT9045.cpp:247（TfBarCode :243 之後）→ 建構子 MonitorInterface.cpp:32 LoadTCPIPParament()：只讀 D:\HT9045\system\MVData.ini（檔不在＝預設值，不建檔）；MVCtrl（TCP）不做。本體 FileRW/Monitor.cpp，冪等 */  { extern void FileRW_GroundMan_Boot(); FileRW_GroundMan_Boot(); }  { extern void FileRW_Rotate_Boot(); FileRW_Rotate_Boot(); }  { extern void W906_TrayMapping_BootFlags(); W906_TrayMapping_BootFlags(); }  { extern void FileRW_AutoAlignment_Boot(); FileRW_AutoAlignment_Boot(); }  { extern void FileRW_Winway_Boot(); FileRW_Winway_Boot(); }  /* AI(W906-FRW-S109) 20260926 (Steven 團隊)：golden CreateForm(TfWinway) HT9045.cpp:272（TfAutoAlignment :270 之後）→ 建構子 WinWaySetting.cpp:12：4 站 LoadCommData＋SaveCommData ⇒ 每次開機讀、並原樣寫回 D:\HT9045\config\ATCWinWay.ini（golden 同，不看 ATC_SYSTEM；檔不在會照預設值建）；不開 COM、不送 Modbus。本體 FileRW/Winway.cpp，冪等 */  { extern void FileRW_CounterSel_Boot(); FileRW_CounterSel_Boot(); }  { extern void FileRW_AutoCalSuckZ_Boot(); FileRW_AutoCalSuckZ_Boot(); }  /* AI(W906-FRW-S70) 20260926 (Steven 團隊)：golden CreateForm(TfProductionInfo) HT9045.cpp:257 → 建構子 ProductionInfo.cpp:52 的 AutoCalSuckZ 段（高度差陣列清零、Enable=false、elData 22 筆具名註冊、SearchStartZ=0）；不讀檔、不寫檔；要在 W906_DoReadLastData 之前，冪等 */  /* AI(W906-CRT-CounterSel) 20260926（Steven 團隊，S56）：golden CreateForm(TfCounterSel) HT9045.cpp:203 → 建構子 cCounterSel.cpp:23（NeedRef=false）；只建替身、不讀檔（[Visible] 由 ReadLastSetIni 讀），冪等 */   /* AI(W906-FRW-NoPage) 20260926 (Steven 團隊)：golden CreateForm(TFrmRotate) HT9045.cpp:227 → 建構子 fRotate.cpp:42（bShowRotateBySite 等；不讀檔）；CreateForm(TfTrayMapping) :252 → 建構子 cTrayMapping.cpp:92-104 的 bUseTrayMap 等五旗標（移植樹原本在靜態初始化算，看不到 Gerneral.ini）；CreateForm(TfAutoAlignment) :270 → 建構子 AutoAlignment.cpp:47（Gerneral.ini [Auto_Alignment] CCD 位址／埠，缺鍵照 golden 補寫）。三者與 BarCode／GroundMan 沒有共用狀態、也沒有 HTEditList，順序不影響；都要在 LoadMachineConfig 之後、W906_DoReadLastData 之前 */   //Steven 團隊 20260925 (S12-C TestIF_File_BarCode)：golden CreateForm(TfBarCode) HT9045.cpp:243（TfQAMode :228 之後）；只建替身（DFM 狀態＋建構子），不讀檔（讀檔在 W906_DoReadLastData golden :9361）   //AI(W906-CRT-GroundMan) 20260926 (Steven 團隊)：golden CreateForm(TfGroundMan) HT9045.cpp:262（TfBarCode :243 之後）→ 建構子 GroundMan.cpp:24 → :136 ReadGroundOffset()：讀 D:\HT9045\system\GroundMan.ini（CheckAndReadIniData 缺鍵照 golden 補寫，路徑是 golden 寫死的字面值）；要在 LoadMachineConfig 之後（HSys.iGroundManScanPoint、CUSTOMER_CODE）
    {   { extern void W906_InstallUpdateMainOperateMode(); W906_InstallUpdateMainOperateMode(); std::printf("UpdateMainOperateMode body installed = yes\n"); }  { extern void W906_BootInitialATC(); W906_BootInitialATC(); }  /* AI(W906-ATC1) 20260926: golden FormShow main.cpp:9357-9361 InitialATC＋ATCIniPath，在 InitialHandler（:9559）之前 —— 沒有它 USE_ATC_MODE=4 的機台 ChangeATCSiteUse 會對空的 ATC_SYS_PAL 取 [0]（NB2 R72 ATC-1，forms/fMain_ATCSiteUse.cpp 檔尾） */   // AI(W906-OPMODE) 20260926: 在 InitialHandler 之前裝 —— 它的 LoadMachineRecord（cinitial.cpp:10231）→ SetMainRunStartMode → UpdateMainOperateMode 在 golden 開機就會跑（切加熱器繼電器、送 ATC 命令、寫 lastdata），0922 裁決「都要，全部動作都要執行」；fTemp_Set 在 :3111 已建［⚠ 20260926 NB2 R72 OPM-4 更正：:3111 在 W906_DoReadLastData 裡，比 InitialHandler 晚 —— InitialHandler 裡那一次呼叫時 fTemp_Set 還沒讀檔］
        { extern void GetTimeInfo(); GetTimeInfo(); }   /* AI(W906-CLOCK) 20260924: golden TfMain ctor main.cpp:1683 在 InitialHandler（:9559）之前先更新一次掛鐘。沒有這一次，網頁命令在第一個 500 ms 拍子之前就被排空（每 50 ms 一圈）時會讀到哨兵 9999 —— 非模擬探針實測 lot.start 寫出 LotStartTime=9999-9999-00 9999:9999:9999 */  { extern void W906_CfgTrayPlate_MainFormShowFlags(); W906_CfgTrayPlate_MainFormShowFlags(); }  /* AI(W906-FRW-S98) 20260926 (Steven 團隊)：golden TfMain::FormShow main.cpp:9987-9988 bHasTrayCSV=FileExists(TrayTablePath); bHasPlateCSV=FileExists(PlateTablePath);（golden 緊接著 :9992 InitialHandler）。本體 FileRW/CfgTrayPlate.cpp，只查檔在不在。插在同一行，不移動行號 */  InitialHandler(); const bool okWP = true;  { extern void W906_BootSummary(bool); extern bool W906_WbServeHas1203(); W906_BootSummary(W906_WbServeHas1203()); }   /* AI(W906-BOOTSUM) 20260925: 開機摘要（BootSummary.cpp），純觀測 */   // AI(W906-A4-2) 20260924: golden main.cpp:9559 —— 完整 bring-up（InitialClass、InitHontechHardware〔InitSucker／InitialSwitch／InitialSensor／OpenPCI132Card／InitialMotorParameter／InitCylinder〕、LoadMachineRecord、ReadPassword、SetWorkParameter、SetMotorSpeed）取代這裡原本只叫 SetWorkParameter 的整合接縫；位置在 golden ctor 那段（吸嘴格數、ARM_OFFSET，上面）之後，順序與 golden 相同。U23／U24（A9）由此照 golden 順序接上
        std::printf("InitialHandler (golden main.cpp:9559) done -> %s   (Tech.iInArmPlate1Y=%d iInArmPlate2Y=%d)\n",
                    okWP ? "true" : "false",
                    (int)Tech.iInArmPlate1Y, (int)Tech.iInArmPlate2Y);
    }

    // --- ContactForce 的四張表（P2a）----------------------------------------
    //
    //AI(W906-P2a-CF) 20260919: golden 在 `TfContactForce` 的 ctor 裡載這四張表
    // （ContactForce.cpp:421-652），也就是程式起來的時候。移植樹沒有那個表單，
    // 所以載入時機要由整合者決定 —— 這裡就是那個決定。
    //
    // ⚠ **必須在 LoadMachineConfig() 之後**：驅動讀的
    //   `CosFunction.bUseDynamicKitDiameter` / `EP_Install` / `INSTALL_DOUBLE_EP`
    //   / `IniConfig.bSPILFunction` / `CUSTOMER_CODE` 全部是 LoadMachineConfig
    //   從 Gerneral.ini 填的。先載會得到一組預設值算出來的表。
    //
    // ⚠ 為什麼一定要在這裡接一行，而不是留給 P2b：
    //   20260919 量到 `ContactForceTables()` 與四支 `Load*SlkTable()`
    //   **全樹呼叫點都是 0** —— 計算核心翻得很完整，但沒有人叫它。
    //   如果我只交付 `LoadContactForceTables()` 而不接，那就是同一個缺陷
    //   換一層皮（memory: ht9045-v906-archive-extraction-trap，
    //   「build 綠」證明不了接上了）。
    //
    // ⓘ 只讀不寫：golden ctor 尾端的 `else WriteFile();`（整份 ContactInfo.ini
    //   重寫，golden :1180-1374）**沒有翻**，理由寫在 ContactForceLoad.cpp。
    //   所以這一行不會動到 `system\ContactInfo.ini`（已加進
    //   tools/realfile_guard.py 的 TARGETS，實測 same）。
    {
        const int nCF = LoadContactForceTables();
        SlkForceTables& cft = ContactForceTables();
        std::printf("ContactForce tables loaded: %d entries "
                    "(SLK=%u Ind=%u DieForce=%u DieForce1by1=%u)\n",
                    nCF,
                    (unsigned)cft.SLKClass.size(), (unsigned)cft.SLKIndClass.size(),
                    (unsigned)cft.DieForceSLKClass.size(),
                    (unsigned)cft.DieForceOneByOneSLKClass.size());
    }

    // AI(W906-FW1e) 20260820: golden's startup mirror (main.cpp:9440
    // cbSetupFileName->Text=GetLastOpenFN()) -- the CURRENT recipe name, read
    // once from setup.inf (LastDataPath, read-only). Done here, BEFORE the
    // ShowMyMessage hook is installed, so a missing setup.inf logs once to
    // stdout instead of broadcasting a modal every tick. "Fail Open" on
    // failure is golden's own combo text, published as-is.
    fMain->cbSetupFileName->Text = GetLastOpenFN();  { extern void W906_FRW_InstallBackupSetupFile(); W906_FRW_InstallBackupSetupFile(); }  /* AI(W906-FRW-S92) 20260926 (Steven 團隊)：裝上 golden TfMain::BackupSetupFile（main.cpp:34053）本體 FileRW/MainBackup.cpp —— 之後每一頁存檔都照 golden 重寫 <配方>\*.MD5；要在 cbSetupFileName->Text 設好之後。接在同一行，不移動行號 */  { extern void W906_FRW_InstallSaveRunMode(); W906_FRW_InstallSaveRunMode(); }  /* AI(W906-PROD-S95R) 20260926 (Steven 團隊)：裝上 golden TfMain::SaveRunMode（main.cpp:33648）本體 FileRW/MainClose.cpp —— 之後 SetRunStartMode（RunStartMode.cpp:883）照 golden main.cpp:1129 寫 d:\HT9045\system\RunMode.txt（一行 RunMode=%d）；LastSet 已在 LoadMachineConfig 讀過。接在同一行，不移動行號 */  { extern void (*W906_UpdateRecordScreenBody)(bool); extern void W906_TfMain_UpdateRecordScreen(bool); W906_UpdateRecordScreenBody = &W906_TfMain_UpdateRecordScreen; }  /* AI(W906-PROD-S113) 20260927 (Steven 團隊)：S119 R1 —— 裝上 golden TfMain::UpdateRecordScreen（main.cpp:8584，本體 FileRW/MainRecord.cpp）給 Main.Record 的 CLEAR（golden spbClearRecordClick :31137 UpdateRecordScreen(true)；指標在 JsonBridge/actions/MainRecordClear.cpp:24，St02 2e7e3e15）；ctest 不裝＝照舊略過。接在同一行，不移動行號 */  { extern void W906_SC_SocketIDLog(); extern void (*W906_SocketIDLogBody)(); W906_SocketIDLogBody = &W906_SC_SocketIDLog; }  /* AI(W906-W11) 20260927 (Steven 團隊 St01)：裝上 golden TfStartCondition::SocketIDLog（V912 cStartCondition.cpp:1435-1492）的本體 FileRW/StartCondition.cpp W906_SC_SocketIDLog——St02 的呼叫點 atester.cpp:944-945（golden atester.cpp:882，GetTesterResult case 1）呼叫 W906_SocketIDLogBody；每次測試開始寫 D:\HT9045_Log\SocketIDLog\YYYYMM\SocketID_Lifetime_YYYYMMDD.csv（W906_SOCKETIDLOG_ROOT 可改根目錄，decisions R68～R70）；ctest 不裝＝不呼叫。接在同一行，不移動行號 */
    std::printf("recipe.current = %s\n", fMain->cbSetupFileName->Text.c_str());

    // AI(W906-FW-BIN1) 20260820: the bin.* display chain
    // (docs/RECON_binstar_datasource.md). BinSelect's loader is translated and
    // faithful, but it reads recipe Binasgn*.Data through CheckAndReadIniData,
    // whose missing-key seeding WRITES the file it reads (and ReadFile itself
    // carries one WriteIniData) -- against DataPath, a THIRD hardcoded shared
    // production path family --dry did not yet cover. Same protection pattern
    // as asGeneralPath, extended: copy the recipe folder to scratch and point
    // DataPath there for the whole run. --real keeps golden's true paths, the
    // same explicit-opt-in contract as the Gerneral.ini handling above.
    const AnsiString savedDataPath = DataPath;
    gRealDataPath = savedDataPath.c_str();         //Steven 20260916 (A1)
    AnsiString recipeScratchRoot;
    bool binSelLoaded = false;
    bool tempLoaded = false;   // AI(W906-FW-TEMP2) 20260820: see the temp.* block below
    {
        const AnsiString recipe = fMain->cbSetupFileName->Text;
        bool pathReady = (recipe.Length() > 0 && recipe != "Fail Open");
#if 0 // AI(W906-NODRY) 20260924: --dry 的配方資料夾暫存導向隨 --dry 退場（DataPath 永遠是真實配方路徑）
            char tmp[MAX_PATH];
            ::GetTempPathA(MAX_PATH, tmp);
            recipeScratchRoot = AnsiString(tmp) + "wb_serve_recipe\\";
            const AnsiString src = savedDataPath + recipe;
            const AnsiString dst = recipeScratchRoot + recipe;
            ::CreateDirectoryA(recipeScratchRoot.c_str(), 0);
            ::CreateDirectoryA(dst.c_str(), 0);
            WIN32_FIND_DATAA fd;
            HANDLE h = ::FindFirstFileA((src + "\\*").c_str(), &fd);
            int copied = 0;
            if (h != INVALID_HANDLE_VALUE) {
                do {
                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                    if (::CopyFileA((src + "\\" + fd.cFileName).c_str(),
                                    (dst + "\\" + fd.cFileName).c_str(), FALSE)) ++copied;
                } while (::FindNextFileA(h, &fd));
                ::FindClose(h);
            }
            if (copied > 0) {
                DataPath = recipeScratchRoot;
                std::printf("--dry: recipe folder scratch-copied (%d files), DataPath redirected\n", copied);
            } else {
                pathReady = false;
                std::printf("--dry: recipe folder empty/missing (%s) -- bin.* stays null\n", src.c_str());
            }
#endif // AI(W906-NODRY) 20260924
        if (pathReady) {
            W906_DoReadLastData(true, binSelLoaded, tempLoaded);   // AI(W906-RCHG) 20260925: 本段原本的內容（golden DoReadLastData 開機那一次）搬到 main() 前的 W906_DoReadLastData，開機與換配方共用；照 golden 順序重排
        }
    }
    { extern void W906_FRWBoot_ShowLotInfoDownloadFlag(); W906_FRWBoot_ShowLotInfoDownloadFlag(); }  /* AI(W906-FRW-S65) 20260926 (Steven 團隊)：golden TfMain::FormShow main.cpp:10041-10046（V912 HT9045_ref）if(IniConfig.bShowLotInfo){ bHasDownloadFile=false; WriteLastDataFile(); } —— 在 W906_DoReadLastData(true)（golden :9993／:9995）之後、:10594 之前；本體 FileRW/MainBoot.cpp，會寫 system\lastdata.dat */  { extern void W906_BootTestCategory(); W906_BootTestCategory(); }  { extern void W906_FRWBoot_ResetLotInfo(); W906_FRWBoot_ResetLotInfo(); }  /* AI(W906-FRW-WC1) 20260927 (Steven 團隊)：golden TfMain::FormShow main.cpp:10991 fLotInfo->ResetLotInfo()（在 :10041-10046 之後；GATE WC-1 退役）；本體 FileRW/MainBoot.cpp；接在同一行，不移動行號 */   // Steven 20260925 (Data.TestCategory)：golden TfMain::FormShow main.cpp:9882 InitCateCell ＋ :10594 DoShowUserDefFrom 的 fTestCategory 分支（本體 cTestCategory.cpp 檔尾）；要在 ReadLastSetIni／SetWorkParameter 之後
    fCounterClear->ReadCTInfo();  { extern void FileRW_VacuumUnit_Boot(); FileRW_VacuumUnit_Boot(); }  { extern void W906_RC_LaserReadLaserFile(); W906_RC_LaserReadLaserFile(); }  { extern void W906_FRWBoot_LotListsRead(); W906_FRWBoot_LotListsRead(); }  { extern void W906_InstallLotInfoClearListBody(); W906_InstallLotInfoClearListBody(); }   /* AI(W906-FRW-S94) 20260926（Steven 團隊）：前一個＝golden TfMain::FormShow main.cpp:11344-11365（V912 主 repo；在 :11191 ReadLaserFile 之後）開機讀回 D:\HT9045_Log\2DBarCode\LotData.txt → list2DByLot／map2DList、TrayIDByLot.txt → fTrayMapping->listTrayIDByLot（本體 FileRW/MainBoot.cpp，只讀）；後一個＝裝上 TfLotInfo::btClearBarcodeListClick 的本體（WebLotInfo.cpp；裝上後呼叫它的地方會清 2D 重複碼清單、把 LotData.txt 寫成空檔、再清 Barcode 計數）。接在同一行，不移動行號 */   /* AI(W906-FRW-NoPage) 20260926：golden TfMain::FormShow main.cpp:11189 fLaserSensor->ReadLaserFile()（開機；DoReadLastData :9993 之後，無條件）。本體 OmronLaser/LaserSensor.cpp:1474；USE_LASER_DISTANCE==0 第一行 return */   /* Steven 團隊 20260925 (S12-C TestIF_File_VacuumUnit)：golden TfMain::FormShow main.cpp:10933 fVacuumUnit->Initial()（elVacuumUnit 具名註冊；golden 開機／換配方都不呼叫 fVacuumUnit->ReadFile） */   // Steven 20260925 (Data.ContactCT)：golden TfMain::FormShow main.cpp:10596（DoReadLastData 之後、GetHotPlateYHalfPos 之前，無條件呼叫）。讀 system\Arm*.dat／ArmByLot*.dat／ArmHis*.dat 進 ArmData／ArmDataLot／ArmHistory（TArm::ReadFile cSocket.cpp:537）；沒讀的話 TfContactCT 的格子全是 0。WriteCTInfo（csystem.cpp:4090 W7C2_FCOUNTER_WRITECTINFO）仍是空巨集，這次不開
    ht9045::SetWebBinSelLoaded(binSelLoaded);
    ht9045::SetWebTempLoaded(tempLoaded);   // AI(W906-FW-TEMP2) 20260820
    { extern void W906_CreateLogObjects(); W906_CreateLogObjects(); }  { MyDBUpdateDB(); }  /* AI(W906-CMYDB-P3) 20260926: golden TfMain::FormShow main.cpp:10655 (after InitialHandler :9559 -- Cylinder[] -- and the ctor log objects) */  { extern void W906_ElaStart(); extern void W906_ElaPost(int); extern void (*W906_ElaPostHook)(int); W906_ElaStart(); W906_ElaPostHook = &W906_ElaPost; }  /* AI(W906-ELA-P3) 20260927: Event Log Analyzer hub (golden EventlogAnalyzer.exe at boot); boot ReadConfig WRITES missing keys into the real config.ini (#17 A) */   // AI(W906-CSVONLY-P1) 20260926: golden TfMain ctor main.cpp:1550-1724 log objects (LogObjects.cpp), after the boot DoReadLastData (IniConfig.bSPILFunction / Prod.iTrayType loaded) and outside if(pathReady); on the old blank line, so no line moves
    // AI(W906-WEB-W2b) 20260917 (裁決 A1.2)：**改成無條件**。RealRecipeDir() 是
    // gRealDataPath + GetLastOpenFN()，而 :457 才剛把 cbSetupFileName->Text 設成
    // GetLastOpenFN() —— 兩者建構上同值，所以這道檢查守的就是真實路徑的輸入。
    // 沒有作用中配方時真實路徑是一段垃圾字串，服務它沒有意義。
    const AnsiString recipeName = fMain->cbSetupFileName->Text;
    if (recipeName.IsEmpty() || recipeName == "Fail Open") {
        std::printf("web API: no active recipe; refusing to serve\n");
        return 2;
    }
    std::printf("recipe API (REAL, ruling A1): %s\n", RealRecipeDir().c_str());

    // --- 1b. arm the golden spine pump --------------------------------------
    //
    // AI(W906-ST-S3-B2) 20260918: until now NOTHING in wb_serve advanced the
    // machine state. `git grep MainProc|PumpTick|EachCycle|ScanSystemSensor` over
    // tools/wb_serve.cpp returned nothing -- the loop below published tags and
    // drained commands, and that was all. The consequence is specific, not
    // abstract: S3-B1 just un-gated the only three `SoftStart = true` in
    // StartFromWeb(), and the chain onwards is ckernel.cpp:816
    // `if(SoftStart==true)` -> ckernel.cpp:1015 `SystemStart=true`. That chain is
    // driven by MainProc(). No pump, no tick, no SystemStart -- a START button
    // that reaches the right code and still cannot change anything.
    //
    // ⚠ MUST COME AFTER LoadMachineConfig(). ReadGeneralIni overwrites five of   // AI(W906-INBOX114) 20260929: stale: the pins are retired (WebBridgeTags.cpp:569-583, INBOX 114) -- PumpInit now prints the machine shape it runs with; the order still matters for the canary and InitAllProcessTask
    // the globals PumpInit() pins (database.cpp:454/:593/:703/:1079/:1430); if
    // the load lands after the pin, the tick shape changes silently and
    // everything stays green. wb_publish.cpp:174 carries the same warning.
    //
    // ⚠ THE SIM CANARY CAN REFUSE, AND THAT IS THE POINT. PumpInit() refuses when
    // MOTION_CARD_TYPE==MotionCard_Contec AND LastSet.iRealDummy==REALLY, because
    // that combination makes the hardware interlock at ainarm9045.cpp:1891-1893
    // LIVE, and pumping would be the first thing ever to run it. On this laptop
    // the first term is already true (cmydef.h:104 MotionCard_Contec==1, and the
    // real system/Gerneral.ini:32 says MOTION_CARD_TYPE=1), so whether we pump at   // AI(W906-INBOX114) 20260929: stale: the laptop now carries the HT9050 Gerneral.ini, MOTION_CARD_TYPE=0 (line 40), so the canary does not trip
    // all depends on a runtime value. A refusal is printed, not swallowed -- the
    // 1203 control block below takes the same shape for the same reason.
    //
    // ⚠ IT DOES NOT FORCE SystemStart. wb_publish.cpp:186's banner still says
    // "this process forced SystemStart/fAllMotorHome"; that text is STALE --
    // WebBridgeTags.cpp's AI(W906-IdlePump) 20260817 note records that the user
    // rejected forcing it and that "the two lines are simply GONE". PumpInit()
    // now opens the machine IDLE, which is what a freshly started HT9045 is.
    {
        std::string whyNotPump;
        WdMark("startup: PumpInit()");                                          // AI(W906-WD) 20260923
        if (ht9045::PumpInit(whyNotPump)) {
            std::printf("spine pump: ARMED -- MainProc() runs on the %d ms server tick\n",
                        kServeTickMs);  { extern void W906_MotorAccessEngineHooks(); W906_MotorAccessEngineHooks(); }   //AI(W906-MT-E3c) 20260925: the engine's Motor Test / Teach hooks (csystem.h, VerifyMotorAction + MainProc pause, EastSun R8) in place before the first MainProc (WebMotorAccessLive.cpp). Same line, so no line below moves
            // ⚠ 500 ms is DELIBERATE here and must not be "tuned up" casually.
            //   Measured (docs/RULINGS_20260917.md B13): golden's driver is
            //   uruncontrol.cpp's TRunControl thread at timeBeginPeriod(1), so
            //   MainProc() runs at ~1 ms idle and ~2 ms per 3 ticks running.
            //   500 ms is not 16x slow, it is 500x slow. For "press START and
            //   watch SystemStart flip" that is fine (worst case half a second).
            //   For actually running a lot it is three orders of magnitude out,
            //   and the fix is not a smaller number -- it is a different
            //   execution model (golden pumps on the UI thread via Synchronize).
            std::printf("            tick is 500 ms vs golden's ~1 ms: fine for watching a flag\n"
                        "            flip, NOT for running a lot -- see RULINGS_20260917.md B13\n");
        } else {
            std::printf("spine pump: NOT armed (%s)\n"
                        "            machine state will not advance; SystemStart cannot change.\n",
                        whyNotPump.c_str());
        }
    }
    if (In_Shuttle_Auto_Latch == eInSHAutoLtc) { extern void W906_ShuttleMoveReadData(); W906_ShuttleMoveReadData(); }  { extern void FileRW_AOAOffset_Boot(); FileRW_AOAOffset_Boot(); }   /* AI(W906-FRW-AOA) 20260926（Steven 團隊）：golden TfMain::FormShow main.cpp:11679-11724＋:11736（iAOA_* → Ed_*Offset_X/Y，開機一次；要在 LoadMachineConfig 與 W906_DoReadLastData(true) 之後）；golden 在 :11740 ShuttleMove 之前，兩者資料無關 */   //Steven 團隊 20260926 (S12-C ShuttleMove)：golden TfMain::FormShow main.cpp:11740-11741（D:\HT9045 V912；在 :11768 SetTechDataToProd 之前）fShuttleMove->ReadData()：HandlerCondition.Data [Shuttle] → Prod.iInSH?SenICAddPos；S47 只有 latch 機台
    SetTechDataToProd();  // AI(W906-T4-C2) 20260924: golden TfMain::FormShow main.cpp:11327（`//JerryYang 20240604 : 開啟程式initial時沒有更新到Prod.IfError, 這邊多做一次`），在 InitialOK=true（golden :10464；這裡是 PumpInit）之後再推一次 —— SetWorkParameter 裡那一次跑在 InitialOK 之前，只會做到 _Tray 就 return（cinitial.cpp:15431）。放在這個空行是為了不移動本檔行號
    webbridge::TagSnapshot snap;
    // AI(W906-P4-SECS) 20260920: 掛上 SECS 的 SV 目錄發布者。
    //   它會在 PublishHandlerTags() 內部、commitPublish() **之前**被呼叫
    //   （接在外面會落在 commit 之後被清掉，實測過）。
    //   只有 wb_serve 掛 —— 把 WebBridgeTags 與 SECS 的相依切開，
    //   那些把 WebBridgeTags.cpp 當來源編的測試才不會被波及。
    // AI(W906-JSONBRIDGE-S0) 20260923: 改掛組合版（SECS SV ＋ 開機配置 def.*）。
    //   直接掛 PublishConfigTags 會把 SECS 的擠掉 —— 那個 hook 只有一格。
    ht9045::SetExtraTagPublisher(&PublishExtraTags);
    // ===== AI(W906-SJSON-S11) 20260923 BEGIN -- 裝上 Clarn_Data 的本體 =====
    //   ⚠⚠ 這一行讓 TfMain::Clarn_Data 從 no-op 變成 golden 的真本體
    //   （JsonBridge/actions/MainClarnData.cpp，V912 main.cpp:15458-15648）。
    //   影響的不只 act.main.clarnData：移植樹既有的 **20 個 live**
    //   fMain->Clarn_Data() 呼叫點（全部 28 個，8 個在 #if 0 內）一起變成
    //   真的會清計數，並呼叫 WriteLastDataFile()
    //   -> 寫 D:\HT9045\system\lastdata.dat
    //   （cprod.cpp:2022，硬編路徑，--dry 蓋不到；該檔不在版控裡）。
    //   ⚠ 本註解前兩版都錯過：第一版寫「22 個」且漏列 WebStart.cpp；
    //     第二版寫「28 個」但沒區分 live 與被閘住的。現行數字 20/28 的
    //     逐筆判定、量法、以及「為什麼不能用往回找最近 #if 0 的寫法」，
    //     全部寫在 forms/fMain.h 的安裝座註解。
    //   ⚠ 其中 WebStart.cpp 三筆（:1352 Tag11 LowYield／:2355 Tag1 TSMC／
    //     :2367 Tag1 JCET）全在 START 路徑上且都是 live ——
    //     按一次網頁 START 就可能寫檔。
    //   ⚠ 為什麼一定要有這一行（而不是讓那個 TU 自我登錄）：靜態 archive
    //     的成員只有在解析未定義符號時才被抽出，自我登錄的 TU 會連都不連
    //     進來而 build 依然全綠（CLAUDE.md 的陷阱 #2）。
    ht9045::sjson::InstallClarnDataBody();
    std::printf("Clarn_Data body installed = %s\n",
                ht9045::sjson::ClarnDataBodyInstalled() ? "yes" : "NO");
    // ===== AI(W906-SJSON-S11) 20260923 END =====
    const std::size_t staged = ht9045::PublishHandlerTags(snap);
    const ht9045::TagCoverage cov = ht9045::HandlerTagCoverage();
    std::printf("published %u tags, %u of %u carry a loaded value\n",
                (unsigned)staged, (unsigned)cov.live, (unsigned)cov.total);
    { extern void W906_FRWBoot_FTPDownloadDataSnapshot(); W906_FRWBoot_FTPDownloadDataSnapshot(); }  /* AI(W906-FTP-START) 20260928 [W906] (Steven 團隊 St01)：golden TfMain::FormShow main.cpp:11495-11513（V912 主 repo）FTP 下載資料比對的開機快照，緊接在 golden :11515 LoadAutoCleanData 之前；本體 FileRW/MainBoot.cpp 檔尾。沒有它，出貨版開 FTP＋On-Line 的每次 START 都被 NETDownloadDataCheck 擋。接在同一行，不移動行號 */  { extern void FileRW_Cleaning_MainFormShow(); FileRW_Cleaning_MainFormShow(); }  { extern void W906_FRWBoot_JamRawDataRecord(); W906_FRWBoot_JamRawDataRecord(); }  /* AI(W906-FRW-Boot) 20260926 (Steven 團隊)：golden TfMain::FormShow main.cpp:11576-11580（V912）if(IniConfig.bN26_UseJamRawDataRecord){ fObserver->ReadLoaderCount(); fObserver->StatisticalJamCount(); } —— 在 :11513-11526（本行前半）之後、:11591 LotSummary.ReadFile（下一行）之前；本體 FileRW/MainBoot.cpp（slEventLog 為 NULL 時不呼叫 StatisticalJamCount，見該檔）。接在同一行，不移動行號 */   //Steven 20260925 (S12-C TestIF_File_Cleaning)：golden TfMain::FormShow main.cpp:11513-11526（LoadAutoCleanData→SearchCleanNum→SetAutoCleanStringGrid→ReadWriteAutoCleanCount(false)→SetAutoCleanICCount(false)），InitialOK 之後
    { extern void W906_BootReadLotSummary(); W906_BootReadLotSummary(); }  // AI(W906-T4-U82) 20260924: golden FormShow main.cpp:11154 LotSummary.ReadFile()（本體在 cSocket.cpp 檔尾；純讀取）。放在空行上，不移動行號；刻意不用 :3212（分支 t4-c2-sim 用了）
#ifdef WB_PUMP_1203_CONTROL
    // AI(W906-BU-C4) 20260918: arm the PCIE-1203 write surface.
    //
    //   The mode is a BUILD decision, not a flag: MachineType.h's
    //   WB_PUMP_1203_CONTROL_LIVE decides LIVE vs dry run, per the user's
    //   20260918 ruling ("我不要任何參數且全部都要執行").
    //
    //   Enable() FAILS on a binary built without HAVE_PCI1203 -- which is this
    //   laptop. That is not an error path to hide: Pci1203Control() then stays
    //   null and every pci1203.* command is refused with the reason, which is a
    //   result a browser can SEE and check. A silent "ok" would be the bug.
    {
#ifdef WB_PUMP_1203_CONTROL_LIVE
        const bool ctlDry = false;
#else
        const bool ctlDry = true;
#endif
        std::string why1203;
        if (ht9045::Pci1203ControlEnable(ctlDry, why1203)) {
            std::printf("1203 control: ENABLED, mode=%s\n",
                        ctlDry ? "DRY RUN (validates and records, issues nothing)"
                               : "*** LIVE -- commands reach the card ***");
        } else {
            std::printf("1203 control: not armed (%s)\n"
                        "              pci1203.* commands will be refused with this reason.\n",
                        why1203.c_str());
        }
    } { extern void W906_InstallPci1203IoRoute(); W906_InstallPci1203IoRoute(); }  { extern void W906_InstallPci1203MotorRoute(); W906_InstallPci1203MotorRoute(); }   //AI(W906-INDEXZ-1203) 20260930: review round 2 C -- the Index Z1 Gali route's installer is NO LONGER called here: the 1203 monitor does not exist yet at this point (Pci1203MonitorEnable is in the INSTALL_1203_MONITOR block below), so the route's "card open" check always refused and it never installed. It is called once, on the "entering tick loop" line after that block   // AI(W906-IOWEB-P17) 20260925: A4-7 -- MyLaneIO's 1203 backend -> monitor samples (read) / Pci1203Control (write); EtherCAT/Pci1203IoRoute.cpp. On the block's closing line, so no line below moves   //AI(W906-ECAT-ROUTE) 20260929: + the ENGINE MOTOR route (EtherCAT/Pci1203MotorRoute.cpp, design 7.2): installs only with WB_ENGINE_MOTOR_1203 (MachineType.h EOF, OFF by default) + INSTALL_1203_MONITOR + WB_PUMP_1203_CONTROL + WB_PUMP_1203_START_RING (Q9), no SOFT_SIMULTE and an armed control; otherwise prints "NOT routed" with the reason. After InitialHandler (its boot InitMotor sees no route) and before the monitor opens (axes are claimed on first use)
#endif
    { extern void W906_InstallStateRecordBody(); W906_InstallStateRecordBody(); std::printf("State Record body installed = yes\n"); }  { extern void W906_BootRegisterTaskList(); W906_BootRegisterTaskList(); }  /* AI(W906-TASKLIST) 20260927: golden TfMain::FormShow main.cpp:9717-10036 task 紀錄環登錄（cStateRecord.cpp 檔尾）；同一行，不移動行號 */  // AI(W906-STATEREC) 20260924: 裝上 DoStateRecord 本體（cStateRecord.cpp，golden main.cpp:26340-26678）；之後既有的 fMain->DoStateRecord 呼叫點與 act.main.stateRecord 都會真的錄製（SOFT_SIMULTE 組態 golden 自己只做快照）。佔用原本的空行，不移動行號
    // --- 2. stand the bridge up ---------------------------------------------
    webbridge::WebBridgeConfig cfg;
    cfg.port         = port;
    cfg.documentRoot = root;
    cfg.maxConnections = 64;  { extern void W906_OpLogInit(int port); W906_OpLogInit((int)port); }   //AI(W906-OPLOG) 20260928: operation log (end of file; on only when W906_OPLOG_DIR is set)   [machine patch 0019; merged without 0018 TOKEN-OFF, Jimmy to decide]   //AI(W906-WSFANOUT) 20260926: default 16 was FULL on this machine (measured 23 TCP connections: every HMI window opens 1-2 WebSockets + the pages' HTTP polls), so /api/struct/* requests sat in the backlog forever and Motor Test froze. FD_SETSIZE 128 - 8 = 120 cap (WebBridgeServer.cpp:578). Same line, no line moves.   // bindAddress and readOnly keep their safe defaults on purpose.

    // AI(W906-Q30-8) 20260922: 警報信箱的目錄。**從 root 導出，不要各自寫死。**
    //   同事的 JSON-Simulator/README.md 與 JsonBridge.cs:41-43 寫死
    //   `D:\HT9045\JSON\` —— 那個目錄不存在（20260922 實測）。
    //   執行期根目錄是 `D:\HT9045\web`（sync_web.py 檔頭的 20260911 裁決）。
    //   寫錯目錄的症狀：檔案乖乖產生、瀏覽器什麼都沒看到。
    g_dialogMailboxDir = w906dlg::MailboxDir(root);
    {
        // 目錄不在就建（js 子目錄一起）——`web/JSON/js` 今天是存在的，
        // 但這支要能在乾淨的部署上跑起來，不要假設。
        ::CreateDirectoryA((root + "\\JSON").c_str(), NULL);  ::CreateDirectoryA(g_dialogMailboxDir.c_str(), NULL);   // AI(W906-MAILBOX-RT) 20260924: 信箱在 JSON\runtime —— 乾淨部署上 JSON 也可能不在，先建父目錄
        ::CreateDirectoryA((g_dialogMailboxDir + "\\js").c_str(), NULL);
        const DWORD a = ::GetFileAttributesA(g_dialogMailboxDir.c_str());
        if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY)) {
            std::printf("  ⚠ 警報信箱目錄不可用：%s —— 警報只會走 WebSocket\n",
                        g_dialogMailboxDir.c_str());
            g_dialogMailboxDir.clear();
        } else {
            std::printf("  警報信箱：%s（.json ＋ js/*.js 兩份都寫）\n",
                        g_dialogMailboxDir.c_str());
            // AI(W906-Q30-IDLE) 20260923: 開機時把上一個行程留下的待答請求重設成
            //   idle 種子（使用者裁決；Ifor 同日現場回報 MES0920 框關不掉）。
            //   理由與範圍見 wb_dialog_mailbox.h 的 MailboxResetStale() 檔頭。
            //   ⚠ 放在 server.Start() **之前**：瀏覽器連上來之前信箱就已經乾淨，
            //     不會有「頁面先讀到舊 pending、C++ 才清掉」的窗口。
            //   ⚠ 已是 idle 的通道完全不寫；最壞情況（三個通道都要重設且都撞到
            //     sharing violation）是 AtomicWrite 的 6 × 1 秒重試，只發生在開機、
            //     主迴圈還沒開始，不會卡住 PumpTick。
            const int nReset = w906dlg::MailboxResetStale(g_dialogMailboxDir);
            std::printf("  警報信箱：開機檢查完成，重設 %d 個通道\n", nReset);
        }
    }
    // AI(W906-Q30-IDLE) 20260923: seq 的起點改成開機當下的 Unix 毫秒數，
    //   讓這個行程發出的每一則 seq 都大於前一個行程發過的（理由見 g_dialogSeq 宣告處）。
    g_dialogSeq = w906dlg::UnixMillisNow();

    webbridge::WebBridgeServer server(cfg);
    server.SetSnapshot(&snap);

    // AI(W906-FW-W1) 20260819: the command channel. Queue is attached
    // unconditionally (harmless while read-only); the read-only gate is what
    // --allow-cmd actually opens. Dispatch happens on THIS thread's tick
    // below -- the single-process stand-in for "the UI thread drains on its
    // existing timer tick" (design doc section 1).
    webbridge::CommandQueue cmdQueue;
    server.SetCommandQueue(&cmdQueue);
    server.SetReadOnly(!allowCmd);  g_tcAllowCmd = allowCmd;   //AI(W906-GB-P3) 20260926: H2 -- hands allowCmd to H1b. AI 20260927: allowCmd is always true since ZEROARG (:3670; --allow-cmd only re-sets it), so the testercomm / ela POST 403 arms are ctest-only

    std::string err;
    // AI(W906-FW-C1WIRE) 20260911: the recipe route. Installed BEFORE Start()
    // because the socket thread reads the hook without a lock -- see
    // WebBridgeServer.h's contract. Read-only over HTTP; the write arrives as a
    // recipe.doc.put command, which the --allow-cmd gate already governs.
    // AI(W906-WEB-W2b) 20260917：WebBridgeServer::SetHttpRoute **只存一條路由**
    //（我們樹上 WebBridge/WebBridgeServer.cpp:1630-1633 是直接指派，不是加進表），
    // 所以先註冊 "/api/recipe" 再註冊 "/api/system" 會**無聲取代**前者，之後每個
    // /api/recipe/* 回 404 而且哪裡都不報錯。改成在共用的 "/api" 前綴上掛一條，
    // 由 ApiRoute() 分流 —— WebBridgeServer 本身不用動。
    g_scrubWebRoot = root;   // Steven 20260925：W906_JsonScrubRoute 讀檔用（與 cfg.documentRoot 同一個根目錄）
    server.SetHttpRoute("/", ApiRoute, 0);  W906_ApiCacheInit();   // Steven 20260925：前綴由 "/api" 改 "/"——ApiRoute 另外接 /JSON/*（清密碼值）；其餘非 /api 路徑照舊 return false 交回靜態檔  // AI(W906-IOWEB-P6) 20260924: fill the cache BEFORE server.Start() -- the monitor is not enabled yet (:3727), so this build reads no card  //AI(W906-MERGE-56bbf785) 20260926: machine P6 appended W906_ApiCacheInit() to this line, laptop changed the prefix "/api" -> "/" -- both kept (the cache still serves StructRoute's four /api/struct bodies; ApiRoute still returns false for every path that is neither /api/* nor a scrubbed /JSON/*.json|.js)

    WdMark("startup: server.Start()");                                          // AI(W906-WD) 20260923
    if (!server.Start(&err)) {
        std::printf("server failed to start: %s\n", err.c_str());
        return 1;
    }

    // AI(W906-FW-W5a) 20260819: from here on, every ShowMyMessage anywhere in
    // the linked machine code also reaches the browser as an info modal.
    g_modalServer = &server;
    W906_ShowMyMessage_Hook = &ForwardShowMyMessage;  { extern void W906_MsgBoxHostInstall(); W906_MsgBoxHostInstall(); }   // AI(W906-SMM) 20260925: MyMessageBox 家族照 golden 顯示＋等回答（檔尾）
    // AI(W906-FW-W5b) 20260819: and every ShowErrorMessage becomes a browser
    // query whose K answer flows back (pump above).
    g_pumpQueue = &cmdQueue;
    W906_ShowErrorMessage_Hook = &ForwardShowErrorMessage;  { extern void (*W906_ShowMotorErrorMessage_Hook)(const char*, int, int, const char*, const char*); extern void ForwardShowMotorErrorMessage(const char*, int, int, const char*, const char*); W906_ShowMotorErrorMessage_Hook = &ForwardShowMotorErrorMessage; }   //AI(W906-JAM-STOP) 20260930: golden ShowMotorErrorMessage's fNote->ShowModal() (note.cpp:1133) -- host at EOF; same line, no line moves
    // AI(W906-YESNO) 20260925: 從這裡起，每個 ShowMyMessageBox_YES_NO 都送到網頁、等操作員按是／否
    //   （使用者 20260925 裁決第 10 條）。在這一行之前（開機序列）呼叫的仍回 0，見 ForwardShowMyMessageBoxYesNo。
    W906_ShowMyMessageBoxYesNo_Hook = &ForwardShowMyMessageBoxYesNo;
    // AI(W906-P6b-C) 20260921: 從這裡起，`Command.cpp` 的 Bit4_HandlerDiagnostics
    // 由視窗狀態總表回答，而不是硬編 0。
    // ⚠ 今天這在本機仍然恆 false：`D:\HT9045\web\background.html` 的
    //   `ui.windows.put` 命中數 = 0 ⇒ 沒有人送總表 ⇒ `EverAnyFrame()` 恆 false
    //   ⇒ Q8-B 那一關就回關著。要它真的生效得先部署 Steven 那份新網頁。
    W906_DiagnosticsWindowOpen_Hook = &ForwardDiagnosticsWindowOpen;  { extern void W906_TesterCommInit(); W906_TesterCommInit(); }  { extern void W906_PageTableArm(int (*)(), void (*)(const char*), void (*)(const char*), void (*)(const char*), void (*)(const char*)); extern const char* W906_PageTableJson(); extern void W906_PageProgramSet(const char*, bool, const char*); extern void (*W906_FormProgramShowHook)(const char*, bool, const char*); extern const char* (*W906_UiPagesJsonHook)(); W906_FormProgramShowHook = &W906_PageProgramSet; W906_UiPagesJsonHook = &W906_PageTableJson;  { extern bool (*W906_PageStartAllowedHook)(const char*); extern bool W906_PageStartAllowed(const char*); W906_PageStartAllowedHook = &W906_PageStartAllowed;  { extern unsigned long (*W906_PageStartMarkHook)(); extern bool (*W906_PageStartRefusedSinceHook)(unsigned long, bool); extern unsigned long W906_PageStartMark(); extern bool W906_PageStartRefusedSince(unsigned long, bool); W906_PageStartMarkHook = &W906_PageStartMark; W906_PageStartRefusedSinceHook = &W906_PageStartRefusedSince; }   /*AI(W906-PAGETAB-R144) 20260929*/ }  { extern void W906_PageTableEdgeHookSet(void (*)(const char*, bool)); extern void W906_EditPageWindowClosed(const char*); extern void W906_EditPageWindowEdgesArm(); W906_EditPageWindowEdgesArm(); extern void W906_EvB10A_WindowEdge(const char*, bool); W906_PageTableEdgeHookSet([](const char* form, bool open) { W906_EvB10A_WindowEdge(form, open); if (!open) W906_EditPageWindowClosed(form); }); }  /* AI(W906-EVB10A) 20260929 [W906] 事件批次 B10 part a：邊緣先交給 FileRW/MainClick.cpp W906_EvB10A_WindowEdge（Yield／Setup／Offset／Contact／Configuration 關窗的 golden 程式，表 FileRW/WindowEdgeTails.h），再清 C 路「開過了」——順序要緊：各表單用 filerw::PageShownNow 判斷「這一次開過、FormClose 還沒跑」；同一個 lambda 內改，不移動行號 */  /* AI(W906-PAGETAB-Q51) 20260928 [W906] 步驟 3～5：(1) MainProc 的 SoftStart 閘（csystem.cpp:30425，SECS／HOME 的 START 沒有畫面就擋）；(2) 網頁列「開→關」邊緣 ⇒ C 路頁清掉「開過了」（FileRW/_EditPage.cpp 檔尾），下次開窗＝重新開頁、記 Enter（R108／R110） */  W906_PageTableArm([]() -> int { return g_modalServer ? g_modalServer->LiveWebSocketCount() : 0; }, [](const char* why) { if (g_webMain) { std::printf("[PAGETAB] PauseFromWeb(\"%s\")\n", why); g_webMain->PauseFromWeb(AnsiString(why)); } }, [](const char* why) { if (fHome != 0 && fHome->fShow) { std::printf("[PAGETAB] fHome->Close() -- %s\n", why); fHome->Close(); } }, [](const char* why) { extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool); std::string ack; const bool ok = W906_MotorAccessWire("{\"action\":\"stop\",\"source\":\"uMotorTest\",\"button\":\"btnStop\",\"kind\":\"control\",\"motors\":[]}", 0, ack, true); std::printf("[PAGETAB] motor.stop (%s) -> %s\n", why, ok ? "ok" : ack.c_str()); }, [](const char* why) { RecordProcess(AnsiString("MES16441 ") + why + " -- stopped by the page table (Steven Q-P1 / Jimmy E#36=C 20260928)"); ShowErrorMessage("MES16441", 0, MMSystem, false, AnsiString("PageTable--NoScreen")); }); }  /* AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表開機：程式開／關畫面的 hook、ui.pages tag 的 hook、WebSocket 數（規則 6／7 與「有沒有畫面」）、瀏覽器全關的正常 STOP（golden TfMain::Pause＝PauseFromWeb／fHome->Close／motor.stop＝btnStopClick），停下來之後出 golden 告警 MES16441（R143 改號，原 MES1690；ShowErrorMessage kcode 0＝通知，寫進告警信箱，畫面回來就彈出；Jimmy 20260928 E#36＝C）。本體 WebPageTable.cpp；接在同一行，不移動行號 */   //AI(W906-GB-P3) 20260926: H3 -- registers the GPIB / RS232Standard engines, creates THandlerTesterSide, installs the fMain tester seat (P2b); starts nothing

    std::printf("\n  http://127.0.0.1:%u/?src=ws     (live handler data)\n",
                (unsigned)server.BoundPort());
    std::printf("  serving %s\n", root.c_str());
    std::printf(allowCmd ? "  COMMANDS ENABLED (--allow-cmd; dispatch: sys.ping), loopback only\n"
                         : "  read-only, loopback only\n");
    std::printf("  most values will read \"---\": that is the truth, see WebBridgeTags.h\n");
    std::printf(seconds > 0 ? "  exiting after %d s\n\n" : "  Ctrl-C to stop\n\n", seconds);
    // AI(W906-F5Ready) 20260915: publish readiness through debugger stdout before the idle loop.
    std::fflush(stdout);

    //AI(W906-Q34-1) 20260923: 開卡（唯讀）。落點是 §2 之後、§3 之前，**不是**
    //  同事在 wb_publish.cpp:668 的相對位置。
    //
    //  為什麼位置是承重的：Open() 在**本執行緒**上同步阻塞，在有從站的 ring 上
    //  可達 ~10 秒。照抄他的相對位置（§1 那個 1203 控制面區塊旁邊）會落在
    //  `server.Start()` 之前 —— 那時還沒有人連得上，於是它內部設的每一個
    //  phase 都到不了瀏覽器，操作員看到的是十秒空白畫面，跟當機分不出來。
    //  放在這裡，橋已經起來、`server.SetSnapshot(&snap)` 已經掛上，所以下面
    //  「先講再阻塞」那三行是真的會到畫面上的。
    //
    //  ⚠ 這三行的順序照翻自他的原話「TELL THE PAGE FIRST, THEN BLOCK」，
    //  但多了一行 `server.Wake()`：wb_publish 有自己的 publisher，wb_serve 是
    //  `Wake()` 才把快照推給已連線的瀏覽器（見 §3 迴圈的 `PublishHandlerTags`
    //  ＋ `Wake()` 配對）。少了它，phase 只會停在 snap 裡沒人看得到。
#ifdef INSTALL_1203_MONITOR
    {
        std::printf("INSTALL_1203_MONITOR: opening the REAL motion card, "
                    "read-only (%d axis slots, %d DI ports, %d DO ports)...\n",
                    (int)ht9045::kPci1203TagAxes, (int)ht9045::kPci1203TagDiPorts,
                    (int)ht9045::kPci1203TagDoPorts);
        ht9045::Pci1203NoteOpening();
        ht9045::PublishHandlerTags(snap);
        server.Wake();
        std::fflush(stdout);

        //AI(W906-Q34-8) 20260923: ⚠ 這一行現在可能讓本執行緒（也就是之後跑 PumpTick、
        //  drain、ack 的同一條）多阻塞最多 WB_1203_OPEN_WAIT_SEC 秒（預設 90，環境變數
        //  上限 3600）—— 在有 SDK（HAVE_PCI1203）的機台上、Acm_DevOpen 回 0x83000002
        //  「從站還沒就緒」（剛開機）時，1203COLD-1 在 Open() 裡每秒重試一次
        //  （Pci1203Monitor.cpp 的 kEcSubDevicesNotReady 與開卡重試迴圈）。
        //  使用者 20260923 裁決第 3 條：開機與網頁「重新掃描」兩條路都照翻、都等，
        //  含 90 秒預設與環境變數。等待期間 Open() 會逐秒 printf 進度，但網頁看不到
        //  （上面那次 PublishHandlerTags + Wake 之後，要等 Open() 返回才會再發布）。
        //  ⓘ 上面 Q34-1 說的「~10 秒」與 Pci1203NoteOpening() 的 phase 文字
        //    「up to about 10 seconds」講的是暖開機；冷開機要再加上這段等待。
        //  ⓘ 這台筆電沒有 HAVE_PCI1203：Open() 走 `#if !HAVE_PCI1203` 分支立刻回
        //    "not linked"，重試迴圈根本沒編進來。
        std::string why;
        // AI(W906-Q34-7-L2) 20260923 夜間：COLD-1 在有 SDK 沒卡／從站未就緒時會在這裡等最多 90 秒，
        //   沒有這一行的話停擺看門狗會把它報成卡在上一個標記 "startup: server.Start()"（獨立審查 L2）。
        WdMark("startup: Pci1203MonitorEnable() (COLD-1 may wait up to 90 s)");
        if (ht9045::Pci1203MonitorEnable(ht9045::kPci1203TagAxes,
                                         ht9045::kPci1203TagDiPorts, why,
                                         ht9045::kPci1203TagDoPorts)) {
            const ht9045::Pci1203CardSample& c = ht9045::Pci1203Monitor()->card();
            std::printf("  card OPEN: dev=%lu \"%s\" subDevices=%d axesOpened=%d\n",
                        (unsigned long)c.devNum, c.devName.c_str(),
                        c.subDevices, c.axesOpened);
            std::printf("  mode=%s  slavesFound=%d  axByIdMode=%s\n",
                        ht9045::Pci1203ModeText(c.mode), c.slavesFound,
                        c.axByIdMode ? "yes" : "NO (physical-index fallback)");
            std::printf("  DI channels=%s  DO channels=%s  ring0=%s ring1=%s\n",
                        c.diMaxChanValid ? std::to_string(c.diMaxChan).c_str() : "---",
                        c.doMaxChanValid ? std::to_string(c.doMaxChan).c_str() : "---",
                        c.ringCountValid ? std::to_string(c.ring0Slaves).c_str() : "---",
                        c.ringCountValid ? std::to_string(c.ring1Slaves).c_str() : "---");
            //  照翻自 wb_publish.cpp:697-709：帶著 SubDevice ID 衝突開起來的卡，
            //  **不可以**印成單純的 "card OPEN" —— ID 還在撞的時候，讀數可能被
            //  歸到錯的站，而啟動日誌是第一個有人會看的地方。
            if (c.idConflict) {
                std::printf("  *** WARNING: opened WITH A SubDevice ID CONFLICT "
                            "(0x8300002B) ***\n");
                std::printf("      %s\n", why.c_str());
                std::printf("      Readings may be attributed to the WRONG STATION.\n");
                std::printf("      Fix: reassign the SubDevice IDs, then POWER-CYCLE\n"
                            "      the EtherCAT stations (a PC reboot does not do it).\n");
            }
        } else {
            //  ⚠ 刻意**不是**致命錯誤，而且 `Pci1203MonitorEnable()` 在 Open()
            //  失敗時仍然會設 g_monitor（`Pci1203MonitorEnable()` 失敗分支的 `g_monitor = m;`；
            //  原本寫的 `:3050-3055` 沒有包到那一行 —— 獨立審查 L1，20260923 夜間改成錨點）——
            //  tag 層要讀得到 lastErrorText 才能把**原因**顯示出來。null monitor
            //  會渲染成「這裡什麼都沒有」，那正是一個診斷畫面對一個它知道原因的
            //  失敗絕對不能說的話。在這裡 return 1 也會把另外 117 個 tag 一起帶走。
            std::printf("  card NOT opened: %s\n", why.c_str());
            std::printf("  continuing -- the pci1203.* tags will carry this reason\n");
        }
        std::fflush(stdout);  { extern void W906_OutputYieldHook(); if (ht9045::Pci1203Monitor()) ht9045::Pci1203Monitor()->SetYieldHook(&W906_OutputYieldHook); }   // AI(W906-IOWEB-P25) 20260925: Poll() lends this thread to queued OUTPUT commands between two stations / axes / DI bytes / DO bytes / SDO reads (EOF: W906_ServiceOutputs)
    }
#else
    //  說出來而不是留白：「畫面上沒有 pci1203 的值」與「監視器根本沒編進來」
    //  從瀏覽器看是同一件事，而 tag 層正是為了這個情況發布 pci1203.enabled=false。
    std::printf("INSTALL_1203_MONITOR is not defined -- the motion card is NOT "
                "opened.\n"
                "        every pci1203.* tag publishes null; "
                "#define it in MachineType.h to monitor the card\n");
    std::fflush(stdout);
#endif

    // --- 3. republish on a slow tick -----------------------------------------
    // The real handler will do this from its existing UI timer. Nothing here
    // reads machine state off the socket thread; the snapshot is the only seam.
    WdMark("startup: entering tick loop");  { extern void W906_InstallPci1203GaliRoute(); W906_InstallPci1203GaliRoute(); }  { extern void W906_PublishYieldHook(); ht9045::SetPublishYieldHook(&W906_PublishYieldHook); }   //AI(W906-INDEXZ-1203) 20260930: review round 2 C -- the Index Z1 (M14) Gali_* -> 1203 route + the torque hook (EtherCAT/Pci1203GaliRoute.cpp; MachineType.h EOF WB_ENGINE_INDEXZ_1203, OFF by default), installed HERE: after the INSTALL_1203_MONITOR block above (its "card open" / slot checks need the monitor that Pci1203MonitorEnable creates) and before the tick loop, in every build (the gate prints why when it does not install). ONE call site (tools/pci1203_control_gate.ps1 check 6 pins it after Pci1203MonitorEnable; ctest GaliRouteLive checks the order too). Same line, no line moves   // AI(W906-WD) 20260923  // AI(W906-LAT-1) 20260925: the tag publish lends this thread to queued OUTPUT commands too, between its blocks (WebBridgeTags.h SetPublishYieldHook; EOF: W906_PublishYieldHook). Installed here, after the two start-up publishes, whatever the monitor build: outputs exist only with WB_PUMP_1203_CONTROL, and without it W906_ServiceOutputs finds none
    const DWORD started = ::GetTickCount();
    std::vector<webbridge::WebCommand> drained;
    //AI(W906-Q34-7) 20260923: 截止時間式的雙時鐘迴圈，取代原本「開頭 Sleep(kServeTickMs)、
    //  每圈固定全做一次」的形狀。對應同事 wb_publish.cpp:758-790（1203CTL-21 雙時鐘與
    //  絕對截止時間）、:825-837（1203MON-6 的 Poll）、:935-988（1203FAST-2 可中斷睡眠）。
    //
    //  每一圈依序：
    //    (1) 睡到兩個時鐘較近的截止時間（上限 50 ms、下限 1 ms），佇列裡有命令就提早醒；
    //    (2) PumpTick 到期 → PumpTick() → Pci1203AxisIniTick()；
    //    (3) IO 時鐘到期 → Pci1203Monitor()->Poll()；
    //    (4) drain + dispatch —— 每圈都做，所以提早醒來的那一圈只做這一步；
    //    (5) (2)(3)(4) 有任何一步真的做了事，才發布快照並 Wake()；
    //    (6) `seconds` 到期就離開。
    //  順序照他的 pump → INI → Poll → drain → publish。
    //
    //  ⚠⚠ PumpTick 的節拍是 B13 裁決（kServeTickMs = 500），這個迴圈**不可以**讓它變快：
    //    * 提早醒來（有命令）只 drain，不跑 PumpTick —— (2) 看的是截止時間，不是「醒了」。
    //    * IO 時鐘（kIoTickMs = 200）只跑 Poll，不跑 PumpTick。
    //    * 截止時間照他的寫法（wb_publish.cpp:800-802）：`nextPump += kServeTickMs`，
    //      若已經又過了一整個週期就重設成 now + kServeTickMs（不連發補拍，:786-788）。
    //      所以 PumpTick 只落在「進迴圈時間 + k×500 ms」這張格子上，而且只在到期**之後**
    //      才跑 —— 長期頻率正好每秒 2 拍，不會更多。
    //    ⚠ 取捨，量過才選：某一拍被 Poll 拖晚之後，下一拍仍在原本的格子上，所以**單一**
    //      間隔可以短於 500 ms。另一種寫法 `nextPump = now + kServeTickMs`（以實際觸發時間
    //      為起點）保證每個間隔 ≥ 500，但會讓引擎整體變慢。20260923 用一支照抄本迴圈時序
    //      骨架的獨立小程式量（Poll 以 Sleep(140) 代替，140 是他量到的完整 Poll 成本，
    //      wb_publish.cpp:877；另一條執行緒隨機塞命令），各跑 30 秒：
    //        保持相位（本寫法）：60 拍，間隔 422–578 ms，平均 498.9 ms
    //        以觸發為起點      ：50 拍，間隔 578–625 ms，平均 600.4 ms
    //      後者被 200 ms 的 IO 時鐘鎖在 600 ms —— 正是他 1203CTL-21 第 (2) 條記的失敗：
    //      「A rate nobody asked to change, changed, as a side effect of a rate somebody
    //      did」（wb_publish.cpp:775-779）。B13 定的是頻率，所以照他的寫法。
    //      Poll 成本為 0 時（沒有卡的建置）同一支程式量到 60 拍、484–516 ms、平均 500.0。
    //    * 舊迴圈的週期是「Sleep 500 ms + 整圈本體耗時」（本體耗時沒有量過），新的是 500 ms
    //      的格子。也就是長期頻率比舊迴圈快了「本體耗時」那麼一點 —— 回到 B13 字面上的 500 ms。
    //    * 第一拍仍然在進迴圈 500 ms 之後，與舊的「先 Sleep 再 PumpTick」相同。
    //  ⓘ 睡眠放在迴圈**開頭**（舊迴圈的 Sleep 也在開頭），不是他放的結尾：dispatch 裡有
    //    `continue`（目前都在內層迴圈），放開頭就不必逐條證明沒有路徑會跳過睡眠而空轉。
    //  ⓘ 發布改成「有事才發」：他的迴圈每圈都發（睡眠上限 50 ms ⇒ 每秒可到 20 次），
    //    A 的舊迴圈是每 500 ms 一次。照抄會把 PublishHandlerTags 的次數放大到約十倍。
    //    改了之後：沒有卡的建置（這台筆電）仍是每拍一次、外加每批命令一次；
    //    握著卡的機台再多出每 200 ms 一次 —— 那正是「燈號 0.2 秒」這個要求本身。
    { extern void W906_NativeFormsStart(); W906_NativeFormsStart(); }  /* AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生 HW.IoSetView／Main.MotorView 唯讀視窗（-DW906_NATIVE_FORMS=ON 才有本體 ui/native/NativeFormsWbServe.cpp；OFF＝檔尾空函式）。主迴圈開始前、主迴圈這條執行緒建窗 —— 同 golden 單執行緒 UI、1203 單執行緒規則（EtherCAT/Pci1203Control.h:800-803）。同一行插入，其後行號不動 */  DWORD nextPump = started + static_cast<DWORD>(kServeTickMs);  { extern void W906_NativeFormsSetPumpClock(DWORD*); W906_NativeFormsSetPumpClock(&nextPump); }  /* AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生視窗被拖曳時的保活要共用這個截止時間（B13：500 ms 不能變快），見檔尾 W906_NativeKeepaliveMain */  WriteIniDataGeneral("Record", "Program Close", 0);  { extern void WriteBootLog(AnsiString); WriteBootLog("All CreateForm Done, before Application->Run"); }  /* AI(W906-BOOTLOG) 20260926: golden HT9045.cpp:285 -- the main loop below is Application->Run */  { extern BOOL WINAPI W906_ConsoleCtrl(DWORD); ::SetConsoleCtrlHandler(W906_ConsoleCtrl, TRUE); }  { extern void W906_SessionEndWatchStart(); W906_SessionEndWatchStart(); }  /* AI(W906-D012-A3W) 20260930 [W906]: hidden top-level window on its own thread for WM_QUERYENDSESSION / WM_ENDSESSION -- a console process that loads user32 does not get CTRL_LOGOFF_EVENT / CTRL_SHUTDOWN_EVENT (Microsoft SetConsoleCtrlHandler remarks). Logoff / shutdown = send the Q44 stop, no MES2109, no Program Close=1 (golden VCL 6 does not run FormClose then). Body FileRW/MainClose.cpp; same line, later line numbers unchanged */  // AI(W906-A4-4) 20260924: 使用者裁決 A4「按照舊版本作法」—— golden main.cpp:10398 開機做完寫 Program Close=0（「增加偵測不正常關閉程式時，需手動清料」），正常關閉（下面 server.Stop 之前，golden FormClose :11927）與 Ctrl-C／關視窗（W906_ConsoleCtrl，檔尾）寫 1。被直接砍掉（F5 停止鈕、taskkill）時不會寫 1 ⇒ 下次開機 LoadMachineRecord／U20 看到的是異常關機，與砍掉舊版 exe 相同
#ifdef INSTALL_1203_MONITOR
    DWORD nextIo   = started;  { extern void W906_NativeFormsSetIoClock(DWORD*); W906_NativeFormsSetIoClock(&nextIo); }  /* AI(W906-NATIVE-PROTO) 20260928 [W906]: 同上，1203 Poll 的截止時間（kIoTickMs）。同一行插入 */
#endif
    for (;;) {
        //AI(W906-Q34-7) 20260923: (1) 睡到較近的截止時間 —— 照翻 wb_publish.cpp:935-988。
        //  上限 50 ms 是他的（讓 `seconds` 到期能及時被看到）；下限 1 ms 讓本體超時的
        //  那一圈不會對著驅動空轉。
        //  1203FAST-2（他的原話「我點按鈕的反應速度很慢 就是我點下去可能過一段時間
        //  線圈才有做動」）：命令是在睡眠**期間**進來的，所以睡前檢查佇列沒用，睡眠本身
        //  必須可以被打斷 —— 切成 2 ms 一片，每片看一次 `cmdQueue.size()`，有東西就醒。
        //  `size()` 是 CommandQueue.h:22 明列「ANY THREAD」可呼叫的觀察函式，
        //  不動到 tryPush/drain 的執行緒契約。
        //  ⓘ 他只在 `controlArmed` 時切片（沒有命令可能進來就不必醒）。A 對應的條件是
        //    `allowCmd`，而它自 ZEROARG（20260918）起恆為 true、沒有任何參數會把它關掉，
        //    所以這裡直接切片，不留一個永遠走不到的 else。
        //  ⓘ 本程式沒有呼叫 timeBeginPeriod（git grep tools/ WebBridge/：只有註解提到），
        //    所以一片 Sleep(2) 實際多長取決於本行程的計時器解析度 —— 沒有量過。
        //  ⓘ 他 FAST-2 的另外兩處（TcpTagPublisher 的 select() 50→10 ms、wb_gateway
        //    轉送迴圈 50→10 ms）在 A **不適用**：A 沒有 wb_gateway 這一跳，瀏覽器直連本行程
        //    的 WebBridgeServer；它的 socket 執行緒 select() 集合裡有每個 client socket
        //    （進來的命令一到就醒），回 ack 走 CompleteCommand() → Wake() 的 self-pipe
        //    （WebBridgeServer.cpp:740-745、:1594），兩個方向都不是等 50 ms 的逾時。
        {
            DWORD next = nextPump;
#ifdef INSTALL_1203_MONITOR
            if ((long)(nextIo - nextPump) < 0) next = nextIo;
#endif
            const DWORD n0 = ::GetTickCount();
            DWORD wait = ((long)(next - n0) > 0) ? (next - n0) : 1u;
            if (wait > 50u) wait = 50u;
            const DWORD until = ::GetTickCount() + wait;
            for (;;) {
                if (cmdQueue.size() > 0 || !g_carry.empty() || g_outputsServed) break;   // AI(W906-IOWEB-P25) 20260925: a carried command is also work  // AI(W906-LAT-1) 20260925: + an output served INSIDE the last publish (its yield points): that snapshot may mix before/after-output state and does not show the read-back, so go round at once -- no sleep -- and publish again (publishNow sees g_outputsServed)
                const DWORD left = until - ::GetTickCount();
                if ((long)left <= 0) break;
                cmdQueue.waitForPush(left);   // AI(W906-IOWEB-P25) 20260925: was Sleep(2) slices: without timeBeginPeriod a slice is up to ~15.6 ms (the click log's 13-35 ms 其他). The queue now signals an event on every accepted push, so a click wakes this thread at once
            }
        }
        { extern void W906_NativeFormsPump(int); W906_NativeFormsPump(0); }  /* AI(W906-NATIVE-PROTO) 20260928 [W906]: 每一圈（≤50 ms，上面截止時間迴圈的上限）泵原生視窗訊息、IO 每 200 ms／馬達每 500 ms 灌一次（只讀）；site 0＝拖曳視窗時保活跑一拍主迴圈（檔尾 W906_NativeKeepaliveMain）。OFF＝空函式 */  { extern void W906_ServiceOutputs(int); W906_ServiceOutputs(0); }  { extern void W906_TesterCommTick(); W906_TesterCommTick(); }  { extern void W906_StateRecordDrainLog(); W906_StateRecordDrainLog(); }  { extern void W906_Timer2TestSecondsTick(); W906_Timer2TestSecondsTick(); }  /* AI(W906-J12) 20260926: golden Timer2Timer :21157-21205 test seconds, right after ProcessHVisionConnect like golden (forms/fMain_TestSeconds.cpp) */  /* AI(W906-GB-P3) 20260926: H4 -- bridge packets + TCP pump; every 1000 ms golden Timer2 ProcessHVisionConnect */  { extern int W906_MainCtlButtonTick(); W906_MainCtlButtonTick(); }  /* AI(W906-FLOW-4) 20260930: main-screen ONE CYCLE / TRAY FEED / ALARM RESET clicks St01 queued (act.main.ctlButton, FileRW/MainClick.cpp) -> golden OnClick bodies, once per click, every pass so a click runs before the next MainProc (WebMainCtlButtons.cpp; RESET and the site cell deliberately not taken, see there) */   // AI(W906-IOWEB-P25) 20260925: outputs that arrived during the sleep go out BEFORE PumpTick / Poll. On the old blank line, so no line below moves
        //AI(W906-Q34-7) 20260923: (2) 只看截止時間。提早醒來的那一圈 pumpBeat 是 false。
        const DWORD now = ::GetTickCount();
        const bool pumpBeat = (long)(now - nextPump) >= 0;
        if (pumpBeat) {
            nextPump += static_cast<DWORD>(kServeTickMs);
            if ((long)(now - nextPump) >= 0) nextPump = now + static_cast<DWORD>(kServeTickMs);
        }

        // AI(W906-ST-S3-B2) 20260918: the golden spine tick, BEFORE the drain and
        //   before the publish, in that order and for two different reasons.
        //   * Before the PUBLISH so the snapshot describes the state this tick
        //     just produced, not the previous one (wb_publish.cpp:233 does the
        //     same and says so).
        //   * Before the DRAIN because a command dispatched this tick should see
        //     a machine state that has already advanced, not one tick stale.
        //   PumpTick() is a no-op when PumpInit() refused, and it never throws --
        //   it catches everything and counts it (WebBridgeTags.cpp PumpTick).
        WdMark("tick");                                                         // AI(W906-WD) 20260923
        //AI(W906-Q34-7) 20260923: 只在 500 ms 的拍子上跑 —— 見迴圈前的說明。
        //  ⓘ rebase 到 bafb25e 之後：WdMark("tick") 放在 if 外面，**每一圈都執行**
        //    （它只是停擺看門狗的心跳；雙時鐘之後一圈最長 50 ms，比 500 ms 更密，不會誤報）。
        { extern void W906_IoTiming(int); if (pumpBeat) { W906_IoTiming(1); ht9045::PumpTick(); W906_IoTiming(2); extern void W906_ServiceOutputs(int); W906_ServiceOutputs(0); }  if (pumpBeat) g_apiCacheDirty = true; }   // AI(W906-IOWEB-P6) 20260924: rebuild on the tick thread (covers builds without INSTALL_1203_MONITOR and IO_Table reloads)  // AI(W906-IOWEB-P21) 20260925: W906_IoTiming marks the phase for the click-latency log (JsonBridge/IoBtnPanelClick.cpp)  // AI(W906-IOWEB-P25) 20260925: outputs right after PumpTick, before INI/Poll; the api cache is only MARKED here and rebuilt after the drain
        if (pumpBeat) { extern void W906_HmiKeeperTick(); W906_HmiKeeperTick(); }  /*AI(W906-HMI-KEEP) 20260929: reopen the HMI page when it is closed (end of file)*/  if (pumpBeat) { extern void W906_StateRecordTimer2Pump(); W906_StateRecordTimer2Pump(); }  if (pumpBeat) { extern void W906_MotorAccessTick(bool); extern bool W906_OwnerHeldSince(unsigned long long, unsigned long long&); static unsigned long long s_w906OwnerTick = 0; W906_MotorAccessTick(W906_OwnerHeldSince(server.ControlOwner(), s_w906OwnerTick)); }  // AI(W906-STATEREC) 20260924: golden Timer2Timer 的 State Record 段（main.cpp:21142-21153，本體 cStateRecord.cpp，內部限每秒一次）—— 推 StateRecordImage 的格 1～6，格 6 發延後警報 JAM0316/JAM0317（會像其他警報一樣等瀏覽器回答）。佔用原本的空行，不移動行號
#ifdef WB_PUMP_1203_CONTROL
        //AI(W906-Q34-5b) 20260923: 把 Pci1203Axis.ini 記下的卡片設定放回去（1203INI-1）
        //  —— 照翻同事 wb_publish.cpp:806-823。卡片斷電不保留軸設定，所以兩個
        //  HLMT Logic 值在卡片與 INI 不一致時由這裡寫回卡片（Acm_SetU32Property，
        //  卡片 RAM 屬性；不碰驅動器、沒有 SDO、沒有 1010h）。
        //  ⚠ 這是全包唯一「沒人按也會寫卡」的路徑，使用者 20260923 裁決照收。
        //    每拍最多寫 1 個參數；每個（軸, 參數）試 3 次就停並回報。
        //  ⓘ 他的 `if (controlArmed)` 在 A 是兩層：命令面只在 WB_PUMP_1203_CONTROL
        //    編進來時存在（這個 #ifdef），而 Pci1203ControlEnable() 失敗時
        //    g_control 是 null，Pci1203AxisIniTick() 一開頭就回 false。
        //  ⓘ 位置照他的順序 pump → INI → Poll → drain：PumpTick 之後、drain 之前。
        //    它比對的是監控器的 limitValid/limitVal，而那只有 Poll() 會填 ——
        //    Poll 由 Q34-7 接在這段之後、drain 之前（IO 時鐘）。
        //  ⓘ AI(W906-Q34-7-M1) 20260923 夜間：**每一圈都跑**，照同事的程式（wb_publish.cpp:819-823，
        //    在 pump 的 `if` 外面，只看 `controlArmed`）。原本 Q34-7 依他註解裡的「On the pump clock」
        //    （:810）改成只在 500 ms 那一拍跑 —— 獨立審查（M1）指出那句話的意思是「週期性、不是開卡時只跑一次」
        //    （同一段註解接著寫 "rather than once at open"），而他模組檔頭也寫
        //    "Called every tick rather than once at open, deliberately"（Pci1203Control.cpp:2871）。
        //    使用者裁決第 2 條是「照同事原樣收」，改拍子的證據不到 ≥90%，所以還原成他的程式。
        //    後果：斷電後開機重套 HLMT 極性（保護用設定）的時間回到他的量級（20 軸×2 參數約 2～6 秒，
        //    改拍子版約 20 秒）。沒有卡時 g_control 是 null，Pci1203AxisIniTick() 一開頭就回 false。
        {
            std::string iniNote;
            if (ht9045::Pci1203AxisIniTick(iniNote) && !iniNote.empty())
                std::printf("%s\n", iniNote.c_str());
        }
#endif

#ifdef INSTALL_1203_MONITOR
        //AI(W906-Q34-7) 20260923: (3) 輪詢 1203 監控器 —— 照翻同事 wb_publish.cpp:825-837
        //  （1203MON-6：在發布它的同一條執行緒上觀察卡片，讓 pci1203.* 與機台 tag 描述同一刻；
        //  1203CTL-21：改走 IO 時鐘，預設 200 ms）。截止時間的寫法照他的
        //  `nextIo += period`、已經過了就重設成 now + period（不連發補拍）。
        //  ⚠ 和 PumpTick、drain、ack 同一條執行緒。唯讀、每次有上限、連續
        //    kMaxConsecutiveFailures（Pci1203Monitor.h:1336，= 10）次全失敗就自己停用。
        //    他在自己的環上量到一次完整 Poll 約 140 ms（wb_publish.cpp:877），沒有 timeout
        //    參數 —— 那段時間這條執行緒不做別的，所以到期的 PumpTick 最多晚那麼多
        //    （只會晚、不會提前；下一拍仍在 500 ms 的格子上，見迴圈前的取捨說明）。
        //  ⓘ 沒開 HAVE_PCI1203 的建置（這台筆電）：Pci1203Monitor() **不是** null ——
        //    Enable() 在 Open() 回 "not linked" 時照樣把物件掛上去 —— 但 Open() 的
        //    `#if !HAVE_PCI1203` 分支只設 disabled、不設 opened，所以 Poll() 的第一行
        //    `if (!impl_ || !impl_->opened || impl_->disabled) return false;` 就返回：
        //    0 個廠商呼叫、不配置記憶體。每 200 ms 的成本就是那一個判斷。
        //  ⓘ 要不要為這次 Poll 多發布一次，看 card().open（「現在握著 device handle」）：
        //    沒握著卡時 Poll 什麼都沒更新；Poll 前後各看一次，涵蓋這一輪剛好放掉卡
        //    （附掛模式下量產那邊關卡 → detached）的轉換。
        bool ioPolled = false;
        if ((long)(now - nextIo) >= 0) {
            nextIo += static_cast<DWORD>(kIoTickMs);
            if ((long)(now - nextIo) >= 0) nextIo = now + static_cast<DWORD>(kIoTickMs);
            if (ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor()) {
                const bool wasOpen = mon->card().open;
                // AI(W906-Q34-7-L2) 20260923 夜間：Poll 沒有 timeout 參數（同事量到一次約 140 ms），
                //   卡住時要讓看門狗報成 "1203 Poll" 而不是 "tick"（否則看起來像 PumpTick／MainProc 卡住）。
                //   Poll 之後改回 "tick"，後面 drain／發布卡住時的報法與改動前相同。
                WdMark("1203 Poll");
                { extern void W906_IoTiming(int); W906_IoTiming(3); mon->Poll(); W906_IoTiming(4); }  { extern void W906_MotorAccessPollTick(bool); extern bool W906_OwnerHeldSince(unsigned long long, unsigned long long&); static unsigned long long s_w906OwnerPoll = 0; W906_MotorAccessPollTick(W906_OwnerHeldSince(server.ControlOwner(), s_w906OwnerPoll)); }   //AI(W906-MERGE-0929) 20260929: review TK-1: a takeover (owner X -> Y) counts as "operator gone" for X for one beat, see W906_OwnerHeldSince at EOF   // AI(W906-IOWEB-P21) 20260925: phase mark for the click-latency log  //AI(W906-MT-E2) 20260925: right after Poll -- a 1203 LoopMove sees its arrival on the fresh sample (golden leg times), the Motor Test copy is refreshed (WebMotorAccessLive.cpp). Same line, so no line below moves
                WdMark("tick");
                ioPolled = wasOpen || mon->card().open;  g_apiCacheDirty = true;   // AI(W906-IOWEB-P6) 20260924: right after Poll(), same thread  // AI(W906-IOWEB-P25) 20260925: was rebuilt HERE, before the drain, so every click queued during the Poll also waited 20-160 ms for four JSON strings; now marked, rebuilt after the drain
            }
        }
#endif

        // AI(W906-FW-W1) 20260819: drain + dispatch on the tick, ack via
        // CompleteCommand (ticket == WebCommand.id, see QueuePush). FW-W1's
        // dispatch table is deliberately just sys.ping -- proving the
        // browser->ws->queue->tick->ack round trip end to end; real commands
        // land per design doc section 6 (FW-W2+).
        //AI(W906-Q34-7) 20260923: (4) 每圈都 drain。提早醒來的那一圈 PumpTick 與 Poll 都還沒
        //  到期，所以只做這一步 —— 這就是 1203FAST-2 要的：命令不必等到下一拍。
        //  ⓘ 上面 ST-S3-B2 說的「drain 排在 PumpTick 之後，命令才不會看到晚一拍的狀態」
        //    在拍子上照舊成立（同一拍裡順序沒變）。提早 drain 的命令看到的是最近一拍
        //    PumpTick 之後的狀態；舊迴圈會讓它先等到下一拍跑完再看。
        drained.clear();
        W906_TakeCarry(drained);  cmdQueue.drain(drained);   // AI(W906-IOWEB-P25) 20260925: carried commands first -- arrival order is kept
        for (size_t i = 0; i < drained.size(); ++i) {
            WdMark2("dispatch: ", drained[i].cmd.c_str());                      // AI(W906-WD) 20260923
            const webbridge::WebCommand& wc = drained[i];  { extern bool W906_Q44CmdRefused(const std::string&, const std::string&, std::string*); std::string q44Why; if (W906_Q44CmdRefused(wc.cmd, (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(), &q44Why)) { server.CompleteCommand((unsigned long long)wc.id, false, q44Why); continue; } }  /*AI(W906-D012) 20260929 [W906] Q44 A2: while closing (after Exit's 2nd confirm, or console Ctrl-C / X) only close / stop / read-only commands run, the rest get "closing: ..." (allow-list FileRW/MainClose.cpp w906q44::CmdAllowedWhileClosing; golden FormClose runs with the UI blocked). Before the anti-double-click guard, so a refused one stamps nothing*/  W906CmdGuardScope cmdGuard(wc);  if (cmdGuard.busy()) { server.CompleteCommand((unsigned long long)wc.id, false, cmdGuard.why()); continue; }   // AI(W906-CMDGUARD) 20260926: S107-3 防連點 —— 同 cmd+tag+value 在上一條還在跑、或完成後 W 內（預設 400 ms；env W906_CMDGUARD_MS，0＝關）到達就回 "busy: ..."、不執行；白名單與判定式見 WebCmdGuard.cpp 檔頭。continue 只跳過本條的分派鏈（for 迴圈體在分派鏈之後沒有別的事；g_carry 在迴圈前就取完了）；cmdGuard 解構＝這一圈結束時蓋完成時間，被擋的不蓋。接在同一行，其後行號不動
            if (wc.cmd == "sys.ping") {
                server.CompleteCommand((unsigned long long)wc.id, true, std::string());
            } else if (wc.cmd == "cfg.resync") {
                // AI(W906-JSONBRIDGE-S0) 20260923: 重送開機配置。
                //   value = 對方手上的 cfg.ver（省略或 0 = 「我什麼都還沒有」）。
                //   回應走第三個參數，和 start.run / pause.run 同一個慣例（:3550）。
                //
                //   為什麼需要它：連上 WS 本來就會收到 hello + 完整 snapshot
                //   （TcpTagLink.cpp:260），所以正常情況用不到這條。它是給
                //   (a) C++ 先開、HTML 後開之後又斷線重連、(b) 磁碟上的組態被
                //   別的程式改過、(c) 瀏覽器自己發現 cfg.ver 落後 這三種情況的。
                //   冪等：同一個 since 問幾次答案都一樣。
                unsigned long long since = 0;
                // ⚠ TagValue 的存取子是**嚴格**的：asInt() 對非 Int
                //   一律回 fallback，不做型別轉換（WebBridge/TagValue.h 的
                //   Inspection 那節）。而 JSON 的 `1` 在指令通道上會變成 Double，
                //   所以只寫 isNumber()+asInt() 的話 since 永遠是 0 ——
                //   cfg.resync 於是永遠回「有變」並夾帶完整內容，
                //   unchanged 那條捷徑等於不存在。
                //   20260923 由 tools/webprobe/s1_eventlog_probe.py 抓到，不是看出來的。
                if (wc.hasValue) {
                    if (wc.value.isInt())
                        since = (unsigned long long)wc.value.asInt(0);
                    else if (wc.value.isDouble())
                        since = (unsigned long long)wc.value.asDouble(0.0);
                }
                server.CompleteCommand((unsigned long long)wc.id, true,
                                       ht9045::sjson::ConfigResyncJson(since));
            } else if (wc.cmd == "struct.put") {
                // AI(W906-JSONBRIDGE-S6) 20260923: JSON -> 結構。
                //   tag   = 綁定名（"testIF.file"）
                //   value = {"values":{…},"dryRun":bool}
                //
                //   ⚠ dryRun=false 目前一律被 ApplyJson 拒絕，理由在
                //     JsonBridge/StructApply.cpp 的檔頭：缺 Clamp* 與
                //     ini 鍵對照表。寧可誠實拒絕，也不要在沒有 golden
                //     存檔鉗制的情況下寫真實配方檔。
                //
                //   ⚠ 這條**不**在 modal 的豁免名單裡：它會改機台設定，
                //     跟 cfg.resync / log.event 是不同性質的東西。
                const std::string bname = wc.hasTag ? wc.tag : std::string();
                const ht9045::sjson::Binding* b =
                    ht9045::sjson::FindBinding(bname.c_str());
                if (b == 0) {
                    server.CompleteCommand((unsigned long long)wc.id, false,
                                           "unknown binding (tag must be a name "
                                           "from GET /api/struct)");
                } else {
                    const std::string payload = (wc.hasValue && wc.value.isString())
                                              ? wc.value.asString() : std::string();
                    // ⚠⚠ dryRun 一定要**真的解析 JSON**，不能用子字串掃描。
                    //   20260923 審查抓到：原本寫成「"dryRun" 後面 20 個字元內
                    //   出現字面 false 就關掉 dry」，於是
                    //       {"dryRun":true,"x":false}
                    //   ——payload 明講 true——會被當成 dryRun=false。
                    //   今天的後果只是「該預覽的變成被拒」（ApplyJson 無條件擋
                    //   下所有 dryRun=false），但一旦開放 persist，這就是
                    //   「操作員按預覽、系統寫進真實配方檔」，而且 payload 完全
                    //   合法，沒有任何人看得出來。
                    //
                    //   這是同一類缺陷今天的第三次（?install=1 的子字串比對、
                    //   res.find("\"applied\":true")、這個），**第一次落在寫入
                    //   路徑**。本檔其他**五**處 dryRun 全部用 cJSON
                    //   —— 四條指令（:3307、:3402、:3485、:3578）加共用 helper
                    //   PeekDryRun（:1319/:1322）。也就是說**既有慣例本來就是
                    //   對的，是這段新程式碼偏離了它**。現在對齊。
                    //   （行號量於補上本註解之後；先前寫的 :3251/:3346/:3429
                    //    正好差 21 行 —— 就是本註解自己插進去的行數。
                    //    教訓：註解裡的行號要在寫完註解後重量。）
                    bool dry = true;   // 預設 dryRun：沒寫、壞掉、型別不對，一律不寫
                    cJSON* proot = cJSON_Parse(payload.c_str());
                    if (proot) {
                        const cJSON* jdry =
                            cJSON_GetObjectItemCaseSensitive(proot, "dryRun");
                        // 只認真正的 JSON bool false。字串 "false" 不算 ——
                        // 關掉一個保護開關要明確，不能靠型別轉換巧合。
                        if (jdry && cJSON_IsBool(jdry) && cJSON_IsFalse(jdry))
                            dry = false;
                        cJSON_Delete(proot);
                    }
                    // payload 本身壞掉時不在這裡回報：ApplyJson 會用它自己的
                    // reason 字串講清楚是哪裡壞，兩邊各報一次會互相矛盾。
                    const std::string res =
                        ht9045::sjson::ApplyJson(*b, payload, dry);
                    const bool ok =
                        (res.find("\"applied\":true") != std::string::npos);
                    server.CompleteCommand((unsigned long long)wc.id, ok, res);
                }
            } else if (wc.cmd == "log.event") {
                // AI(W906-JSONBRIDGE-S1) 20260923: 瀏覽器的操作留痕。
                //   value = {"kind":"process|alarm|change","msg":"…",
                //            "debug":"…","alarmCode":"…","page":"…","ctrl":"…"}
                //
                //   每一筆同時進 ring 與 golden 的 RecordProcess 族。回應會帶
                //   `sinks` —— 瀏覽器有權知道它剛那筆實際去了哪，因為移植樹的
                //   三個入口分別落到 stdout／空 body／TU-local no-op，
                //   一律回「已記錄」會是假的。
                //
                //   ⚠ 這條**不是**給有對應 act.* 的動作用的。那些動作的 golden
                //     處理器自己就會叫 RecordProcess（btnClearCountClick 裡的
                //     MyDBIProductionData 就是），HTML 再送一筆會變成重複留痕。
                //     log.event 是給沒有 act.* 的純畫面操作用的（切頁、
                //     改欄位未存、登入登出）。
                const std::string payload = (wc.hasValue && wc.value.isString())
                                          ? wc.value.asString() : std::string();
                const std::string res = ht9045::sjson::HandleLogEvent(payload);
                // ok 由 payload 是否合法決定；壞的 payload 要讓瀏覽器看到
                // ack 失敗，不能因為「我們收下了」就回成功。
                // ⚠ 看的是 "accepted" 不是 "ok"：ack 外層自己有一個 ok，
                //   HandleLogEvent 再寫一個會變成重複鍵（20260923 審查抓到）。
                const bool ok = (res.find("\"accepted\":true") != std::string::npos);
                server.CompleteCommand((unsigned long long)wc.id, ok, res);
            // ===== AI(W906-SJSON-S11) 20260923 BEGIN -- 動作通道 act.* =====
            //   value = {"…args…","dryRun":bool}，回應由 JsonBridge/ChanAction.cpp
            //   組出來。本分支**不含任何演算法**（派工單第 2 條）。
            //
            //   ⚠ 成敗看 `executed` 不看 `ok` —— AckJson 外層自己有一個 ok，
            //     再寫一個會變成重複鍵（S0/S1 各踩過一次，見上面 log.event）。
            //
            //   ⚠ 這條**不在** modal 的豁免名單裡：它會動機台資料並寫
            //     lastdata.dat，跟 cfg.resync 那種純查詢不同性質。
            //
            //   ⚠ 不阻塞：ChanAction 的每一條路都是純計算＋記憶體操作＋檔案寫入，
            //     沒有等對話框、沒有 Sleep 迴圈。要確認框的動作由瀏覽器先問完
            //     再送，不在這裡等（派工單第 3 條）。
            } else if (wc.cmd == "smartdiag.op" || wc.cmd == "builder.op") {
                // Steven 20260925 (Data.SmartDiagnostic／Data.Builder)：golden TfSmartDiagnostic／TfBuilder。本體在 WebSmartDiag.cpp／
                //   WebBuilder.cpp（只在 wb_serve），自己持 FormLock（可重入）；value＝JSON 字串。needConfirm 也算 ok。
                //   Builder 建立／刪除配方 Steven 20260925 同意開放（golden 的「使用中不能刪」與移植樹守衛 P1-P5 在本體）。
                extern std::string W906_SmartDiagOp(const std::string& payloadJson, bool* ok);
                extern std::string W906_BuilderOp(const std::string& payloadJson, bool* ok);
                const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
                bool opOk = false;
                std::string opRes;
                try { opRes = (wc.cmd == "smartdiag.op") ? W906_SmartDiagOp(payload, &opOk) : W906_BuilderOp(payload, &opOk); }
                catch (...) { opOk = false; opRes = std::string("exception in golden ") + (wc.cmd == "smartdiag.op" ? "TfSmartDiagnostic" : "TfBuilder"); }
                std::printf("%s -> %s\n", wc.cmd.c_str(), opOk ? "ok" : opRes.substr(0, 160).c_str());
                server.CompleteCommand((unsigned long long)wc.id, opOk, opRes);
            } else if (wc.cmd == "act.trayEdit") { extern std::string W906_TrayEditCmd(const std::string& payloadJson, bool* ok); const std::string tePayload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool teOk = false; std::string teRes; try { teRes = W906_TrayEditCmd(tePayload, &teOk); } catch (...) { teOk = false; teRes = "exception in golden TTrayEditForm"; } std::printf("act.trayEdit -> %s\n", teOk ? "ok" : teRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, teOk, teRes); } else if (wc.cmd == "act.sortCT.clearCount") {   // AI(W906-S10) 20260929 (St02-E, claim): golden uTrayEditForm -> WebTrayEdit.cpp (wb_serve only)
                // Steven 20260925 (Data.SortCT)：golden TfSortCT::btnClearCountClick（cSortCT.cpp:585-708；兩段式確認
                //   {"confirmed":false|true}）。本體在 WebSortCT.cpp（只在 wb_serve），自己持 FormLock。不放進
                //   JsonBridge/ChanAction.cpp：那支也編進 tests/ 的 target，連結不到 WebSortCT.cpp。
                extern std::string W906_SortCTClearCount(const std::string& payloadJson, bool* ok);
                const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
                bool scOk = false;
                std::string scRes;
                try { scRes = W906_SortCTClearCount(payload, &scOk); }
                catch (...) { scOk = false; scRes = "exception in golden TfSortCT::btnClearCountClick"; }
                std::printf("act.sortCT.clearCount -> %s\n", scOk ? "ok" : scRes.substr(0, 160).c_str());
                server.CompleteCommand((unsigned long long)wc.id, scOk, scRes); } else if (wc.cmd == "act.main.peModel") { /*AI(W906-FRW-P8) 20260926: golden TfMain::sbPEModelClick（V912 main.cpp:33724）；本體 FileRW/MainClick.cpp（自己持 FormLock）；value={"op":"get"|"click"}；接在同一行，不移動行號*/ extern std::string W906_Main_PEModelOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool peOk = false; std::string peRes; try { peRes = W906_Main_PEModelOp(payload, &peOk); } catch (...) { peOk = false; peRes = "exception in golden TfMain::sbPEModelClick"; } server.CompleteCommand((unsigned long long)wc.id, peOk, peRes); } else if (wc.cmd == "act.main.autoSkip" || wc.cmd == "act.main.setToDefineValue") { /*AI(W906-FRW-S100) 20260926: golden TfMain::cb_MainAutoSkipSwitchClick（V912 main.cpp:31841）／SetToDefineValue1Click（:26672）；本體 FileRW/MainClick.cpp（自己持 FormLock）；value 見該檔；接在同一行，不移動行號*/ extern std::string W906_Main_AutoSkipSwitchOp(const std::string& payloadJson, bool* ok); extern std::string W906_Main_SetToDefineValueOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool mcOk = false; std::string mcRes; try { mcRes = (wc.cmd == "act.main.autoSkip") ? W906_Main_AutoSkipSwitchOp(payload, &mcOk) : W906_Main_SetToDefineValueOp(payload, &mcOk); } catch (...) { mcOk = false; mcRes = "exception in golden TfMain main-screen handler"; } std::printf("%s -> %s\n", wc.cmd.c_str(), mcOk ? "ok" : mcRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, mcOk, mcRes); } else if (wc.cmd == "act.main.runMode" || wc.cmd == "act.main.tempMode" || wc.cmd == "act.main.light" || wc.cmd == "act.main.fan" || wc.cmd == "act.main.funcView" || wc.cmd == "act.main.indexTorque" || wc.cmd == "act.main.hp2View" || wc.cmd == "act.main.checkEncoder" || wc.cmd == "act.main.sg2DblClick" || wc.cmd == "act.main.siteClick" || wc.cmd == "act.main.ctlButton" || wc.cmd == "act.main.cleanOut" || wc.cmd == "act.main.ftrt") { /*AI(W906-B8-M11) 20260930 [W906]: + act.main.ftrt＝主畫面 FT／RT 小方塊 golden palFTClick／palRTClick（V912 main.cpp:30700／:30705 → DoFTRTClick :35752），本體 FileRW/MainClick.cpp 檔尾 W906_Main_FtRtOp；同一行附加*/ /*AI(W906-EVB6) 20260928 [W906]: 批次 B6＋B9（Steven 20260928 事件派工；RULINGS_20260926 S167／S169、Q47／Q48）—— golden TfMain 主畫面圖示／按鈕／子頁事件：runMode＝imgRunModeClick（V912 main.cpp:29796）、tempMode＝Panel42Click（:22392）、light＝spbLightClick（:26780）、fan＝spbFanClick（:26763）、funcView＝btnViewClick（:8689）、indexTorque＝btnSetZ1/Z2／btnReadZ1/Z2Click（:22635-22661）、hp2View＝palHP2ViewClick（:31875）、checkEncoder＝cbCheckEncoderEveryTimeClick（:29548）、sg2DblClick＝StringGrid2DblClick（:25128）、siteClick＝mtDutOnOffMouseUp（:29932，S169 只登記事件）、ctlButton＝RESET／ONE CYCLE／TRAY FEED／ALARM RESET（S169 只登記事件）、cleanOut＝BtnCleanOutClick（:4395，已翻 cCleanOut.cpp:64）；本體 FileRW/MainClick.cpp 檔尾 W906_Main_EvB6Op（自己持 FormLock）；value＝JSON 字串，見該檔；權杖照常要；防連點＝本迴圈頭的 WebCmdGuard（不進白名單）；接在同一行，不移動行號*/ extern std::string W906_Main_EvB6Op(const std::string& cmd, const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool b6Ok = false; std::string b6Res; try { b6Res = W906_Main_EvB6Op(wc.cmd, payload, &b6Ok); } catch (...) { b6Ok = false; b6Res = "exception in golden TfMain main-screen handler (B6/B9)"; } std::printf("%s -> %s\n", wc.cmd.c_str(), b6Ok ? "ok" : b6Res.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, b6Ok, b6Res); } else if (wc.cmd == "ttlcfg.op") { /*AI(W906-FRW-S101) 20260926: golden TfDIOFrom::spbDeleteClick（V912 DIOInterFaceCFG.cpp:249）；本體 FileRW/TTLCfg.cpp FileRW_TTLCfg_DeleteOp（呼叫端持 FormLock，同 editlist.*）；value={"op":"list"}｜{"op":"delete","file":"<名>.ini"}*/ extern std::string FileRW_TTLCfg_DeleteOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool tcOk = false; std::string tcRes; ht9045::formjson::FormLock(); try { tcRes = FileRW_TTLCfg_DeleteOp(payload, &tcOk); } catch (...) { tcOk = false; tcRes = "exception in golden TfDIOFrom::spbDeleteClick"; } ht9045::formjson::FormUnlock(); std::printf("ttlcfg.op -> %s\n", tcOk ? "ok" : tcRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, tcOk, tcRes); } else if (wc.cmd == "act.showBinSelect.clearCount") { /*AI(W906-PROD-S114) 20260926（Steven 團隊）：golden TfShowBinSelect::btnClearCountClick（V912 cShowBinSelect.cpp:2411-2426；兩段式確認 value={"confirmed":false|true}）；本體 WebShowBinSelect.cpp（只在 wb_serve，自己持 FormLock）；不加進 WebCmdGuard 白名單；接在同一行，不移動行號*/ extern std::string W906_ShowBinSelectClearCount(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool sbOk = false; std::string sbRes; try { sbRes = W906_ShowBinSelectClearCount(payload, &sbOk); } catch (...) { sbOk = false; sbRes = "exception in golden TfShowBinSelect::btnClearCountClick"; } std::printf("act.showBinSelect.clearCount -> %s\n", sbOk ? "ok" : sbRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, sbOk, sbRes); } else if (wc.cmd == "act.main.closeProgram") { /*AI(W906-PROD-S95) 20260926（Steven 團隊）：主畫面 Exit 鈕＝golden TfMain::sbCloseProgramClick（V912 main.cpp:29051-29131）→ Close() 進 FormClose 的生產資料段（:11861／:11919／:12195）；value={"step":0|1|2}（golden 兩個確認框拆三步）；本體 FileRW/MainClose.cpp（自己持 FormLock）；存完不關站（wb_serve 沒有正常關站的入口）；不加進 WebCmdGuard 白名單；接在同一行，不移動行號*/ extern std::string W906_Main_CloseProgramOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool cpOk = false; std::string cpRes; try { cpRes = W906_Main_CloseProgramOp(payload, &cpOk); } catch (...) { cpOk = false; cpRes = "exception in golden TfMain::sbCloseProgramClick"; } std::printf("act.main.closeProgram -> %s\n", cpOk ? "ok" : cpRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, cpOk, cpRes); } else if (wc.cmd == "form.event") { /*AI(W906-FRW-S157) 20260927 [W906]: WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157；格式 FROM_STEVEN 20260927 10:15）—— golden 表單控制項事件（HotPlate cbSelectHPFromDB、TrayForm cbTrayType1..3、Cleaning cbbSelectTray 的 OnChange）；tag＝頁名（Setup.HotPlate），value＝{"form","control","event","itemIndex","text","checked","state"}；本體 FileRW/_FormEvent.cpp W906_FormEvent（自己持 FormLock；unknown-page／running／bad-payload…都在那裡判）；防連點＝本迴圈頭的 WebCmdGuard（不加白名單）；權杖同 form.save；接在同一行，不移動行號*/ extern bool W906_FormEvent(const std::string& tag, const std::string& valueJson, std::string* ack, std::string* err); std::string feAck, feErr; bool feOk = false; if (!wc.hasTag) feErr = "unknown-page: form.event needs tag=<page>"; else if (!wc.hasValue || !wc.value.isString()) feErr = "bad-payload: form.event needs value=<json string>"; else { try { feOk = W906_FormEvent(wc.tag, wc.value.asString(), &feAck, &feErr); } catch (...) { feOk = false; feErr = "handler-failed: exception in form.event"; } } std::printf("form.event %s -> %s\n", wc.tag.c_str(), feOk ? feAck.substr(0, 160).c_str() : feErr.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, feOk, feOk ? feAck : feErr);
            } else if (wc.cmd == "recipe.change") {   // AI(W906-RCHG) 20260925（Steven 團隊）：主畫面 cbSetupFileName（golden cbSetupFileNameDropDown／cbSetupFileNameChange／ChangeSetUpFile）；value={"op":"list"}｜{"op":"change","name":"<配方>"}。本體 WebRecipeChange.cpp（自己持 FormLock；讀檔鏈＝本檔 W906_DoReadLastData）
                extern std::string W906_RecipeChangeOp(const std::string& payloadJson, bool* ok);
                const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
                bool rcOk = false; std::string rcRes;
                try { rcRes = W906_RecipeChangeOp(payload, &rcOk); } catch (...) { rcOk = false; rcRes = "exception in golden TfMain::cbSetupFileNameChange"; }
                server.CompleteCommand((unsigned long long)wc.id, rcOk, rcRes);
            } else if (wc.cmd == "lotinfo.op") { extern std::string W906_LotInfoOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool liOk = false; std::string liRes; try { liRes = W906_LotInfoOp(payload, &liOk); } catch (...) { liOk = false; liRes = "exception in golden TfLotInfo"; } std::printf("lotinfo.op -> %s\n", liOk ? "ok" : liRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, liOk, liRes); } else if (ht9045::sjson::IsActionCommand(wc.cmd)) {   //Steven 20260925 (Data.LotInfo 其餘分頁)：前面那一臂 lotinfo.op ＝ golden TfLotInfo（WebLotInfo.cpp，只在 wb_serve，自己持 FormLock）；barcode.clearCount／testerLog.get／selection.get／selection.save。寫在同一行，不移動本檔行號
                const std::string payload = (wc.hasValue && wc.value.isString())
                                          ? wc.value.asString() : std::string();
                const std::string res = ht9045::sjson::HandleActionWithTag(
                    wc.cmd, payload, wc.hasTag ? wc.tag : std::string());
                const bool ok = (res.find("\"executed\":true") != std::string::npos);
                server.CompleteCommand((unsigned long long)wc.id, ok, res);
            // ===== AI(W906-SJSON-S11) 20260923 END =====
            } else if (wc.cmd == "main.runStartMode") { extern void W906_RunStartModeCommand(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_RunStartModeCommand(server, wc);   /* AI(W906-RSMODE) 20260927: 主畫面「起動模式」下拉＝golden cbRunStartModeChange（本體檔尾）；同一行，不移動行號 */ } else if (wc.cmd == "sys.echoModal") {
                // AI(W906-FW-W5a) 20260819: probe surface for the modal path,
                // sys.ping's sibling -- drives the REAL ShowMyMessage (capture
                // seam included), whose hook then broadcasts the modal frame.
                // Text rides `value` (free-form JSON string), not `tag` (the
                // server's tag-name charset rejects spaces).
                // AI(W906-SMM) 20260925: 分派改到檔尾 W906_EchoModal —— tag 選 golden 的哪一支
                //   （pause／ok＝ShowMyMessage、yesno＝ShowMyMessageBox_YES_NO、nonstop／modeless＝ShowUnloaderTrayMessage），
                //   value 純文字＝S1 或 {"s1","s2"}。阻塞的會等網頁回答完才回 ack（golden ShowModal）。
                { extern void W906_EchoModal(const webbridge::WebCommand&, webbridge::WebBridgeServer&); W906_EchoModal(wc, server); }
            } else if (wc.cmd == "sim.di.set") { extern bool W906_SimDiCommand(const webbridge::WebCommand&); W906_SimDiCommand(wc);   // AI(W906-SMM-IO) 20260925: 模擬 DI（只有 SOFT_SIMULTE；檔尾）
            } else if (wc.cmd == "sys.echoErrorModal") {
                // AI(W906-FW-W5b) 20260819: probe surface for the ANSWER path.
                // tag = alarm code (sane charset), value = golden K button
                // mask (default RETRY|SKIP). This call BLOCKS in the pump
                // until a browser answers -- that is the point.
                AnsiString qcode = wc.hasTag ? AnsiString(wc.tag.c_str()) : AnsiString("WAR0000");
                int qmask = (wc.hasValue && wc.value.isNumber())
                            ? (int)wc.value.asInt(K_RETRY | K_SKIP) : (K_RETRY | K_SKIP);
                const int k = ShowErrorMessage(qcode, qmask, 0, false, AnsiString(""));
                std::printf("sys.echoErrorModal: ShowErrorMessage returned K=%d\n", k);
                server.CompleteCommand((unsigned long long)wc.id, true, std::string());
            } else if (wc.cmd == "sys.echoYesNo") {
                // AI(W906-YESNO) 20260925: sys.echoErrorModal 的是／否版探針 —— 走**真的**
                //   ShowMyMessageBox_YES_NO（含 hook、含 golden 的停機），讓手動驗證不必先湊出
                //   「確定要儲存測距數值？」那種機台狀態。value = S1（題目），S2 固定帶一個 ';'
                //   好讓 lblSubMsg 那一格也被畫到。本指令會**阻塞到有人按是／否**，那正是要驗的。
                //   ⚠ 會照 golden StopAllMotor、SystemStart=false —— 機台在跑時不要按。
                AnsiString q = (wc.hasValue && wc.value.isString())
                               ? AnsiString(wc.value.asString().c_str()) : AnsiString("Probe: YES or NO?");
                const int v = ShowMyMessageBox_YES_NO(q, AnsiString("探針：請按是或否;sys.echoYesNo"), AnsiString(""));
                std::printf("sys.echoYesNo: ShowMyMessageBox_YES_NO returned %d\n", v);
                char vbuf[32];
                std::snprintf(vbuf, sizeof(vbuf), "{\"value\":%d}", v);
                server.CompleteCommand((unsigned long long)wc.id, true, vbuf);
            } else if (wc.cmd == "modal.answer" || wc.cmd == "dialog.response") {
                // AI(W906-FW-W5b) 20260819: an answer with no query pending --
                // the pump consumes matching answers itself, so reaching the
                // normal dispatch means nobody is asking.
                //
                // AI(W906-Q30-8) 20260922: `dialog.response` 走同一條。
                //   ⚠ 這裡是**正常路徑**不是錯誤：操作員按 F5 之後
                //     dialog-bridge 的 lastSeq 歸 0，一個還沒退役的 request
                //     會再彈一次；他再按一次，答案就會落到這裡。
                //     回 "no query pending" 是誠實的 —— 前一次已經答掉了。
                //   ⚠ 但如果這行**常常**出現，代表 DialogMailboxRetire() 沒生效，
                //     那是真缺陷（症狀：答完按 F5 同一個警報又跳出來）。
                { extern bool W906_MsgBoxModelessAnswer(const webbridge::WebCommand&); if (!W906_MsgBoxModelessAnswer(wc)) server.CompleteCommand((unsigned long long)wc.id, false, "no query pending"); }   // AI(W906-SMM) 20260925: 非阻塞 MyMessageBox（ShowUnloaderTrayMessage）的關框回答在這裡收（檔尾），其餘照舊
            } else if (wc.cmd == "dialog.notifyAck") { extern void W906_NoticeAckCommand(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_NoticeAckCommand(server, wc);   /*AI(W906-J5-ACK) 20260930: INBOX 119 -- the operator acknowledges a kCode==0 notice (body at EOF; never waits, never posts a query). Same line, so no line below moves*/ } else if (wc.cmd == "dialog.auth") {
                // AI(W906-Q30-8) 20260922: 密碼驗證。
                //   契約 dialogAuth.verifier 明文「C++ only. HTML never compares
                //   passwords or access levels.」⇒ 這條**必須**由 C++ 回答。
                //
                //   ⛔ 但 `fSecurity->GetJamLevel` / `TfNote::DoPassword` /
                //     `DoUnlockPassword` / `CheckEmployeeID` 在本樹都還沒移植
                //     （契約 dialogAuth.kinds 列的四種來源，一種都沒有）。
                //   ⇒ 現在**誠實拒絕**，不要假裝驗過。
                //     假裝通過 = 把密碼保護整個拿掉；假裝失敗 = 操作員被鎖在外面
                //     而且看不出原因。兩個都比明講「還沒接」糟。
                //   ⇒ 網頁那側會在 15 秒後顯示 'C++ auth result timeout'
                //     （dialog-bridge.js:246-261），Alert 仍開著可重試。
                server.CompleteCommand((unsigned long long)wc.id, false,
                                       "dialog auth not wired: fSecurity/DoPassword "
                                       "not ported yet (contract dialogAuth.kinds)");
            } else if (wc.cmd == "system.file.put") {
                // AI(W906-FW-SYSFILE) 20260915: write one of the four system\ files.
                //   tag   = gerneral | teach | motTable | ioTable
                //   value = JSON string, the SAME shape the matching GET returns:
                //           ini -> {"sections":{sec:{key:{"raw":"..."}}}, "dryRun":bool}
                //           csv -> {"rows":{rowKey:{col:"raw"}}, "dryRun":bool}
                //
                // Refuses without --allow-system-write even when --allow-cmd is on.
                // These are shared PRODUCTION parameters, not recipe data.
                std::string perr;
                int nChanged = 0, nSame = 0, nMissing = 0;
                std::string bpath;
                //Steven 20260916 (review C): the gate stops APPLY, not dryRun.
                if (!gAllowSystemWrite && !(wc.hasValue && wc.value.isString() && PeekDryRun(wc.value.asString()))) {
                    perr = "system writes need --allow-system-write (system\\ is "
                           "shared production configuration)";
                } else if (!wc.hasTag) {
                    perr = "system.file.put needs tag=<file>";
                } else if (!wc.hasValue || !wc.value.isString()) {
                    perr = "system.file.put needs value=<json string>";
                } else if (!SafeDocName(wc.tag)) {
                    perr = "file name rejected";
                } else {
                    const SysFileEntry* ent = FindSysFile(wc.tag);
                    const char* own = ent ? CRouteOwner(SysFilePath(ent)) : nullptr;   //Steven 20260924（C 路，檔案擁有者閘）
                    if (!ent) {
                        perr = "not one of the system files";
                    } else if (own && !PeekDryRun(wc.value.asString())) {
                        perr = std::string("409 owned by C route (golden form bridge): ") + own;
                    } else {
                        cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                        if (!root) {
                            perr = "value is not parseable JSON";
                        } else {
                            bool dry = false;
                            const cJSON* jdry = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
                            if (jdry && cJSON_IsBool(jdry)) dry = (cJSON_IsTrue(jdry) != 0);
                            const std::string fp = SysFilePath(ent);
                            ht9045::RecipeWriteResult wr;
                            wr.ok = false; wr.changed = wr.identical = wr.notFound = 0;
                            if (!ent->csv) {
                                const cJSON* secs = cJSON_GetObjectItemCaseSensitive(root, "sections");
                                std::vector<ht9045::RecipeFieldEdit> edits;
                                if (secs && cJSON_IsObject(secs)) {
                                    for (const cJSON* sec = secs->child; sec; sec = sec->next) {
                                        if (!sec->string || !cJSON_IsObject(sec)) continue;
                                        for (const cJSON* f = sec->child; f; f = f->next) {
                                            if (!f->string) continue;
                                            const cJSON* jraw = cJSON_IsObject(f)
                                                ? cJSON_GetObjectItemCaseSensitive(f, "raw") : f;
                                            if (!jraw || !cJSON_IsString(jraw) || !jraw->valuestring) continue;
                                            ht9045::RecipeFieldEdit e;
                                            e.section = sec->string; e.key = f->string;
                                            e.rawValue = jraw->valuestring;
                                            edits.push_back(e);
                                        }
                                    }
                                }
                                if (edits.empty()) perr = "no sections/<key>/raw fields in the payload";
                                else wr = ApplyIniGuarded(vclcompat::AnsiString(fp.c_str()), edits, dry);   //Steven 20260916 (B3/B4)
                            } else {
                                const cJSON* rows = cJSON_GetObjectItemCaseSensitive(root, "rows");
                                std::vector<CsvCellEdit> edits;
                                if (rows && cJSON_IsObject(rows)) {
                                    for (const cJSON* rw = rows->child; rw; rw = rw->next) {
                                        if (!rw->string || !cJSON_IsObject(rw)) continue;
                                        for (const cJSON* c = rw->child; c; c = c->next) {
                                            if (!c->string || !cJSON_IsString(c) || !c->valuestring) continue;
                                            CsvCellEdit e;
                                            e.rowKey = rw->string; e.column = c->string;
                                            e.rawValue = c->valuestring;
                                            edits.push_back(e);
                                        }
                                    }
                                }
                                if (edits.empty()) perr = "no rows/<key>/<column> fields in the payload";
                                else wr = CsvApplyEdits(fp, edits,
                                        dry ? ht9045::kRecipeWriteDryRun : ht9045::kRecipeWriteApply,
                                        CsvKeyColumn(ent));   //Steven 20260916
                            }
                            if (perr.empty()) {
                                nChanged = wr.changed; nSame = wr.identical; nMissing = wr.notFound;
                                bpath = wr.backupPath;
                                if (!wr.ok) perr = wr.error;
                                std::printf("system.file.put %s%s: changed=%d identical=%d "
                                            "notFound=%d%s%s\n",
                                            wc.tag.c_str(), dry ? " (dry)" : "",
                                            nChanged, nSame, nMissing,
                                            bpath.empty() ? "" : "  backup=", bpath.c_str());
                            }
                            cJSON_Delete(root);
                        }
                    }
                }
                {
                    webbridge::JsonWriter aw;
                    aw.BeginObject();
                    aw.Key("changed").Number((wb_int64)nChanged);
                    aw.Key("identical").Number((wb_int64)nSame);
                    aw.Key("notFound").Number((wb_int64)nMissing);
                    if (!bpath.empty()) aw.Key("backup").String(bpath);
                    aw.EndObject();
                    server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                           perr.empty() ? aw.Str() : perr);
                }
            } else if (wc.cmd == "system.csv.rows") {
                //Steven 20260916: add / delete whole rows of a csv system file.
                //   tag   = motTable | ioTable (any csv SysFileEntry)
                //   value = {"add":[{col:"raw",...}], "delete":["key",...], "dryRun":bool}
                // Same gate as system.file.put. See CsvApplyRows for the rules.
                std::string perr;
                int nAdded = 0, nDeleted = 0, nMissing = 0;
                std::string bpath;
                if (!gAllowSystemWrite && !(wc.hasValue && wc.value.isString() && PeekDryRun(wc.value.asString()))) {   //Steven 20260916 (C)
                    perr = "system writes need --allow-system-write (system\\ is shared production configuration)";
                } else if (!wc.hasTag) {
                    perr = "system.csv.rows needs tag=<file>";
                } else if (!wc.hasValue || !wc.value.isString()) {
                    perr = "system.csv.rows needs value=<json string>";
                } else if (!SafeDocName(wc.tag)) {
                    perr = "file name rejected";
                } else {
                    const SysFileEntry* ent = FindSysFile(wc.tag);
                    if (!ent)           perr = "not one of the system files";
                    else if (!ent->csv) perr = "system.csv.rows is for csv files only";
                    else {
                        cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                        if (!root) perr = "value is not parseable JSON";
                        else {
                            bool dry = false;
                            const cJSON* jdry = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
                            if (jdry && cJSON_IsBool(jdry)) dry = (cJSON_IsTrue(jdry) != 0);
                            std::vector<std::map<std::string, std::string> > adds;
                            std::vector<std::string> dels;
                            const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, "add");
                            if (ja && cJSON_IsArray(ja)) {
                                for (const cJSON* rw = ja->child; rw; rw = rw->next) {
                                    if (!cJSON_IsObject(rw)) continue;
                                    std::map<std::string, std::string> row;
                                    for (const cJSON* c = rw->child; c; c = c->next) {
                                        if (!c->string) continue;
                                        if (cJSON_IsString(c) && c->valuestring) row[c->string] = c->valuestring;
                                        else if (cJSON_IsNumber(c)) {
                                            char buf[64]; std::snprintf(buf, sizeof(buf), "%.15g", c->valuedouble);   //Steven 20260916 (E5): %g turned 1000000 into 1e+06
                                            row[c->string] = buf;
                                        }
                                    }
                                    adds.push_back(row);
                                }
                            }
                            const cJSON* jd = cJSON_GetObjectItemCaseSensitive(root, "delete");
                            if (jd && cJSON_IsArray(jd)) {
                                for (const cJSON* k = jd->child; k; k = k->next)
                                    if (cJSON_IsString(k) && k->valuestring) dels.push_back(k->valuestring);
                            }
                            if (adds.empty() && dels.empty()) perr = "no add/delete entries in the payload";
                            else {
                                const CsvRowsResult rr = CsvApplyRows(SysFilePath(ent), adds, dels, dry,
                                                                      CsvKeyColumn(ent), CsvKeyPattern(ent));   //Steven 20260916 (R6)
                                nAdded = rr.added; nDeleted = rr.deleted; nMissing = rr.notFound; bpath = rr.backupPath;
                                if (!rr.ok) perr = rr.error;
                                std::printf("system.csv.rows %s%s: added=%d deleted=%d notFound=%d%s%s\n",
                                            wc.tag.c_str(), dry ? " (dry)" : "", nAdded, nDeleted, nMissing,
                                            bpath.empty() ? "" : "  backup=", bpath.c_str());
                            }
                            cJSON_Delete(root);
                        }
                    }
                }
                {
                    webbridge::JsonWriter aw;
                    aw.BeginObject();
                    aw.Key("added").Number((wb_int64)nAdded);
                    aw.Key("deleted").Number((wb_int64)nDeleted);
                    aw.Key("notFound").Number((wb_int64)nMissing);
                    if (!bpath.empty()) aw.Key("backup").String(bpath);
                    aw.EndObject();
                    server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                           perr.empty() ? aw.Str() : perr);
                }
            } else if (wc.cmd == "system.levels.put") { extern std::string W906_LevelSetPut(const std::string& tag, const std::string& payloadJson, bool allowSystemWrite, bool* ok); const std::string lvPayload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool lvOk = false; std::string lvRes; try { lvRes = W906_LevelSetPut(wc.hasTag ? wc.tag : std::string(), lvPayload, gAllowSystemWrite, &lvOk); } catch (...) { lvOk = false; lvRes = "exception in golden TfSecurity (LevelSet)"; } if (!lvOk) { std::printf("system.levels.put -> %s\n", lvRes.substr(0, 160).c_str()); } server.CompleteCommand((unsigned long long)wc.id, lvOk, lvRes); /*AI(W906-FRW-S64) 20260926 (Steven 團隊)：golden TfSecurity::FormClose（V912 cSecurity.cpp:439-468）→ SetLevelSet（:1511），本體在 WebLevelSet.cpp（自己持 FormLock）。下面舊本體（BinApplyValues 直接改檔）保留但不可達；寫在同一行，不移動行號*/ } else if (false) {
                //Steven 20260916 (W906-FW-LEVELSET): 寫二進位定長陣列的個別元素。
                //   tag   = levelset
                //   value = {"values":{"<idx>":<int>,...}, "dryRun":bool}
                //
                // 稀疏：只有列出來的索引會被碰，其餘位元組原樣保留。這很重要 ——
                // Status.Security 一次只改幾格，整塊回送會把「這台機器目前是 5 階
                // 模式、某些索引是 4」的狀態壓成瀏覽器那份 4 階畫面能表達的東西。
                //
                // 閘門與 system.file.put 完全相同（--allow-cmd 之外還要
                // --allow-system-write），語意也相同：dryRun 先跑、notFound>0
                // 一律 all-or-nothing 拒寫並回 ok:false。
                std::string perr;
                int nChanged = 0, nSame = 0, nMissing = 0;
                std::string bpath, levelsClamped;
                if (!gAllowSystemWrite && !(wc.hasValue && wc.value.isString() && PeekDryRun(wc.value.asString()))) {
                    perr = "system writes need --allow-system-write (system\\ is shared production configuration)";
                } else if (!wc.hasTag) {
                    perr = "system.levels.put needs tag=<file>";
                } else if (!wc.hasValue || !wc.value.isString()) {
                    perr = "system.levels.put needs value=<json string>";
                } else if (!SafeDocName(wc.tag)) {
                    perr = "file name rejected";
                } else {
                    const SysBinEntry* bent = FindSysBin(wc.tag);
                    if (!bent) {
                        perr = "not a binary system projection (try levelset)";
                    } else {
                        cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                        if (!root) {
                            perr = "value is not parseable JSON";
                        } else {
                            bool dry = false;
                            const cJSON* jdry = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
                            if (jdry && cJSON_IsBool(jdry)) dry = (cJSON_IsTrue(jdry) != 0);
                            const cJSON* vals = cJSON_GetObjectItemCaseSensitive(root, "values");
                            std::map<int, int> edits;
                            bool badKey = false;
                            if (vals && cJSON_IsObject(vals)) {
                                for (const cJSON* v = vals->child; v; v = v->next) {
                                    if (!v->string) continue;
                                    // 索引鍵必須是純十進位數字。用 atoi 會讓 "3x" 靜靜
                                    // 變成 3 而寫到錯的格子，所以自己掃過一遍。
                                    const char* s = v->string;
                                    if (!*s) { badKey = true; break; }
                                    for (const char* q = s; *q; ++q)
                                        if (*q < '0' || *q > '9') { badKey = true; break; }
                                    if (badKey) break;
                                    if (!cJSON_IsNumber(v)) { badKey = true; break; }
                                    const double d = v->valuedouble;
                                    // 非整數的值同樣拒收，不四捨五入。
                                    if (d != (double)(int)d) { badKey = true; break; }
                                    edits[std::atoi(s)] = (int)d;
                                }
                            }
                            if (badKey) {
                                perr = "values keys must be decimal indices and values whole numbers";
                            } else if (edits.empty()) {
                                perr = "no values/<index> entries in the payload";
                            } else {
                                //Steven 20260924（審查第 8 輪 M-1）：levelset → golden TfSecurity::FormClose 的三條鉗制
                                //   套在「檔案現值＋送來的值」上，鉗制結果併進這次的寫入（dryRun 也回 clamped）。
                                if (StemFoldEq(std::string(bent->key), "levelset")) {
                                    extern std::string W906_SecurityClampLevels(int* lv);
                                    int lv[256] = {0};
                                    if (FILE* fp = std::fopen(SysBinPath(bent).c_str(), "rb")) {
                                        unsigned char buf[1024];
                                        const std::size_t n = std::fread(buf, 1, sizeof(buf), fp);
                                        std::fclose(fp);
                                        for (std::size_t i = 0; i + 3 < n && i / 4 < 256; i += 4)
                                            lv[i / 4] = (int)((unsigned)buf[i] | ((unsigned)buf[i + 1] << 8) |
                                                              ((unsigned)buf[i + 2] << 16) | ((unsigned)buf[i + 3] << 24));
                                    }
                                    for (std::map<int, int>::const_iterator it = edits.begin(); it != edits.end(); ++it)
                                        if (it->first >= 0 && it->first < 256) lv[it->first] = it->second;
                                    int before[256];
                                    std::memcpy(before, lv, sizeof(lv));
                                    levelsClamped = W906_SecurityClampLevels(lv);
                                    for (int i = 0; i < 256; ++i)
                                        if (lv[i] != before[i]) edits[i] = lv[i];
                                }
                                const BinWriteResult wr = BinApplyValues(bent, edits, dry);
                                nChanged = wr.changed; nSame = wr.identical; nMissing = wr.notFound;
                                bpath = wr.backupPath;
                                if (!wr.ok) perr = wr.error;
                                //Steven 20260924 (S12 LevelSet)：golden TfSecurity::FormClose 的 LevelSet 段 —— 讀回記憶體、
                                //   三條鉗制、SetLevelSet（見 cSecurity.cpp 檔尾）。以前網頁寫的值要等重開機才生效。
                                if (wr.ok && !dry && StemFoldEq(std::string(bent->key), "levelset")) {
                                    extern std::string W906_SecurityFormCloseLevels();
                                    FormLockGuard lock;
                                    W906_SecurityFormCloseLevels();   // 讀回記憶體（檔案已含鉗制結果）
                                }
                                std::printf("system.levels.put %s%s: changed=%d identical=%d "
                                            "notFound=%d%s%s\n",
                                            wc.tag.c_str(), dry ? " (dry)" : "",
                                            nChanged, nSame, nMissing,
                                            bpath.empty() ? "" : "  backup=", bpath.c_str());
                            }
                            cJSON_Delete(root);
                        }
                    }
                }
                {
                    webbridge::JsonWriter aw;
                    aw.BeginObject();
                    aw.Key("changed").Number((wb_int64)nChanged);
                    aw.Key("identical").Number((wb_int64)nSame);
                    aw.Key("notFound").Number((wb_int64)nMissing);
                    if (!bpath.empty()) aw.Key("backup").String(bpath);
                    aw.Key("clamped").String(levelsClamped);   // golden FormClose 鉗制改掉的索引（逗號分隔）
                    aw.EndObject();
                    server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                           perr.empty() ? aw.Str() : perr);
                }
            } else if (wc.cmd == "editlist.get") {
                // Steven 20260924 (S12-C，審查 C-A)：golden 開 TfConfiguration 頁 = FormShow（會重讀 config.ini、
                // lastdata.dat 進 LastSet）。必須在主迴圈跑，不能在 HTTP socket 執行緒 —— 所以是 WS 命令。
                // 只在頁面「開啟」時送，不要輪詢（每次都會把 LastSet 回到 lastdata.dat 的值，golden 開頁亦同）。
                std::string perr, ack;
                const filerw::PageDesc* pg = wc.hasTag ? filerw::FindPage(wc.tag) : nullptr;   // Steven 20260924：其他 C 路結構（_EditPage）
                if (!wc.hasTag || (wc.tag != "IniConfig" && wc.tag != "Teach" && wc.tag != "BinSelect" && wc.tag != "Offset_File" && !pg)) {   /*AI(W906-W5-TEACH) 20260925：Teach 有自己的入口；Steven 20260925：BinSelect、Offset_File 同*/
                    perr = "editlist.get needs tag=IniConfig|Teach or a registered C-route struct (FileRW/_EditPage.h)";
                } else if (filerw::OpenGateRefused(wc.tag, false, &perr)) {   /*AI(W906-FRW-S158) 20260927 [W906]：Q41 盤點 C-1／C-2 —— golden 開這一頁要先過選單鈕（工具選單 V912 main.cpp:29030-29037 sbSettingClick：SystemStart 不開、Insufficient(0)；設定選單 :29009-29016 sbConfigClick：SystemStart 不開、Insufficient(1)）再過頁面鈕自己的等級（Yield 39、BarCode 88、DIO 31…），表與理由在 FileRW/_EditPage.cpp kOpenGates。拒絕時 golden FormShow 一行都不跑（golden 按鈕在 ShowModal 之前 return）。分工照 Q42：網頁開窗表歸 Jimmy，這裡是 C++ 重查。接在同一行，不移動行號*/ std::printf("editlist.get %s -> refused (%s)\n", wc.tag.c_str(), perr.substr(0, perr.find(':')).c_str()); } else {
                    int st;
                    ht9045::formjson::FormLock();
                    try {
                        st = pg ? filerw::PageJson(*pg, &ack) : wc.tag == "Teach" ? FileRW_Teach_Page(&ack)
                           : wc.tag == "BinSelect" ? FileRW_BinSelect_Page(&ack) : wc.tag == "Offset_File" ? FileRW_Offset_Page(&ack)
                           : FileRW_IniConfig_Page(&ack);   /*AI(W906-W5-TEACH) 20260925*/
                    } catch (...) {
                        st = 500;
                        ack = "exception in golden FormShow";
                    }
                    ht9045::formjson::FormUnlock();
                    if (st != 200) { perr = ack; ack.clear(); }
                    std::printf("editlist.get %s -> %d (%u bytes)\n", wc.tag.c_str(), st, (unsigned)ack.size());
                }
                server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                       perr.empty() ? ack : perr);
            } else if (wc.cmd == "contactct.get") {
                // Steven 20260925 (Data.ContactCT)：golden TfContactCT（cContactCT.cpp V912）。value 可帶
                // {"yieldType":n}：沒帶＝開窗（FormShow，ItemIndex=3）；帶 n＝點 rgYieldType 第 n 個選項。
                // 回 sgYieldDrawCell 逐格擷取（text／bg／fg，golden clXxx 色名）＋ rgYieldType 選項，見 cContactCT.cpp 檔尾
                // W906_ContactCTJson。只讀記憶體（ArmData／ArmHistory），不寫檔。
                extern std::string W906_ContactCTJson(int yieldType);
                std::string perr, ack;
                int yt = -1;
                if (wc.hasValue && wc.value.isString() && !wc.value.asString().empty()) {
                    cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                    const cJSON* jy = root ? cJSON_GetObjectItemCaseSensitive(root, "yieldType") : 0;
                    if (!root) perr = "value is not parseable JSON";
                    else if (jy && cJSON_IsNumber(jy) && jy->valuedouble == (double)(int)jy->valuedouble) yt = (int)jy->valuedouble;
                    else if (jy) perr = "yieldType must be a whole number";
                    if (root) cJSON_Delete(root);
                }
                if (perr.empty()) {
                    ht9045::formjson::FormLock();
                    try {
                        ack = W906_ContactCTJson(yt);
                    } catch (const std::exception& e) {
                        perr = std::string("contactct.get: ") + e.what();
                    } catch (...) {
                        perr = "exception in golden TfContactCT";
                    }
                    ht9045::formjson::FormUnlock();
                }
                std::printf("contactct.get yieldType=%d -> %s (%u bytes)\n", yt, perr.empty() ? "ok" : perr.c_str(), (unsigned)ack.size());
                server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                       perr.empty() ? ack : perr);
            } else if (wc.cmd == "observer.get") {
                // Steven 20260925 (Data.Observer)：golden TfObserver（cObserver.cpp V912）。value 可省略（＝開窗 FormShow），
                // 或 {"act":"open|timer|tab|rowNo|form|year|month|file|filter|query","arg":n,"text":"..."}：每個 act 重播
                // golden 的一個操作（Timer1 每秒、點頁籤、點 rgRowNo／Display Form、選年月、點記錄檔、選 Filter、按 Query），
                // 對照見 cObserver.cpp 檔尾 W906_ObserverJson。讀記憶體（LastSet／ArmData／TastCategory）與 EventLogTxt CSV。
                extern std::string W906_ObserverJson(const std::string& act, int arg, const std::string& text);
                std::string perr, ack, oact = "open", otext;
                int oarg = -1;   if (!ht9045::WebCmdGuard::Exempt(wc) && server.ControlOwner() != (unsigned long long)wc.connId) perr = "not-operator";   //AI(W906-Q2-OBS) 20260927 (St02-E): Steven S124 = B -- only the READ acts skip the control token. yieldSite / yieldMax / yieldMin / yieldClear change memory (golden TfObserver Yield tab; yieldClear clears HistroyBin, SpeedButton1Click), so a caller that is not the token holder gets not-operator and nothing runs. WebCmdGuard::Exempt applies the kObserverActs read list (WebCmdGuard.cpp:112-113, act missing = open); checked BEFORE any act / arg / text validation (St01 probe data_observer_token_probe.py G). wc.connId = the sender (WebBridgeServer.cpp:224). Same line, so nothing below moves
                if (perr.empty() && wc.hasValue && wc.value.isString() && !wc.value.asString().empty()) {   //AI(W906-Q2-OBS) 20260927 (St02-E): perr.empty() -- no validation after a not-operator
                    cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                    if (!root) perr = "value is not parseable JSON";
                    else {
                        const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, "act");
                        const cJSON* jn = cJSON_GetObjectItemCaseSensitive(root, "arg");
                        const cJSON* jt = cJSON_GetObjectItemCaseSensitive(root, "text");
                        if (ja && cJSON_IsString(ja) && ja->valuestring) oact = ja->valuestring;
                        else if (ja) perr = "act must be a string";
                        if (jn && cJSON_IsNumber(jn) && jn->valuedouble == (double)(int)jn->valuedouble) oarg = (int)jn->valuedouble;
                        else if (jn) perr = "arg must be a whole number";
                        if (jt && cJSON_IsString(jt) && jt->valuestring) otext = jt->valuestring;
                        else if (jt) perr = "text must be a string";
                        cJSON_Delete(root);
                    }
                }
                if (perr.empty()) {
                    FormLockGuard lock;
                    try {
                        ack = W906_ObserverJson(oact, oarg, otext);
                    } catch (const std::exception& e) {
                        perr = std::string("observer.get: ") + e.what();
                    } catch (...) {
                        perr = "exception in golden TfObserver";
                    }
                }
                if (oact != "timer" || !perr.empty())   // act=timer 是頁面照 golden Timer1 每秒送一次，成功不印，免得每秒一行
                    std::printf("observer.get act=%s arg=%d -> %s (%u bytes)\n", oact.c_str(), oarg, perr.empty() ? "ok" : perr.c_str(), (unsigned)ack.size());
                server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                       perr.empty() ? ack : perr);
            } else if (wc.cmd == "editlist.save") {
                // Steven 20260924 (S12-C 寫方向)：tag = 結構（目前只有 "IniConfig"），
                // value = JSON 字串 {"widgets":{名稱:{text?|checked?|itemIndex?|position?|dateTime?|cells?}},
                //                   "answers":{"<golden 英文題目>":1|2}}。
                // 走 golden TfConfiguration::FormClose（→ CheckConfigurationBeforeSave → SaveConfiguration
                // → LoadConfiguration），config.ini／LastSet.ini／configByRecipe.ini 由 golden
                // SaveEditTextToFile 寫。詳見 FileRW/IniConfig.cpp。
                std::string perr, ack;
                const filerw::PageDesc* pg = wc.hasTag ? filerw::FindPage(wc.tag) : nullptr;   // Steven 20260924：其他 C 路結構（_EditPage）
                if (!wc.hasTag || (wc.tag != "IniConfig" && wc.tag != "Teach" && wc.tag != "BinSelect" && wc.tag != "Offset_File" && !pg)) {   /*AI(W906-W5-TEACH) 20260925：Teach 有自己的入口；Steven 20260925：BinSelect、Offset_File 同*/
                    perr = "editlist.save needs tag=IniConfig|Teach or a registered C-route struct (FileRW/_EditPage.h)";
                } else if (SystemStart || SoftStart) {   /*AI(W906-R0927-7) 20260927: RULINGS_20260927 第 2 條第 7 題 A（NIGHT_REPORT §0 第 7 列）—— 運轉中（或正要啟動／回原點）一律不從網頁存設定，在任何解析與寫檔之前擋；兩句訊息照 IO 頁（JsonBridge/IoBtnPanelClick.cpp:225-242）的寫法。golden 運轉中打不開設定畫面：V912 main.cpp:3970-3978 DoMainPadProcess SystemStart 時把 palSetup／palConfig 藏起來。只擋 editlist.save（form.save、recipe.doc.put、system.file.put 等另列給 Steven 決定）。接在同一行，不移動行號*/ perr = SystemStart ? "機台運轉中（SystemStart）不能從網頁存設定 —— golden 運轉中打不開設定畫面（V912 main.cpp:3970-3978 DoMainPadProcess 把 palSetup／palConfig 藏起來）" : "機台正要啟動或回原點（SoftStart）——這時不能從網頁存設定；golden 在設定畫面開著時根本不會進入這個狀態"; std::printf("editlist.save %s -> refused (%s)\n", wc.tag.c_str(), SystemStart ? "SystemStart" : "SoftStart"); } else if (filerw::OpenGateRefused(wc.tag, true, &perr)) {   /*AI(W906-FRW-S158) 20260927 [W906]：Q41 盤點 C-1／C-2 —— 存檔前再查一次開這一頁的 golden 閘（選單等級＋頁等級，FileRW/_EditPage.cpp kOpenGates；每次操作都重查，同 WebBuilder.cpp:255-267）：開頁之後 levelset.dat 可能被改。在任何解析與寫檔之前。接在同一行，不移動行號*/ std::printf("editlist.save %s -> refused (%s)\n", wc.tag.c_str(), perr.substr(0, perr.find(':')).c_str()); } else if (!wc.hasValue || !wc.value.isString()) {
                    perr = "editlist.save needs value=<json string>";
                } else {
                    cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                    const cJSON* jw = root ? cJSON_GetObjectItemCaseSensitive(root, "widgets") : 0;
                    const cJSON* ja = root ? cJSON_GetObjectItemCaseSensitive(root, "answers") : 0;  struct W906ReauthWipe { ~W906ReauthWipe() { extern void W906_ReauthClear(); W906_ReauthClear(); } } reauthWipe; extern std::string W906_ReauthTake(cJSON* root, const std::string& tag); extern void W906_ReauthAck(std::string* ack); const std::string reauthRefused = W906_ReauthTake(root, wc.tag);   /*AI(W906-Q45-B5) 20260930: Q45 甲重新登入（WebReauth.h、WebLogin.cpp 檔尾）—— value 的 "reauth"（跟 widgets 並列）一解析就拿出來、JSON 裡的字清成 0，放進暫存給存檔流程裡的 golden DoPassword（FileRW/TestIF_File_SetUp.gen.inc SU_DoPassword、FileRW/IniConfig.cpp IC_ReauthM01）；放進 widgets 裡、點不對、這一頁沒有重新登入點 ⇒ 下一行拒存；reauthWipe 在這一臂結束時一律清掉暫存（密碼先清成 0）。密碼不印（下面兩個 printf 只有 tag 與理由）。同一行*/
                    if (!reauthRefused.empty()) { perr = reauthRefused; std::printf("editlist.save %s -> refused (reauth)\n", wc.tag.c_str()); } else if (!jw || !cJSON_IsObject(jw)) {   /*AI(W906-Q45-B5) 20260930: 前半是新的（見上一行）；同一行*/
                        perr = "value must be {\"widgets\":{...},\"answers\":{...}}";
                    } else {
                        char* s1 = cJSON_PrintUnformatted(jw);
                        char* s2 = (ja && cJSON_IsObject(ja)) ? cJSON_PrintUnformatted(ja) : 0;
                        const std::string widgets = s1 ? s1 : "{}";
                        const std::string answers = s2 ? s2 : "{}";
                        if (s1) cJSON_free(s1);
                        if (s2) cJSON_free(s2);
                        int st;
                        {
                            ht9045::formjson::FormLock();
                            try {
                                st = wc.tag == "BinSelect" ? FileRW_BinSelect_Save(wc.value.asString(), &ack, &perr)   // Steven 20260925：整包 value（widgets／answers／bin／actions）
                                   : wc.tag == "Offset_File" ? FileRW_Offset_Save(widgets, answers, &ack, &perr)   // Steven 20260925：widgets＝{offsets:{…},common:{…}}
                                   : pg ? filerw::PageSave(*pg, widgets, answers, &ack, &perr)
                                   : wc.tag == "Teach" ? FileRW_Teach_Save(widgets, answers, &ack, &perr)   /*AI(W906-W5-TEACH) 20260925*/
                                   : FileRW_IniConfig_Save(widgets, answers, &ack, &perr);
                            } catch (...) {
                                st = 500;
                                perr = "exception in golden save path";
                            }
                            ht9045::formjson::FormUnlock();
                        }
                        if (st != 200 && perr.empty()) perr = "editlist.save failed";  if (st == 200) W906_ReauthAck(&ack);   /*AI(W906-Q45-B5) 20260930: 存檔成功時回應開頭加 "reauth":{...}（golden DoPassword 的結果＋登入狀態，不含密碼；沒帶 reauth、golden 也沒問就不加）；同一行*/
                        std::printf("editlist.save %s -> %d %s\n", wc.tag.c_str(), st, st == 200 ? "ok" : perr.c_str());
                    }
                    if (root) cJSON_Delete(root);
                }
                server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                       perr.empty() ? ack : perr);
            } else if (wc.cmd == "form.save") {
                // Steven 20260924 (S12 第二型)：tag = 頁面（"Setup.TesterIF.html"），
                // value = JSON 字串 {"widgets":{id:{text?,itemIndex?,checked?}}}。
                // 走 golden 存檔鈕（例：TFTestIF::spbSaveClick）由 golden 原檔產生的 bridge：
                // 權限守衛 → SaveSetupFile（WriteIniData，含 Change Log）→ SECS → 備份 → SetWorkParameter。
                // 讀檔端缺口（sourceGap）非空的頁面一律拒寫，理由見 JsonBridge/FormJson.cpp FormSave()。
                std::string perr, ack;
                if (!wc.hasTag) {
                    perr = "form.save needs tag=<page>";
                } else if (!wc.hasValue || !wc.value.isString()) {
                    perr = "form.save needs value=<json string>";
                } else {
                    std::string widgets = "{}";
                    cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                    const cJSON* jw = root ? cJSON_GetObjectItemCaseSensitive(root, "widgets") : 0;
                    if (!jw || !cJSON_IsObject(jw)) {
                        perr = "value must be {\"widgets\":{...}}";
                    } else {
                        char* s = cJSON_PrintUnformatted(jw);
                        widgets = s ? s : "{}";
                        if (s) cJSON_free(s);
                        const int st = ht9045::formjson::FormSave(wc.tag, widgets, &ack, &perr);
                        std::printf("form.save %s -> %d %s\n", wc.tag.c_str(), st,
                                    st == 200 ? "ok" : perr.c_str());
                    }
                    if (root) cJSON_Delete(root);
                }
                server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                       perr.empty() ? ack : perr);
            } else if (wc.cmd == "recipe.doc.put") {
                // AI(W906-FW-C1WIRE) 20260911: tag = document name, value = the
                // SAME {sections:{<sec>:{<key>:{raw}}}} shape GET returns. That
                // symmetry is the point: the browser reads a document, edits
                // `raw`, and posts it back. `value` and `type` are ignored if
                // present, so a whole unmodified document can be posted.
                //
                // Only `raw` is honoured because only `raw` is lossless --
                // rebuilding a value from the parsed number would rewrite
                // "16.9900" as "16.99" on a field nobody touched.
                //
                // JSON rides a string value because the wire's TagValue carries
                // no object or array (WebCommand is {cmd, tag, value}), and
                // changing that would change a protocol the web author's 92
                // pages already speak.
                std::string perr;
                std::string reloadNote;   // Steven 20260924 (S12)：存檔後重讀的結果，進 ack
                int nChanged = 0, nSame = 0, nMissing = 0;
                if (!wc.hasTag) {
                    perr = "recipe.doc.put needs tag=<document>";
                } else if (!wc.hasValue || !wc.value.isString()) {
                    perr = "recipe.doc.put needs value=<json string>";
                } else if (!SafeDocName(wc.tag)) {
                    perr = "document name rejected";
                } else {
                    std::vector<std::pair<std::string, std::string> > docs;
                    EnumRecipeDocs(&docs);
                    std::string full;
                    for (std::size_t d = 0; d < docs.size(); ++d) {
                        if (StemFoldEq(docs[d].first, wc.tag)) { full = docs[d].second; break; }
                    }
                    const char* own = full.empty() ? nullptr : CRouteOwner(full);   //Steven 20260924（C 路，檔案擁有者閘）
                    if (full.empty()) {
                        perr = "no such document in the active recipe";
                    } else if (own && !PeekDryRun(wc.value.asString())) {
                        perr = std::string("409 owned by C route (golden form bridge): ") + own;
                    } else {
                        cJSON* root = cJSON_Parse(wc.value.asString().c_str());
                        if (!root) {
                            perr = "value is not parseable JSON";
                        } else {
                            const cJSON* secs =
                                cJSON_GetObjectItemCaseSensitive(root, "sections");
                            bool dry = false;
                            const cJSON* jdry =
                                cJSON_GetObjectItemCaseSensitive(root, "dryRun");
                            if (jdry && cJSON_IsBool(jdry)) dry = (cJSON_IsTrue(jdry) != 0);
                            std::vector<ht9045::RecipeFieldEdit> edits;
                            if (secs && cJSON_IsObject(secs)) {
                                for (const cJSON* sec = secs->child; sec; sec = sec->next) {
                                    if (!sec->string || !cJSON_IsObject(sec)) continue;
                                    for (const cJSON* f = sec->child; f; f = f->next) {
                                        if (!f->string) continue;
                                        const cJSON* jraw = cJSON_IsObject(f)
                                            ? cJSON_GetObjectItemCaseSensitive(f, "raw")
                                            : f;
                                        if (!jraw || !cJSON_IsString(jraw) || !jraw->valuestring)
                                            continue;
                                        ht9045::RecipeFieldEdit e;
                                        e.section  = sec->string;
                                        e.key      = f->string;
                                        e.rawValue = jraw->valuestring;
                                        edits.push_back(e);
                                    }
                                }
                            }
                            if (edits.empty()) {
                                perr = "no sections/<key>/raw fields in the payload";
                            } else {
                                const ht9045::RecipeWriteResult wr =
                                    //AI(W906-WEB-W2a) 20260917: see ApplyIniGuarded above.
                                    ApplyIniGuarded(AnsiString(full.c_str()), edits, dry);
                                nChanged = wr.changed;
                                nSame    = wr.identical;
                                nMissing = wr.notFound;
                                if (!wr.ok) perr = wr.error;
                                std::printf("recipe.doc.put %s%s: changed=%d identical=%d "
                                            "notFound=%d%s%s\n",
                                            wc.tag.c_str(), dry ? " (dry)" : "",
                                            nChanged, nSame, nMissing,
                                            wr.backupPath.empty() ? "" : "  backup=",
                                            wr.backupPath.c_str());
                                // Steven 20260924 (S12)：寫檔真的改了東西才重讀（golden 存檔 → ReadFile()）。
                                if (wr.ok && !dry && nChanged > 0) {
                                    reloadNote = ReloadRecipeDocAfterSave(wc.tag);
                                    std::printf("recipe.doc.put %s: %s\n", wc.tag.c_str(), reloadNote.c_str());
                                }
                            }
                            cJSON_Delete(root);
                        }
                    }
                }
                // The ack carries ok + the reason. The new CONTENT comes back
                // through a fresh GET /api/recipe/<doc> -- the browser has to
                // re-read anyway, and that is exactly the
                // HT_SIMULATOR_PUBLISHED -> re-read pattern the web author's
                // pages already implement.
                //
                //AI(W906-WEB-W2a) 20260917: ...but the COUNTS have to come back,
                // and they were not. This handler only printf'd the numbers to the
                // server console and passed `perr` (empty on success), so the
                // browser received a bare {"type":"ack","ok":true}. The wire
                // engine needs `notFound` to enforce rule 2 (a notFound means the
                // mapping is wrong -- refuse the whole page rather than write part
                // of it) and `changed` to show the operator what is about to be
                // written; with both absent it read them as empty, so rule 2 could
                // never fire and save() always stopped at "nothing changed".
                //
                // ⚠ THIS IS THE OTHER HALF OF WEB-W1. That change taught AckJson to
                // forward a JSON object passed on SUCCESS; this is the first caller
                // that passes one. Either alone is inert.
                //
                // Shape copied from the colleague's system.file.put so the client
                // reads one contract, not two.
                {
                    webbridge::JsonWriter aw;
                    aw.BeginObject();
                    aw.Key("changed").Number((wb_int64)nChanged);
                    aw.Key("identical").Number((wb_int64)nSame);
                    aw.Key("notFound").Number((wb_int64)nMissing);
                    if (!reloadNote.empty()) aw.Key("reload").String(reloadNote);   // Steven 20260924 (S12)
                    aw.EndObject();
                    server.CompleteCommand((unsigned long long)wc.id, perr.empty(),
                                           perr.empty() ? aw.Str() : perr);
                }
            } else if (wc.cmd == "auth.login") {
                // AI(W906-FW-W2) 20260819: tag = user name, value = password
                // (both strings). Verification = golden's password-book arm
                // (WebAuth.cpp); success drives the SAME global golden's
                // btLogin drives: AccessLevel. Book path: golden's pwPath
                // global, test-overridable via W906_PWBOOK_PATH (call-time
                // getenv, the tree's established env-seam shape).
                AnsiString book = getenv("W906_PWBOOK_PATH")
                                  ? AnsiString(getenv("W906_PWBOOK_PATH")) : pwPath;
                AnsiString u = wc.hasTag   ? AnsiString(wc.tag.c_str())   : AnsiString("");
                AnsiString p = (wc.hasValue && wc.value.isString())
                                  ? AnsiString(wc.value.asString().c_str()) : AnsiString("");
                //Steven 20260924：驗證改走 golden cbUserSelectChange 的密碼本分支（WebLogin_BookLogin）：
                //  本機客戶 791 盛合晶微 bUseLoginDatToSetLevel=true（CosFunction.cpp:1773）→ login.dat（USER 結構＋DecodeStr），
                //  原本的 WebAuthVerify 只會讀文字密碼本。W906_PWBOOK_PATH 仍可指定文字密碼本做測試。
                std::string lerr;
                const bool allowed = WebLogin_LoginAllowed();
                const int lr = allowed ? WebLogin_BookLogin(u, p, getenv("W906_PWBOOK_PATH") ? book : AnsiString(""), &lerr)
                                       : WEBLOGIN_BAD_ARG;
                if (!allowed) {                          //Steven 20260924：golden btLoginClick main.cpp:28080 if(SystemStart) return;
                    server.CompleteCommand((unsigned long long)wc.id, false, "machine is running (SystemStart)");
                } else if (lr == WEBLOGIN_OK) {
                    // AI(W906-FW1d) 20260820: golden's login path reaches
                    // ChangeLevelAttr whose FIRST act is DoChangeLevel
                    // (main.cpp:12410) -- mirror the level name the same way.
                    server.CompleteCommand((unsigned long long)wc.id, true, WebLogin_StateJson());
                } else if (lr == WEBLOGIN_BAD_CREDENTIALS) {
                    server.CompleteCommand((unsigned long long)wc.id, false, "bad credentials (" + lerr + ")");
                } else {
                    server.CompleteCommand((unsigned long long)wc.id, false, lerr);
                }
            } else if (wc.cmd == "auth.logout") {
                //Steven 20260924：golden btLoginClick 的 Logout 半邊（main.cpp:28095-28127），含 SystemStart 擋、MES2140 記錄
                std::string lerr;
                const bool ok = WebLogin_Logout(&lerr);
                server.CompleteCommand((unsigned long long)wc.id, ok, ok ? WebLogin_StateJson() : lerr);
            } else if (wc.cmd == "auth.mode") {
                //Steven 20260924：登入模式（golden FormShow :10863 密碼本 or 下拉選單）＋目前等級
                server.CompleteCommand((unsigned long long)wc.id, true, WebLogin_StateJson());
            } else if (wc.cmd == "auth.select") {
                //Steven 20260924：下拉選單模式（golden cbUserSelectChange 無密碼本分支 → stOperatorClick）。
                //  tag = cbUserSelect ItemIndex（0 Operator／1 Engineer／2 Supervisor／3 HonPrec），value = 密碼（需要時）。
                //  需要密碼而沒帶 → ok:true + needPassword:true，狀態不變；頁面開小鍵盤後再送。
                std::string lerr;
                const int idx = (wc.hasTag && wc.tag.size() == 1 && wc.tag[0] >= '0' && wc.tag[0] <= '9')
                                    ? (wc.tag[0] - '0') : -1;   // 審查 L1：嚴格解析，"Operator" 之類不會變成 0
                const bool hasPw = wc.hasValue && wc.value.isString();
                const int r = WebLogin_Select(idx, hasPw, hasPw ? AnsiString(wc.value.asString().c_str()) : AnsiString(""), &lerr);
                if (r == WEBLOGIN_OK) {
                    server.CompleteCommand((unsigned long long)wc.id, true, WebLogin_StateJson());
                } else if (r == WEBLOGIN_BAD_PASSWORD) {
                    // 密碼不對：狀態已照 golden 回到 Operator；ok:true + wrongPassword，頁面發不停機告警 WAR1677
                    std::string j = WebLogin_StateJson();
                    j.insert(1, "\"wrongPassword\":true,");
                    server.CompleteCommand((unsigned long long)wc.id, true, j);
                } else if (r == WEBLOGIN_NEED_PASSWORD) {
                    std::string j = WebLogin_StateJson();
                    j.insert(1, "\"needPassword\":true,");
                    server.CompleteCommand((unsigned long long)wc.id, true, j);
                } else {
                    server.CompleteCommand((unsigned long long)wc.id, false, lerr);
                }
            } else if (wc.cmd == "security.jam") { extern std::string W906_SecurityJamOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool sjOk = false; std::string sjRes; try { sjRes = W906_SecurityJamOp(payload, &sjOk); } catch (...) { sjOk = false; sjRes = "exception in golden TfSecurity (Jam)"; } std::printf("security.jam -> %s\n", sjOk ? "ok" : sjRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, sjOk, sjRes); /*Steven 團隊 20260926 (Status.Security Jam)：golden V912 TfSecurity Jam 分頁（WebSecurityJam.cpp，自己持 FormLock）；寫在同一行，不移動行號*/ } else if (wc.cmd == "security.passwd") { extern std::string W906_SecurityPasswdOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool spOk = false; std::string spRes; try { spRes = W906_SecurityPasswdOp(payload, &spOk); } catch (...) { spOk = false; spRes = "{\"executed\":false,\"guard\":\"exception\"}"; } std::printf("security.passwd -> %s\n", spOk ? "ok" : "refused"); server.CompleteCommand((unsigned long long)wc.id, spOk, spRes); /*AI(W906-SEC-S55) 20260926 Steven 團隊：Status.Security 改密碼（WebLogin.cpp 檔尾；回應不含密碼，這裡也不印 payload／回應）；寫在同一行，不移動行號*/ } else if (wc.cmd == "towerlight.op") { extern std::string W906_TowerLightOp(const std::string& payloadJson, bool* ok); const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); bool tlOk = false; std::string tlRes; try { tlRes = W906_TowerLightOp(payload, &tlOk); } catch (...) { tlOk = false; tlRes = "exception in golden TfTowerLight"; } std::printf("towerlight.op -> %s\n", tlOk ? "ok" : tlRes.substr(0, 160).c_str()); server.CompleteCommand((unsigned long long)wc.id, tlOk, tlRes); } else if (wc.cmd == "counterclear.get" || wc.cmd == "counterclear.click" || wc.cmd == "counterclear.exe") {   //AI(W906-TOWERLIGHT) 20260925 (Steven 團隊)：前面那一臂 towerlight.op ＝ golden V912 TfTowerLight（WebTowerLight.cpp，自己持 FormLock）：get＝FormShow、click＝RGB00Click（寫 lastdata.dat）、music＝FormShow→換框→FormClose→WriteLastDataFile。寫在同一行，不移動行號
                // Steven 20260925 (W906-CC-PAGE)：Data.CounterClear.html ↔ fCounterClear（golden V912 cCounterClear.cpp）。
                //   get = FormShow :92-114；click tag=<cb> value={"checked":b} = cbSelectAllMouseUp :50-77／cbAlarmDataMouseUp :79-88；
                //   exe value={"checked":{cb:b},"dryRun":b} = spbExeClick :399-455（勾幾個清幾類，前後一對 Clarn_Data(10)）。
                //   本體在 JsonBridge/ChanAction.cpp 檔尾；這裡只分派＋持 FormLock（exe 寫 LastSet／lastdata.dat）。
                extern std::string W906_CounterClearGet();
                extern std::string W906_CounterClearClick(const std::string&, const std::string&, bool*);
                extern std::string W906_CounterClearExe(const std::string&, bool*);
                const std::string ccVal = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
                std::string ccRes;
                bool ccOk = false;
                {
                    FormLockGuard lock;
                    try {
                        if (wc.cmd == "counterclear.get")        { ccRes = W906_CounterClearGet(); ccOk = true; }
                        else if (wc.cmd == "counterclear.click") ccRes = W906_CounterClearClick(wc.hasTag ? wc.tag : std::string(), ccVal, &ccOk);
                        else                                     ccRes = W906_CounterClearExe(ccVal, &ccOk);
                    } catch (...) {
                        ccOk = false;
                        ccRes = "exception in golden TfCounterClear";
                    }
                }
                std::printf("%s %s -> %s\n", wc.cmd.c_str(), wc.hasTag ? wc.tag.c_str() : "", ccOk ? "ok" : ccRes.c_str());
                server.CompleteCommand((unsigned long long)wc.id, ccOk, ccRes);
            } else if (wc.cmd == "counter.clear") {
                // AI(W906-FW-W4) 20260819: tag = counter family, mirroring one
                // checkbox of golden spbExeClick (cCounterClear.cpp golden
                // :394-450). Authorization is golden's OWN gate replayed: the
                // FormShow rule "an unauthorized checkbox can never be checked"
                // becomes a per-family refusal here, read from the same
                // Security_new.def via GetCountClrAuth() (cAuthority.cpp
                // golden :379-384). WRITE CAVEAT, stated not discovered:
                // golden's own readers seed defaults back into that file --
                // CheckFile when the FILE is missing, and CheckAndReadIniData
                // (common.cpp:409) when a KEY is missing. Faithful, but it
                // means this dispatch can write config\Security_new.def on a
                // machine with a stale def; every e2e run MD5-checks the file
                // stayed untouched.
                // Deliberately NOT called (documented deviations):
                //   * MyDBIProductionData -- golden runs it with the sqlite
                //     production DB open; wb_serve never brings the DB layer
                //     up, so its precondition does not exist here (and
                //     MyDBExecSQL on a null dbReadWrite is a crash, not a row).
                //   * the CC_KYEC_LEE re-auth branch -- already GATE CC3
                //     (#if 0) in the translated core itself.
                // ===== AI(W906-SJSON-S11) 20260923 BEGIN -- 收編成 act.* 的別名 =====
                //   上面那段註解是這個分支的歷史，**一個字都沒改**。
                //   改的是本體：family 表、權限閘、四個呼叫，整段原樣搬到
                //   JsonBridge/ChanAction.cpp 的 DoCounterClearExe()。
                //   理由：派工單第 2 條（實質邏輯一律在 JsonBridge/）＋
                //         SKILL §4.5 規則 1（counter.clear 是
                //         act.counterClear.exe 的別名）。
                //
                //   ⚠ **行為必須逐行相同**：同一張 family 表、同一個
                //     GetCountClrAuth()/authCounterClr[] 閘、同樣四個呼叫、
                //     同一行 printf。既有的 e2e 已經驗過這條路，改動等於作廢它。
                //     唯一的新增是 payload 可選的 `"dryRun":true`（預設仍是執行）。
                //
                //   ⚠ 回應形狀變了：原本成功回空字串，現在回一個 JSON 物件
                //     （帶 sideEffectsSkipped）。ack 的 `ok` 旗標語意不變，
                //     所以只看 ok 的呼叫端不受影響。
                {
                    const std::string payload = (wc.hasValue && wc.value.isString())
                                              ? wc.value.asString() : std::string();
                    const std::string res = ht9045::sjson::HandleActionWithTag(
                        "act.counterClear.exe", payload,
                        wc.hasTag ? wc.tag : std::string());
                    const bool ok = (res.find("\"executed\":true") != std::string::npos);
                    server.CompleteCommand((unsigned long long)wc.id, ok, res);
                }
            } else if (wc.cmd == "motor.access" || wc.cmd == "motor.stop") { extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool); std::string maAck; const bool maOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(), (long long)wc.id, maAck, wc.cmd == "motor.stop"); server.CompleteCommand((unsigned long long)wc.id, maOk, maAck);   // ===== AI(W906-SJSON-S11) 20260923 END =====（S11 臂在上一行的 } 結束）  AI(W906-W4-MOTOR) 20260925: 本行起是 motor.access 臂 —— HW.MotorTest／HW.teach 的馬達按鈕（WebMotorAccess.h）；放在 #ifdef WB_PUMP_1203_CONTROL 外面，沒有 1203 控制層的建置也要誠實回拒絕，不能讓命令落到「unknown cmd」
#ifdef WB_PUMP_1203_CONTROL
            // AI(W906-BU-C4) 20260918: the PCIE-1203 command surface.
            //
            //   ⚠ THIS IS THE BRANCH THAT CAN MOVE A SERVO. On a binary built
            //   with HAVE_PCI1203 and WB_PUMP_1203_CONTROL_LIVE, a click in a
            //   web page reaches Acm_* from here. Everything below exists to
            //   make that traceable rather than convenient.
            //
            //   The colleague's module owns every decision that matters --
            //   the allowlist, range checking, dry run, and the audit log. This
            //   branch only translates WebCommand -> Pci1203WireCmd, and the two
            //   structs line up field for field (id/cmd/tag/value), which is not
            //   a coincidence: he designed the wire struct for this seam.
            //
            //   TWO REFUSAL PATHS, deliberately different:
            //     * no control object  -> the binary cannot reach a card at all
            //       (this laptop: built without HAVE_PCI1203). Nothing is logged
            //       to the module, because there is no module.
            //     * ParseWire says no  -> a real, enabled module refused real
            //       input. NoteRefusal() puts it in the audit log, so "the
            //       button did nothing" has an entry behind it.
            // AI(W906-ST-S3-B4) 20260918: START. ⚠⚠ THIS IS THE ARMED BRANCH.
            //
            //   What one call does: StartFromWeb() runs golden's TfMain::Start()
            //   translation (2,530 lines). If its checks pass it reaches the
            //   W6-F block un-gated by S3-B1, sets SoftStart = true, and from
            //   there ckernel.cpp:816 -> ckernel.cpp:1015 sets SystemStart on the
            //   next pump tick. On a machine with hardware, that is the machine
            //   starting.
            //
            //   ⚠ It also DOES things on the way, not just checks -- measured:
            //   SW[SwCCDLight].Off() (:821), two WriteIniData (:902-903), a
            //   whole-file rewrite of system/Gerneral.ini via
            //   WriteIniDataGeneral (:1806), and COM2->
            //   iWriteAndCheckMotorTorqueDelay.On() (:2080). There is no
            //   check-only mode yet; the user's 20260918 ruling was to get
            //   SystemStart moving first and add checkOnly after.
            //
            //   The single-operator token is NOT re-checked here: the server
            //   layer already refuses commands from a connection that does not
            //   hold it (WebBridge/WebBridgeServer.h:99), which is where recipe
            //   writes are gated too. One gate, one place.
            } else if (wc.cmd == "start.run") {
                if (!g_webMain) {
                    server.CompleteCommand((unsigned long long)wc.id, false,
                                           "fMain was not repointed to TfMainWeb -- start is not armed in this binary");
                } else if ([&]() { extern bool W906_MotorAccessStartBlocked(std::string&); std::string tw; if (!W906_MotorAccessStartBlocked(tw)) return false; std::printf("start.run REFUSED: %s\n", tw.c_str()); server.CompleteCommand((unsigned long long)wc.id, false, tw); return true; }()) { } else {   // AI(W906-W5-b) 20260925: 覆核 W5B-R5 手動教導中（伺服關、操作員可能推著軸）不准啟動 —— golden fTeachShow 是 ShowModal，開著時 START 按不到（上一版是「先結束手動教導並自動開伺服」）
                    const AnsiString who = wc.hasValue && wc.value.isString()
                                         ? AnsiString(wc.value.asString().c_str())
                                         : AnsiString("web");
                    std::printf("start.run: calling StartFromWeb(\"%s\") ...\n", who.c_str());
                    WdMark("start.run: inside TfMainWeb::StartFromWeb()");      // AI(W906-WD) 20260923
                    const bool started = g_webMain->StartFromWeb(who);
                    //AI(W906-ST-S3-B4) 20260918: iStartIn is NOT reported here.
                    //  It has no declaration in any header -- it is WebStart.cpp's
                    //  own state -- and exposing it would mean widening a header
                    //  just to print a number. SoftStart/SystemStart are the two
                    //  that actually answer "did anything happen", and both are
                    //  already extern in cmydef.h:221/:223.
                    std::printf("start.run: StartFromWeb returned %s  (SoftStart=%d SystemStart=%d)\n",
                                started ? "true" : "false",
                                (int)SoftStart, (int)SystemStart);
                    webbridge::JsonWriter aw;
                    aw.BeginObject();
                    aw.Key("accepted").Bool(started);
                    aw.Key("softStart").Bool(SoftStart != 0);
                    aw.Key("systemStart").Bool(SystemStart != 0);
                    aw.EndObject();
                    // ⓘ SystemStart is reported as it stands RIGHT NOW, which is
                    //   normally still false: ckernel only flips it on a later
                    //   pump tick. The browser should watch the machine.state
                    //   tag, not this one field, to see the machine start.
                    server.CompleteCommand((unsigned long long)wc.id, started, aw.Str());
                }
            //AI(W906-T3-PAUSE) 20260918: pause.run —— START 的鏡像。
            //
            //  ⚠ 這條指令會讓機台停下來（PauseFromWeb 內部 StopAllMotor()）。
            //    停機比啟動安全，但它仍然是一個真的會動硬體的命令，
            //    所以走跟 start.run 一模一樣的守衛：沒有 g_webMain 就誠實拒絕，
            //    不要假裝成功。
            //
            //  ⓘ 回報 SoftStop 而不是等 SystemStart 翻。ckernel.cpp:1046
            //    的「暫停檢查」要到**下一個 pump tick** 才把 SystemStart 清成
            //    false（並順手把 SoftStop 清回 false），所以這支回來的當下
            //    SystemStart 正常仍是 1。瀏覽器要看 machine.state tag，
            //    跟 start.run 的處置一致。
            } else if (wc.cmd == "main.home") { extern void W906_MainHomeCommand(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_MainHomeCommand(server, wc);   /* AI(W906-FLOW-1) 20260927: 主畫面 HOME 鈕＝golden BtnHomeClick main.cpp:7120-7144（本體檔尾）；同一行，不移動行號 */ } else if (wc.cmd == "pause.run") {
                if (!g_webMain) {
                    server.CompleteCommand((unsigned long long)wc.id, false,
                                           "fMain was not repointed to TfMainWeb -- pause is not armed in this binary");
                } else {
                    const AnsiString who = wc.hasValue && wc.value.isString()
                                         ? AnsiString(wc.value.asString().c_str())
                                         : AnsiString("web");
                    std::printf("pause.run: calling PauseFromWeb(\"%s\") ...\n", who.c_str());
                    const bool paused = g_webMain->PauseFromWeb(who);
                    std::printf("pause.run: PauseFromWeb returned %s  (SoftStop=%d SystemStart=%d)\n",
                                paused ? "true" : "false",
                                (int)SoftStop, (int)SystemStart);
                    webbridge::JsonWriter aw;
                    aw.BeginObject();
                    aw.Key("accepted").Bool(paused);
                    aw.Key("softStop").Bool(SoftStop != 0);
                    aw.Key("systemStart").Bool(SystemStart != 0);
                    aw.EndObject();
                    server.CompleteCommand((unsigned long long)wc.id, paused, aw.Str());
                }
            //AI(W906-LOT-W1) 20260919: lot.start —— START 的前置。
            //
            //  為什麼需要它：20260919 實測，teach 資料層通了之後，
            //  `start.run` 的下一個否決是 `WebStart.cpp:2086`
            //  「Please Enter LotID and Operator ID!!」（golden main.cpp:5216）。
            //  這台 CUSTOMER_CODE=868=CC_CYUEAN，命中 golden :5204 那一支，
            //  它要 LotID / OperatorID 非空、且 `RunInfo.bLotStart==true`。
            //
            //  ⛔ 刻意**不**直接寫 `RunInfo.bLotStart=true`，也**不**在
            //     `StartFromWeb` 裡塞假值 —— 那是把 golden 的安全閘挖掉。
            //     這裡走 golden 自己的路：
            //       設兩個 widget 的 Text -> `fLotInfo->SetLotStart(sFunc)`
            //       -> `SetLotComponents(false)` -> `RunInfo.bLotStart=true`
            //
            //  訊框形狀（沿用既有欄位，不新增解析）：
            //    { "type":"cmd", "id":N, "cmd":"lot.start",
            //      "tag":"<LotID>", "value":"<OperatorID>" }
            //  `tag` 與 `value` 是 WebCommand 本來就有的兩個欄位
            //  （WebBridge/CommandQueue.h:77/:80），所以不必在 value 裡塞 JSON。
            //
            //  ⚠ 會寫真實檔：`SetLotStart` -> `SetLotID` / `ReadWriteLotInfo(false)`
            //    會寫 `D:\HT9045\config\config.ini` 的 `[Lot Info]`。
            //    驗證照 docs/PLAN_START_TO_RUN.md §0.6（tools/realfile_guard.py）。
            } else if (wc.cmd == "lot.start") {
                if (!fLotInfo) {
                    server.CompleteCommand((unsigned long long)wc.id, false,
                                           "fLotInfo is null -- lot start is not armed in this binary");
                } else {
                    const AnsiString lotId = wc.hasTag ? AnsiString(wc.tag.c_str()) : AnsiString("");
                    const AnsiString opId  = (wc.hasValue && wc.value.isString())
                                           ? AnsiString(wc.value.asString().c_str()) : AnsiString("");
                    if (lotId == "" || opId == "") {
                        //  golden 自己也是這樣擋的（main.cpp:5213-5217），
                        //  所以在這裡先擋掉只是讓錯誤訊息更早、更準，不是新增規則。
                        server.CompleteCommand((unsigned long long)wc.id, false,
                                               "lot.start needs a non-empty tag (LotID) and value (OperatorID)");
                    } else {
                        fLotInfo->edtSysLotID->Text      = lotId;
                        fLotInfo->edtSysOperatorID->Text = opId;
                        std::printf("lot.start: LotID=\"%s\" OperatorID=\"%s\" -> SetLotStart ...\n",
                                    lotId.c_str(), opId.c_str());
                        fLotInfo->SetLotStart("wb_serve::lot.start");
                        std::printf("lot.start: RunInfo.bLotStart=%d\n", (int)RunInfo.bLotStart);

                        webbridge::JsonWriter aw;
                        aw.BeginObject();
                        aw.Key("lotId").String(lotId.c_str());
                        aw.Key("operatorId").String(opId.c_str());
                        aw.Key("lotStart").Bool(RunInfo.bLotStart != 0);
                        aw.EndObject();
                        //  ⓘ ok 用 `RunInfo.bLotStart` 而不是無條件 true：
                        //    那才是呼叫端真正在乎的東西，而且它是 golden 的
                        //    狀態不是我們的回報。
                        server.CompleteCommand((unsigned long long)wc.id,
                                               RunInfo.bLotStart != 0, aw.Str());
                    }
                }
            } else if (wc.cmd == "io.btnPanelClick") { extern void W906_DispatchIoClick(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_DispatchIoClick(server, wc); } else if (wc.cmd.compare(0, 8, "pci1203.") == 0) { extern void W906_Dispatch1203(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_Dispatch1203(server, wc);   // AI(W906-IOWEB-P17) 20260925: io.btnPanelClick = HW.IoSetView output button -> golden BtnPanelClick (JsonBridge/IoBtnPanelClick.cpp) -> MyLaneIO -> 1203 route -> Pci1203Control. BEFORE the pci1203. prefix branch (that one would answer "unknown command").  // AI(W906-IOWEB-P25) 20260925: both bodies moved to EOF (W906_DispatchIoClick / W906_Dispatch1203) so the output-first service runs the SAME code -- one copy, not two
#endif
            // AI(W906-P3-OLP) 20260919: OLP 設定指令面 —— 白名單 35 條，表在
            //   WebOlp.cpp（每一列帶 golden 的分支行號可以對帳）。
            //
            //   ⛔ 這條路**刻意不碰** TfAutomation::ProcessBuffer()。理由寫在
            //   WebOlp.h 檔頭，一句話版本：那條路每個分支最後都會走的共用回覆
            //   函式 SendClient() 在 handle 對不上時**強制 iConnect = 0**
            //   （automation.cpp:1034-1039），而瀏覽器不持有任何 OLP socket，
            //   所以它的 *_REPLY 會被推進當下第一個連著的真實工廠主機 ——
            //   線上 MES 連線的協定失步。改成直接叫 setter 並把 golden 的
            //   分支守衛與後置動作逐條複製過來，ack 回瀏覽器自己。
            //
            //   資料欄：value 是字串就用逗號切（對應 golden 訊框的
            //   Data[0..n-1]，golden 那邊是 automation.cpp:1337 的
            //   SplitDataBySoh）；value 是數字就是單欄的方便寫法 ——
            //   35 條裡有 26 條只要一個欄位。
            // AI(W906-P6-WINREG) 20260920: 視窗狀態總表（取代 golden 的 `fShow`）。
            //   契約：Steven `WINDOW_REGISTRY_CONTRACT.md` §4。
            //   送法（他的 `ht9045_recipe_client.js:606`，照著讀不是照著猜）：
            //       cmd('ui.windows.put', {tag:'registry', value: JSON.stringify(frame)})
            //   ⇒ value 是**一個 JSON 字串**，不是物件。
            //
            //   ⚠ 這條指令**不需要**單一操作員權杖（已在
            //   WebBridgeServer.cpp 的閘門豁免）。它是回報不是寫入；
            //   讓常駐的 background.html 去搶權杖會讓存檔頁永遠拿不到。
            //
            //   ⚠ 壞訊框**不動**既有快取 —— 解析失敗就回 ok:false 並保留舊值。
            //   把快取洗成空的 = 「當成全部關閉」= 契約 §6 明令禁止的那件事。
            } else if (wc.cmd == "ui.windows.put") {
                std::string why;
                const std::string frame =
                    (wc.hasValue && wc.value.isString()) ? wc.value.asString()
                                                         : std::string();
                if (frame.empty()) {
                    server.CompleteCommand((unsigned long long)wc.id, false,
                                           "ui.windows.put: value must be the frame JSON string");
                } else if (!ht9045::WebWindowRegistryPut(wc.connId, frame, why)) {
                    std::printf("ui.windows.put REFUSED: %s\n", why.c_str());
                    server.CompleteCommand((unsigned long long)wc.id, false,
                                           "ui.windows.put: " + why);
                } else {
                    const ht9045::WinRegistryStats st = ht9045::WebWindowRegistryStats();
                    std::printf("ui.windows.put conn=%llu seq=%lld windows=%u open=%u"
                                " conns=%u stale=%u topmost=%s\n",
                                (unsigned long long)wc.connId, (long long)st.lastSeq,
                                (unsigned)st.windowsTotal, (unsigned)st.openLike,
                                (unsigned)st.connections, (unsigned)st.staleConns,
                                st.topmost.empty() ? "(none)" : st.topmost.c_str());
                    webbridge::JsonWriter uw;
                    uw.BeginObject();
                    uw.Key("accepted").Bool(true);
                    uw.Key("seq").Number((wb_int64)st.lastSeq);
                    uw.Key("windows").Number((wb_int64)st.windowsTotal);
                    uw.Key("open").Number((wb_int64)st.openLike);
                    uw.Key("connections").Number((wb_int64)st.connections);
                    uw.Key("stale").Number((wb_int64)st.staleConns);
                    uw.EndObject();
                    server.CompleteCommand((unsigned long long)wc.id, true, uw.Str());
                }
            } else if (wc.cmd.compare(0, 4, "olp.") == 0) {
                std::vector<std::string> olpArgs;
                if (wc.hasValue && wc.value.isString()) {
                    const std::string& s = wc.value.asString();
                    // 空字串 = 零個欄位，不是一個空欄位。這個分野跟
                    // CommandQueue.h:64-68 對 tag/value 的 absent-vs-empty
                    // 是同一條規則。
                    if (!s.empty()) {
                        std::string::size_type b = 0;
                        for (;;) {
                            const std::string::size_type e = s.find(',', b);
                            if (e == std::string::npos) { olpArgs.push_back(s.substr(b)); break; }
                            olpArgs.push_back(s.substr(b, e - b));
                            b = e + 1;
                        }
                    }
                } else if (wc.hasValue && wc.value.isInt()) {
                    olpArgs.push_back(std::to_string((long long)wc.value.asInt()));
                } else if (wc.hasValue && wc.value.isDouble()) {
                    char nb[64];
                    std::snprintf(nb, sizeof(nb), "%g", wc.value.asDouble());
                    olpArgs.push_back(nb);
                }

                const WebOlpResult orr = WebOlpInvoke(wc.cmd, olpArgs);
                std::printf("%s: accepted=%d status=%d  %s\n",
                            wc.cmd.c_str(), orr.accepted ? 1 : 0, orr.status,
                            orr.detail.c_str());

                const bool olpOk = orr.accepted && orr.status == kWebOlpOk;

                // ★ 只有 status == 0 才是成功。1/2/3 是機台**合法地拒絕**
                //   （還有 IC／正在跑／值不合法），對瀏覽器是 ok:false 加理由，
                //   不是例外，也不可以壓成一句罐頭 "ok" —— 操作員要看得出
                //   「機台在跑」跟「機台裡還有 IC」是兩件不同的事。
                //
                // ⚠⚠ ack 的形狀**由 AckJson 決定，不是我們決定的**
                //     （WebBridge/WebBridgeServer.cpp:1215-1245，量過）：
                //       ok=true  -> 第三參數若長得像 JSON 物件，欄位會被
                //                   **攤平內嵌**進 ack（瀏覽器直接讀 ack.status）
                //       ok=false -> 第三參數被當成字串塞進 "error"，**會被加引號**
                //
                //     所以「成功失敗都送 JSON」是錯的：失敗時會變成
                //       {"ok":false,"error":"{\"status\":-2,...}"}
                //     JSON 字串裡再包一層 JSON。而且它違反這個檔自己的慣例 ——
                //     上面的 pci1203 分支就是 `r.accepted ? aw.Str() : r.why`。
                //     （這一版是第三版：第一版對、第二版我自己改壞、這版改回來
                //      並補上失敗側真正缺的東西。）
                //
                //     失敗側真正缺的是**穩定可取的狀態碼**，不是 JSON。
                //     所以純字串開頭固定 `status=<N> `，瀏覽器用
                //     /^status=(-?\d+)/ 取，不必比對中文。
                if (olpOk) {
                    webbridge::JsonWriter ow;
                    ow.BeginObject();
                    ow.Key("accepted").Bool(orr.accepted);
                    ow.Key("status").Number((wb_int64)orr.status);
                    if (!orr.golden.empty()) ow.Key("golden").String(orr.golden);
                    ow.Key("detail").String(orr.detail);
                    ow.EndObject();
                    server.CompleteCommand((unsigned long long)wc.id, true, ow.Str());
                } else {
                    char sb[32];
                    std::snprintf(sb, sizeof(sb), "status=%d ", orr.status);
                    std::string why = sb;
                    if (!orr.golden.empty()) why += orr.golden + ": ";
                    why += orr.detail;
                    server.CompleteCommand((unsigned long long)wc.id, false, why);
                }
            } else {
                //AI(W906-T3-DISPATCH) 20260923: 原本這裡附一串手抄的「支援哪些命令」清單（答應 Ifor 修）。
                //  那串已經過期兩次：S0 補過 cfg.resync／dialog.response，到今天又漏了 start.run、pause.run、
                //  system.file.put、system.csv.rows、system.levels.put、recipe.doc.put —— 對著它除錯的人會以為
                //  那些命令不支援。手抄清單只要分派鏈一改就會再過期，所以整句拿掉，改成回報**收到的是什麼**，
                //  並指出分派鏈在哪（上面這一串 `else if (wc.cmd == ...)`）。
                //AI(W906-T3-DISPATCH) 20260923 合併 Steven 56319e0 時的取捨：他那邊的 S11 又往手抄清單補了 act.*
                //  （剛好示範了這份清單一改分派鏈就過期）。保留 T3 的「回報收到什麼」，只吸收他指向
                //  GET /api/struct/act/schema 的那一句 —— 那是活的端點，不會跟著過期。
                server.CompleteCommand((unsigned long long)wc.id, false,
                                       "unknown cmd '" + wc.cmd + "' (no branch in tools/wb_serve.cpp's "
                                       "wc.cmd dispatch chain matched it; act.* list: GET /api/struct/act/schema)");
            }
        }
        if (g_apiCacheDirty) { g_apiCacheDirty = false; extern void W906_IoTiming(int); W906_IoTiming(5); W906_ApiCacheRefresh(); W906_IoTiming(6); }   // AI(W906-IOWEB-P25) 20260925: once per loop, after the drain (the cache serves /api/struct/{io,motor}/*, which the pages poll every 500 ms). On the old blank line
        //AI(W906-Q34-7) 20260923: (5) 有事才發布（理由見迴圈前的說明）。
        //  拍子上一定發（與舊迴圈相同），一批命令之後也發 —— 讓命令的結果與它造成的狀態
        //  在同一個快照裡出去（他 1203CTL-6 的理由，wb_publish.cpp:838-844）。
        const bool publishNow = pumpBeat || !drained.empty() || g_outputsServed   // AI(W906-IOWEB-P25) 20260925: an output served inside Poll / after the sleep publishes its read-back too
#ifdef INSTALL_1203_MONITOR
                                || ioPolled
#endif
                                ;
        if (publishNow) {
            ht9045::SetWebControlOwner(server.ControlOwner());   // AI(W906-FW-W3) 20260819: control.owner tag feed
            g_outputsServed = false;  extern void W906_IoTiming(int); W906_IoTiming(7); ht9045::PublishHandlerTags(snap); W906_IoTiming(8);   // AI(W906-P4-SECS): secs.sv.* 在它內部  // AI(W906-LAT-1) 20260925: (a) 7/8 = the publish's own phase in the click-latency log (JsonBridge/IoBtnPanelClick.cpp); (b) g_outputsServed is cleared BEFORE the publish (was: after it, on the line below): outputs now also run at the publish's yield points (SetPublishYieldHook), and clearing it afterwards would erase exactly that fact -- the next loop would then neither skip its sleep nor publish the read-back
            //  ⓘ AI(W906-LAT-1) 20260925: the flag cleared above is set again by any output served inside that publish -> see the sleep loop's wake condition
            server.Wake();
        }

        if (seconds > 0 &&
            (::GetTickCount() - started) >= static_cast<DWORD>(seconds) * 1000u) {
            break;
        }  if (pumpBeat) { extern void W906_MainRecordTimer1Tick(); W906_MainRecordTimer1Tick(); extern void W906_MainRunInfoTimer2Tick(); W906_MainRunInfoTimer2Tick(); }  { extern void W906_SortCTTimer1Tick(); W906_SortCTTimer1Tick(); }  if (pumpBeat) { extern void W906_TeachLeaveTick(bool*, bool); W906_TeachLeaveTick(&fAllMotorHome, SystemStart); }  /* AI(W906-FRW-S122) 20260927 [W906]（Steven 團隊）：關掉／打開 Teach／Motor Test ⇒ fAllMotorHome=false（停機中才清；運轉中不清、只印一行 [S122]）。照 golden V912 main.cpp:28846-28847（fTeach->ShowModal 回來就清）、uteach.cpp:2442-2443（fMotorTest->ShowModal 回來就清）、uteach.cpp:1610（TfTeach::FormShow 也清）；[W906] 不在 golden：用視窗總表 WebWindowRegistryFShowPolicy 的邊緣代替 ShowModal 回來（跟 MainProc 暫停同一個判斷；回報全部過期時沿用上一拍，不算邊緣）。RULINGS_20260927 第 2 條第 18 題＝B、decisions-pending R80～R83＝A；本體 WebTeachLeave.cpp；500 ms 拍子（pumpBeat）。接在同一行，不移動行號 */  /* AI(W906-PROD-S113) 20260926（Steven 團隊）：golden Timer1Timer main.cpp:3284 UpdateRecordScreen(false)（每拍累計 LastSet.SystemAccSecond）＋ Timer2Timer :21666 UpdateRunInfo()（本體每 5 分鐘推一次 RunInfo.iYieldChart／MTBA／MUBA）；本體 FileRW/MainRecord.cpp；500 ms 拍子（pumpBeat）。AI(W906-FRW-S97) 20260927：golden TfSortCT::Timer1Timer cSortCT.cpp:869-884（1000 ms，W906_SortCTTimer1Tick 自己節流；只有 CC_ASE_M 開頁後啟用，其他機台一進來就 return），本體 WebSortCT.cpp／cSortCT.cpp。20260927 從 :4598（Jimmy 的 STATEREC／MotorAccessTick 行）搬到這一行 St01 自己的行尾，避免合併撞到別人的行；接在同一行，不移動行號 */  if (pumpBeat) { extern void W906_PageTableTick(bool, bool, bool); extern bool (*W906_MotorHomingHook)(); W906_PageTableTick(SystemStart, fHome != 0 && fHome->iHomeStep != 1, W906_MotorHomingHook != 0 && W906_MotorHomingHook()); }  /* AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表每拍（500 ms）：網頁列的邊緣、操作員關掉 Home Monitor ⇒ golden fHome->Close()、瀏覽器全關滿 10 秒而且（SystemStart 或 HOME ALL 或網頁馬達工作）⇒ 正常 STOP（Steven Q-P1 20260928）；本體 WebPageTable.cpp PageTableTick。上面 S122 那一呼叫的取樣也改問頁面表（WebTeachLeave.cpp WindowEdgeSample）。接在同一行，不移動行號 */  { extern bool W906_ServeQuitDue(); if (W906_ServeQuitDue()) break; }  { extern bool W906_ExternalQuitDue(); if (W906_ExternalQuitDue()) break; }  /*AI(W906-STOPBTN) 20260929: the VS Code stop button / "關閉程式" task (tools/stop_wb_serve.ps1) asks for the same normal close as the main Exit, through a named event (EOF)*/   // AI(W906-PROD-S121) 20260926（Steven 團隊）：主畫面 Exit 第二框確認後（FileRW/MainClose.cpp W906_Main_CloseProgramOp step 2 設 W906_ServeQuitRequested）晚一圈離開主迴圈（這一圈的 ack 先送出去），走下面的正常關站路；接在同一行，不移動行號  AI(W906-FRW-S167) 20260928 [W906] Q44（Steven 20260928「至少馬達跟溫度的要停下來」）：W906_ServeQuitDue 晚一圈之後先停兩個（golden StopAllMotor＋1203 軸、SW[SwHeaterRelay].Off()；SIM 接真卡時另經 1203 命令面寫 0）並讀回確認，確認了（或網頁強制關閉）才回 true；沒停就留在迴圈，網頁用 act.main.closeProgram {"op":"status"|"retry"|"force"}；本體 FileRW/MainClose.cpp 檔尾；--seconds 在上面先 break，不等它；只改這一行的註解
    }

    // AI(W906-FW-W5a) 20260819: unhook before the server object dies.
    W906_ShowMyMessage_Hook = 0;  { extern void W906_MsgBoxHostUninstall(); W906_MsgBoxHostUninstall(); }   // AI(W906-SMM) 20260925
    W906_ShowErrorMessage_Hook = 0;   // AI(W906-FW-W5b) 20260819
    W906_ShowMyMessageBoxYesNo_Hook = 0;   // AI(W906-YESNO) 20260925: 同一時間卸下（伺服器物件死掉之前）
    // AI(W906-P6b-C) 20260921: 同樣在伺服器物件死掉之前卸下。
    // ⚠ 卸下之後 `Command.cpp` 的 Bit4 回到恆 0（hook 為 NULL 的分支），
    //   也就是解閘前的行為 —— 這是刻意的，關站過程不該還在查總表。
    W906_DiagnosticsWindowOpen_Hook = 0;
    g_pumpQueue = 0;  { extern void W906_ProdCloseSave(); W906_ProdCloseSave(); }  { extern void W906_ProdCloseShutdown(); W906_ProdCloseShutdown(); }  /*AI(W906-PROD-S121) 20260926（Steven 團隊）：golden FormClose 存檔以外的關站段（main.cpp:11881-12474：StopAllMotor＋1203 軸、IndexMotorBreakerOFF、加熱器繼電器／風扇／蜂鳴器 Off…；本體 FileRW/MainClose.cpp，只呼叫已有真本體的，其餘標「未停」），在 Program Close=1 與 server.Stop() 之前（1203 命令面與監看器還活著）；插在同一行、本行 // 註解之前，不移動行號*/   // AI(W906-PROD-S95) 20260926（Steven 團隊）：golden FormClose 的生產資料段（:11861 WriteLastDataFile(true)、:11919 SaveJamRateByDay(false)、:12195 WriteCTInfo；本體 FileRW/MainClose.cpp），在下一行寫 Program Close=1（golden :12444）之前；只有正常關站（--seconds 到期）走得到，Ctrl-C 那條（W906_ConsoleCtrl，另一條執行緒）不做；接在同一行，不移動行號
    g_modalServer = 0;  WriteIniDataGeneral("Record", "Program Close", 1);  { extern void W906_HmiShellQuit(); W906_HmiShellQuit(); }  /*AI(W906-HMI-SHELL) 20260930: the HMI program window closes with the program (golden: the main window IS the program)*/  { extern void W906_SecurityJamShutdown(); W906_SecurityJamShutdown(); }  /*AI(W906-SEC-S54) 20260926：收掉背景 Jam 匯出執行緒*/  { extern void WriteBootLog(AnsiString); WriteBootLog("Application->Run returned (normal exit)"); }  /* AI(W906-BOOTLOG) 20260926: golden HT9045.cpp:287 */   // AI(W906-A4-4) 20260924: golden FormClose main.cpp:11927（正常關站：--seconds 到期）

    { extern void W906_NativeFormsStop(); W906_NativeFormsStop(); }  /* AI(W906-NATIVE-PROTO) 20260928 [W906]: 正常關閉時關原生視窗。OFF＝空函式 */  server.Stop();
    { extern void W906_TesterCommShutdown(); W906_TesterCommShutdown(); }  { extern void W906_DestroyLogObjects(); W906_DestroyLogObjects(); }  /* AI(W906-CSVONLY-P1) 20260926: golden FormDestroy :12508-12550 log deletes */  { extern void (*W906_ElaPostHook)(int); W906_ElaPostHook = 0; extern void W906_ElaStop(); W906_ElaStop(); }  /* AI(W906-ELA-P3) 20260927: hook first, then join the worker */   //AI(W906-GB-P3) 20260926: H6 -- uninstall the fMain tester seat, stop the engine (golden FormClose of the bridge)
#ifdef WB_PUMP_1203_CONTROL
    // AI(W906-BU-C4) 20260918: after the server is down, so no in-flight command
    //   can find a half-closed control object. Safe when it was never armed --
    //   Disable() is a no-op on a null handle.
    ht9045::Pci1203ControlDisable();
#endif

#ifdef INSTALL_1203_MONITOR
    //AI(W906-Q34-5a) 20260923: 把卡還回去 —— 照翻同事 wb_publish.cpp:993-1008
    //  （1203MON-6 關站那一半）。Q34-1 只接了開卡那一半（§2 之後的
    //  Pci1203MonitorEnable()），關站時從來沒人關監控器。
    //  ⚠ 順序是承重的：先解除命令面（上面的 Pci1203ControlDisable），再關監控器。
    //    命令面借用監控器的 device／axis handle，反過來做會留下一個還武裝著、
    //    手上握著剛被關掉的 handle 的寫入端。
    //  ⚠ 為什麼要關：沒被關的 device 會讓**下一次**開卡失敗，而那個下一次可能是
    //    量產程式自己的開卡。Close() 只在 ownsDev 時才 Acm_DevClose
    //    （Pci1203Monitor.cpp 的 1203MON-8 守衛），附掛模式不會關掉量產的卡。
    //  ⓘ 沒開 HAVE_PCI1203 的建置（這台筆電）上 g_monitor 也不是 null ——
    //    Enable() 在 Open() 回 "not linked" 時照樣把物件掛上去 —— 所以這裡會
    //    真的 delete 它；Close() 在那個分支只清欄位，不碰任何廠商呼叫。
    //  ⓘ 只有 `seconds` 到期那條路會走到這裡；本程式沒有 Ctrl-C handler，
    //    這一點與同事的 wb_publish 相同。
    ht9045::Pci1203MonitorDisable();
#endif

#if 0 // AI(W906-NODRY) 20260924: 關站還原隨 --dry 退場（asGeneralPath／DataPath 從未被導走，沒有東西要還原）
        CloseGeneralIniFile();
        asGeneralPath = savedGeneralPath;
        ::DeleteFileA(scratch.c_str());
        // AI(W906-FW-BIN1) 20260820: restore the recipe-path redirect too.
        // The scratch folder is left for the OS temp cleaner (it may hold
        // seeded keys useful for post-mortem diffing against the real one).
        DataPath = savedDataPath;
#endif // AI(W906-NODRY) 20260924

    std::printf("stopped\n");
    return 0;
}

// =============================================================================
//  AI(W906-ALARMSTOP) 20260924: 告警照 golden 停機；告警框按 START 照 golden 重走啟動檢查。
//
//  使用者 20260924 裁決（三題）：
//    1) 「兩者都停機」—— kcode≠0 的阻塞告警與 kcode==0 的通知都照 golden ShowErrorMessage（note.cpp:795-801）
//       StopAllMotor＋SoftStop／SoftStart／SystemStart=false。kcode==0 仍依 20260923 裁決「不阻塞」（ForwardShowErrorMessage 裡那段），
//       所以通知一出來機台就停，操作員要再按 START 才繼續 —— 這就是 golden 的結果，只是不用在框上按鍵。
//    2) StopAllMotor 統一：aHotPlateSubstrate.cpp 的 no-arg 空殼改轉呼叫真本體 StopAllMotor(true)（Motor/myGALILmotor.cpp，golden :4712）。
//       在這之前，這裡叫 StopAllMotor 也只是呼叫空殼。
//    3) 回答 START：golden TfNote::Start（note.cpp:3665-3678）在 Close() 之前呼叫 fMain->Start("fNote::Start 1") 與
//       SendCommand_ESD(ESD_SYSTEM_START)。移植樹的等價物是 g_webMain->StartFromWeb()（它刻意不是 TfMain::Start 的 override，
//       見 WebStart.h 檔頭；這裡是 start.run 之外第二個、也是唯一新增的呼叫點）。
//  ⚠ 呼叫點在狀態機的呼叫鏈裡面（ShowErrorMessage 的 hook），golden 也一樣：fNote 在 ShowModal 期間呼叫 fMain->Start，
//    狀態機停在 ShowErrorMessage 裡等回傳值。StartFromWeb 內若再跳阻塞告警，外層的 query 已經 ClearQuery，巢狀沒有衝突；
//    iStartIn 守衛（WebStart.cpp:1164，golden :4394）擋掉「在 StartFromWeb 裡的告警上按 START」的遞迴，與 golden 相同。
// =============================================================================
#include "Interface/InterfaceSYS.h"   // SendCommand_ESD / ESD_SYSTEM_START（golden note.cpp:3677）
void W906_AlarmStopLikeGolden(const char* code)
{
    extern void StopAllMotor(bool bIndexCanStop);                               // Motor/myGALILmotor.cpp（golden Motor/myGALILmotor.cpp:4712）
    if (AnsiString(code ? code : "") != "WAR1635")                              // golden note.cpp:795  Steven 20220309 : 避免Galil Command Error時, 不能Alarm
        StopAllMotor(true);                                                     // golden note.cpp:796（golden 的 StopAllMotor() 預設參數即 true）
    else
        StopAllMotor(false);                                                    // golden note.cpp:798
    SoftStop    = false;                                                        // golden note.cpp:799
    SoftStart   = false;                                                        // golden note.cpp:800
    SystemStart = false;  { extern void W906_MotorAccessOnAlarm(const char*); W906_MotorAccessOnAlarm(code); }   /*AI(W906-W4D) 20260925: NB2 R23 W4C-4 golden uMotorTest Timer1Timer fNote->fShow -> HOME/LoopMove up; 1203 axes opened by the EastSun monitor are not reached by StopAllMotor, stop them too (WebMotorAccess.h MotorAccessOnAlarm)*/   // golden note.cpp:801
    std::printf("  [alarm-stop] %s: StopAllMotor(%s); SoftStop=SoftStart=SystemStart=0 (golden note.cpp:795-801)\n",
                code ? code : "", (AnsiString(code ? code : "") != "WAR1635") ? "true" : "false");
}
bool W906_AlarmAnswerStartLikeGolden(const char* qidStr)
{
    if (!g_webMain) {
        std::printf("query qid=%s: START answer -- no TfMainWeb, StartFromWeb not reachable\n", qidStr ? qidStr : "");
        return false;
    }
    { extern bool W906_MotorAccessStartBlocked(std::string&); std::string tw; if (W906_MotorAccessStartBlocked(tw)) { std::printf("query qid=%s: START answer REFUSED -- %s\n", qidStr ? qidStr : "", tw.c_str()); return false; } }  /*AI(W906-W5-b) 20260925: 覆核 W5B-R5 同 start.run：手動教導中不啟動*/  const bool ok = g_webMain->StartFromWeb(AnsiString("fNote::Start 1"));     // golden note.cpp:3669  JerryYang 20241118 : fix
    fMain->bStartKeyPressCheck=true;                                            // golden note.cpp:3670  Steven 20110309 : for 安全門未關按Start時IndexArm會先動作   //AI(W906-INDEXZ-1203) 20260930: review round 2 A -- live now (was a note 「不做」: the TfMain facade had no member): forms/fMain.h has bStartKeyPressCheck and its one reader, csystem.cpp GATE G22 (golden DoSystem's one-shot VS/SP after START), is lifted
    //   (the old note said: 它唯一的讀者 csystem.cpp:16717 在 `#if 0 // GATE G22` 裡 —— 缺相依，不是選擇; both dependencies exist since 20260930; WebStart.cpp's golden :5927 setter is live too)
    SendCommand_ESD(ESD_SYSTEM_START);                                          // golden note.cpp:3677  Steven 20140722
    std::printf("query qid=%s: START answer -> StartFromWeb(\"fNote::Start 1\")=%s, ESD_SYSTEM_START (golden TfNote::Start)\n",
                qidStr ? qidStr : "", ok ? "true" : "false");
    return ok;
}

// =============================================================================
//  AI(W906-YESNO) 20260925: ShowMyMessageBox_YES_NO 在「真的去問操作員」時，照 golden 做的機台狀態那幾行。
//    呼叫者只有 ForwardShowMyMessageBoxYesNo（本檔前段）；沒有人能回答時（hook 回 0）兩支都不跑。
//
//  golden 的順序：ShowMyMessageBox_YES_NO 本體 :1020 StopAllMotor → :1049-1053 RENESAS FT-CT →
//    ShowModal → FormShow（:64-383）→ 操作員按 pnlYes/pnlNo（pnlYesClick :1132-1142：iValue=Tag; Close()）
//    → FormClose（:385-442）→ 回到本體 :1056-1061 記 MyDBIProcess → return iValue。
//
//  **只翻狀態，不翻畫面**：FormShow／FormClose 裡的 Width/Height/Caption/Visible/Color、
//  SECS EventReport、OEE、MTI/PTI DoCommandBuffer 都是 golden 的 VCL 畫面或另一個子系統，
//  網頁那一側（dialog-page.js）已經畫了框。下面沒翻的狀態行各自寫理由。
// =============================================================================
#include "mysensor.h"                                                           // Sen[] / TMySensor::Enable（golden FormClose :390）
extern bool       bSupplyNewICTrayPause;                                        // asendic_Loader.cpp:326（golden mymessbox.cpp:49 extern）
extern bool       bHangTimePause;                                               // atester.cpp:181（golden mymessbox.cpp:48 extern）
extern TQPF_Timer hAutoCleanHangUp;                                             // csystem_predicates.cpp:476（golden csystem.cpp:157）
extern TQPF_Timer tGalilTwoYMoveDelay;                                          // Motor/myGALILmotor.cpp:5432（golden mymessbox.cpp:384 extern）

void W906_YesNoShowLikeGolden()
{
    extern void StopAllMotor(bool bIndexCanStop);                               // Motor/myGALILmotor.cpp（golden Motor/myGALILmotor.cpp:4712）
    StopAllMotor(true);  { extern void W906_MotorAccessOnAlarm(const char*); W906_MotorAccessOnAlarm("YESNO"); }   /*AI(W906-MODAL-SAFETY) 20260925: golden uMotorTest.cpp:921-926／:977-980 MyMessageBox->fShow 時放棄 HOME／LoopMove；1203 監看器開的軸 StopAllMotor 停不到，比照告警那側 :5629*/                                                         // golden mymessbox.cpp:1020 `StopAllMotor();`（預設參數 true）
    // golden :1022 `bDisableKeypad=true;` 不翻：它只給 MyMessageBox::Timer1Timer（:560-566）判斷實體 Pause/Retry/Skip
    //   鍵能不能關框；移植樹等待期間 tick 停住、沒有人掃面板鍵，這個旗標沒有讀者（全樹 0 個定義）。
    // SAFETY-GATE(W906-YESNO-FTCT) golden :1049-1053（RogerYang 20251002 瑞薩FT-CT 解綁manualstart）整段不翻 —— 缺相依兩個：
    //   (1) :1051 `bAutoRestartAfterFTCTAlarm=false;` —— cmydef.h:5867 有宣告，但定義 cmydef.cpp:6172 在 `#if 0 // TODO(W6)`
    //       裡（20260925 build_ship 實測 undefined reference）；全樹沒有讀者能連得起來。
    //   (2) :1052 `fMain->RENESAS_Server->FTCTManStartUnlock();` —— 移植樹的 fMain->RENESAS_Server 是替身類別
    //       TfMainRENESASServer（forms/fMain.h:122-140），沒有這個方法；真的 TRENESAS_Server::FTCTManStartUnlock
    //       （Automation/uRENESAS_Server.cpp:3032）沒有掛在 fMain 上。
    //   只在 TestIF_File.bRENESAS_EnableFTCT（瑞薩 FT-CT）時走到。後果：不送 FT-CT 的 manual-start 解綁命令。
#if 0
    if (TestIF_File.bRENESAS_EnableFTCT == true)                                // golden :1049
    {
        bAutoRestartAfterFTCTAlarm = false;                                     // golden :1051
        fMain->RENESAS_Server->FTCTManStartUnlock();                            // golden :1052
    }
#endif // SAFETY-GATE(W906-YESNO-FTCT)
    // ↓ ShowModal 觸發的 TMyMessageBox::FormShow（golden :64-383）的狀態行
    bAlarmReset = false;                                                        // golden :70   Steven 20140905
    if (!iUnLoaderCount)                                                        // golden :302  Jou 20150721
    {
        SystemStart = false;                                                    // golden :304
        SoftStart   = false;                                                    // golden :305  ChungHung 20110829
        if (SystemInitialOK == true)                                            // golden :306  Steven 20121102 : 馬達還沒好,不可以下命令
            StopAllMotor(true);                                                 // golden :307
        bSupplyNewICTrayPause = true;                                           // golden :308
        bHangTimePause        = true;                                           // golden :309  Steven 20090827 : Hang Up dectector
    }
    lHandlerStopTime.LatchCycleTime(true);                                      // golden :353  jou 2014-09-21 Show Handler Stop Time
    // 不翻的 FormShow 狀態行（理由：golden 的 ShowModal 期間 Timer1Timer 仍呼叫 fMain->Timer1Timer 等主迴圈，
    //   這幾個旗標是給「框開著、主迴圈還在轉」那段時間用的；移植樹的等待迴圈把整個 tick 停住，沒有那段時間）：
    //   :67-69 bPauseInMotor/bPauseOutMotor/bPauseSortMotor=true —— ⚠ bPauseSortMotor 在移植樹**沒有任何清除點**
    //          （git grep `bPauseSortMotor\s*=` 只有 Motor/mymotor.cpp:97 的定義），照抄會變成永久暫停；
    //   :336   bAlarmBuzzer=!bDisableAlarmBuzzer —— [AI(W906-MODAL-WAKE) 20260926：已由檔尾 W906ModalWaitScope(1) 翻；第 9 條讓等待迴圈每圈 tick
    //          FlushFlag＋DoSystemMessage，框開著時就叫，面板 Alarm Reset 在是／否框也能消音（W906_ModalWaitTick）⇒ 原本「框關了才叫」的理由不成立]
    //   :338-342 bLifterPause[]/bAuto2Pause[]=true —— 同上，給 modal 期間仍在轉的 magazine 迴圈用。
    //   ⓘ 告警那一側（W906_AlarmStopLikeGolden）同樣只翻了 note.cpp:795-801 的停機核心，這裡是同一個取捨。
}

void W906_YesNoCloseLikeGolden(const char* s1, const char* s3, int iValue)
{
    // ↓ TMyMessageBox::FormClose（golden :385-442）的狀態行 —— pnlYesClick（:1132-1142）的 Close() 觸發
    // golden :387 `bNeedBigMsg=false;`（JerryYang 20250120）不翻 —— 缺相依：cmydef.h:5916 有宣告，定義 cmydef.cpp:6257
    //   在 `#if 0 // TODO(W6)` 裡（20260925 build_ship 實測 undefined reference）。它只決定 golden 框的大小。
    if (!iUnLoaderCount)                                                        // golden :388  Jou 20150721
    {
        if (Sen[SnSafeDoor3].Enable == false)                                   // golden :390
        {
            bIsTestSitICFallDown = false;                                       // golden :392
            bContactCTOverCHK    = false;                                       // golden :393
        }
    }
    else
    {
        iUnLoaderCount = 0;                                                     // golden :398
    }
    // golden :403-406 OEE fProductionInfo->bFTPError=false、:410-422 SECS EventReport(DoPause) 不翻：
    //   OEE 與 SECS 的事件報告在移植樹走各自的通道（SecsTagPublish），MyMessageBox 關閉事件沒有接上 —— 缺相依。
    SendCommand_ESD(ESD_SYSTEM_STOP);                                           // golden :429
    bEnterTestIF = true;                                                        // golden :430  ChungHung 20121221
    bAlarmReset  = false;                                                       // golden :431  Steven 20140905
    lHandlerStopTime.LatchCycleTime(true);                                      // golden :433  jou 2014-09-21
    hAutoCleanHangUp.SetSecAndOn(Prod.iHangupMaxTime);                          // golden :440  Steven 20220823 : 機台有暫停就要重新計算
    tGalilTwoYMoveDelay.SetSecAndOn(60);                                        // golden :441
    // ↓ 回到 ShowMyMessageBox_YES_NO 本體（golden :1056-1061）
    AnsiString S1(s1 ? s1 : "");
    if (iValue == 1)                                                            // golden :1056
        S1 = S1 + "  Yes";                                                      // golden :1057
    else if (iValue == 2)                                                       // golden :1058
        S1 = S1 + "  No";                                                       // golden :1059
    MyDBIProcess("Message", S1, AnsiString(s3 ? s3 : ""));                      // golden :1061
    // golden :1062-1066 pnl*->Visible／bDisableKeypad=false 是畫面與上面那個沒有讀者的旗標，不翻。
}

// AI(W906-A4-4) 20260924: golden 的 FormClose 在 VCL 裡一定會跑（main.cpp:11927 寫 Program Close=1）；
//   wb_serve 是主控台程式，Ctrl-C／關閉主控台視窗／登出／關機會直接結束行程，走不到上面 server.Stop 那一段。
//   這個 handler 只補「寫 1」這一件事，然後回 FALSE 交給預設處理（照舊結束行程）—— 不在這裡做關卡等收尾，
//   那些仍然只有 --seconds 到期的正常路徑會做（與同事 wb_publish 相同，見上面 Pci1203MonitorDisable 的註解）。
//   ⚠ handler 跑在系統另開的執行緒上；寫 Gerneral.ini 與 tick 執行緒同時寫的機率極低，golden 的 FormClose 也不加鎖。
BOOL WINAPI W906_ConsoleCtrl(DWORD ev)
{
    if (ev == CTRL_C_EVENT || ev == CTRL_BREAK_EVENT || ev == CTRL_CLOSE_EVENT ||
        ev == CTRL_LOGOFF_EVENT || ev == CTRL_SHUTDOWN_EVENT)
        { extern bool W906_ConsoleQuit(unsigned long); if (W906_ConsoleQuit((unsigned long)ev)) return TRUE;  WriteIniDataGeneral("Record", "Program Close", 1);  extern void W906_HmiShellQuit(); W906_HmiShellQuit(); }   /*AI(W906-HMI-SHELL) 20260930: console closed at once -> the HMI program window goes too*/   //AI(W906-D012) 20260929 [W906] Q44 A3 (supersedes the "only writes 1" note above): the machine is stopped first, like golden FormClose. 1st Ctrl-C/Break -> TRUE, the main loop sends the stop, then takes the normal close path (save, golden shutdown, Program Close=1, server.Stop); 2nd Ctrl-C/Break -> write 1 and exit at once (today's behaviour); X / logoff / shutdown -> wait (max 3.5 s) until the main loop has SENT the stop (no read-back), then write 1 and exit. Body + per-event notes: FileRW/MainClose.cpp W906_ConsoleQuit (this handler runs on a system thread; the vendor API is tick-thread only, so it only sets flags) | AI(W906-D012-A3W) 20260930: logoff / shutdown (5 / 6, and the hidden window's WM_ENDSESSION) now send the stop only -- W906_ConsoleQuit returns true for them, so neither Program Close=1 nor the MES2109 row is written (golden VCL 6 does not run FormClose on logoff / shutdown); X and the 2nd Ctrl-C unchanged
    return FALSE;
}
//AI(W906-MERGE-56bbf785) 20260926: both sides appended here -- machine: the output-first service, the api cache and the publish
//  yield hook (IOWEB-P25 / P6 / LAT-1); laptop: the MyMessageBox web host, SMM-IO and BOOTSUM (AI(W906-SMM) block below).
//  Kept both, machine block first; they share no names (each has its own anonymous-namespace helpers).
// ===========================================================================
//  AI(W906-IOWEB-P25) 20260925: OUTPUT FIRST -- the output commands' dispatch, and the
//  service that runs them ahead of everything else on the tick thread.
//
//  User 20260925 (real HT9050): 「輸出一定是要0延遲 這是工業機台 不能延遲的 讀取IO狀態是次要的
//  輸出一定要第一優先發出去 所以你迴圈價購必須更改」, then 「IO 輸出還是有延遲 我希望可以做到幾乎是瞬發」.
//  The click log (runcfg/logs/io_click_timing.csv) put the wait in three places: the 1203 Poll
//  (up to 5221 ms, its configuration sweeps -- removed in Pci1203Monitor.cpp), the api cache
//  rebuild (20-160 ms, now after the drain) and the Sleep(2) slices (now an event wait).
//
//  W906_ServiceOutputs() runs ONLY the output commands -- io.btnPanelClick, pci1203.do.setBit,
//  pci1203.do.setByte -- and is called
//    * right after the loop's sleep, and right after PumpTick,
//    * from INSIDE TPci1203Monitor::Poll(), between two stations / axes / DI bytes / DO bytes /
//      SDO reads (the monitor's yield hook -- W906_OutputYieldHook), and
//    * between the api cache's JSON builds, and between blocks of the tag publish (AI(W906-LAT-1) 20260925, W906_PublishYieldHook).
//  All on this one tick thread: the vendor API stays single-threaded (Pci1203Control.h), and
//  PumpTick never runs inside a Poll, so the engine globals the click touches are not in use.
//
//  ORDER: the queue is drained into g_carry; an output runs if every command before it in
//  g_carry is also an output or one of four that change no machine state (sys.ping, log.event,
//  ui.windows.put, cfg.resync -- these are left in place, not run). The first other command
//  stops the scan: it and everything after it wait for the main drain, in arrival order. So an
//  output never overtakes a start / stop / rescan / axis command, and outputs never reorder
//  among themselves. ⚠ pci1203.card.rescan (Close + Open under Poll's feet) is never run here.
// ===========================================================================
extern bool W906_IoBtnPanelClick(const std::string&, int, std::string&, unsigned long long);   // JsonBridge/IoBtnPanelClick.cpp
extern void W906_IoTiming(int);                                                                // same file: click-latency phases
extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool);          // WebMotorAccessLive.cpp (motor.stop = stop-only)

static bool W906_IsOutputCmd(const std::string& c)
{
#ifdef WB_PUMP_1203_CONTROL
    //AI(W906-IOWEB-P25c) 20260925: + the three STOP commands (laptop review Q2-1): a stop must not wait
    //  behind a Poll either. They still obey the barrier rule in W906_ServiceOutputs, so a stop never
    //  overtakes a jog / move / home queued before it ("stop first, move after" is the dangerous order).
    return c == "io.btnPanelClick" || c == "pci1203.do.setBit" || c == "pci1203.do.setByte"
        || c == "pci1203.ax.stop" || c == "pci1203.ax.emgStop" || c == "motor.stop";
#else
    (void)c;
    return false;              // no command surface in this binary: nothing to run early
#endif
}

static bool W906_IsInertCmd(const std::string& c)
{
    return c == "sys.ping" || c == "log.event" || c == "ui.windows.put" || c == "cfg.resync";
}

#ifdef WB_PUMP_1203_CONTROL
void W906_DispatchIoClick(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)
{
    W906IoClickGuardScope ioClickGuard(wc); if (ioClickGuard.busy()) { server.CompleteCommand((unsigned long long)wc.id, false, ioClickGuard.why()); return; }  std::string ioAck;   // AI(W906-R0927-9) 20260927 (Steven 團隊)：RULINGS_20260927 #9 同一顆鈕＋同一個 down 值 400 ms 內只算一次（本體 WebCmdGuard.cpp；Jimmy 08:3x 同意在此同行插入）
    const int ioDown = !wc.hasValue ? -1
                     : wc.value.isInt()    ? (int)wc.value.asInt(-1)
                     : wc.value.isDouble() ? (int)wc.value.asDouble(-1.0)
                     : wc.value.isBool()   ? (wc.value.asBool() ? 1 : 0) : -1;
    const bool ioOk = W906_IoBtnPanelClick(wc.hasTag ? wc.tag : std::string(), ioDown, ioAck,
                                           (unsigned long long)wc.pushedUs);
    server.CompleteCommand((unsigned long long)wc.id, ioOk, ioAck);
    if (ioOk) g_apiCacheDirty = true;   // /api/struct/io/runtime shows the read-back (was: rebuilt right here)
}

//  The pci1203.* branch of main()'s dispatch chain, moved here unchanged (AI(W906-BU-C4) 20260918).
//AI(W906-IOWEB-P25c) 20260925: outputsOnly = called by the output-first service (possibly from inside
//  Poll). The name was already matched exactly; this checks the PARSED kind as a second wall, so no
//  other pci1203.* command (card.rescan = Close+Open under Poll's feet, Fn008 with its 6 s Sleep loop)
//  can ever run there (laptop review Q1-1a).
void W906_Dispatch1203Ex(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc, bool outputsOnly);
void W906_Dispatch1203(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)
{
    W906_Dispatch1203Ex(server, wc, false);
}
void W906_Dispatch1203Ex(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc, bool outputsOnly)
{
    ht9045::TPci1203Control* ctl = ht9045::Pci1203Control();
    if (ctl == 0) {
        server.CompleteCommand((unsigned long long)wc.id, false,
                               "1203 control is not armed in this binary "
                               "(built without HAVE_PCI1203, or Open() refused)");
    } else {
        ht9045::Pci1203WireCmd w;
        w.name = wc.cmd;
        w.id   = (long long)wc.id;
        if (wc.hasTag) w.target = wc.tag;
        if (wc.hasValue) {
            if (wc.value.isString()) {
                w.hasStr = true;  w.str = wc.value.asString();
            } else if (wc.value.isInt()) {
                // asDouble() returns its fallback for a non-Double,
                // so an integer payload has to be widened by hand.
                w.hasNum = true;  w.num = (double)wc.value.asInt();
            } else if (wc.value.isDouble()) {
                w.hasNum = true;  w.num = wc.value.asDouble();
            }
        }
        ht9045::Pci1203Cmd cmd;
        std::string why1203;
        if (!ht9045::Pci1203ParseWire(w, cmd, why1203)) {
            ctl->NoteRefusal(w.id, w.name, why1203);
            std::printf("%s REFUSED: %s\n", wc.cmd.c_str(), why1203.c_str());
            server.CompleteCommand((unsigned long long)wc.id, false, why1203);
        } else if (outputsOnly && cmd.kind != ht9045::kCmdDoSetBit && cmd.kind != ht9045::kCmdDoSetByte &&
                   cmd.kind != ht9045::kCmdAxStop && cmd.kind != ht9045::kCmdAxEmgStop) {
            server.CompleteCommand((unsigned long long)wc.id, false,
                                   "refused: only DO writes and axis stops may run in the output-first service");
        } else {
            //AI(W906-Q34-8) 20260923: ⚠ 重新掃描（pci1203.card.rescan）走這裡：
            //  Execute() → Pci1203Control.cpp 的 kCmdCardRescan → TPci1203Monitor::
            //  Rescan() → Close() + Open()。Open() 裡的 1203COLD-1 重試迴圈在
            //  Acm_DevOpen 回 0x83000002 時會等最多 WB_1203_OPEN_WAIT_SEC 秒
            //  （預設 90，上限 3600），而這裡就是主迴圈那條唯一的執行緒 ——
            //  等待期間 PumpTick、Poll、其他命令的 drain 與 ack、快照發布全部停住，
            //  這個命令自己的 ack 也要等 Open() 返回才發（網頁端的 ack 逾時可能先到）。
            //  使用者 20260923 裁決第 3 條：知道會這樣，照翻，不另開偏離。
            //  只在有 SDK（HAVE_PCI1203）的機台上成立；這台筆電 ctl 是 null，
            //  根本走不到這一行。
            const ht9045::Pci1203CmdResult r = ctl->Execute(cmd);  { extern void W906_EngineRouteForeignStop(const ht9045::Pci1203Cmd&, const ht9045::Pci1203CmdResult&); W906_EngineRouteForeignStop(cmd, r); }   //AI(W906-ENG1203) 20260929: review HIGH-1 -- the pci1203 page's ax.stop / ax.emgStop of an engine-claimed axis is told to the engine motor route (the glue: EtherCAT/Pci1203GaliRoute.cpp EOF since AI(W906-INDEXZ-1203) 20260930, was WebMotorAccessLive.cpp EOF; it also tells the Index Z1 Gali route, as an operator stop = golden ST); no route installed = nothing
            const std::string tail = r.why.empty() ? std::string()
                                                   : ("  -- " + r.why);
            std::printf("%s: accepted=%d issued=%d ret=0x%08lX %s%s\n",
                        wc.cmd.c_str(), (int)r.accepted, (int)r.issued,
                        r.ret, r.wouldCall.c_str(), tail.c_str());
            webbridge::JsonWriter aw;
            aw.BeginObject();
            aw.Key("accepted").Bool(r.accepted);
            aw.Key("issued").Bool(r.issued);
            aw.Key("ret").Number((wb_int64)r.ret);
            aw.Key("dryRun").Bool(ctl->IsDryRun());
            aw.Key("wouldCall").String(r.wouldCall);
            if (!r.why.empty()) aw.Key("why").String(r.why);
            aw.EndObject();
            server.CompleteCommand((unsigned long long)wc.id,
                                   r.accepted,
                                   r.accepted ? aw.Str() : r.why);
        }
    }
}
#endif

void W906_ServiceOutputs(int phase)
{
    static bool busy = false;
    if (busy || g_pumpQueue == 0 || g_modalServer == 0) return;
    if (!g_carryRunnable && g_pumpQueue->size() == 0) return;          // the common case: one lock, nothing to do
    busy = true;
    //AI(W906-IOWEB-P25c) 20260925: carry cap (laptop review Q2-3). If the carry already holds a command
    //  that stops the scan (not an output, not one of the four inert ones), nothing behind it may run
    //  early anyway -- so leave the rest in the BOUNDED queue (64; "command queue full" keeps its
    //  meaning) instead of moving it into this unbounded deque.
    bool barrier = false;
    for (std::size_t b = 0; b < g_carry.size() && !barrier; ++b)
        barrier = !W906_IsOutputCmd(g_carry[b].cmd) && !W906_IsInertCmd(g_carry[b].cmd);
    if (!barrier) {
        std::vector<webbridge::WebCommand> fresh;
        g_pumpQueue->drain(fresh);
        for (std::size_t k = 0; k < fresh.size(); ++k) g_carry.push_back(fresh[k]);
    }
    g_carryRunnable = false;
    bool ran = false;
    for (;;) {
        //  Re-scanned from the front every time: a dispatch may itself touch g_carry
        //  (a blocking alarm inside it would run the modal wait, which takes g_carry).
        std::size_t k = 0;
        while (k < g_carry.size() && !W906_IsOutputCmd(g_carry[k].cmd) && W906_IsInertCmd(g_carry[k].cmd)) ++k;
        if (k >= g_carry.size() || !W906_IsOutputCmd(g_carry[k].cmd)) break;
        const webbridge::WebCommand wc = g_carry[k];
        g_carry.erase(g_carry.begin() + (long)k);  { extern bool W906_Q44CmdRefused(const std::string&, const std::string&, std::string*); std::string q44Why; if (W906_Q44CmdRefused(wc.cmd, (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(), &q44Why)) { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, q44Why); continue; } }   //AI(W906-D012) 20260929 [W906] Q44 A2: same allow-list as the main dispatch head (this output-first path bypasses it): while closing io.btnPanelClick / pci1203.do.* are refused here, the stops still run
        if (!ran && phase > 0) W906_IoTiming(phase + 1);   // close the phase this interrupts (click log)
        ran = true;
        WdMark2(phase == 3 ? "1203 Poll > dispatch(output-first): " : "dispatch(output-first): ", wc.cmd.c_str());
#ifdef WB_PUMP_1203_CONTROL
        if (wc.cmd == "io.btnPanelClick") W906_DispatchIoClick(*g_modalServer, wc);
        else if (wc.cmd == "motor.stop") {              // stop-only by construction (stopOnly = true)
            std::string msAck;
            const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(),
                                                   (long long)wc.id, msAck, true);
            g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck);
        }
        else                              W906_Dispatch1203Ex(*g_modalServer, wc, true);
#endif
    }
    if (ran) {
        g_outputsServed = true;
        if (phase > 0) W906_IoTiming(phase);               // and reopen it
        WdMark(phase == 3 ? "1203 Poll" : "tick");
    }
    busy = false;
}

//  Installed on the monitor (TPci1203Monitor::SetYieldHook) right after it is enabled.
void W906_OutputYieldHook()
{
    W906_ServiceOutputs(3);
}

// ===========================================================================
//  AI(W906-IOWEB-P6) 20260924: TICK-THREAD CACHE FOR THE FOUR MONITOR-READING /api/struct ENDPOINTS
//
//  WebBridgeServer runs HTTP routes on its SOCKET thread (WebBridge/WebBridgeServer.h:199),
//  but IoSchemaJson / IoRuntimeJson / MotorSchemaJson / MotorRuntimeJson iterate the 1203
//  monitor's samples (mon->di(i) / do_(i) / axis(i)), which the TICK thread owns: Poll()
//  rewrites them every 200 ms, and pci1203.card.rescan -> Rescan() -> Close()/Open() re-assigns
//  the sample vectors (Pci1203Monitor.cpp dis.assign / dos.assign). A page polling
//  /api/struct/io/runtime every 500 ms while an operator presses 重新掃描 could therefore read
//  freed memory. ChanIoPoints.cpp itself says it is meant to be called single-threaded ("UI tick").
//  The field tree never had this path: there the monitor is only touched by the publisher's loop.
//
//  So the four bodies are built ONLY on the tick thread (main(): once before server.Start(),
//  then after every Poll() and on every pump beat) and the socket thread copies a finished
//  string under a CRITICAL_SECTION -- same shape as g_wdLock above. JSON contract unchanged.
//  config endpoints (IoConfigJson / MotorConfigJson) read only HSys tables and stay as they were.
// ===========================================================================
void W906_OpLogMotorRuntime(const std::string& json);  namespace {   //AI(W906-OPLOG) 20260928: declared at global scope HERE (a block-scope extern inside the unnamed namespace would name a different function); defined at the end of the file
CRITICAL_SECTION g_apiCacheLock;
bool             g_apiCacheReady = false;
std::string      g_apiCache[4];

std::string W906_ApiCacheBuild(int which)
{
    switch (which) {
    case 0: return ht9045::sjson::IoSchemaJson();
    case 1: return ht9045::sjson::IoRuntimeJson();
    case 2: return ht9045::sjson::MotorSchemaJson();
    case 3: return ht9045::sjson::MotorRuntimeJson();
    }
    return std::string("{\"error\":\"unknown cache slot\"}");
}

//AI(W906-IOWEB-P25) 20260925: (1) the two SCHEMA strings depend only on whether the monitor
//  exists and on its DI / DO / axis counts (ChanIo.cpp IoSchemaJson, ChanMotor.cpp
//  MotorSchemaJson), so they are rebuilt only when that key changes (open, rescan) -- not on
//  every Poll. (2) Between two builds the output-first service runs, so a click that arrives
//  while the runtime JSON is being generated does not wait for it.
std::string g_apiSchemaKey;

void W906_ApiCacheRefresh()
{
    if (!g_apiCacheReady) return;
    char key[96];
#ifdef INSTALL_1203_MONITOR
    {
        ht9045::TPci1203Monitor* m = ht9045::Pci1203Monitor();
        std::snprintf(key, sizeof(key), "%d/%d/%d/%d", m ? 1 : 0,
                      m ? m->diCount() : -1, m ? m->doCount() : -1, m ? m->axisCount() : -1);
    }
#else
    std::snprintf(key, sizeof(key), "no-monitor");   // header not included in this binary; the schemas never change
#endif
    const bool schemas = (g_apiSchemaKey != key);
    std::string fresh[4];
    bool built[4] = { false, false, false, false };
    for (int i = 0; i < 4; ++i) {
        if ((i == 0 || i == 2) && !schemas) continue;
        fresh[i] = W906_ApiCacheBuild(i);   // built outside the lock: the socket thread never waits on JSON generation
        built[i] = true;
        W906_ServiceOutputs(5);
    }
    g_apiSchemaKey = key;
    ::EnterCriticalSection(&g_apiCacheLock);
    for (int i = 0; i < 4; ++i) if (built[i]) g_apiCache[i].swap(fresh[i]);
    ::LeaveCriticalSection(&g_apiCacheLock);  if (built[3]) ::W906_OpLogMotorRuntime(g_apiCache[3]);   //AI(W906-OPLOG) 20260928: diff the motor runtime body the page reads (tick thread = the only writer of g_apiCache, so no lock needed to read it here)
}

void W906_ApiCacheInit()
{
    ::InitializeCriticalSection(&g_apiCacheLock);
    g_apiCacheReady = true;
    W906_ApiCacheRefresh();
}

std::string W906_ApiCacheGet(int which)
{
    if (!g_apiCacheReady || which < 0 || which > 3)
        return std::string("{\"error\":\"api cache not ready\"}");
    ::EnterCriticalSection(&g_apiCacheLock);
    const std::string copy = g_apiCache[which];
    ::LeaveCriticalSection(&g_apiCacheLock);
    return copy;
}
}  // namespace

// ===========================================================================
//  AI(W906-LAT-1) 20260925: installed on the tag publish (ht9045::SetPublishYieldHook) before the
//  tick loop. PublishHandlerTags() stages ~7,000 tags after every Poll, beat, drained batch and served
//  output, and had no yield point -- the likeliest owner of the click log's unexplained "其他" (up to
//  43.6 ms, outputlatency report). Same service as the Poll's hook; phase 7 = the publish's mark in
//  the click log, so a click served here is timed as "waited behind the publish", not as "other".
//  ⚠ Consistency: a snapshot built across a yield may show some tags from before an output and some
//  from after it (e.g. fAllMotorHome / HomeFlag reset by SwMotorRelay). It is committed as it is and
//  REPAIRED by the next publish, which follows at once: g_outputsServed is cleared before the publish
//  and set by the output, and the loop's sleep does not wait while it is set. The alternative --
//  discarding the half-built map and staging again -- doubles the cost of exactly the publishes an
//  operator is waiting on. Nothing here can re-enter the publish (PublishHandlerTags refuses nesting),
//  and pci1203.card.rescan never runs as an output (W906_Dispatch1203Ex outputsOnly), so the monitor
//  samples the publish holds pointers into cannot be re-allocated under it.
//  At the end of the file so no line above moves.
// ===========================================================================
void W906_PublishYieldHook()
{
    W906_ServiceOutputs(7);
}

// =============================================================================
//  AI(W906-SMM) 20260925: golden TMyMessageBox（mymessbox.cpp）的網頁宿主。
//
//  Steven 20260925 指示（優先處理）：C++ 的 ShowMyMessage 在網頁上看不到，操作員會漏看
//  golden 的警告。golden = HT9011UC_Code_V3.33.912.0_20260908_Jimmy\mymessbox.cpp（cp950），
//  以下行號都指那一份。RULINGS_20260925 第 2 條（阻塞框照 golden 跳框並等待）、第 10 條
//  （YES/NO 照 golden 跳網頁對話框等操作員回答）。
//
//  ## 斷點（20260925 實測）
//  (1) C++ 端：ForwardShowMyMessage 只廣播 {"type":"modal","title":"Message","text":"S1 | S2"}，
//      **不寫 Message 信箱、不等回答**（golden 是 ShowModal，:879）。
//  (2) 網頁端：沒有任何頁面處理 WS 的 modal 訊框；background.html 的 dialog-bridge.js 只讀
//      Message 信箱，而 C++ 從來不寫它 ⇒ 兩條路都是空的。
//  (3) 回答方向：ht9045_dialog_host.js 只認 WS query 訊框（告警），訊息通道的回應
//      一律被判成「沒有待答的 query」。
//
//  ## 通道（本次接上的）
//    C++ → 網頁：信箱 JSON\runtime\Message-dialog-request.json（＋js 墊片，同告警的 MailboxPut），
//                契約 web\JSON\Dialog-bridge-contract.json 的 showMyMessage；另廣播舊形狀的 WS modal
//                訊框當「立刻去讀信箱」的觸發（ht9045_modal.js 用）。
//    網頁 → C++：WS `dialog.response`，tag = requestId（"msg-<N>"），value = 動作名
//                （OK／PAUSE：pnlPause；YES／NO：pnlYes／pnlNo；ACKNOWLEDGE：不停機頁的確認鍵）。
//                dialog.response 本來就豁免單一操作員權杖（WebBridgeServer.cpp:1446）。
//    阻塞型：本宿主在 tick 執行緒上等（同 ForwardShowErrorMessage 的 pump；golden ShowModal 期間
//            狀態機也停），答完把信箱退役成 idle（DialogMailboxRetire 同理，否則 F5 會再彈）。
//    非阻塞（ShowUnloaderTrayMessage）：貼上就回；回答在主 dispatch（或任何一個 pump）收，
//            照 golden FormClose 收尾。
//
//  ## 刻意沒做（報告裡逐條列）
//    * golden Timer1Timer（:542-776）在框開著時掃實體面板鍵（ScanPannelKey :566，Pause／Retry／Skip
//      一鍵關框）與自動關框條件（安全門 :596-642、Bin 資料 :644-663）—— 本宿主等回答時不掃 IO
//      （與告警 pump 相同），只能從網頁按。
//    * ServoOff InArm（:851-868）：會真的 ServoOff 馬達，而它的復原點 ainarm9045.cpp:906 仍是   //AI(W906-FLOW-5) 20260929: DONE now -- translated at the end of this file (W906_MbServoOffInArmLikeGolden); the recovery point named below is live since FLOW-2
//      `#if 0 // TODO(W7)` —— 只做一半會讓 InArm 一直 ServoOff。不做，印出來。   //AI(W906-FLOW-5) 20260929: (history -- see the line above)
//    * SECS GEM（bSECSGEMAlarm 的密碼／條碼關框 :471-526、EventReport :414-426、moSecsGem）、
//      客戶專屬（MTI/PTI automation :347-351、OEE :358-362/:407-410、PANTHER :379-382、
//      RENESAS FTCT :873-878/:535-538、Greatek 底色 :940-944）：RULINGS 第 25 條「客戶專屬條件
//      先暫時跳過，註記就好」。
//    * TMyMessageBoxShim::Close()（acatchtray_shims.cpp:97）是空殼，呼叫端的
//      `if(MyMessageBox->Visible) MyMessageBox->Close();` 關不到網頁上的框（不在本次可改清單）。
// =============================================================================
#include "mymessbox_web.h"
#include "mymessbox_shim.h"          // MyMessageBox->fShow／Visible（golden 的 fShow；讀者 ckernel.cpp:1629/1974、Command.cpp:9889/16336、csystem.cpp:30297…）
#include "mysensor.h"                // Sen[SnSafeDoor3]（golden FormClose :394）

extern bool       bHangTimePause;          // atester.cpp:181（golden mymessbox.cpp:48 同一句 extern）
extern bool       bSupplyNewICTrayPause;   // asendic_Loader.cpp:326（golden mymessbox.cpp:49）
extern TQPF_Timer tGalilTwoYMoveDelay;     // Motor/myGALILmotor.cpp:5432（golden mymessbox.cpp:388）
extern TQPF_Timer hAutoCleanHangUp;        // csystem.h:291 —— 只要這一個，不為它 include csystem.h
extern bool       bPauseInMotor, bPauseOutMotor;   // Motor/myGALILmotor.h:81
extern bool       bPauseSortMotor;         // Motor/mymotor.h:394
// ⚠ 下面三個宣告**一定要在檔案層級**：寫在 anonymous namespace 裡的函式內的區域 extern 會綁到那個
//   無名命名空間（本檔 :129 踩過同一個坑），連結時變成 undefined reference。
void StopAllMotor(bool bIndexCanStop);                                          // Motor/myGALILmotor.cpp（golden Motor/myGALILmotor.cpp:4712）
void DoAvoidIndexMotorFallDown();                                               // csystem.cpp:20013（csystem.h:120）  //AI(W906-MERGE-56bbf785) 20260926: MbWait (below, inside the unnamed namespace) now calls it like the other two modal waits do (:537 / :806). Declared at FILE scope for the reason in the ⚠ above: a block-scope extern inside the unnamed namespace would bind to it and fail to link
bool W906_MsgBoxModelessAnswer(const webbridge::WebCommand& wc);                // 本檔下方
bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool);   // WebMotorAccess（告警 pump 同一支）
void ShowUnloaderTrayMessage(AnsiString S1, AnsiString S2);                     // canary_support.h:57（同 :152 的本地宣告慣例，逐字同簽名）
// AI(W906-SMM-IO) 20260925: 框開著時的實體 IO 解除（RULINGS 第 42 條；本體在檔尾 AI(W906-SMM-IO) 區塊）
std::string W906MbIoDismiss();
void W906_DialogCloseRequest(const char* channel, const std::string& requestId, unsigned long long requestSeq,
                             const std::string& actionName, int actionCode, const char* inputName);
bool W906_SimDiCommand(const webbridge::WebCommand& wc);
unsigned long long g_mbBlockingSeq = 0;   // MbPost 記下的「目前阻塞框」信箱 seq（Dialog-close-request 的 target.requestSeq）
bool W906_IoPageNoGuards();   //AI(W906-IO-NOGUARD) 20260929: JsonBridge/IoBtnPanelClick.cpp; declared at global scope HERE (the MyMessageBox wait below is inside the unnamed namespace). Occupies a blank line
namespace {

// ---- golden TMyMessageBox 的成員與 mymessbox.cpp 檔頭全域 ------------------------
struct W906MbModeless {                    // 目前顯示中的非阻塞框（ShowUnloaderTrayMessage）；qid 空 = 沒有
    std::string qid, s1, s2;
    bool        nonStop;
};
W906MbModeless     s_mbModeless = { std::string(), std::string(), std::string(), false };
unsigned long long s_mbNextQid           = 1;      // "msg-<N>"：與告警 qid（純數字）分開，兩邊的等待迴圈不會互收
int                s_iValue              = 0;      // golden mymessbox.cpp:51 iValue
bool               s_bDisableKeypad      = false;  // golden :52（給實體面板鍵用；本宿主不掃鍵，保留狀態照 golden 設）
bool               s_bDisableAlarmBuzzer = false;  // golden TMyMessageBox::bDisableAlarmBuzzer（ctor :61 false；只有 ShowMyMessagePWD 設 true，本樹未移植）

// 這棵樹的字串字面值是 UTF-8，從檔案讀進來的是 ANSI（cp950）—— 兩種都可能進到 S1/S2。
std::string MbU8(const char* p)
{
    const std::string s(p ? p : "");
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return webbridge::SanitizeToUtf8(s);
    std::wstring w((size_t)wn, L'\0');
    ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int un = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string u((size_t)(un > 0 ? un : 0), '\0');
    if (un > 0) ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &u[0], un, NULL, NULL);
    return u;
}

std::string MbEsc(const std::string& s) { return w906dlg::JsonEscape(s); }
const char* MbB(bool b) { return b ? "true" : "false"; }

std::string MbIsoNow()
{
    SYSTEMTIME t; ::GetLocalTime(&t);
    char b[40];
    std::snprintf(b, sizeof(b), "%04u-%02u-%02uT%02u:%02u:%02u.%03u",
                  (unsigned)t.wYear, (unsigned)t.wMonth, (unsigned)t.wDay,
                  (unsigned)t.wHour, (unsigned)t.wMinute, (unsigned)t.wSecond, (unsigned)t.wMilliseconds);
    return b;
}

// value 的形狀與告警相同："<ACTION>" 或 "<ACTION>:<pressedButton>"；回傳大寫的 ACTION。
std::string MbAnswerOf(const webbridge::WebCommand& wc)
{
    std::string a = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
    const std::string::size_type colon = a.find(':');
    if (colon != std::string::npos) a = a.substr(0, colon);
    for (size_t i = 0; i < a.size(); ++i) if (a[i] >= 'a' && a[i] <= 'z') a[i] = (char)(a[i] - 'a' + 'A');
    return a;
}

bool MbIn(const std::string& a, const char* const* opts, int n)
{
    for (int i = 0; i < n; ++i) if (a == opts[i]) return true;
    return false;
}

// ---- 一個框在網頁上長什麼樣子（golden 的表單狀態，逐項對到 mymessbox.dfm 的元件）-------
struct MbPanel { bool visible = false; std::string caption; int left = -1; };   // left < 0 = 頁面預設位置
struct MbView {
    std::string function;
    bool        blocking = false, stopAllMotor = true, nonStop = false, pauseHandler = false;
    std::string s1, s2, s3; bool ok = false, servoOff = false, servoOffInArmXY = false;   //AI(W906-FLOW-5) 20260929: + servoOffInArmXY: set when this ShowMyMessage really servo-offed the In Arm X/Y (golden mymessbox.cpp:832-849); MbPost reports it
    std::string primary, secondary, sub;   // lblMainMsg / lblChineseMsg / lblSubMsg
    bool        changeForm = false;        // golden bChangeForm（S2 有 ';'，:1057/:313-320）
    MbPanel     pause, yes, no, alarmReset;
    std::string layout = "normal";         // FormShow 的尺寸分支（:118-301）
    bool        employeeIdCheck = false;
};

std::string MbPanelJson(const MbPanel& p)
{
    char left[24];
    if (p.left >= 0) std::snprintf(left, sizeof(left), "%d", p.left);
    else             std::snprintf(left, sizeof(left), "null");
    return std::string("{\"visible\":") + MbB(p.visible) + ",\"caption\":\"" + MbEsc(p.caption) +
           "\",\"left\":" + left + "}";
}

// golden FormShow :118-301 的尺寸分支。只回名稱（網頁照名稱排版），並照 golden 清掉用過的一次性旗標。
std::string MbLayoutOnShow()
{
    if ((IniConfig.bC08_SocketSensor && bIsSocketSensor) || bIsContactforce || bBigMyMessage)   // :118-121（&& 先於 ||，照 golden）
    {
        if (bBigMyMessage) { bBigMyMessage = false; return "bigLeft"; }                         // :141-150
        return "big";                                                                            // :123-140
    }
    if (bSECSGEMAlarm && CUSTOMER_CODE == CC_KYEC_LEE) return "secsFull";                        // :152-232
    // :233-258 bNeedBigMsg：定義在 cmydef.cpp:6257 仍在 `#if 0 // TODO(W6)` 裡（連結不到），而且全樹沒有任何移植碼寫它
    //   ⇒ 執行期恆 false，這個分支到不了。解閘 cmydef.cpp 那一行時把這裡換回 `if (bNeedBigMsg) return "full";`。
    if (bShowIndexMotorError) { bShowIndexMotorError = false; return "big"; }                    // :259-280
    return "normal";                                                                             // :281-301
}

// 信箱一則（契約 showMyMessage；欄位名沿用 web\JSON\Message-dialog-request.json 樣本，另加 panels 等）。
bool MbPost(const MbView& v, const std::string& qid)
{
    if (g_dialogMailboxDir.empty()) return false;
    const unsigned long long seq = ++g_dialogSeq;
    if (v.blocking) g_mbBlockingSeq = seq;                                      // AI(W906-SMM-IO): IO 解除時 Dialog-close-request 要對到這一則
    char seqs[32]; std::snprintf(seqs, sizeof(seqs), "%llu", seq);
    std::string j;
    j.reserve(2048);
    j += "{\"schemaVersion\":\"1.0.0\",\"channel\":\"show-my-message\",\"seq\":"; j += seqs;
    j += ",\"requestId\":\"" + MbEsc(qid) + "\",\"state\":\"pending\",\"requestedAt\":\"" + MbIsoNow() + "\"";
    j += ",\"function\":\"" + v.function + "\",\"blocking\":" + MbB(v.blocking) + ",\"nonStop\":" + MbB(v.nonStop);
    j += ",\"arguments\":{\"s1\":\"" + MbEsc(v.s1) + "\",\"s2\":\"" + MbEsc(v.s2) + "\",\"s3\":\"" + MbEsc(v.s3) +
         "\",\"ok\":" + MbB(v.ok) + ",\"servoOff\":" + MbB(v.servoOff) + ",\"code\":\"\"}";
    j += ",\"display\":{\"primaryText\":\"" + MbEsc(v.primary) + "\",\"secondaryText\":\"" + MbEsc(v.secondary) +
         "\",\"subText\":\"" + MbEsc(v.sub) + "\",\"buttonLabel\":\"" + MbEsc(v.pause.caption) +
         "\",\"showAlarmReset\":" + MbB(v.alarmReset.visible) + ",\"buttonEnabled\":" + MbB(v.pause.visible) +
         ",\"layout\":\"" + v.layout + "\",\"changeForm\":" + MbB(v.changeForm) + ",\"mainColor\":null" +
         ",\"panels\":{\"pnlPause\":" + MbPanelJson(v.pause) + ",\"pnlYes\":" + MbPanelJson(v.yes) +
         ",\"pnlNo\":" + MbPanelJson(v.no) + ",\"pnlAlarmReset\":" + MbPanelJson(v.alarmReset) + "}}";
    // haltHandler：FormHS->bHaltHandler 在本樹沒有成員（forms/fHS.h 只有註解），HS 功能未移植 ⇒ 執行期就是 false。
    j += std::string(",\"runtime\":{\"secsGemAlarm\":") + MbB(bSECSGEMAlarm) + ",\"haltHandler\":false,\"systemInitialOK\":" +
         MbB(SystemInitialOK) + ",\"employeeIdCheck\":" + MbB(v.employeeIdCheck) + "}";
    j += std::string(",\"requestedSideEffects\":{\"pauseHandler\":") + MbB(v.pauseHandler) + ",\"stopAllMotor\":" +
         MbB(v.stopAllMotor) + ",\"servoOffInArmXY\":" + MbB(v.servoOffInArmXY) + "}";   //AI(W906-FLOW-5) 20260929: was the constant false (the servo-off was not done); now whether this box did it (field of the showMyMessage mailbox, informational: no page acts on it)
    j += ",\"auth\":{\"required\":false,\"kind\":\"none\",\"level\":null,\"title\":\"Password\",\"prompt\":\"\","
         "\"userIdRequired\":true,\"defaultUserId\":null},\"error\":null}";
    const bool ok = w906dlg::MailboxPut(g_dialogMailboxDir, "Message-dialog-request", j);
    // WS：舊形狀不變（title/text），網頁的 ht9045_modal.js 拿它當「馬上去讀信箱」的觸發。
    if (g_modalServer) {
        std::string text = v.s1;
        if (!v.s2.empty()) { text += " | "; text += v.s2; }
        g_modalServer->PostModal("Message", text);
    }
    return ok;
}

// 答完／關掉之後把信箱改回 idle（樣板逐字，只換 seq）。不做的話 F5 會再彈一次。
void MbRetire()
{
    if (g_dialogMailboxDir.empty()) return;
    std::string idle = w906dlg::seed::kSeedCompact_Message_dialog_request;
    const std::string::size_type p = idle.find("\"seq\":0,");
    if (p != std::string::npos) {
        char b[48]; std::snprintf(b, sizeof(b), "\"seq\":%llu,", ++g_dialogSeq);
        idle.replace(p, 8, b);
    }
    if (!w906dlg::MailboxPut(g_dialogMailboxDir, "Message-dialog-request", idle))
        std::printf("  ⚠ Message 信箱退役失敗 —— 下次重整會再彈一次\n");
}

// ---- golden TMyMessageBox::FormShow（:65-386）的機台副作用 ------------------------------
void MbFormShow()
{
    bPauseInMotor   = true;                                                     // :68
    bPauseOutMotor  = true;                                                     // :69
    bPauseSortMotor = true;                                                     // :70  RogerYang 20250510 Add for 9046AU
    bAlarmReset     = false;                                                    // :71
    ExString        = "";                                                       // :72
    if (!iUnLoaderCount)                                                        // :303  Jou 20150721 : 重新啟用功能
    {
        SystemStart = false;                                                    // :305
        SoftStart   = false;                                                    // :306
        if (SystemInitialOK == true) StopAllMotor(true);                       // :307-308（golden 預設參數 true）
        bSupplyNewICTrayPause = true;                                           // :309
        bHangTimePause        = true;                                           // :310
    }
    bAlarmBuzzer = !s_bDisableAlarmBuzzer;                                      // :337
    for (int i = 0; i < 7; i++) { bLifterPause[i] = true; bAuto2Pause[i] = true; }   // :339-343
    MyMessageBox->fShow   = true;                                               // :345
    MyMessageBox->Visible = true;                                               // VCL：Show／ShowModal 之後 Visible 為真
    lHandlerStopTime.LatchCycleTime(true);                                      // :354
    bSendRealCCDSendStart = true;                                               // :378
}

// ---- golden TMyMessageBox::FormClose（:389-446）--------------------------------------
void MbFormClose()
{
    // :391 bNeedBigMsg=false —— 同 MbLayoutOnShow 的註解：定義還在 cmydef.cpp 的 #if 0 裡，恆 false，不用清。
    if (!iUnLoaderCount)                                                        // :392
    {
        if (Sen[SnSafeDoor3].Enable == false)                                   // :394
        {
            bIsTestSitICFallDown = false;                                       // :396
            bContactCTOverCHK    = false;                                       // :397
        }
    }
    else
        iUnLoaderCount = 0;                                                     // :402
    MyMessageBox->fShow   = false;                                              // :412
    MyMessageBox->Visible = false;
    SendCommand_ESD(ESD_SYSTEM_STOP);                                           // :433
    bEnterTestIF = true;                                                        // :434
    bAlarmReset  = false;                                                       // :435
    lHandlerStopTime.LatchCycleTime(true);                                      // :437
    bHasQwertyKeyForm = false;                                                  // :442
    bHasPasswordForm  = false;                                                  // :443
    hAutoCleanHangUp.SetSecAndOn(Prod.iHangupMaxTime);                          // :444  Steven 20220823 : 機台有暫停就要重新計算
    tGalilTwoYMoveDelay.SetSecAndOn(60);  W906_ModalOutputsRefresh();           // :445  // AI(W906-MODAL-WAKE) 20260926: 關框後立刻重算塔燈／音樂（檔尾）
}

// golden pnlPauseClick（:448-539）在 Close() 之前的那幾行（bSECSGEMAlarm 分支見檔頭「沒做的」）。
void MbPauseClickPre()
{
    bStartMoveSpeed = false;                                                    // :455  Steven 20231018 : Fixed for G14
    if (bSECSGEMAlarm)
        std::printf("  ⚠ [MyMessageBox] bSECSGEMAlarm=1：golden :471-526 要 DoPassword_MBox／Barcode_Reader 才關得掉，"
                    "本樹未移植（SECS GEM 未移植）→ 照一般訊息關閉\n");
    s_iValue = 0;                                                               // :528
    if (TestIF_File.bIndexCycleTimeMonitor == true && bResetflag == false)      // :529
        bResetflag = true;                                                      // :531
}

// 把目前的非阻塞框當成被 Close()（golden ShowMyMessage :808-820、YES_NO :1036-1039、
// ShowUnloaderTrayMessage 的呼叫端先 Close 再 Show）。keepUnloader：呼叫端已先設好
// 新的 iUnLoaderCount（Close 要在它之前，這裡事後才做，所以要還原）。
void MbCloseModeless(const char* why, bool keepUnloader)
{
    if (s_mbModeless.qid.empty()) return;
    const int keep = iUnLoaderCount;
    MbFormClose();
    if (keepUnloader) iUnLoaderCount = keep;
    ht9045::sjson::ClearAlarm(s_mbModeless.qid, "", "", 0);
    std::printf("  [MyMessageBox] 非阻塞框 %s 關閉（%s）\n", s_mbModeless.qid.c_str(), why);
    s_mbModeless.qid.clear();
}

// ---- 等回答（golden ShowModal）—— 形狀同 ForwardShowErrorMessage 的 pump ---------------
//   只收 tag==qid 的 modal.answer／dialog.response，其餘一律 modal-pending；
//   例外同告警 pump：cfg.resync（純讀）、motor.stop（停機不能被擋），另加非阻塞框的回答。
std::string MbWait(const std::string& qid, const char* const* opts, int nOpts)
{
    WdMark2("modal wait (MyMessageBox, needs a browser answer): ", qid.c_str());
    std::vector<webbridge::WebCommand> local;  W906ModalWaitScope wakeScope(2);   // AI(W906-MODAL-WAKE) 20260926: ShowMyMessage 框（fShow／蜂鳴器由 MbFormShow 設）—— 只做 tick 與開瀏覽器
    for (;;) {
        if (g_carry.empty()) g_pumpQueue->waitForPush(100);  DoAvoidIndexMotorFallDown();  W906_ModalWaitTick(2);  { ::W906_TesterCommPoll(); }  /* AI(W906-GB-P3) 20260926: H5 -- golden ShowModal keeps handling WM_COPYDATA from the bridge */   //AI(W906-MERGE-56bbf785) 20260926: laptop wrote Sleep(100); machine P25c's event wait, same as ForwardShowErrorMessage's (an answer / motor.stop wakes it at once, still bounded to 100 ms)  //AI(W906-MERGE-56bbf785) 20260926: + DoAvoidIndexMotorFallDown on every pass, at the same place as the YES/NO wait (:806). golden runs it from TMyMessageBox::Timer1Timer (mymessbox.cpp:650) during ShowModal, and ShowMyMessage is that same form; the port's tick is stopped during this wait, so without it EMG / power loss / Index Z servo-off would not lock the Index brake while the box is open. Declared at file scope (:6519)
        local.clear();
        W906_TakeCarry(local);  g_pumpQueue->drain(local);   //AI(W906-MERGE-56bbf785) 20260926: machine IOWEB-P25 -- every drain takes g_carry first (see the same line in ForwardShowErrorMessage / ForwardShowMyMessageBoxYesNo)
        for (size_t i = 0; i < local.size(); ++i) {
            const webbridge::WebCommand& wc = local[i];
            if ((wc.cmd == "modal.answer" || wc.cmd == "dialog.response") && wc.hasTag && wc.tag == qid) {
                const std::string a = MbAnswerOf(wc);
                if (!MbIn(a, opts, nOpts)) {
                    std::printf("  [MyMessageBox] %s: 回答 %s 不是這一則的選項 -> not an offered option\n", qid.c_str(), a.c_str());
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "not an offered option");
                    continue;
                }
                if ((a == "OK" || a == "PAUSE") && bWaitSecsGemReply) {        // golden pnlPauseClick :450-453
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                                   "bWaitSecsGemReply: golden pnlPauseClick ignores the key while waiting for EAP");
                    continue;
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true, std::string());
                WdMark("serve loop (MyMessageBox answered)");
                if (i + 1 < local.size()) { g_carry.insert(g_carry.begin(), local.begin() + (i + 1), local.end()); g_carryRunnable = true; }   //AI(W906-MERGE-56bbf785) 20260926: same fix as the ShowErrorMessage / YES-NO answers (:627 / :826) -- local[i+1..] was dropped at this return (never run, never acked); back to the front of g_carry (machine IOWEB-P25 carry, drained first by the main loop), in arrival order
                return a;
            }
            if (W906_MsgBoxModelessAnswer(wc)) continue;
            if (W906_SimDiCommand(wc)) continue;                                // AI(W906-SMM-IO): 模擬 DI＝實體鍵，框開著也要收（golden 的面板鍵在 modal 期間仍有效）
            if (wc.cmd == "cfg.resync") {
                unsigned long long since = 0;
                if (wc.hasValue) {
                    if (wc.value.isInt())         since = (unsigned long long)wc.value.asInt(0);
                    else if (wc.value.isDouble()) since = (unsigned long long)wc.value.asDouble(0.0);
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true, ht9045::sjson::ConfigResyncJson(since));
            } else if (wc.cmd == "ui.windows.put") { std::string whyW; const std::string frameW = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); if (!frameW.empty() && ht9045::WebWindowRegistryPut(wc.connId, frameW, whyW)) g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"accepted\":true,\"duringModal\":true}"); else g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "ui.windows.put: " + (frameW.empty() ? std::string("value must be the frame JSON string") : whyW));   //AI(W906-MERGE-56bbf785) 20260926: machine MT-FIX1's modal exception (window registry must not go stale during a long wait -> fMotorTest/fTeach "open" -> MainProc paused); laptop says this pump's exceptions are "同告警 pump", so it takes this one too
            } else if (wc.cmd == "motor.stop") {
                std::string msAck;
                const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(),
                                                       (long long)wc.id, msAck, true);
                g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck);
            } else if (wc.cmd == "io.btnPanelClick" && ::W906_IoPageNoGuards()) { ::W906_DispatchIoClick(*g_modalServer, wc); } else {   //AI(W906-IO-NOGUARD) 20260929: IO click served during the MyMessageBox wait too
                if (wc.cmd == "modal.answer" || wc.cmd == "dialog.response")   // 答錯對象（例如舊的一則）—— 印出來，現場查「按了沒反應」用
                    std::printf("  [MyMessageBox] %s 等待中收到給 %s 的回答 -> modal-pending\n", qid.c_str(),
                                wc.hasTag ? wc.tag.c_str() : "(no tag)");
                g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "modal-pending");
            }
        }
        // AI(W906-SMM-IO) 20260925: golden Timer1Timer（mymessbox.cpp:558-663）—— 面板鍵／開門等條件成立 = golden 的 Close()
        const std::string io = W906MbIoDismiss();
        if (!io.empty()) {
            std::printf("  [MyMessageBox] %s 由 IO 解除：%s（golden mymessbox.cpp Timer1Timer → Close()）\n", qid.c_str(), io.c_str());
            WdMark("serve loop (MyMessageBox closed by IO)");
            return "IO:" + io;
        }
    }
}

// 能不能真的等？不能的時候照舊只顯示（不等）—— 等一個送不回來的答案 = 整台 wb_serve 永久停住。
//   read-only（沒帶 --allow-cmd）：網頁的 dialog.response 在 socket 層就被拒。
const char* MbCannotWait()
{
    if (!g_modalServer)                return "no server";
    if (!g_pumpQueue)                  return "no command queue";
    if (g_dialogMailboxDir.empty())    return "no mailbox";
    if (g_modalServer->IsReadOnly())   return "bridge is read-only (--allow-cmd off)";
    return 0;
}

std::string MbNewQid()
{
    char b[32]; std::snprintf(b, sizeof(b), "msg-%llu", s_mbNextQid++);
    return b;
}

} // namespace

// =============================================================================
//  ShowMyMessage（golden mymessbox.cpp:780-883，含 ShowModal 期間的 FormShow／pnlPauseClick／FormClose）
// =============================================================================
static void ForwardShowMyMessage(const char* s1, const char* s2);   // 本檔上方；用它的位址判斷「有沒有收集器」
void W906MbShowMyMessage(const char* s1, const char* s2, const char* s3, bool Ok, bool bServoOff)
{
    // 0. WebBuilder／WebSmartDiag 正在收這一次請求的訊息（舊 hook 被換成它們的 CaptureHook）：
    //    訊息會跟著 ack 的 messages 回到那一頁（C 路設計，頁面自己顯示），這裡不再跳框、不停機。
    if (W906_ShowMyMessage_Hook != &ForwardShowMyMessage) {
        std::printf("  [MyMessageBox] 頁面動作的訊息收集中 —— 走 ack.messages，不另跳框：%s\n", s1 ? s1 : "");
        return;
    }
    AnsiString S1(s1 ? s1 : ""), S2(s2 ? s2 : ""), S3(s3 ? s3 : "");
    if (InitialOK == false)                                                     // :784  Ifor 20151230 : fixed
    {
        MyDBIProcess("Exception", S1, S3);                                      // :786
        std::printf("  [MyMessageBox] InitialOK=0：照 golden :784-788 只記錄 Exception，不顯示\n");
        return;
    }
#ifdef SOFT_SIMULTE
    if (S1.Pos(" port error") != 0)                                             // :789-794
        return;
#endif
    bHandlerPause      = true;                                                  // :795  JerryYang 20200407
    iHandlerStartCount = 0;                                                     // :796
    const bool haltHandler = false;                                             // FormHS->bHaltHandler：本樹沒有（見 MbPost）
    if (W906_FormShowing("MyMessageBox", MyMessageBox->fShow) == true && iUnLoaderCount == 0)                     // :804  已有框在（且不是不停機的那種）  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
    {
        std::printf("  [MyMessageBox] 已有框顯示中（fShow=1、iUnLoaderCount=0）：照 golden :804-807 不再顯示：%s\n", s1 ? s1 : "");
        return;
    }
    else if (W906_FormShowing("MyMessageBox", MyMessageBox->fShow) == true && iUnLoaderCount != 0)                // :808  不停機的框在：先關掉再顯示  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
    {
        // :810-815 KYEC 在 Close 前後備份 bSECSGEMAlarm —— 那是因為 golden FormClose 的 SECS 段會清它（:419）；
        //   本宿主的 FormClose 不做 SECS 段（見檔頭），旗標不會被動到，不需要備份。
        MbCloseModeless("superseded by ShowMyMessage (golden :808-820)", false);
    }
    if (SystemInitialOK == true) StopAllMotor(true);                           // :830-831

    MbView v;
    v.function = "ShowMyMessage";
    v.blocking = true; v.nonStop = false; v.pauseHandler = true;
    // stopAllMotor：阻塞型的狀態機一定停在這裡（SystemStart=false＋tick 凍在等待迴圈）。SystemInitialOK=0 時 golden
    //   不下 StopAllMotor 只是因為馬達還不能收命令 —— 沒有東西在動。報 false 會被 ht9045_nonstop_alarm.js 路由到
    //   「機台未停機，仍在運轉」那一頁，那是謊話。
    v.stopAllMotor = true;
    v.s1 = MbU8(s1); v.s2 = MbU8(s2); v.s3 = MbU8(s3); v.ok = Ok; v.servoOff = bServoOff;
    const bool secs = bSECSGEMAlarm || haltHandler;
    v.primary   = secs ? std::string() : v.s1;                                  // :837-846（SECS 走 moSecsGem，頁面依 runtime 顯示）
    v.secondary = v.s2;                                                         // :848
    v.changeForm = false;
    v.employeeIdCheck = bEnableEmployeeIDCheck;
    v.pause.visible = !bEnableEmployeeIDCheck;                                  // :870-871
    v.pause.caption = (Ok || secs) ? "OK" : "Pause";                            // :832-835
    v.pause.left = -1;
    v.yes.visible = false; v.yes.caption = "Yes"; v.yes.left = -1;
    v.no.visible  = false; v.no.caption  = "No";  v.no.left  = -1;
    v.alarmReset.visible = secs; v.alarmReset.caption = "AlarmReset"; v.alarmReset.left = -1;   // :798-801
    MyDBIProcess("Message", S1, S3);                                            // :849
    { extern bool W906_MbServoOffInArmLikeGolden(bool bServoOff); v.servoOffInArmXY = W906_MbServoOffInArmLikeGolden(bServoOff); }   // :851-868  //AI(W906-FLOW-5) 20260929: golden ShowMyMessage In Arm servo-off (906 golden mymessbox.cpp:832-849 = V912 :851-868) translated, body at the end of this file. The reason it was skipped is dead: the recovery point ainarm9045.cpp:906 is live (FLOW-2), and csystem.cpp DoServoOn (In Arm arm :22032-22117, live) re-energises X/Y, jogs back and clears the flag on the next START
    //  was (print-only stand-in): if (IniConfig.bAlarmNeedServoOff && bServoOff)                              // :851-868 —— 不做，見檔頭 std::printf("  ⚠ [MyMessageBox] golden 會 ServoOff InArm X/Y（:851-868）；本樹不做 —— 復原點 ainarm9045.cpp:906 仍是 #if 0\n");   //AI(W906-FLOW-5) 20260929: kept as a comment so no line moves

    const char* why = MbCannotWait();
    const std::string qid = MbNewQid();
    v.layout = MbLayoutOnShow();
    MbFormShow();                                                               // ShowModal -> OnShow
    if (why) {
        v.blocking = false;
        const bool posted = MbPost(v, qid);
        std::printf("  [MyMessageBox] %s 顯示但不等回答（%s；信箱=%s）：%s\n", qid.c_str(), why, posted ? "yes" : "no", s1 ? s1 : "");
        // 沒有人能回答 ⇒ 等同立刻被關掉：照 FormClose 收尾，不讓 fShow 卡住下一則。
        MbFormClose();
        return;
    }
    if (!MbPost(v, qid)) {
        std::printf("  ⚠⚠ [MyMessageBox] %s 信箱寫入失敗 —— 網頁看不到，**不等**（等下去就是永久停住）：%s\n", qid.c_str(), s1 ? s1 : "");
        MbFormClose();
        return;
    }
    ht9045::sjson::EmitAlarm(ht9045::sjson::kSrcShowMyMessage, v.s1, 0, 0, qid, true);
    std::printf("  [MyMessageBox] %s ShowMyMessage -- waiting (button=%s)：%s\n", qid.c_str(), v.pause.caption.c_str(), s1 ? s1 : "");
    std::fflush(stdout);

    static const char* const kOpts[] = { "OK", "PAUSE" };                        // 只有 pnlPause 一顆能關（pnlAlarmReset 不關框，:1289-1304）
    const std::string a = MbWait(qid, kOpts, 2);

    MbRetire();
    if (a.compare(0, 3, "IO:") == 0)                                            // AI(W906-SMM-IO): Timer1Timer 的 Close() 只走 FormClose，不經 pnlPauseClick
        W906_DialogCloseRequest("show-my-message", qid, g_mbBlockingSeq, "PAUSE", 0, a.c_str() + 3);
    else
        MbPauseClickPre();                                                      // pnlPauseClick :450-532
    MbFormClose();                                                              // Close() -> OnClose
    ht9045::sjson::ClearAlarm(qid, a, "pnlPause", 0);
    std::printf("  [MyMessageBox] %s answered %s -> ShowMyMessage returns (golden :879)\n", qid.c_str(), a.c_str());
    std::fflush(stdout);
}

// =============================================================================
//  ShowUnloaderTrayMessage（golden mymessbox.cpp:930-955）—— MyMessageBox->Show()，不等
// =============================================================================
void W906MbShowUnloaderTray(const char* s1, const char* s2)
{
    const std::string u1 = MbU8(s1), u2 = MbU8(s2);
    // golden :932 `if(MyMessageBox->fShow==true) return;`。
    //   ⚠ 決斷：已有非阻塞框時**換成新的**而不是丟掉。golden 的呼叫端 16 處裡有 11 處
    //   （acatchtray.cpp:5511/5563/5650/5721、asendic_Color.cpp:1511、asendic_Empty.cpp:1174、
    //   Motor/mymotor.cpp 五處）在呼叫前先 `if(MyMessageBox->Visible) MyMessageBox->Close();`，
    //   所以 golden 實際上是「關舊的、顯示新的」；本樹那個 Close() 是空殼（acatchtray_shims.cpp:97），
    //   照 :932 丟掉就會讓第二則警告永遠不出現。asendic_Auto.cpp 五處沒有先關 —— golden 在那裡是丟掉，
    //   這裡改成顯示（寧可多顯示一則，不要漏）。同一段文字重複呼叫（連續移動中每圈檢查）不重貼。
    if (!s_mbModeless.qid.empty())
    {
        if (s_mbModeless.s1 == u1 && s_mbModeless.s2 == u2 && s_mbModeless.nonStop == (iUnLoaderCount != 0))
            return;                                                             // 同一則還在畫面上
        MbCloseModeless("replaced by a newer ShowUnloaderTrayMessage (callers Close() first)", true);
    }
    else if (W906_FormShowing("MyMessageBox", MyMessageBox->fShow) == true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
    {
        return;                                                                 // :932（阻塞框顯示中 —— 單執行緒下到不了，留著照 golden）
    }
    AnsiString S1(s1 ? s1 : "");
    MyDBIProcess("Message", S1, AnsiString(""));                                // :938

    MbView v;
    v.function = "ShowUnloaderTrayMessage";
    v.blocking = false; v.pauseHandler = false;
    v.nonStop = (iUnLoaderCount != 0);                                          // FormShow :303 —— 呼叫端事先設好（「必須不為0 Handler才不停機」）
    v.stopAllMotor = !v.nonStop;
    v.s1 = u1; v.s2 = u2; v.s3 = ""; v.ok = false; v.servoOff = false;
    v.primary = u1; v.secondary = u2; v.changeForm = false;
    v.employeeIdCheck = bEnableEmployeeIDCheck;
    v.pause.visible = true; v.pause.caption = "PAUSE"; v.pause.left = -1;       // :934  JerryYang 20160811 顯示PAUSE
    v.yes.visible = false; v.yes.caption = "Yes"; v.yes.left = -1;
    v.no.visible  = false; v.no.caption  = "No";  v.no.left  = -1;
    v.alarmReset.visible = false; v.alarmReset.caption = "AlarmReset"; v.alarmReset.left = -1;   // :937
    v.layout = MbLayoutOnShow();
    MbFormShow();                                                               // Show() -> OnShow（iUnLoaderCount!=0 就不停機）

    const std::string qid = MbNewQid();
    const char* why = MbCannotWait();
    const bool posted = MbPost(v, qid);
    if (!posted || (why && std::string(why).find("read-only") != std::string::npos)) {
        // 看不到或答不回來：不留一個永遠關不掉的 fShow（它會擋住之後每一則 ShowMyMessage，:804）。
        std::printf("  ⚠ [MyMessageBox] %s 非阻塞框%s（%s）：%s\n", qid.c_str(),
                    posted ? "已顯示但答不回來" : "信箱寫入失敗", why ? why : "mailbox", u1.c_str());
        MbFormClose();
        return;
    }
    s_mbModeless.qid = qid; s_mbModeless.s1 = u1; s_mbModeless.s2 = u2; s_mbModeless.nonStop = v.nonStop;
    ht9045::sjson::EmitAlarm(ht9045::sjson::kSrcShowMyMessage, u1, 0, 0, qid, false);
    std::printf("  [MyMessageBox] %s ShowUnloaderTrayMessage (%s, iUnLoaderCount=%d) -- not waiting：%s\n",
                qid.c_str(), v.nonStop ? "NonStop" : "stops machine", iUnLoaderCount, u1.c_str());
    std::fflush(stdout);
}

// 非阻塞框的回答。任何一個 dispatch（主迴圈、告警 pump、訊息 pump）都先問這一支；
// 回 true = 這一則是它的（已回 ack）。
bool W906_MsgBoxModelessAnswer(const webbridge::WebCommand& wc)
{
    if (!(wc.cmd == "modal.answer" || wc.cmd == "dialog.response")) return false;
    if (s_mbModeless.qid.empty() || !wc.hasTag || wc.tag != s_mbModeless.qid) return false;
    const std::string a = MbAnswerOf(wc);
    static const char* const kOpts[] = { "OK", "PAUSE", "ACKNOWLEDGE" };        // pnlPause（:934 PAUSE）或不停機頁的確認鍵
    if (!g_modalServer) return false;
    if (!MbIn(a, kOpts, 3)) {
        g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "not an offered option");
        return true;
    }
    if (bWaitSecsGemReply) {                                                    // golden pnlPauseClick :450-453
        g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                       "bWaitSecsGemReply: golden pnlPauseClick ignores the key while waiting for EAP");
        return true;
    }
    const std::string qid = s_mbModeless.qid;
    s_mbModeless.qid.clear();
    MbRetire();
    MbPauseClickPre();                                                          // pnlPauseClick :450-532
    MbFormClose();                                                              // Close() -> OnClose（iUnLoaderCount!=0 → 歸 0，:402）
    ht9045::sjson::ClearAlarm(qid, a, "pnlPause", 0);
    g_modalServer->CompleteCommand((unsigned long long)wc.id, true, std::string());
    std::printf("  [MyMessageBox] %s 非阻塞框 answered %s -> FormClose (golden :389-446)\n", qid.c_str(), a.c_str());
    std::fflush(stdout);
    return true;
}

// =============================================================================
//  問答型：ShowMyMessageBox_YES_NO（golden mymessbox.cpp:1029-1089）
// =============================================================================
#if 0   // 合併 main（Steven 20260925 計畫 A）：YES/NO 改用 Jimmy ee5de164 的 ForwardShowMyMessageBoxYesNo
int W906MbAsk(int kind, const char* s1, const char* s2, const char* s3)
{
    if (kind != W906_MB_YES_NO) return -1;
    if (MyMessageBox->fShow == true && iUnLoaderCount == 0)                     // :1032-1035
        return 3;
    else if (MyMessageBox->fShow == true && iUnLoaderCount != 0)                // :1036-1039
        MbCloseModeless("superseded by ShowMyMessageBox_YES_NO (golden :1036-1039)", false);
    StopAllMotor(true);                                                         // :1040（golden 這裡沒有 SystemInitialOK 守衛，照翻）
    s_bDisableKeypad = true;                                                    // :1042

    MbView v;
    v.function = "ShowMyMessageBox_YES_NO";
    v.blocking = true; v.nonStop = false; v.pauseHandler = false; v.stopAllMotor = true;
    v.s1 = MbU8(s1); v.s2 = MbU8(s2); v.s3 = MbU8(s3); v.ok = false; v.servoOff = false;
    v.primary = v.s1;                                                           // :1050
    const std::string::size_type semi = v.s2.find(';');                         // :1052-1067  kevin 20150713
    if (semi != std::string::npos) {
        v.changeForm = true;                                                    // :1057 bChangeForm
        v.secondary  = v.s2.substr(0, semi);                                    // :1059 SubString(1,Lengh-1)
        v.sub        = v.s2.substr(semi + 1);                                   // :1061 SubString(Lengh+1,Lengh1)
    } else {
        v.changeForm = false;
        v.secondary  = v.s2;                                                    // :1066
    }
    v.employeeIdCheck = bEnableEmployeeIDCheck;
    v.alarmReset.visible = false; v.alarmReset.caption = "AlarmReset"; v.alarmReset.left = -1;   // :1044
    v.pause.visible = false; v.pause.caption = "Pause"; v.pause.left = -1;      // :1045
    v.yes.visible   = true;  v.yes.caption   = "Yes";   v.yes.left   = -1;      // :1046（dfm Caption='Yes'，Tag=1）
    v.no.visible    = true;  v.no.caption    = "No";    v.no.left    = -1;      // :1047（dfm Caption='No'，Tag=2）

    const char* why = MbCannotWait();
    if (why) {
        s_bDisableKeypad = false;
        std::printf("  [MyMessageBox] ShowMyMessageBox_YES_NO 無法等回答（%s）-> 回 -1，呼叫端走離線值：%s\n", why, v.s1.c_str());
        return -1;
    }
    const std::string qid = MbNewQid();
    v.layout = MbLayoutOnShow();
    MbFormShow();                                                               // ShowModal -> OnShow
    if (!MbPost(v, qid)) {
        std::printf("  ⚠⚠ [MyMessageBox] %s YES/NO 信箱寫入失敗 —— 不等，回 -1（呼叫端走離線值）：%s\n", qid.c_str(), v.s1.c_str());
        MbFormClose();
        s_bDisableKeypad = false;
        return -1;
    }
    ht9045::sjson::EmitAlarm(ht9045::sjson::kSrcShowMyMessage, v.s1, 0, 0, qid, true);
    std::printf("  [MyMessageBox] %s ShowMyMessageBox_YES_NO -- waiting：%s\n", qid.c_str(), v.s1.c_str());
    std::fflush(stdout);

    static const char* const kOpts[] = { "YES", "NO" };                          // pnlPause 隱藏（:1045），實體鍵 bDisableKeypad
    const std::string a = MbWait(qid, kOpts, 2);

    MbRetire();
    if (a.compare(0, 3, "IO:") == 0) {                                          // AI(W906-SMM-IO): 開門類條件關框 —— golden 只 Close()，iValue 不動（照 golden 回上一次的值）
        W906_DialogCloseRequest("show-my-message", qid, g_mbBlockingSeq, "NO", 0, a.c_str() + 3);
        std::printf("  [MyMessageBox] %s YES/NO 被 IO 條件關掉 —— golden iValue 不變（=%d）\n", qid.c_str(), s_iValue);
    } else
    s_iValue = (a == "YES") ? 1 : 2;                                            // pnlYesClick :1156 iValue=P->Tag（pnlYes Tag=1、pnlNo Tag=2）
    MbFormClose();                                                              // pnlYesClick :1161 Close()
    AnsiString S1(s1 ? s1 : "");
    if (s_iValue == 1)      S1 = S1 + "  Yes";                                  // :1076-1079
    else if (s_iValue == 2) S1 = S1 + "  No";
    MyDBIProcess("Message", S1, AnsiString(s3 ? s3 : ""));                      // :1081
    s_bDisableKeypad = false;                                                   // :1086
    ht9045::sjson::ClearAlarm(qid, a, a == "YES" ? "pnlYes" : "pnlNo", 0);
    std::printf("  [MyMessageBox] %s answered %s -> iValue=%d (golden :1088)\n", qid.c_str(), a.c_str(), s_iValue);
    std::fflush(stdout);
    return s_iValue;                                                            // :1088
}
#endif

// =============================================================================
//  sys.echoModal（探針入口；AI(W906-FW-W5a) 20260819 原本只驅動 ShowMyMessage）
//    tag   ""/"pause"  ShowMyMessage(S1,S2,"",false,false)   按鈕 Pause
//          "ok"        ShowMyMessage(S1,S2,"",true,false)    按鈕 OK
//          "yesno"     ShowMyMessageBox_YES_NO(S1,S2)        ack 帶 returned（1=Yes 2=No）
//          "nonstop"   iUnLoaderCount=8 + ShowUnloaderTrayMessage（golden 呼叫端的「必須不為0」，acatchtray.cpp:5513）
//          "modeless"  iUnLoaderCount=0 + ShowUnloaderTrayMessage（停機但不阻塞，Motor/mymotor.cpp:4305 那一種）
//    value 純文字 = S1；或 JSON 字串 {"s1":"…","s2":"…"}
//  阻塞的三種會卡在這裡直到網頁回答 —— 探針要驗的就是這件事；ack 在答完之後才送。
// =============================================================================
void W906_EchoModal(const webbridge::WebCommand& wc, webbridge::WebBridgeServer& server)
{
    const std::string raw = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
    std::string s1 = raw, s2;
    if (!raw.empty() && raw[0] == '{') {
        if (cJSON* r = cJSON_Parse(raw.c_str())) {
            const cJSON* a = cJSON_GetObjectItemCaseSensitive(r, "s1");
            const cJSON* b = cJSON_GetObjectItemCaseSensitive(r, "s2");
            s1 = (cJSON_IsString(a) && a->valuestring) ? a->valuestring : "";
            s2 = (cJSON_IsString(b) && b->valuestring) ? b->valuestring : "";
            cJSON_Delete(r);
        }
    }
    const std::string variant = wc.hasTag ? wc.tag : std::string();
    int returned = 0;
    if (variant.empty() || variant == "pause" || variant == "ok") {
        ShowMyMessage(AnsiString(s1.c_str()), AnsiString(s2.c_str()), AnsiString(""), variant == "ok", false);
    } else if (variant == "yesno") {
        returned = ShowMyMessageBox_YES_NO(AnsiString(s1.c_str()), AnsiString(s2.c_str()), AnsiString(""));
    } else if (variant == "nonstop" || variant == "modeless") {
        iUnLoaderCount = (variant == "nonstop") ? 8 : 0;
        ShowUnloaderTrayMessage(AnsiString(s1.c_str()), AnsiString(s2.c_str()));
    } else {
        server.CompleteCommand((unsigned long long)wc.id, false,
                               "sys.echoModal: tag must be one of pause|ok|yesno|nonstop|modeless");
        return;
    }
    char ack[160];
    std::snprintf(ack, sizeof(ack), "{\"variant\":\"%s\",\"returned\":%d,\"modeless\":\"%s\"}",
                  variant.empty() ? "pause" : variant.c_str(), returned, s_mbModeless.qid.c_str());
    server.CompleteCommand((unsigned long long)wc.id, true, ack);
}

void W906_MsgBoxHostInstall()
{
    W906_ShowMyMessageEx_Hook         = &W906MbShowMyMessage;
    W906_ShowUnloaderTrayMessage_Hook = &W906MbShowUnloaderTray;
    std::printf("  MyMessageBox 網頁宿主：ShowMyMessage／ShowUnloaderTrayMessage／ShowMyMessageBox_YES_NO -> "
                "信箱 Message-dialog-request（阻塞型照 golden 等 dialog.response）\n");
}

void W906_MsgBoxHostUninstall()
{
    W906_ShowMyMessageEx_Hook         = 0;
    W906_ShowUnloaderTrayMessage_Hook = 0;
}

// =============================================================================
//  AI(W906-SMM-IO) 20260925: 框開著時的實體 IO 解除 —— RULINGS_20260925 第 42 條
//
//  Steven 原話：「這是因為有些訊息的解除需要先開門，然後還需要按K_Skip 或 K_Retry + K_Pause。
//  目前C++版本改成由c++接收IO訊號後，通知html端進行解除」
//
//  golden 在框開著時靠框自己的 Timer 掃 IO：
//    MyMessageBox  mymessbox.cpp Timer1Timer :542-776（10 ms，dfm :210-211）
//    fNote         note.cpp     Timer1Timer :3172 起 → ScanKey :2894-3121
//  wb_serve 的兩個等待迴圈（檔尾 MbWait、上方 ForwardShowErrorMessage 的 pump）每 100 ms 呼叫這裡；
//  條件成立 = 等同操作員按下 → C++ 自己結案，並通知網頁收框：
//    * 信箱退役成 idle（ht9045_modal.js 看到不是這一則就收框，AI(W906-SMM) D21）
//    * 寫 Dialog-close-request（契約 dialogClose：「C++ detects and resolves IO; HTML only closes」；
//      background.html 的 dialog-bridge.js inspectClose() 靠它收框）
//  IO 一律走移植樹現成的讀法：ScanPannelKey（ckernel.cpp:3147，Sen[] → MyLaneIO）、
//  CheckSafeDoorForICFallDown（csystem.cpp:15361）、CheckTestBinData（csystem.cpp:22327）。不自己開卡。
//
//  SOFT_SIMULTE 的測法：sim.di.set（tag=感測器名，例 SnFKPause；value 1/0）把位元寫進模擬 IO 後端
//  （IOBackend.h TSimIOBackend —— SOFT_SIMULTE 建置三路都是它，MyLaneIo.cpp:139／SelectVendorBackends），
//  之後 Sen[].IsOn() 讀回來就是那個值。**只有 SOFT_SIMULTE 建置才有**；出貨組態回拒絕。
// =============================================================================
#include "myswitch.h"                // SW[SwFKAlarmReset]（golden mymessbox.cpp:582-583、note.cpp ScanKey 尾）
#include "mykitsuck.h"               // FTestSuck／BTestSuck（golden mymessbox.cpp:649/657）
#include "MyLaneIo.h"                // MyLaneIO（sim.di.set 寫模擬後端）
#include "forms/fNote.h"             // fNote->IsTestSitICFallDown()（golden note.cpp:2797）

int  ScanPannelKey();                                                           // ckernel.h:92（本檔不 include ckernel.h，同 :152 的本地宣告慣例）
void CheckSafeDoorForICFallDown();                                              // csystem.h:112
bool CheckTestBinData(class TMyKitSuck& Ptr);                                   // csystem.cpp:22327（golden mymessbox.cpp:541 同一句 extern）
bool IsSafeLockCheck();                                                         // csystem.h:119
bool RespondASECom(AnsiString S1);                                              // cpublic.h（canary_support.cpp 的本體）

unsigned long long        g_dialogAlarmSeq = 0;   // DialogMailboxPostAlarm 記下的告警信箱 seq（同上，給告警用）

// Dialog-close-request（契約 dialogClose；樣板 w906dlg::seed::kSeedPretty_Dialog_close_request）
void W906_DialogCloseRequest(const char* channel, const std::string& requestId, unsigned long long requestSeq,
                             const std::string& actionName, int actionCode, const char* inputName)
{
    if (g_dialogMailboxDir.empty()) return;
    const unsigned long long seq = ++g_dialogSeq;
    char b[1400];
    SYSTEMTIME t; ::GetLocalTime(&t);
    char at[40];
    std::snprintf(at, sizeof(at), "%04u-%02u-%02uT%02u:%02u:%02u.%03u", (unsigned)t.wYear, (unsigned)t.wMonth,
                  (unsigned)t.wDay, (unsigned)t.wHour, (unsigned)t.wMinute, (unsigned)t.wSecond, (unsigned)t.wMilliseconds);
    const int n = std::snprintf(b, sizeof(b),
        "{\"schemaVersion\":\"1.0.0\",\"channel\":\"dialog-close\",\"seq\":%llu,\"closeRequestId\":\"close-%llu\","
        "\"state\":\"pending\",\"requestedAt\":\"%s\",\"target\":{\"channel\":\"%s\",\"requestId\":\"%s\",\"requestSeq\":%llu},"
        "\"trigger\":{\"source\":\"io\",\"inputName\":\"%s\",\"detectedAt\":\"%s\"},"
        "\"resolvedAction\":{\"name\":\"%s\",\"code\":%d},\"closeReason\":\"external-io\",\"error\":null}",
        seq, seq, at, channel, w906dlg::JsonEscape(requestId).c_str(), requestSeq,
        w906dlg::JsonEscape(inputName ? inputName : "").c_str(), at, w906dlg::JsonEscape(actionName).c_str(), actionCode);
    if (n < 0 || (size_t)n >= sizeof(b)) { std::printf("  ⚠ Dialog-close-request 被截斷，不寫\n"); return; }
    if (!w906dlg::MailboxPut(g_dialogMailboxDir, "Dialog-close-request", b))
        std::printf("  ⚠ Dialog-close-request 寫入失敗 —— background 的框要等操作員自己按\n");
}

// golden mymessbox.cpp:574-586 / note.cpp ScanKey 尾：Alarm Reset 鍵（只消音、不關框）
static void W906_PanelAlarmReset(const char* where)
{
    bAlarmReset = true;                                                         // :576
    NewRecordProcess("MES2116", "ALARM RESET pressed", where);                  // :577
    bAlarmBuzzer    = false;                                                    // :578
    bLampAlarmReset = false;                                                    // :579
    SECS_GEM_PPMUSIC_CONTROL_flag = false; SECS_GEM_PPSIGNALTOWER_CONTROL_flag = false;   // :580-581（AI(W906-R70) 20260926 YN-4：原本註解說「本樹沒有這兩個旗標」—— 0924 起已有定義，ckernel.cpp:1272／:1275）
    SW[SwFKAlarmReset].Off();                                                   // :582
    SW[SwRKAlarmReset].Off();                                                   // :583
    { extern void W906_EventReportDoAlarmReset(); W906_EventReportDoAlarmReset(); }   // AI(W906-ALMRST-SECS) 20260926: golden mymessbox.cpp:577-578 / note.cpp:3077-3078 `if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.DoAlarmReset);` (CEID 30, both callers of this function) -- body at EOF. Was the comment ":584-585 (V912 numbering) ... SECS 未移植", stale since SECS moved into wb_serve
}

// ---- MyMessageBox：golden Timer1Timer 的解除部分（:558-663）。回傳觸發原因；"" = 沒有 -------------
//   golden 在這些條件成立時呼叫 Close() —— 那是 FormClose，**不是** pnlPauseClick（iValue 不動）。
std::string W906MbIoDismiss()
{
    if (bSECSGEMAlarm == false)                                                 // :558
    {
        // :560-564 iControlPanelMode → fPadInterface->Main232()：uPadInterface 未移植（mysensor.cpp:119 同一個閘）
        const int ret = ScanPannelKey();                                        // :566
        if (ret == SnFKPause || ret == SnFKRetry || ret == SnFKSkip)            // :567
        {
            if (s_bDisableKeypad == false)                                      // :569（YES/NO／LotEnd 會設 true：實體鍵不關框）
                return ret == SnFKPause ? "SnFKPause" : (ret == SnFKRetry ? "SnFKRetry" : "SnFKSkip");   // :571 Close()
        }
        else if (ret == SnFKAlarmReset && bAlarmBuzzer)                         // :574
            W906_PanelAlarmReset("MessageBox_Timer");
    }
    // :589-592 燈號閃爍、:594-595 DoSystemMessage：與解除無關，不在本次範圍
    if (bIsTestSitICFallDownResetHT9045)                                        // :596
    {
        CheckSafeDoorForICFallDown();                                           // :599
        if (bIsTestSitICFallDownResetHT9045 == false) return "bIsTestSitICFallDownResetHT9045";   // :600-601
    }
    if (bAutoCleanCheckOpenDoor)                                                // :604
    {
        CheckSafeDoorForICFallDown();                                           // :607
        if (bAutoCleanCheckOpenDoor == false) return "bAutoCleanCheckOpenDoor"; // :608-609
    }
    if (bChangeCleanPad)                                                        // :612
    {
        CheckSafeDoorForICFallDown();                                           // :615
        if (bChangeCleanPad == false) return "bChangeCleanPad";                 // :616-617
    }
    if (bContactCTOverCHK)                                                      // :620
    {
        CheckSafeDoorForICFallDown();                                           // :623
        if (bContactCTOverCHK == false) return "bContactCTOverCHK";             // :624-625
    }
    if (bIsContactforce)                                                        // :628
    {
        CheckSafeDoorForICFallDown();                                           // :631
        if (bIsContactforce == false) return "bIsContactforce";                 // :632-633
    }
    if (IniConfig.bC08_SocketSensor && bIsSocketSensor)                         // :636
    {
        CheckSafeDoorForICFallDown();                                           // :639
        if (IniConfig.bC08_SocketSensor && bIsSocketSensor == false) return "bIsSocketSensor";   // :640-641
    }
    if (IniConfig.bI26TestCloseSiteHaveBin && bTestBinDataError != 0)           // :644
    {
        if (bTestBinDataError == 1)                                             // :647
        {
            if (CheckTestBinData(FTestSuck)) { bTestBinDataError = 0; return "CheckTestBinData(FTestSuck)"; }   // :649-653
        }
        else if (CheckTestBinData(BTestSuck)) { bTestBinDataError = 0; return "CheckTestBinData(BTestSuck)"; }  // :657-661
    }
    // :665 DoAvoidIndexMotorFallDown、:668-678 CCD 位置、:680-684 ScanTrayStatus、:688-773 視窗前後、
    // :775 SocketAirCoolingStart：與解除無關，不在本次範圍
    return std::string();
}

// ---- fNote：golden Timer1Timer（:3172 起）＋ ScanKey（:2894-3121）的解除部分 ----------------------
//   兩段式（使用者 20260922 說明，golden 同）：K_RETRY／K_SKIP… 先「選取」（UpdateButtonStatus :2786），
//   再按 K_PAUSE（BtnPauseClick :3866）或 K_START（Start :3560）確認；ReturnCode = 選取那一顆的 K。
//   回傳 >0 = 解除（*ans 動作名、*pressed "BtnStart"/"BtnPause"、*input 觸發的鍵）；-1 = 還沒。
int W906_AlarmIoAnswer(const char* qid, int kcode, std::string* ans, std::string* pressed, std::string* input)
{
    static std::string curQid;
    static int  sel = -1;  g_w906NoteSelPtr = &sel;   // TfNote::Select[]（互斥，只記一顆）  // AI(W906-MODAL-WAKE) 20260926: W906_NoteFlushLabel 讀它（檔尾）
    static int  iOldKey = -1;                // TfNote::iOldKey（FormShow :2365 設 -1）
    static bool bOpenChamberDoor = false;    // TfNote 成員（Timer :3187-3205 維護）
    static bool bOpenLeftDoor = false;       // TfNote 成員（Timer :3345-3362 維護）
    if (curQid != (qid ? qid : "")) {        // 新的一則 = FormShow
        curQid = qid ? qid : ""; sel = -1; iOldKey = -1; bOpenChamberDoor = false; bOpenLeftDoor = false;
    }
    if (bSECSGEMAlarm) return -1;            // :3176（bSECSGEM_NoteAlarm 本樹沒有 ⇒ 當 false）
    if (InitialOK == false) return -1;       // :3178

    if (IniConfig.bIndexJamInArmAway == true && bOpenChamberDoor == false)      // :3187
    {
        if (Sen[SnHeaterDoor2].Enable == true || Sen[SnHeaterDoor].Enable == true)
        {
            if ((Sen[SnHeaterDoor2].Enable == true && Sen[SnHeaterDoor2].IsOff() == true) ||
                (Sen[SnHeaterDoor].Enable == true && Sen[SnHeaterDoor].IsOff() == true))
            { bOpenChamberDoor = true; RecordProcess("Chamber Door Open!!"); }  // :3196-3197
        }
        else bOpenChamberDoor = true;                                            // :3202
    }
    if (bAutoRetestJam && bOpenAllDoor == false)                                // :3208
    {
        for (int i = 0; i < MAX_SAFE_DOOR_CNT; i++)
            if (Sen[iSafeDoor[i]].Enable == true && Sen[iSafeDoor[i]].IsOff() == true) bOpenAllDoor = true;   // :3210-3218
        // :3219-3237 Tri_Temp_Machine 的門鎖氣缸與提示：會動氣缸、不在本次範圍
    }
    else bOpenAllDoor = true;                                                   // :3241
    if (Sen[SnSafeDoor1].Enable == true) { if (Sen[SnSafeDoor1].IsOff() == true) bOpenLeftDoor = true; }   // :3345-3350
    else if (Sen[SnSafeDoor2].Enable == true) { if (Sen[SnSafeDoor2].IsOff() == true) bOpenLeftDoor = true; }
    else bOpenLeftDoor = true;                                                  // :3359-3362

    // ---- ScanKey :2894 ----
    int result = -1;
    const int Key = ScanPannelKey();                                            // :2900
    if (Key != -1)
    {
        // :2905 CC_ASE_SG／:2939 HandlerResultServer：客戶專屬，跳過（RULINGS 第 25 條）
        // :2908 bErrPan_err && Pwd!=""：SpecialNote 面板未移植（TfNote 成員）⇒ 當 false
        const bool isReset = (Key == SnFKAlarmReset || Key == SnRKAlarmReset);
        bool blocked = false;
        if (bAutoRetestJam && bOpenAllDoor == false) blocked = true;            // :2916
        if (CosFunction.bOpenDoorCheckLoaderAfterTrayEnd && bOpenLeftDoor == false && !isReset) blocked = true;   // :2921
        if (TrayForm.iManualRemoveLoader == 2 && bOpenLeftDoor == false && !isReset) blocked = true;             // :2928
        if (CosFunction.bPickupErrorAtLoaderNeedOpenDoor &&
            (TrayForm.iManualRemoveLoader == 2 || IniConfig.bE87PickupErrorAtLoaderNeedOpenDoor) &&
            bOpenLeftDoor == false && !isReset && Key != SnRKRetry && Key != SnFKRetry) blocked = true;          // :2935
        // :2956 bP59 iICFloattingCheckStep：未移植（TfNote 成員）
        if (!blocked && iOldKey != Key)                                         // :2961-2963
        {
            iOldKey = Key;
            static const char* const kName[9] = { "SKIP", "RETRY", "TRAY_FEED", "TRAY_END", "CLEAN_OUT", "RESET", "HOME", "TRAIN", "ONECYCLE" };
            const int KeyComp[9] = { K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END, K_CLEAN_OUT, K_RESET, K_HOME, K_TRAIN, K_ONECYCLE };
            const int Index2[9]  = { SnFKSkip, SnFKRetry, SnFKTrayFeed, SnFKTrayEnd, SnFKCleanOut, SnFKReset, SnFKHome, K_TRAIN, SnFKOneCycle };   // :2897（golden 第 8 格放 K_TRAIN，照抄）
            for (int i = 0; i < 9; i++)
            {
                if ((kcode & KeyComp[i]) && Key == Index2[i])                   // :2968 Ptr[i]->Visible（note.cpp:1537-1549 由 KeyCode 決定）
                {
                    // UpdateButtonStatus :2786-2862 的閘
                    bool ok = !IsSafeLockCheck();                                           // :2793
#ifndef SOFT_SIMULTE
                    if (fNote && fNote->IsTestSitICFallDown()) ok = false;                  // :2797
#endif
                    if (CUSTOMER_CODE != CC_SCK && IniConfig.bIndexJamInArmAway && IniConfig.bD40IndexICFallDownMustPressFMotorDown &&
                        bOpenChamberDoor == false && bIsTestSitICFallDown == true) ok = false;   // :2801-2809
                    if (bAutoRetestJam && bOpenAllDoor == false) ok = false;                // :2811
                    // :2814 bOpenSixDoor：只由 golden ShowErrorMessage 設（本樹未移植）⇒ 恆 false，不擋
                    // :2820 KYEC MES0923、:2838 TSMC Are you sure：客戶專屬，跳過
                    if (ok) {
                        sel = i;                                                             // :2836 Select[i]=true（其他 false）
                        bAlarmBuzzer = false; bLampAlarmReset = false;                      // :2975-2976
                        static const char* const kRec[9][2] = { {"MES2120","SKIP pressed"}, {"MES2121","RETRY pressed"},
                            {"MES2118","TRAY FEED pressed"}, {"MES2119","TRAY END pressed"}, {"MES2114","CLEAN OUT pressed"},
                            {"MES2113","RESET pressed"}, {"MES2122","HOME & Retry pressed"}, {"",""}, {"MES2115","ONE CYCLE pressed"} };
                        if (kRec[i][0][0]) NewRecordProcess(kRec[i][0], kRec[i][1], "Note_ScanKey");
                        if (i == 8) bManualOneCycle = true;                                  // :3063
                        // i==5 RESET：golden 立刻 BtnResetClick(this)（:3053）—— TfNote 未移植，不做（報告列給 Jimmy）
                        std::printf("  [alarm-io] %s：面板鍵選取 %s（golden note.cpp ScanKey :2968）\n", curQid.c_str(), kName[i]);
                    }
                }
            }
            if (Key == SnFKStart)                                               // :3067
            {
                if (kcode != 0 &&                                                // :3069 BtnStart->Visible（note.cpp:1551-1558）
                    !(IniConfig.bG14UseStartSoundAlarm && bStartMoveSpeed))      // :3074
                {
                    // :3079-3097 AutoClean 的 WAR16102/WAR16103：會巢狀跳告警，本宿主不做（報告列出）
                    NewRecordProcess("MES2110", "START pressed", "Note_ScanKey");          // :3099
                    if (sel >= 0) { result = KeyComp[sel]; *ans = kName[sel]; *pressed = "BtnStart"; *input = "SnFKStart"; }   // Start :3560 ReturnCode=KeyComp[i]
                }
            }
            else if (Key == SnFKPause)                                          // :3102
            {
                NewRecordProcess("MES2111", "PAUSE pressed", "Note_ScanKey");   // :3104
                bStartMoveSpeed = false;                                        // BtnPauseClick :3879
                if (sel >= 0) { result = KeyComp[sel]; *ans = kName[sel]; *pressed = "BtnPause"; *input = "SnFKPause"; }   // BtnPauseClick :3896 起
            }
            else if (Key == SnFKAlarmReset && bAlarmBuzzer)                     // :3107
            {
                W906_PanelAlarmReset("Note_ScanKey");
                RespondASECom("@e02111Done");                                   // :3119
            }
        }
    }
    CheckSafeDoorForICFallDown();                                               // Timer :3404（ScanKey 之後）
    return result;
}

// ---- 模擬 DI（只有 SOFT_SIMULTE 建置）----------------------------------------------------------
bool W906_SimDiCommand(const webbridge::WebCommand& wc)
{
    if (wc.cmd != "sim.di.set") return false;
    if (!g_modalServer) return true;
#ifdef SOFT_SIMULTE
    const std::string name = wc.hasTag ? wc.tag : std::string();
    // ⚠ TagValue 的存取子是嚴格的（asDouble 對 Int 回 fallback，WebBridge/TagValue.h:89-91）⇒ Int／Double 各判一次
    const bool on = wc.hasValue && ((wc.value.isInt() && wc.value.asInt(0) != 0) ||
                                    (wc.value.isDouble() && wc.value.asDouble(0.0) != 0.0) ||
                                    (wc.value.isBool() && wc.value.asBool(false)) ||
                                    (wc.value.isString() && wc.value.asString() == "1"));
    for (int i = 0; i < MAX_SENSOR_ITEM; i++)
    {
        if (Sen[i].Name != AnsiString(name.c_str())) continue;
        if (Sen[i].ISABase == eISABase || Sen[i].ISABase == ePCI1735U) {        // 這兩種走 myio 的原始埠（mysensor.cpp:140-143），不經模擬後端
            g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "sim.di.set: sensor uses raw-port IO (eISABase/ePCI1735U), not the sim backend");
            return true;
        }
        const bool raw = Sen[i].Type ? on : !on;                                // TMySensor::IsOn：Type=0 是反相（mysensor.cpp:145-153）
        if (raw) MyLaneIO.IOBitOn (Sen[i].Ring, Sen[i].IP, Sen[i].Port, Sen[i].Bit, Sen[i].ISABase, Sen[i].Name);
        else     MyLaneIO.IOBitOff(Sen[i].Ring, Sen[i].IP, Sen[i].Port, Sen[i].Bit, Sen[i].ISABase, Sen[i].Name);
        char b[160];
        std::snprintf(b, sizeof(b), "{\"sensor\":\"%s\",\"index\":%d,\"enable\":%s,\"isOn\":%s}",
                      name.c_str(), i, Sen[i].Enable ? "true" : "false", Sen[i].IsOn() ? "true" : "false");
        std::printf("  [sim.di.set] %s=%d -> IsOn()=%d\n", name.c_str(), on ? 1 : 0, Sen[i].IsOn() ? 1 : 0);
        g_modalServer->CompleteCommand((unsigned long long)wc.id, true, b);
        return true;
    }
    g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "sim.di.set: no sensor named " + name);
#else
    g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "sim.di.set: SOFT_SIMULTE builds only (real IO is read from the card)");
#endif
    return true;
}

//AI(W906-BOOTSUM) 20260925: 讓 BootSummary.cpp（在不帶 HAVE_PCI1203 的生產函式庫裡）知道 wb_serve 這個 target 有沒有連 1203 SDK
#if defined(HAVE_PCI1203) && HAVE_PCI1203
bool W906_WbServeHas1203() { return true; }
#else
bool W906_WbServeHas1203() { return false; }
#endif

// ===========================================================================
//  AI(W906-MODAL-WAKE) 20260926: RULINGS_20260926 第 9 條（3A＋B）＋第 26 條 Q3＋第 27 條 3 —— 阻塞框在等的時候
//    ① 照 golden 框自己的 Timer1Timer 推塔燈／蜂鳴器／面板鍵燈；② 沒有網頁 30 秒就自動開瀏覽器；③ 是／否框面板 Alarm Reset 消音。
//  （行號一律是 golden 906 = HT9011UC_Code_V3.33.906.0_20260618；審查 wf_ea672f01-f4b 的 11 條修正已收進本段）
//
//  golden：三種阻塞框都是 ShowModal，框開著時 MainProc（Synchronize）不會回來，主畫面的 ProcessKeyFlush 也在框開著時直接
//  return（main.cpp:4103-4106），塔燈／蜂鳴器／鍵燈全靠框自己的 10 ms Timer1Timer：
//    TfNote::Timer1Timer        note.cpp:3355 fMain->Timer1Timer（FlushFlag 在裡面翻，main.cpp:3181-3188）、:3377 FlushLabel、
//                               :3380 ScanKey、:3382 DoSystemMessage、:3399 DoAvoidIndexMotorFallDown
//    TMyMessageBox::Timer1Timer mymessbox.cpp:542 fMain->Timer1Timer、:554-580 ScanPannelKey（:560-565 Pause／Retry／Skip 受
//                               bDisableKeypad 管、:567-579 Alarm Reset 消音）、:582-585 Pause 燈、:587 DoSystemMessage、:650 DoAvoidIndexMotorFallDown
//  移植樹三個等待迴圈原本只補了 DoAvoidIndexMotorFallDown：tick 停住，塔燈停在框出現前、蜂鳴器不叫 —— 沒開網頁時沒有人會發現。
//
//  W906_ModalWaitTick（每一圈 ≤100 ms，waitForPush(100)）：
//  * 1203 監看器照主迴圈的節奏（kIoTickMs）Poll 一次 —— HT9050 的 Sen[] 讀的是監看器的樣本（Pci1203IoRoute.cpp FillDi），
//    主迴圈停在等待裡時樣本凍住：面板鍵（Alarm Reset 消音、告警框的 Retry／Skip＋Start／Pause、ShowMyMessage 的 Pause）與
//    DoAvoidIndexMotorFallDown 的 EMG／斷電都看不到。Poll 是唯讀的（Pci1203Monitor.h 檔頭的唯讀清單）。
//  * ht9045::W906_FlushFlagTick()（同 PumpTick，250 ms 翻一次）。
//  * 鍵燈：告警框 W906_NoteFlushLabel（note.cpp:3084-3139 的燈號那一半；Visible＝kcode，golden note.cpp:1537-1549；Select＝
//    W906_AlarmIoAnswer 的 sel）；是／否框與 ShowMyMessage 框 bLampPause=FlushFlag（mymessbox.cpp:585；906 的 pnlPause 從來不是
//    "Alarm Reset"，:582-583 那一支不會走）。
//  * 是／否框：ScanPannelKey，Alarm Reset 且在叫 ⇒ 消音（mymessbox.cpp:567-579；:560-565 因 bDisableKeypad :1022 不關框）。
//    告警框與 ShowMyMessage 框的面板鍵本來就在 W906_AlarmIoAnswer／W906MbIoDismiss（同一圈、在這之後）。
//  * DoSystemMessage() 連叫 6 次 = 6 相輪轉整圈（ckernel.cpp；相 0 ShowRunLed＋ShowRunLabel、相 3 DoPanelLamp）。golden 10 ms 一次、
//    60 ms 轉一圈；這裡 ≤100 ms 轉一圈。塔燈只在 FlushFlag 翻的那一次更新（ShowRunLed 的 OldFlushFlag 閂），閃的節奏照樣 250 ms。
//  * 開瀏覽器：「有網頁」＝活著的 WebSocket（WebBridgeServer::LiveWebSocketCount）或 3 秒內有 HTTP 請求（dialog-bridge.js 每 100 ms
//    輪詢信箱，WebSocket 斷了但頁面還開著、照樣看得到框 —— 那種情況不再疊開第二個視窗）。都沒有連續 30 秒 ⇒ 開 Edge 正式版畫面；
//    之後每 2 分鐘最多一次、一個框最多 3 次（ht9045::ModalWake；tests/test_modal_wake.cpp）。
//    --seconds N 或環境變數 W906_NO_BROWSER_WAKE（非 0）時不開（W906_ModalWakeConfigure，main :3723）。
//
//  W906ModalWaitScope（:439 宣告；進迴圈前建，任何 return 都經過解構）＝ golden FormShow／FormClose 的狀態：
//    告警框   edErrorCode->Text＝碼（大寫，note.cpp:841）、AlarmType（見 W906_AlarmTypeOfCode）；bAlarmBuzzer=true（:1480-1484 golden
//             依 GetJemSilent；使用者 #26 Q3／#27-3「告警框期間蜂鳴器一律叫」）；bLampAlarmReset=true（:1495）＋實體 Alarm Reset 燈亮
//             （:1935-1936）；fShow=true（:2167）。關框：fShow=false（:2519）、鍵燈全滅（:2534-2543）、SW[SwManualZ1].Off()（:2544）、
//             edErrorCode->Text=""（:2558 fNote->Reset() → :2673）。
//    是／否框 bAlarmBuzzer=!bDisableAlarmBuzzer（mymessbox.cpp:336；bDisableAlarmBuzzer 只有沒移植的 ShowMyMessagePWD 會設 ⇒ true）、
//             fShow=true（:344）；關框 fShow=false（:408）。開框前 W906_YesNoPreCloseLikeGolden 照 :1012-1019 處理已開著的 MyMessageBox。
//    ShowMyMessage 框：MbFormShow／MbFormClose 本來就設了；MbFormClose 尾呼叫 W906_ModalOutputsRefresh。
//    關框後立刻 W906_ModalOutputsRefresh（DoSystemMessage 整圈）：golden 主畫面的 Timer1Timer 在 FormClose 後約 60 ms 就重算
//    塔燈與音樂；移植樹主 tick 的 DoSystemMessage 每 500 ms 才一相，不補的話框關了還會叫到約 3 秒。
//    golden 兩個 FormClose 都不清 bAlarmBuzzer（note.cpp 寫它的只有 :1482/:1484/:2952/:3070/:5627；mymessbox FormClose :385-442 沒有），照做。
// ===========================================================================
#include "WebModalWake.h"
#include <cctype>
void DoSystemMessage();                                   // ckernel.cpp（ckernel.h:99；本檔不 include ckernel.h，慣例是本地宣告）
namespace ht9045 { void W906_FlushFlagTick(); }           // WebBridgeTags.cpp（只有 namespace ht9045 裡的定義，沒有標頭）

int* g_w906NoteSelPtr = 0;                                // W906_AlarmIoAnswer 的 sel（TfNote::Select[]）；:7286 那一行掛上

namespace {
ht9045::ModalWake g_modalWake;
bool g_modalWakeOff = false;

// GetTickCount 64 位元延伸（MinGW 6.3 的 w32api 沒有 GetTickCount64）；49.7 天回捲時進位。只在 tick 執行緒上呼叫
unsigned long long W906_TickMs64()
{
    static DWORD last = 0;
    static unsigned long long high = 0;
    const DWORD t = ::GetTickCount();
    if (t < last) high += 0x100000000ULL;
    last = t;
    return high + t;
}

// 「有網頁」：活著的 WebSocket，或 3 秒內有 HTTP 請求（dialog-bridge.js 輪詢信箱）
int W906_PagesPresent()
{
    if (!g_modalServer) return 0;
    static bool init = false;
    static unsigned long long lastHttp = 0, lastHttpMs = 0;
    const unsigned long long n = g_modalServer->Stats().httpRequests;
    const unsigned long long now = W906_TickMs64();
    if (!init) { init = true; lastHttp = n; }
    else if (n != lastHttp) { lastHttp = n; lastHttpMs = now; }
    const bool httpRecent = (lastHttpMs != 0 && now - lastHttpMs < 3000ULL);
    return g_modalServer->LiveWebSocketCount() + (httpRecent ? 1 : 0);
}

// golden fNote->AlarmType：由 MyDBIEvent 的最後一段決定（cMyDB.cpp:679-705，DB 查詢之後無條件覆寫）——碼在 AlarmCodeMap 裡時
//   JAM 開頭 1、WAR 開頭 2、其他 3；不在表裡是 0（"Unknown Alarm Code"，走卡料燈）。移植樹的 MyDBIEvent 那一段還在
//   #if 0（cMyDB.cpp:878，AlarmCodeMap 未移植）⇒ 一律當成「在表裡」。已知差異：說明檔沒有的碼，golden 0（卡料燈），這裡照前綴。  //AI(W906-SHOWERR) 20260929: STALE -- the MyDBIEvent segment is live since CMYDB-P3 (cMyDB.cpp:867-897, AlarmCodeMap filled by MyDBUpdateDB, :4166); the record half above (ForwardShowErrorMessage) now sets AlarmType through MyDBIEvent and this prefix rule is only the fallback at :7590 for a box golden would not show
int W906_AlarmTypeOfCode(const std::string& code)
{
    if (code.compare(0, 3, "JAM") == 0) return 1;
    if (code.compare(0, 3, "WAR") == 0) return 2;
    return 3;
}
}  // namespace

void W906_ModalWakeConfigure(int seconds)
{
    const char* e = std::getenv("W906_NO_BROWSER_WAKE");
    const bool envOff = (e && *e && std::strcmp(e, "0") != 0);
    g_modalWakeOff = (seconds > 0) || envOff;
    std::printf("[modal-wake] 阻塞框沒有網頁時自動開瀏覽器：%s\n",
                g_modalWakeOff ? (seconds > 0 ? "關（--seconds 有限期執行）" : "關（W906_NO_BROWSER_WAKE）") : "開（30 s／2 min／每框 3 次）");
}

// golden TfNote::FlushLabel（note.cpp:3084-3139）的燈號那一半（按鈕顏色是畫面，在瀏覽器）
static void W906_NoteFlushLabel(int kcode)
{
    static bool OldFlushFlag = false;
    if (OldFlushFlag == FlushFlag) return;                                      // :3090-3091
    OldFlushFlag = FlushFlag;                                                   // :3092
    bool* bPtr[9] = { &bLampSkip, &bLampRetry, &bLampTrayFeed, &bLampTrayEnd, &bLampCleanOut,
                      &bLampReset, &bLampHome, &bLampTrain, &bLampOneCycle };  // :3087（第 10 格 bLampAlarmReset 由 :3134-3135 蓋掉，見下）
    const int KeyComp[9] = { K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END, K_CLEAN_OUT, K_RESET, K_HOME, K_TRAIN, K_ONECYCLE };
    const int sel = g_w906NoteSelPtr ? *g_w906NoteSelPtr : -1;
    bool flag = false;                                                          // :3093
    for (int i = 0; i < 9; i++)                                                 // :3094
    {
        if (i == sel) { flag = true; *bPtr[i] = true; }                         // :3096-3100 Select[i]
        else if (OldFlushFlag) { if (kcode & KeyComp[i]) *bPtr[i] = true; }     // :3103-3106 Ptr[i]->Visible（note.cpp:1537-1549 由 KeyCode 決定）
        else *bPtr[i] = false;                                                  // :3108-3111
    }
    if ((flag || kcode == 0) && OldFlushFlag)                                   // :3116
    {
        if (IniConfig.bG14UseStartSoundAlarm && bStartMoveSpeed) bLampStart = false;   // :3120-3121
        else                                                     bLampStart = true;    // :3122-3123
        bLampPause = true;                                                      // :3124
    }
    else
    {
        bLampStart = false;                                                     // :3130
        bLampPause = false;                                                     // :3131
    }
    if (bAlarmBuzzer) bLampAlarmReset = OldFlushFlag;                           // :3134
    else              bLampAlarmReset = false;                                  // :3135
}

// 關框後立刻重算塔燈／音樂／鍵燈（見檔頭）
void W906_ModalOutputsRefresh()
{
    for (int i = 0; i < 6; ++i) DoSystemMessage();
}

// golden ShowMyMessageBox_YES_NO（mymessbox.cpp:1012-1019）：已有 MyMessageBox 開著 —— 會停機的那種就回 3、不問；
//   不停機的那種（ShowUnloaderTrayMessage 的非阻塞框，MbFormShow 設了 fShow）先 Close 再問。
int W906_YesNoPreCloseLikeGolden()
{
    if (W906_FormShowing("MyMessageBox", MyMessageBox->fShow) == true && iUnLoaderCount == 0) return 3;          // :1012-1015  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
    if (W906_FormShowing("MyMessageBox", MyMessageBox->fShow) == true && iUnLoaderCount != 0)                     // :1016-1019  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
        MbCloseModeless("superseded by ShowMyMessageBox_YES_NO (golden :1016-1019)", false);
    return 0;
}

W906ModalWaitScope::W906ModalWaitScope(int k, const char* code) : kind(k)
{
    if (kind == 0) {                                                            // 告警框：golden TfNote::FormShow
        if (fNote) {
            std::string c = code ? code : "";
            for (size_t i = 0; i < c.size(); ++i) c[i] = (char)std::toupper((unsigned char)c[i]);   // :841 Code.UpperCase()
            fNote->edErrorCode->Text = AnsiString(c.c_str());
            { extern bool W906_ShowErrorMessage_Recorded; if (!W906_ShowErrorMessage_Recorded) fNote->AlarmType = W906_AlarmTypeOfCode(c); }   // :846（見 W906_AlarmTypeOfCode）  //AI(W906-SHOWERR) 20260929: only when the record half did not run (InitialOK false / an alarm while fNote is up -- golden returns before ShowModal there, note.cpp:541 / :817); otherwise MyDBIEvent already wrote fNote->AlarmType (golden :846), 0 for a code missing from AlarmCodeList.txt
        }
        if (g_w906NoteSelPtr) *g_w906NoteSelPtr = -1;  bHandlerPause = true; iHandlerStartCount = 0;  bPauseInMotor = true; bPauseOutMotor = true; bPauseSortMotor = true;  bHangTimePause = true; bSupplyNewICTrayPause = true;  for (int i = 0; i < 7; i++) { bLifterPause[i] = true; bAuto2Pause[i] = true; }  lHandlerStopTime.LatchCycleTime(true);  /* AI(W906-R71) 20260926 C1：golden TfNote::FormShow 無條件設的暫停標記 note.cpp:1238-1239／:1360-1362／:1455-1462／:2342。沒有它們時，用 START 答掉警報後恢復運轉：手臂不先回 Z 安全位（bPauseInMotor，mymotor.cpp:5080-5094）、tester 逾時與 Hang-up 不重算（bHandlerPause，aTester_Front.cpp:3542-3553）—— 答 PAUSE／RETRY 時 MainProc 的暫停分支會補設，所以只有 START 回答漏掉 */   // FormShow 清 Select[]（同 W906_AlarmIoAnswer 換 qid 時）
        bAlarmBuzzer    = true;  bAlarmReset = false;  /* note.cpp:1363 FormShow（AI(W906-R70) 20260926 A5）*/   // :1480-1484（#26 Q3／#27-3：一律叫）
        bLampAlarmReset = true;                                                 // :1495
        if (bFrontPadActive) SW[SwFKAlarmReset].OnOff(bLampAlarmReset);         // :1935
        else                 SW[SwRKAlarmReset].OnOff(bLampAlarmReset);         // :1936
        { extern bool W906_ShowErrorMessage_Recorded; if (fNote && W906_ShowErrorMessage_Recorded) fNote->W906_FormShowServoOff(); }  if (fNote) { fNote->fShow = true;  extern void W906_PageProgramSet(const char*, bool, const char*); W906_PageProgramSet("fNote", true, "wb_serve.cpp:7597 TfNote::FormShow (golden note.cpp:2167)"); }  /*AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表程式寫入（C++ 自己開的對話框）*/                                         // :2167  //AI(W906-SHOWERR) 20260929: golden FormShow :2035-2128 alarm servo-off before :2167 fShow=true (forms/fNote_ShowError.cpp; In Arm half -- Out Arm / Out Shuttle halves with csystem.cpp DoServoOn H2-G1 / H2-G2 / H2-G3), only for a box golden shows (W906_ShowErrorMessage_Recorded, set at :442); START -> DoServoOn re-energises
    } else if (kind == 1) {                                                     // 是／否框：golden TMyMessageBox::FormShow
        bAlarmBuzzer = true;                                                    // mymessbox.cpp:336 `bAlarmBuzzer=!bDisableAlarmBuzzer;`
        MyMessageBox->fShow = true;  { extern void W906_PageProgramSet(const char*, bool, const char*); W906_PageProgramSet("MyMessageBox", true, "wb_serve.cpp:7600 TMyMessageBox::FormShow (golden mymessbox.cpp:344)"); }  /*AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表程式寫入（C++ 自己開的對話框）*/                                             // :344
    }
    g_modalWake.Begin(W906_TickMs64(), W906_PagesPresent());
}

W906ModalWaitScope::~W906ModalWaitScope()
{
    if (kind == 0) {                                                            // golden TfNote::FormClose
        if (fNote) { fNote->fShow = false;  extern void W906_PageProgramSet(const char*, bool, const char*); W906_PageProgramSet("fNote", false, "wb_serve.cpp:7608 TfNote::FormClose (golden note.cpp:2519)"); }  /*AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表程式寫入（C++ 自己開的對話框）*/  bAlarmReset = false;  /* note.cpp:2522 FormClose（AI(W906-R70) 20260926 A5）*/   // note.cpp:2519
        bLampSkip = false; bLampRetry = false; bLampOneCycle = false; bLampCleanOut = false; bLampTrayFeed = false;  bEnterTestIF = true; lHandlerStopTime.LatchCycleTime(true); bInArmNeedToSafePos = false;  /* AI(W906-R71) 20260926 C1：golden TfNote::FormClose note.cpp:2562／:2564／:2664 */   // :2534-2538
        bLampTrayEnd = false; bLampReset = false; bLampHome = false; bLampTrain = false; bLampFix = false;             // :2539-2543
        SW[SwManualZ1].Off();                                                   // :2544
        if (fNote) fNote->edErrorCode->Text = AnsiString("");                   // :2558 fNote->Reset() → :2673
    } else if (kind == 1) {
        MyMessageBox->fShow = false;  { extern void W906_PageProgramSet(const char*, bool, const char*); W906_PageProgramSet("MyMessageBox", false, "wb_serve.cpp:7614 TMyMessageBox::FormClose (golden mymessbox.cpp:408)"); }  /*AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表程式寫入（C++ 自己開的對話框）*/                                            // mymessbox.cpp:408
    }
    g_modalWake.End();  bLampStart = false; bLampPause = false;   // AI(W906-R70) 20260926: NB2 R70 A1 改法 B —— 框裡寫了這兩個燈旗標（W906_NoteFlushLabel、`bLampPause = FlushFlag`），關框沒人還原 ⇒ DoPanelLamp 之後每一輪都把最後那一相寫到 SW[SwFK/RKStart／Pause]（約一半機會兩顆都常亮）。回到第 9 條之前的全暗；照 golden 翻 ProcessKeyFlush（main.cpp:4100 起，改法 A）排在 INBOX
    if (kind != 2) W906_ModalOutputsRefresh();                                  // kind 2：MbFormClose 尾做（它在 MbWait 回來之後才跑）
}

void W906_ModalWaitTick(int kind, int kcode)
{  { extern void W906_Q44ConsoleServiceTick(); W906_Q44ConsoleServiceTick(); }   //AI(W906-D012) 20260929 [W906] Q44 A3: a blocking box is open (the main loop cannot reach W906_ServeQuitDue) -- console Ctrl-C / X still gets the stop commands sent from here (tick thread); body FileRW/MainClose.cpp
    { extern void W906_NativeFormsPump(int); W906_NativeFormsPump(1); }  /* AI(W906-NATIVE-PROTO) 20260928 [W906]: 告警／是否／ShowMyMessage 三個阻塞等待框期間主迴圈停住，這裡照樣泵原生視窗（否則 5 秒後 Windows 標「沒有回應」）；site 1＝拖曳時只保活 Index 煞車保護（檔尾 W906_NativeKeepaliveModal）。OFF＝空函式 */  {                                                                           // 1203 DI 樣本（見檔頭）
        static unsigned long long nextPoll = 0;
        const unsigned long long now = W906_TickMs64();
        if (now >= nextPoll) {
            nextPoll = now + (unsigned long long)kIoTickMs;
            if (ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor()) { mon->SetYieldHook(0); mon->Poll(); mon->SetYieldHook(&W906_OutputYieldHook); { extern void W906_BrakeAxisTick(); W906_BrakeAxisTick(); } }   /*AI(W906-BRAKE-AXIS) 20260929: the per-axis brake while a box waits (a drive alarm drops SVON -> hold)*/   // AI(W906-R70) 20260926: NB2 R70 MW-A —— Poll 會跑「輸出優先」的 yield hook（:4477 裝、:6353 → W906_ServiceOutputs），框開著時網頁的 io.btnPanelClick／pci1203.do.*／ax.stop 因此被執行（約一半的點擊）。框裡的 Poll 前後把 hook 拿掉 ⇒ 回到第 9 條之前的語意：那些命令一律 modal-pending（golden ShowModal 時其他畫面按不到）；也解掉 MW-B 的巢狀 Poll。MW-E（這一塊沒包 #ifdef INSTALL_1203_MONITOR）刻意不做：要多兩行、後面的行號引用全部位移，而巨集一直開著（MachineType.h:121）—— 排在 INBOX
        }
    }
    if (((fNote != 0 && W906_FormShowing("fNote", fNote->fShow)) || (MyMessageBox != 0 && W906_FormShowing("MyMessageBox", MyMessageBox->fShow))) && bIndexCheckNoStopVaccum == false) CheckIndexAllSuckICFallDown(true, true);  ht9045::W906_FlushFlagTick();  { extern void W906_MainRecordTimer1Tick(); W906_MainRecordTimer1Tick(); }                                               // AI(W906-R70) 20260926 MW-F：golden fMain->Timer1Timer main.cpp:3125-3128 —— 任何框開著時都跑（回傳值 golden 也不看）；REALLY 模式下 Index 吸嘴該有 IC 卻沒真空，INDEX_SUCKER_TYPE==1 時照 golden Normal()。第 9 條之前三個等待迴圈都沒跑它。  golden fMain->Timer1Timer 的 FlushFlag  AI(W906-ELA-W48B-A3) 20260928 (St02-E, laptop-approved claim 17:2x): + golden Timer1Timer :3284 UpdateRecordScreen (FileRW/MainRecord.cpp) -- the record counters keep running while a dialog is open (note.cpp:3355 / mymessbox.cpp:542)
    if (kind == 1 && bSECSGEMAlarm == false) {                                  // mymessbox.cpp:554
        const int ret = ScanPannelKey();                                        // :559
        if (ret == SnFKAlarmReset && bAlarmBuzzer)                              // :567（:560-565 Pause／Retry／Skip：bDisableKeypad ⇒ 不關框）
            W906_PanelAlarmReset("MessageBox_Timer");                           // :568-579
    }
    if (kind == 0) W906_NoteFlushLabel(kcode);                                  // note.cpp:3377
    else           bLampPause = FlushFlag;                                      // mymessbox.cpp:585
    for (int i = 0; i < 6; ++i) DoSystemMessage();                              // note.cpp:3382／mymessbox.cpp:587，一圈轉完 6 相
    { extern void W906_HmiKeeperTick(); W906_HmiKeeperTick(); }  /*AI(W906-HMI-KEEP) 20260929: EastSun「一直持續偵測」-- the main loop is held here while a box waits, so the keeper (5 s, Chrome, W906_HMI_URL) runs here too; MODAL-WAKE below stays as the fallback (it waits 30 s, the keeper reopens first)*/  if (!g_modalWakeOff && g_modalServer && g_modalWake.Tick(W906_TickMs64(), W906_PagesPresent())) {
        std::string why;
        const std::string url = ht9045::ModalWakeUrl(g_modalServer->BoundPort());  extern bool W906_HmiShellLaunch(const std::string&, std::string&);   // AI(W906-HMI-SHELL) 20260930
        const bool ok = W906_HmiShellLaunch(url, why) || ht9045::ModalWakeLaunchEdge(url, why);   // AI(W906-HMI-SHELL) 20260930: the HMI program window first, Edge without it
        std::printf("[modal-wake] 阻塞框在等、沒有網頁：第 %d／%d 次自動開瀏覽器 %s -- %s\n",
                    g_modalWake.launches(), ht9045::ModalWake::kMaxPerDialog, ok ? "OK" : "FAILED", why.c_str());
    }
}

// ===========================================================================
//  AI(W906-ALMRST-SECS) 20260926: the SECS event of a panel Alarm Reset (W906_PanelAlarmReset above; golden mymessbox.cpp:577-578 and
//  note.cpp:3077-3078 -- the same two lines in both Timer1Timer / ScanKey arms).  Here at EOF so the SECS headers stay out of the
//  file head (no line above moves).
// ===========================================================================
#include "SECSGEM/SecsEventType.h"   // SECS_EVENT (:340)
#include "SECSGEM/SecsEventReport.h" // EventReport (:55)
#include "Config.h"                  // IniConfig.bEnable_SECS_GEM
void W906_EventReportDoAlarmReset()
{
    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
        EventReport(SECS_EVENT.DoAlarmReset);                                   //30     按下 Alarm Reset
}

// ===========================================================================
//  AI(W906-ENV-BANNER) 20260927: NB2 R79.  Production code reads 19 W906_* environment variables that redirect machine
//  data or configuration files (census: tools/nb2_assist/env_redirect_census.py on origin/v906/nb2-assist; unset = the golden
//  literal).  main() above turns away only W906_INIDATA_ROOT (recipes, ruling A1.2).  The other 18 stay accepted on purpose
//  for now: this machine's F5 entry "IOWEB" (.vscode/launch.json:109-121) sets 12 of them to keep SetUp.inf, config\,
//  teach.ini and the logs on ../runcfg copies, so turning them away is a user decision (NIGHT_REPORT decision 20).
//  Until then this only SAYS which ones are in effect -- a silent redirect of IO_Table.csv / Mot_Table.csv / Gerneral.ini
//  (e.g. inherited from a cmd window that ran ioweb_probe) is the failure R79 describes.  No behaviour change.
//  Keep the list in step with the census; a name missing here is simply not printed.
// ===========================================================================
void W906_PrintDataRedirects()
{
    static const char* const kNames[] = {
        "W906_AUTH_PATH", "W906_BINCOUNT_PATH", "W906_CLEANPADLOG_ROOT",
        "W906_E84DATA_ROOT", "W906_EVENTLOG_ROOT", "W906_GENERAL_INI_PATH",
        "W906_HT9045LOG_ROOT", "W906_IOTABLE_PATH", "W906_MACHINERECORD_DIR",
        "W906_MOTTABLE_PATH", "W906_PRODLOG_ROOT", "W906_PWBOOK_PATH",
        "W906_SAVEEVENTLOG_ROOT", "W906_SETUPINF_PATH", "W906_SUMMARYLOT_ROOT",
        "W906_TCPDATA_ROOT", "W906_TEACH_INI_PATH", "W906_UNLOADERINFO_ROOT",
    };
    int n = 0;
    for (size_t i = 0; i < sizeof(kNames) / sizeof(kNames[0]); ++i) {
        const char* v = std::getenv(kNames[i]);
        if (v == 0 || *v == 0)
            continue;
        if (n++ == 0)
            std::printf("\n  !! W906_* machine-data redirects in effect (unset = the golden path):\n");
        std::printf("  !!   %s = %s\n", kNames[i], v);
    }
    if (n != 0)
        std::printf("  !! %d redirect(s): the files above are what this wb_serve reads and writes.\n\n", n);
}

// ===========================================================================
//AI(W906-FRW-Q3) 20260927（Steven 團隊 St01）：RULINGS_20260926 S125（Steven 回 todo ★ Q3「A」；todo F-005，S101 交件時查到）。
//   DIO 設定檔的第二個寫入口：TTLCfg 頁（C 路 FileRW/TTLCfg.cpp，golden TfDIOFrom::spbSaveClick DIOInterFaceCFG.cpp:191）寫的
//   DIO ini 檔名是動態的 —— golden GetDIOFileName（DIOInterFaceCFG.cpp:47-70）：
//       config.ini [Tester] bI16TTLSaveInSetupFile ? <配方資料夾>\<cbDIOType->Text>.ini : DIOCFGPath\<cbDIOType->Text>.ini
//   cbDIOType 的清單＝DIOCFGPath\*.ini（golden cTesterIF.cpp:70-106 InitcbDIOType）。CRouteOwner 的 kOwned 表只認固定檔名，
//   所以 B 路 system.file.put tag=dio（路徑＝ResolveDioPath()）與 recipe.doc.put（配方裡的 <DIO>.ini 副本）原本擋不到。
//   這裡認得下面三種就回擁有者 → 兩個呼叫端（system.file.put、recipe.doc.put）回「409 owned by C route」，
//   跟其他 C 路的檔同一種回應；dryRun（預演）照常允許（呼叫端的 PeekDryRun，沒動）：
//     (1) DIOCFGPath 資料夾裡的 *.ini —— 母檔；TTLCfg 可以選到其中任何一個（S101 的 Delete 鈕也只刪這裡的檔）；
//     (2) 作用中配方資料夾（RealRecipeDir，recipe.doc.put 列的就是它）裡、檔名去掉 .ini 後＝DIOCFGPath 某個 *.ini 的檔 —— 配方副本；
//     (3) 目前解析出來的 DIO 檔（ResolveDioPath：Tester.Data [DIO] TypeName ＋ bI16TTLSaveInSetupFile）—— TypeName 不在清單時的保底。
//   比對不分大小寫、'/' 與 '\' 視為相同、連續的 '\' 視為一個。其他檔一律回 nullptr（行為不變）。
//   純讀：FindFirstFile（EnumDioProfiles）＋讀 config.ini／Tester.Data（ResolveDio 的 IniGet，不補鍵），不建檔、不寫檔。
//   放在檔尾是為了不推動本檔其他人登記的行號（前置宣告佔用 CRouteOwner 上方原本的空行）。
//   CRouteOwner 在 :915 開的匿名 namespace 裡（到 :2672）→ 這裡重開同一個匿名 namespace 定義（同一個 TU 的 namespace { } 是同一個）。
// ===========================================================================
namespace {

static std::string CRouteDioNorm(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '/') c = '\\';
        c = (char)std::tolower((unsigned char)c);
        if (c == '\\' && o.size() >= 2 && o[o.size() - 1] == '\\') continue;   // "a\\b" == "a\b"（開頭的 \\server 保留）
        o.push_back(c);
    }
    return o;
}

static const char* CRouteOwnerDio(const std::string& fullPath)
{
    static const char* const kDioOwner =
        "FileRW/TTLCfg.cpp -- use WS editlist.get / editlist.save tag=TTLCfg (Config.DIOInterFaceCFG.html); "
        "the DIO ini name is dynamic: DIOCFGPath\\<DIO>.ini, or <recipe>\\<DIO>.ini when config.ini [Tester] "
        "bI16TTLSaveInSetupFile=1 (golden TfDIOFrom::GetDIOFileName, DIOInterFaceCFG.cpp:47-70)";
    const std::string p = CRouteDioNorm(fullPath);
    const std::size_t sl = p.find_last_of('\\');
    if (sl == std::string::npos) return nullptr;
    const std::string dir = p.substr(0, sl + 1);
    const std::string name = p.substr(sl + 1);
    if (name.size() <= 4 || name.compare(name.size() - 4, 4, ".ini") != 0) return nullptr;   // DIO 設定檔一律 .ini
    const std::string stem = name.substr(0, name.size() - 4);

    std::string dioDir = CRouteDioNorm(std::string(DIOCFGPath.c_str()));
    if (!dioDir.empty() && dioDir[dioDir.size() - 1] != '\\') dioDir += '\\';
    if (!dioDir.empty() && dir == dioDir) return kDioOwner;                                   // (1) 母檔

    std::string recDir = CRouteDioNorm(RealRecipeDir());
    if (!recDir.empty() && recDir[recDir.size() - 1] != '\\') recDir += '\\';
    if (!recDir.empty() && dir == recDir) {                                                    // (2) 配方副本
        std::vector<std::string> profs;
        EnumDioProfiles(&profs);
        for (std::size_t i = 0; i < profs.size(); ++i)
            if (CRouteDioNorm(profs[i]) == stem) return kDioOwner;
    }

    const std::string act = ResolveDioPath();                                                  // (3) 目前的 DIO 檔
    if (!act.empty() && CRouteDioNorm(act) == p) return kDioOwner;
    return nullptr;
}

}  // namespace（匿名；CRouteOwner 同一個）

// ===========================================================================
//  AI(W906-RSMODE) 20260927: WS 指令 main.runStartMode —— value = 起動模式名稱（StartModeName[]，例 "Initial Start"、"Continuous Start"）；空字串＝只查詢。
//  照 golden 操作員的動作：主畫面下拉選單改值（ItemIndex／Text）→ OnChange ＝ golden TfMain::cbRunStartModeChange
//  （main.cpp:23726-23825，照翻在 RunStartMode.cpp 檔尾的 W906_CbRunStartModeChange）。
//  golden 的 handler 第一行是 `if(SystemStart) return;`（運轉中不改）；切 Initial Start 時機台裡有 IC 會改回原值並提示 ——
//  這兩種情況下模式沒變，回 ok=false，ack 帶 requested／caption／iRunStartMode／before 讓網頁知道實際結果。
//  網頁：web/page/main-control.js:146 目前送的是舊 C# 模擬器那條（route A），要改送這個指令（TO_STEVEN §1 RSMODE）。
// ===========================================================================
// ack：requested／caption（下拉目前的 Text）／iRunStartMode／before／systemStart／items（下拉清單 = golden SetStartModeData 填的；
//   網頁照這份畫選項，不要自己寫死 —— main.html:192 目前寫死的三項裡 "Single Start" golden 沒有）。
static std::string W906_RsmAckJson(const std::string& want, int before)
{
    TfLotInfoRunMode* cb = fMain->cbRunStartMode;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("requested").String(want);
    w.Key("caption").String(std::string(cb->Text.c_str()));
    w.Key("iRunStartMode").Number((wb_int64)LastSet.iRunStartMode);
    w.Key("before").Number((wb_int64)before);
    w.Key("systemStart").Bool(SystemStart != 0);
    w.Key("items").BeginArray();
    const int n = cb->Items ? cb->Items->GetCount() : 0;
    for (int i = 0; i < n; ++i) w.String(std::string(AnsiString(cb->Items->Strings[i]).c_str()));
    w.EndArray();
    w.EndObject();
    return w.Str();
}
void W906_RunStartModeCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)
{
    extern void W906_CbRunStartModeChange();
    const std::string want = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
    if (want.empty() && fMain != 0 && fMain->cbRunStartMode != 0) {             // 只查詢（不改）：網頁開頁時拿目前模式與清單
        server.CompleteCommand((unsigned long long)wc.id, true, W906_RsmAckJson(want, LastSet.iRunStartMode));
        return;
    }
    int idx = -1;
    for (int i = 0; i < rsmRunModeTotal; ++i)
        if (StartModeName[i] == AnsiString(want.c_str())) { idx = i; break; }
    if (idx < 0 || fMain == 0 || fMain->cbRunStartMode == 0) {
        server.CompleteCommand((unsigned long long)wc.id, false,
                               "main.runStartMode: value must be one of StartModeName[] (e.g. \"Initial Start\")");
        return;
    }
    // golden 的操作員只能選清單裡有的（TfMain::SetStartModeData main.cpp:24118 依機種／設定填）。清單有內容就照它：不在清單裡＝拒絕，
    // ItemIndex 取清單位置；清單是空的（移植樹 SetStartModeData 目前是空殼，forms/fMain.cpp:456）就用 enum 編號（見 RunStartMode.cpp W906_RsmComboSyncText）。
    TfLotInfoRunMode* cb = fMain->cbRunStartMode;
    int pos = idx;
    if (cb->Items != 0 && cb->Items->GetCount() > 0) {
        pos = cb->Items->IndexOf(StartModeName[idx]);
        if (pos < 0) {
            server.CompleteCommand((unsigned long long)wc.id, false,
                                   "main.runStartMode: not in the start-mode list for this machine/recipe (golden SetStartModeData)");
            return;
        }
    }
    const int before = LastSet.iRunStartMode;
    cb->ItemIndex = pos;                                                        // golden: the operator picks the item
    cb->Text = StartModeName[idx];
    W906_CbRunStartModeChange();                                                // golden OnChange
    std::printf("main.runStartMode: '%s' -> iRunStartMode %d (was %d)%s\n", want.c_str(), LastSet.iRunStartMode, before,
                SystemStart ? "  [SystemStart: golden does not change the mode while running]" : "");
    server.CompleteCommand((unsigned long long)wc.id, SystemStart == 0 && LastSet.iRunStartMode == idx, W906_RsmAckJson(want, before));
}

// ===========================================================================
//  AI(W906-FLOW-1) 20260927: WS 指令 main.home —— 主畫面的 HOME 鈕（golden main.dfm:10635 `object BtnHome: TBtnPanel`，
//  OnClick = BtnHomeClick）。形狀照上面的 main.runStartMode：先照 golden 判斷操作員按不按得到這顆鈕，再跑 golden 的 OnClick。
//  value 不看（它是一顆鈕；訊框 = {cmd:"main.home"}）。
//
//  (1) 按不按得到：BtnHome->Enabled 在 golden 只有 TfMain::ProcessKeyFlush（main.cpp:4098-4151，Timer1Timer :3185 每 250 ms）
//      在管：`if(SystemStart) ... BtnHome->Enabled=false;`（:4107-4111），否則 `BtnHome->Enabled=true;`（:4150）。
//      移植樹沒有 ProcessKeyFlush（WebBridgeTags.cpp:2493 記過）也沒有 BtnHome 這個 widget（forms/fMain.cpp:946
//      GATE W906-HOME-W1-BTNHOME）⇒ 同一條規則在按下的當下算。它在 fNote／MyMessageBox 顯示中提早 return（:4103-4106，
//      維持原狀）—— 那兩種框在這裡是 modal 等待迴圈，等待中別的指令回 modal-pending，根本到不了這裡。
//      另一個 modal：golden 的教導框 fTeachShow 是 ShowModal，開著時主畫面整個按不到 —— 跟 start.run 用同一個判定
//      （W906_MotorAccessStartBlocked，WebMotorAccessLive.cpp:1007；覆核 W5B-R5）。
//  (2) 按下去：W906_BtnHomeClick() = golden BtnHomeClick 逐句。⚠ 沒有 SOFT_SIMULTE 的建置（真機）golden 這顆鈕**什麼都不做**，
//      除非 CosFunction.bEnableSoftWareControlButton（TSMC／建榮等，CosFunction.cpp:288/:1390/:3512）—— 真機的 HOME 是面板實體鍵
//      （TfMain::ScanKey main.cpp:2545-2564；裁決 Q32：面板鍵之後再做，INBOX 第 86 列）。ack 的 outcome 會寫 "noop"。
//  (3) Home() 只是「設旗標」（SoftStart／iHome，forms/fMain.cpp:976-977），真正的歸零在之後的 tick 由 MainProc → ScanSystemSensor
//      → fHome->iHomeStep 跑，所以這條指令不會卡住 tick 迴圈。兩個跟 golden 一樣的例外：ShowMyMessage／ShowErrorMessage 是 modal
//      等待（照 golden 的 modal 框）；真機建置的 DoMotorPowerOn()（csystem.cpp:14386，golden csystem.cpp:19102）等 1 秒。
//  權杖：不在 WebBridgeServer.cpp 的豁免清單 ⇒ 要先 control.acquire（同 start.run）。防連點：不在 WebCmdGuard.cpp 兩張白名單
//  ⇒ 400 ms 內重複的一下回 busy 不執行（預設策略，不用改表）。
//  不是 START：不呼叫 Start 家族、不直接寫啟動旗標（tools/start_sites_census.py 的 34/30/4 不變）。
// ===========================================================================
#include "MainCalcCore.h"       // AI(W906-FLOW-1) 20260927: ComputeCheckAutoOnlySetOneBin／ComputeCheckAuto1OnlyBin1（golden TfMain::CheckAutoOnlySetOneBin／CheckAuto1OnlyBin1 的判定）
#include "CosFunction.h"        // CosFunction.bEnableSoftWareControlButton／bUsePassBinOnlyCanSetOneBin
#include "Config.h"             // IniConfig.bP28Auto1OnlyBin1
#ifdef SOFT_SIMULTE
// golden TfMain::CheckAutoOnlySetOneBin（main.cpp:32523-32553）：判定是 MainCalcCore 的可攜版（MainCalcCore.cpp:556，逐句忠實、
//   不含對話框）；它拿掉的那一行 golden :32544 ShowMyMessage(str1, str2) 在這裡補回。
static bool W906_CheckAutoOnlySetOneBin()
{
    AnsiString str1, str2;                                                      // golden :32527
    if(ComputeCheckAutoOnlySetOneBin(CosFunction.bUsePassBinOnlyCanSetOneBin,   // golden :32528-32543
                                     Prod.iIsPassT6, Prod.iT6CatData, iTestBinCount, s6TrayName, str1, str2))
    {
        ShowMyMessage(str1, str2, "", false, false);                            // golden :32544（S3／Ok／bServoOff 照 golden mymessbox.h:58 的預設）
        return true;                                                            // golden :32545
    }
    return false;                                                               // golden :32552
}
// golden TfMain::CheckAuto1OnlyBin1（main.cpp:32502-32521）：同上（MainCalcCore.cpp:627）；對話框 golden :32511／:32516。
static bool W906_CheckAuto1OnlyBin1()
{
    AnsiString msg;
    if(ComputeCheckAuto1OnlyBin1(CUSTOMER_CODE, LastSet.iTester, Prod.iT6PosCate, iTestBinCount, msg))   // golden :32504-32519
    {
        ShowMyMessage(msg, "", "", false, false);                               // golden :32511／:32516（單參數＝S2 起照預設）
        return true;                                                            // golden :32512／:32517
    }
    return false;                                                               // golden :32505／:32520
}
#endif
// golden TfMain::BtnHomeClick（main.cpp:7120-7144）逐句；自由函式 ⇒ 成員呼叫改 fMain->（同 RunStartMode.cpp W906_CbRunStartModeChange）。
// PORT-ONLY：回傳值（golden 是 void）只為了 ack 說明走了哪一支 —— 0 = 什麼都沒做（golden 非模擬的 else 支）、1 = CheckAutoOnlySetOneBin 擋、
//   2 = P28 CheckAuto1OnlyBin1 擋、3 = Home() 回 false、4 = Home() 回 true。
int W906_BtnHomeClick()
{
    int r = 0;
    if(CosFunction.bEnableSoftWareControlButton)                                //ChungHung 20150609 add only for TSMC
    {
        bHomeByStart=false;
        r = fMain->Home("EnableSoftWareControlButton") ? 4 : 3;
    }
    else
    {
    #ifdef SOFT_SIMULTE
        if(W906_CheckAutoOnlySetOneBin())                                       // golden :7130 CheckAutoOnlySetOneBin()
        {
            return 1;
        }

        if(IniConfig.bP28Auto1OnlyBin1==true && W906_CheckAuto1OnlyBin1())     //Ifor 20171017 P28 功能整理
        {
            return 2;
        }

        bHomeByStart=false;
        r = fMain->Home("BtnHomeClick") ? 4 : 3;
    #endif
    }
    return r;
}
// ack：outcome／code／why／softStart／iHome／fAllMotorHome／systemStart／homeStep（fHome->iHomeStep）／softwareControlButton／softSimulte
static std::string W906_MainHomeAck(const char* outcome, int code, const std::string& why)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("outcome").String(outcome);
    w.Key("code").Number((wb_int64)code);
    w.Key("why").String(why);
    w.Key("softStart").Bool(SoftStart != 0);
    w.Key("iHome").Number((wb_int64)iHome);
    w.Key("fAllMotorHome").Bool(fAllMotorHome != 0);
    w.Key("systemStart").Bool(SystemStart != 0);
    w.Key("homeStep").Number((wb_int64)(fHome ? fHome->iHomeStep : -1));
    w.Key("softwareControlButton").Bool(CosFunction.bEnableSoftWareControlButton);
#ifdef SOFT_SIMULTE
    w.Key("softSimulte").Bool(true);
#else
    w.Key("softSimulte").Bool(false);
#endif
    w.EndObject();
    return w.Str();
}
void W906_MainHomeCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)
{
    extern bool W906_MotorAccessStartBlocked(std::string&);
    if (fMain == 0) {
        server.CompleteCommand((unsigned long long)wc.id, false, "main.home: fMain is null -- no main form in this binary");
        return;
    }
    if (SystemStart) {                                                          // golden ProcessKeyFlush :4107-4111 BtnHome->Enabled=false
        std::printf("main.home REFUSED: BtnHome is disabled while SystemStart (golden ProcessKeyFlush main.cpp:4107-4111)\n");
        server.CompleteCommand((unsigned long long)wc.id, false,
                               W906_MainHomeAck("disabled", -1, "BtnHome is disabled while SystemStart (golden ProcessKeyFlush main.cpp:4107-4111)"));
        return;
    }
    std::string tw;
    if (W906_MotorAccessStartBlocked(tw)) { tw = "HOME 拒絕：教導頁手動教導中（golden 主畫面在教導框 fTeachShow ShowModal 時按不到 HOME）；先在教導頁按確定或取消";   //AI(W906-ARTSEAM) 20260928: 判定照舊（同 start.run），原本把 START 的拒絕文字（WebMotorAccess.cpp:3745）原樣回給 HOME
        std::printf("main.home REFUSED: %s\n", tw.c_str());
        server.CompleteCommand((unsigned long long)wc.id, false, W906_MainHomeAck("teachModal", -1, tw));
        return;
    }
    const int r = W906_BtnHomeClick();                                          // golden OnClick
    static const char* const kOutcome[] = { "noop", "autoOnlySetOneBin", "auto1OnlyBin1", "homeRefused", "homeArmed" };
    static const char* const kWhy[] = {
        "golden BtnHomeClick does nothing in a build without SOFT_SIMULTE unless CosFunction.bEnableSoftWareControlButton (main.cpp:7127-7143); a real machine homes from the panel HOME key (TfMain::ScanKey main.cpp:2545)",
        "golden CheckAutoOnlySetOneBin refused (main.cpp:32544 message shown)",
        "golden CheckAuto1OnlyBin1 refused (IniConfig.bP28Auto1OnlyBin1; main.cpp:32511/32516 message shown)",
        "golden TfMain::Home returned false (main.cpp:6985-7067: SoftStart already set, or one of its checks refused)",
        "golden TfMain::Home armed the home cycle (SoftStart/iHome set, main.cpp:7100-7101); homing runs on the following ticks",
    };
    const int k = (r >= 0 && r <= 4) ? r : 0;
    std::printf("main.home: BtnHomeClick -> %s (code %d)  SoftStart=%d iHome=%d fAllMotorHome=%d\n",
                kOutcome[k], r, (int)SoftStart, (int)iHome, (int)fAllMotorHome);
    server.CompleteCommand((unsigned long long)wc.id, r == 4, W906_MainHomeAck(kOutcome[k], r, kWhy[k]));
}

//==============================================================================
//  AI(W906-FLOW-5) 20260929: golden ShowMyMessage's alarm servo-off of the In Arm X/Y --
//  906 golden mymessbox.cpp:832-849 (= V912 :851-868, the numbering the MyMessageBox host above uses), line for line.
//  Called from W906MbShowMyMessage at golden's position: after StopAllMotor (:811-812) and MyDBIProcess("Message", S1, S3)
//  (:830), before ShowModal (:860) -- so after every early return golden and the host share (SOFT_SIMULTE " port error",
//  a box already up with iUnLoaderCount==0) and after the host's InitialOK==false return (V912 :784); not on the
//  page-action capture path (the host does not stop the machine there either).
//  Only [W906] addition: the return value (true = this call switched the servo off), which MbPost reports as
//  requestedSideEffects.servoOffInArmXY -- that field was the constant false.  golden returns nothing.
//  Callers passing bServoOff=true (git grep 20260929): acarry.cpp:3295 :3973 :3983 :3989 :5829 :5839 :5845 and
//  csystem.cpp:7367 :7378 :24016 -- each already behind IniConfig.bAlarmNeedServoOff, as in golden.
//  WHY THIS CANNOT PARK THE IN ARM FOR GOOD: DoInArm_9045 returns while fNote->bMyServoOffInArm is true
//  (ainarm9045.cpp:904-911 = golden ainarm9045.cpp:4455-4461); three live paths clear it --
//    * DoServoOn's In Arm arm (csystem.cpp:22032-22139 = golden csystem.cpp:20742-20849), which MainProc's SystemStart
//      branch calls every tick (csystem.cpp:31730, plus the form branches :31269 :31367 :31437 :31456 :31483 :31515):
//      START re-energises X/Y, jogs back to iMyServoOffInArmPosX/Y and clears the flag; if a Z picker is still down it
//      stops all motors and homes the In Arm instead (golden :20833-20841); a CW/CCW limit on asks for a manual fix (MES0171/2);
//    * ProcessMotorHome (uhome.cpp:1073 = golden uhome.cpp:1396) -- HOME;
//    * DoInArmPineRelease (AutoClean/AutoClean.cpp:3273/:3282 = golden AutoClean.cpp:5838/:5847).
//  MOT[].ServoOnOff / ReadEncoderPos are NULL-guarded (Motor/mymotor.cpp:1343 / :278): with no card they do nothing,
//  and TMyMotor::MotorMove returns -1 (Motor/mymotor.cpp:5911-5915, truthy) so DoServoOn clears the flag at once.
//==============================================================================
bool W906_MbServoOffInArmLikeGolden(bool bServoOff)
{
    AnsiString str;
    bool bDone=false;                                                           // [W906] only for the mailbox report
    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20110802
    {
        if(fNote->bMyServoOffInArm==false)
        {
            if(bServoOff && InArmZSafe(DETECT_ALL_FLAG)==-1)                    //如果Z軸在上才可以推
            {
                fNote->iMyServoOffInArmPosX=MOT[MInArmX].ReadEncoderPos();
                fNote->iMyServoOffInArmPosY=MOT[MInArmY].ReadEncoderPos();      //要在ServoOff之前
                MySleep(200);
                MOT[MInArmX].ServoOnOff(false);
                MOT[MInArmY].ServoOnOff(false);

                str="InArm ServoOff"+AnsiString("RecordX = ") + AnsiString(fNote->iMyServoOffInArmPosX) +"RecordY = " + AnsiString(fNote->iMyServoOffInArmPosY);
                RecordProcess(str);
                fNote->bMyServoOffInArm=true;
                bDone=true;                                                     // [W906]
            }
        }
    }
    return bDone;
}
//AI(W906-MERGE-0929) 20260929: review TK-1 (takeover merge, machine 0016).  The motor dead-man (WebMotorAccess.h:438-440, :564-567) was keyed on
//  "no owner" (ControlOwner()==0).  Since control.takeover can move the token from connection X to Y directly, X never goes
//  through 0: closing Motor Test after another page took over left its LoopMove / jog running.  Report the beat in which the
//  owner changes from one connection to another as "operator gone" (the old holder's jobs are cancelled -- the running leg
//  finishes -- and its jogs stop, the same as when the operator connection disappears); the next beat reports the new holder.
//  Each call site keeps its own "previous owner" so both ticks see the change.  Same-page takeovers keep the owner and change
//  nothing.
bool W906_OwnerHeldSince(unsigned long long owner, unsigned long long& prev)
{
    const bool changed = (prev != 0 && owner != 0 && owner != prev);
    prev = owner;
    return owner != 0 && !changed;
}
// =============================================================================
// AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生表單原型（HW.IoSetView／Main.MotorView 唯讀）在 wb_serve 這一側的部分。
//   呼叫點是五處同一行插入：:4536 開窗（主迴圈前）＋登記 nextPump、:4538 登記 nextIo、:4575 主迴圈每一圈泵（site 0）、
//   :5967 正常關閉、:7622 阻塞等待框每一圈泵（site 1）。視窗本體與資料膠水在 ui/native/（ui/native/NativeForms.cmake 掛進來）。
//   OFF（預設）：只編下面 #else 的空函式 —— 每個呼叫點只多呼叫一次空函式，行為與沒有這個開關時相同。說明 ui/native/README.md。
// =============================================================================
#ifdef W906_NATIVE_FORMS
// -----------------------------------------------------------------------------
// 拖曳／改大小原生視窗時的保活（為什麼要、怎麼觸發：ui/native/NativeHost.h 檔頭）。
// 這兩支要用本檔的 static（WdMark、g_apiCacheDirty、g_modalServer、kServeTickMs、kIoTickMs）與 main() 的截止時間，所以放在這裡。
//
// W906_NativeKeepaliveMain（site 0：主迴圈頂端的泵正在分派訊息、Windows 的內部迴圈在跑）——
//   照主迴圈的週期工作逐行照抄、順序相同：
//     :4575 W906_ServiceOutputs(0)／W906_TesterCommTick()／W906_Timer2TestSecondsTick()（每一圈）
//     :4577-:4582 500 ms 拍子 —— **共用 main() 的 nextPump**（:4536 登記），所以 PumpTick 不會變快（B13），放開後也不會連發補拍
//     :4593 WdMark（看門狗心跳；寫成 "tick (native window drag keepalive)"，卡住時看得出是在這裡）
//     :4597 PumpTick（W906_IoTiming 1／2 包住）＋W906_ServiceOutputs(0)＋g_apiCacheDirty
//     :4598 W906_StateRecordTimer2Pump()＋W906_MotorAccessTick(owner)
//     :4618-:4624 Pci1203AxisIniTick（WB_PUMP_1203_CONTROL，每一圈）
//     :4646-:4659 1203 Poll —— 共用 main() 的 nextIo（:4538 登記）＋W906_MotorAccessPollTick(owner)
//   ⚠ 沒做：一般網頁指令的 drain／分派、快照發布（main() 裡的內嵌程式，不能在這裡照抄）。
//     ⇒ 拖曳期間一般網頁指令排隊，放開滑鼠後的第一圈才執行；網頁畫面暫停更新。
//     但輸出與停止照常：上面每一圈的 W906_ServiceOutputs(0)（:6303）就是輸出優先服務 —— io.btnPanelClick、
//     pci1203.do.*、pci1203.ax.stop／emgStop、motor.stop（:6193-6205），照它的屏障規則（停止不超過排在前面的 jog／move／home）。
//   ⚠ 主迴圈內容改了，這裡要跟著改（兩邊不同步＝拖曳時少跑某件事）。
//
// W906_NativeKeepaliveModal（site 1：阻塞等待框（告警／是否／ShowMyMessage）裡的泵）——
//   那三個等待迴圈每一圈都跑 DoAvoidIndexMotorFallDown（:537／:806／:6760，golden MyMessageBox::Timer1Timer），
//   拖曳時照跑這一支；不跑 PumpTick（我們本來就在 PumpTick 裡面，重入 MainProc 不可以）、不掃面板鍵（會吃掉等待框自己要的答案）。
// -----------------------------------------------------------------------------
namespace {
DWORD* g_nfPumpClock = 0;
DWORD* g_nfIoClock = 0;
}
void W906_NativeFormsSetPumpClock(DWORD* p) { g_nfPumpClock = p; }
void W906_NativeFormsSetIoClock(DWORD* p) { g_nfIoClock = p; }

void W906_NativeKeepaliveMain()
{
    { extern void W906_ServiceOutputs(int); W906_ServiceOutputs(0); }  { extern void W906_TesterCommTick(); W906_TesterCommTick(); }  { extern void W906_StateRecordDrainLog(); W906_StateRecordDrainLog(); }  { extern void W906_Timer2TestSecondsTick(); W906_Timer2TestSecondsTick(); }  { extern int W906_MainCtlButtonTick(); W906_MainCtlButtonTick(); }  /* AI(W906-FLOW-4) 20260930: main-screen ONE CYCLE / TRAY FEED / ALARM RESET clicks St01 queued (act.main.ctlButton, FileRW/MainClick.cpp) -> golden OnClick bodies, once per click, same as the main loop line (:4575) (WebMainCtlButtons.cpp; RESET and the site cell deliberately not taken, see there) */   //AI(W906-MERGE-NATIVE) 20260929: the main loop line (:4575) gained St02's W906_StateRecordDrainLog (MR !3) after this copy was made -- same order here
    const DWORD now = ::GetTickCount();
    bool pumpBeat = false;
    if (g_nfPumpClock) {
        DWORD& nextPump = *g_nfPumpClock;
        pumpBeat = (long)(now - nextPump) >= 0;
        if (pumpBeat) {
            nextPump += static_cast<DWORD>(kServeTickMs);
            if ((long)(now - nextPump) >= 0) nextPump = now + static_cast<DWORD>(kServeTickMs);
        }
    }
    WdMark("tick (native window drag keepalive)");
    const bool owner = (g_modalServer != 0) && g_modalServer->ControlOwner() != 0;  (void)owner;   //AI(W906-MERGE-NATIVE) 20260929: the two motor ticks below now take W906_OwnerHeldSince (TK-1); kept for the comment above
    { extern void W906_IoTiming(int); if (pumpBeat) { W906_IoTiming(1); ht9045::PumpTick(); W906_IoTiming(2); extern void W906_ServiceOutputs(int); W906_ServiceOutputs(0); }  if (pumpBeat) g_apiCacheDirty = true; }
    if (pumpBeat) { extern void W906_StateRecordTimer2Pump(); W906_StateRecordTimer2Pump(); }  if (pumpBeat) { extern void W906_MotorAccessTick(bool); extern bool W906_OwnerHeldSince(unsigned long long, unsigned long long&); static unsigned long long s_keepOwnerTick = 0; W906_MotorAccessTick(W906_OwnerHeldSince(g_modalServer ? g_modalServer->ControlOwner() : 0ull, s_keepOwnerTick)); }   //AI(W906-MERGE-NATIVE) 20260929: TK-1 (8c6ac9a0): the main loop's motor ticks use W906_OwnerHeldSince (a takeover = operator gone for the old holder, one beat)
#ifdef WB_PUMP_1203_CONTROL
    {
        std::string iniNote;
        if (ht9045::Pci1203AxisIniTick(iniNote) && !iniNote.empty())
            std::printf("%s\n", iniNote.c_str());
    }
#endif
#ifdef INSTALL_1203_MONITOR
    if (g_nfIoClock) {
        DWORD& nextIo = *g_nfIoClock;
        if ((long)(now - nextIo) >= 0) {
            nextIo += static_cast<DWORD>(kIoTickMs);
            if ((long)(now - nextIo) >= 0) nextIo = now + static_cast<DWORD>(kIoTickMs);
            if (ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor()) {
                WdMark("1203 Poll");
                { extern void W906_IoTiming(int); W906_IoTiming(3); mon->Poll(); W906_IoTiming(4); }  { extern void W906_MotorAccessPollTick(bool); extern bool W906_OwnerHeldSince(unsigned long long, unsigned long long&); static unsigned long long s_keepOwnerPoll = 0; W906_MotorAccessPollTick(W906_OwnerHeldSince(g_modalServer ? g_modalServer->ControlOwner() : 0ull, s_keepOwnerPoll)); }   //AI(W906-MERGE-NATIVE) 20260929: TK-1, same as the main loop's Poll line
                WdMark("tick (native window drag keepalive)");
                g_apiCacheDirty = true;
            }
        }
    }
#endif
}

void W906_NativeKeepaliveModal()
{
    { extern void DoAvoidIndexMotorFallDown(); DoAvoidIndexMotorFallDown(); }
}
#else
void W906_NativeFormsStart() {}
void W906_NativeFormsPump(int) {}
void W906_NativeFormsStop() {}
void W906_NativeFormsSetPumpClock(DWORD*) {}
void W906_NativeFormsSetIoClock(DWORD*) {}
#endif

// =============================================================================
//  AI(W906-OPLOG) 20260928: 操作紀錄 —— 不是 golden。
//  EastSun 20260928：「接下來我要測試 motor test 功能，你能夠透過操作的LOG來知道我的動作對吧? 跟你反應問題你能夠處理清楚解嗎?
//  不能清楚解析的話，要強化LOG和state record功能」。在這之前 Motor Test 的按鍵與伺服器的回覆只印在 VS Code 的除錯主控台
//  （沒有檔案），log.event 環也是 0 筆（Motor Test 頁不送）⇒ 事後無法知道按了什麼、伺服器回了什麼、軸當時是什麼狀態。
//
//  檔案：%W906_OPLOG_DIR%\oplog_YYYYMMDD.txt（UTF-8）。沒設這個環境變數就一行都不寫、勾子也不裝（WebBridgeServer 的
//        g_W906OpLogHook 維持 0）。F5「IOWEB(這台)」兩個設定都設成 runcfg\logs。每行寫完就 fflush，跑的同時就讀得到。
//  每一行：HH:MM:SS.mmm  種類  內容
//    START  wb_serve 起來了（pid、port、權杖擋不擋人、exe 建置時間）
//    CMD    網頁送來的指令：c<連線>#<指令 id>  原始 JSON（最多 600 字）
//    OK/NG  它的回覆：c<連線>#<id> <指令名>  結果或拒絕理由（最多 400 字）  (<ms>)。socket 執行緒當場回的
//           （control.*、not-operator、格式錯、佇列滿）標 socket；其他是 tick 執行緒做完才回的
//    MOT    某一軸的狀態變了：只列變了的欄位 舊->新，| 後面附指令位置／編碼器位置；每軸第一筆是 baseline
//           欄位：svo servo、alm 警報、st 卡的軸狀態、busy 在動、homing 回原點中、homeJob／loopJob Motor Test 的工作、
//           homeFlag 引擎的原點旗標、cw／home／ccw／emg／ledAlm／softCw／softCcw 燈號、q 資料品質、err 錯誤文字、
//           cardErr／cardErrText 卡片自己的錯誤碼與廠商文字（Acm_GetLastError）、drvAlm 驅動器 603Fh 警報（20260929 加）
//           （來源＝網頁讀的同一份 /api/struct/motor/runtime，W906_ApiCacheRefresh 每次重建後比一次）
//    PAGE   Motor Test 頁：鎖（locked／lockText）、馬達電源（relay／power／powerPending）、選的馬達、Light Scale
//    ALARM／MODAL／QUERY  伺服器推給畫面的框（警報、訊息框、要按鈕回答的框，原始 JSON 最多 800 字）；
//           QUERY qid N answered / closed = 那一題答掉或放棄了；操作員按的鈕是後面那一行 CMD modal.answer
//           （AI(W906-OPLOG) 20260929：EastSun 問「move 沒反應並跳出 Alarm 要如何查修」—— 框的內容之前沒記）
//    SREC   State Record 背景工作的每一步（開始、批次逾時、完成／沒壓成 zip、7z 的離開碼）—— AI(W906-SR-HANG) 20260929
//    HMI    操作畫面被關掉、自動重開的每一次（W906_HmiKeeperTick，檔尾）—— AI(W906-HMI-KEEP) 20260929
//  太吵的（sys.ping、ui.windows.put、cfg.resync、*.get 查詢）成功時不記，失敗照記。
//  執行緒：CMD 與 socket 回覆在 socket 執行緒；tick 回覆、MOT、PAGE 在 tick 執行緒 —— 共用 g_opLock。
//  放在檔尾，上面的行號都不動。
// =============================================================================
namespace webbridge { extern void (*g_W906OpLogHook)(const char*, unsigned long long, double, bool, const std::string&, const std::string&); }
namespace {
CRITICAL_SECTION g_opLock;
bool             g_opOn = false;
std::string      g_opDir;
std::FILE*       g_opFile = 0;
int              g_opDay = 0;
struct OpPend { std::string cmd; DWORD t0; bool quiet; bool secret; };
std::map<std::pair<unsigned long long, long long>, OpPend> g_opPend;           // (連線, 指令 id) -> 還沒回覆的
typedef std::vector<std::pair<std::string, std::string> > OpFields;
std::map<std::string, OpFields> g_opMot;                                        // 每軸上一次的欄位（tick 執行緒）
OpFields g_opPage;
bool     g_opPageBase = false;

// 換行／tab 換成空白，超過 n 位元組就截掉並註明截了多少（不切在 UTF-8 字的中間）
std::string OpClip(const std::string& s, std::size_t n)
{
    std::string t;
    for (std::size_t i = 0; i < s.size() && t.size() < n; ++i) t += (s[i] == '\r' || s[i] == '\n' || s[i] == '\t') ? ' ' : s[i];
    if (s.size() > n) {
        if (((unsigned char)s[n] & 0xC0) == 0x80) {
            std::size_t k = t.size();
            while (k > 0 && ((unsigned char)t[k - 1] & 0xC0) == 0x80) --k;
            if (k > 0) --k;
            t.resize(k);
        }
        char b[32];
        std::snprintf(b, sizeof(b), "...(+%u)", (unsigned)(s.size() - t.size()));
        t += b;
    }
    return t;
}

void OpLine(const char* kind, const std::string& text)
{
    if (!g_opOn) return;
    ::EnterCriticalSection(&g_opLock);
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    const int day = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
    if (g_opFile == 0 || day != g_opDay) {                                      // 每天一個檔
        if (g_opFile) std::fclose(g_opFile);
        char name[48];
        std::snprintf(name, sizeof(name), "\\oplog_%04d%02d%02d.txt", st.wYear, st.wMonth, st.wDay);
        g_opFile = std::fopen((g_opDir + name).c_str(), "ab");
        g_opDay = day;
        if (g_opFile) { std::fseek(g_opFile, 0, SEEK_END); if (std::ftell(g_opFile) == 0) std::fputs("\xEF\xBB\xBF", g_opFile); }
    }
    if (g_opFile) {
        std::fprintf(g_opFile, "%02d:%02d:%02d.%03d  %-5s %s\r\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, kind, text.c_str());
        std::fflush(g_opFile);
    }
    ::LeaveCriticalSection(&g_opLock);
}

bool OpQuiet(const std::string& c)
{
    // control.acquire／control.release：網頁一直在拿／還（閒置 30 秒自動還、IO 頁 30 秒還、keepAlive），0929 第一次實測
    // 4 分鐘 422 行、佔全檔 36%；權杖現在不擋人（W906_WEB_TOKEN_ENFORCE 0），成功的只是雜訊。control.takeover 照記（誰接管）。
    if (c == "sys.ping" || c == "ui.windows.put" || c == "cfg.resync" || c == "control.acquire" || c == "control.release") return true;
    return c.size() > 4 && c.compare(c.size() - 4, 4, ".get") == 0;
}

// 帳號／密碼類：指令內容與回覆都不寫進檔案（登入、改密碼、密碼簿會帶明碼），只記指令名、成敗、guard
bool OpSecret(const std::string& c, const std::string& frame)
{
    if (c.compare(0, 5, "auth.") == 0 || c.compare(0, 9, "security.") == 0) return true;
    std::string low(frame);
    for (std::size_t i = 0; i < low.size(); ++i) if (low[i] >= 'A' && low[i] <= 'Z') low[i] = (char)(low[i] - 'A' + 'a');
    return low.find("passw") != std::string::npos || low.find("pwd") != std::string::npos || low.find("pwbook") != std::string::npos;
}

// 回覆裡只挑 executed／guard 兩個欄位出來（帳號／密碼類用）
std::string OpSecretSummary(const std::string& r)
{
    std::string s;
    std::string::size_type p = r.find("\"executed\":");
    if (p != std::string::npos) s += r.substr(p, r.compare(p + 11, 4, "true") == 0 ? 15 : 16);
    p = r.find("\"guard\":\"");
    if (p != std::string::npos) { const std::string::size_type e = r.find('"', p + 9); if (e != std::string::npos) s += (s.empty() ? "" : ",") + OpClip(r.substr(p, e - p + 1), 80); }
    return "(內容不記：帳號／密碼類)" + (s.empty() ? std::string() : " " + s);
}

std::string OpId(unsigned long long conn, double id)
{
    char b[48];
    std::snprintf(b, sizeof(b), "c%lu#%.0f", (unsigned long)conn, id);
    return b;
}

void W906_OpHook(const char* kind, unsigned long long conn, double id, bool ok, const std::string& a, const std::string& b)
{
    if (!g_opOn) return;
    const std::pair<unsigned long long, long long> key(conn, (long long)id);
    if (std::strcmp(kind, "RECV") == 0) {
        const bool quiet = OpQuiet(a);
        const bool secret = OpSecret(a, b);
        OpPend p;
        p.cmd = a; p.t0 = ::GetTickCount(); p.quiet = quiet; p.secret = secret;
        ::EnterCriticalSection(&g_opLock);
        g_opPend[key] = p;
        if (g_opPend.size() > 2000) g_opPend.erase(g_opPend.begin());         // 斷線的那些永遠等不到回覆：設上限
        ::LeaveCriticalSection(&g_opLock);
        if (!quiet) OpLine("CMD", OpId(conn, id) + "  " + (secret ? "{\"cmd\":\"" + OpClip(a, 64) + "\"} (內容不記：帳號／密碼類)" : OpClip(b, 600)));
        return;
    }
    if (std::strcmp(kind, "POST") == 0) {                                       // 伺服器推給畫面的框（WebBridgeServer PostAlarm／PostModal／PostQuery／PostQueryOptions／ClearQuery）
        static double lastCleared = -1;                                         // ClearQuery 對同一題可能叫不只一次：只記第一次
        if (a == "clear") {
            if (id == lastCleared) return;
            lastCleared = id;
            char q[64];
            std::snprintf(q, sizeof(q), "qid %.0f answered / closed", id);
            OpLine("QUERY", q);
        } else {
            OpLine(a == "alarm" ? "ALARM" : (a == "modal" ? "MODAL" : "QUERY"), OpClip(b, 800));
        }
        return;
    }
    OpPend p;
    bool have = false;
    ::EnterCriticalSection(&g_opLock);
    std::map<std::pair<unsigned long long, long long>, OpPend>::iterator it = g_opPend.find(key);
    if (it != g_opPend.end()) { p = it->second; have = true; g_opPend.erase(it); }
    ::LeaveCriticalSection(&g_opLock);
    if (!have && ok) return;                                                    // ping 的 ack 之類
    if (have && p.quiet && ok) return;
    const bool sock = (std::strcmp(kind, "ACK") == 0);
    char ms[48];
    if (have) std::snprintf(ms, sizeof(ms), "  (%lu ms%s)", (unsigned long)(::GetTickCount() - p.t0), sock ? ", socket" : "");
    else      std::snprintf(ms, sizeof(ms), "  (%s)", sock ? "socket" : "tick");
    OpLine(ok ? "OK" : "NG", OpId(conn, id) + " " + (have ? p.cmd : std::string("?")) + "  " + (have && p.secret ? OpSecretSummary(a) : OpClip(a, 400)) + ms);
}

const cJSON* OpGet(const cJSON* o, const char* k) { return o ? cJSON_GetObjectItemCaseSensitive(o, k) : 0; }

std::string OpVal(const cJSON* j)
{
    if (!j || cJSON_IsNull(j)) return "null";
    if (cJSON_IsBool(j)) return cJSON_IsTrue(j) ? "1" : "0";
    if (cJSON_IsNumber(j)) {
        char b[48];
        const double v = j->valuedouble;
        if (v > -1e15 && v < 1e15 && v == (double)(long long)v) std::snprintf(b, sizeof(b), "%.0f", v);
        else std::snprintf(b, sizeof(b), "%.3f", v);
        return b;
    }
    if (cJSON_IsString(j) && j->valuestring) return std::string("\"") + OpClip(j->valuestring, 160) + "\"";
    return "?";
}

void OpAdd(OpFields& f, const char* k, const cJSON* j) { f.push_back(std::make_pair(std::string(k), OpVal(j))); }

std::string OpAll(const OpFields& f)
{
    std::string s;
    for (std::size_t i = 0; i < f.size(); ++i) s += (i ? " " : "") + f[i].first + "=" + f[i].second;
    return s;
}

void W906_OpSrNote(const char* s) { OpLine("SREC", s ? std::string(s) : std::string()); }  void W906_OpBrakeNote(const char* s) { OpLine("BRAKE", s ? std::string(s) : std::string()); }   //AI(W906-SR-HANG) 20260929: State Record background worker steps (cStateRecord.cpp g_W906StateRecordNote); AI(W906-BRAKE-AXIS) 20260929: per-axis brake release / hold (WebMotorAccessLive.cpp g_W906BrakeNote) -- 2nd fix: it sat behind this comment

std::string OpDiff(const OpFields& was, const OpFields& now)
{
    std::string d;
    for (std::size_t i = 0; i < now.size(); ++i) {
        const std::string* old = 0;
        for (std::size_t k = 0; k < was.size(); ++k) if (was[k].first == now[i].first) { old = &was[k].second; break; }
        if (old && *old == now[i].second) continue;
        d += (d.empty() ? "" : "  ") + now[i].first + " " + (old ? *old : std::string("-")) + "->" + now[i].second;
    }
    return d;
}
}  // namespace

void W906_OpLogInit(int port)
{
    const char* d = std::getenv("W906_OPLOG_DIR");
    if (!d || !*d) { std::printf("oplog: off (W906_OPLOG_DIR not set)\n"); return; }
    ::InitializeCriticalSection(&g_opLock);
    g_opDir = d;
    while (!g_opDir.empty() && (g_opDir[g_opDir.size() - 1] == '\\' || g_opDir[g_opDir.size() - 1] == '/')) g_opDir.erase(g_opDir.size() - 1);
    ::CreateDirectoryA(g_opDir.c_str(), 0);                                     // 已存在就什麼都不做
    g_opOn = true;
    char exe[MAX_PATH] = "";
    ::GetModuleFileNameA(0, exe, MAX_PATH);
    char built[32] = "?";
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (::GetFileAttributesExA(exe, GetFileExInfoStandard, &fa)) {
        FILETIME lt; SYSTEMTIME s;
        ::FileTimeToLocalFileTime(&fa.ftLastWriteTime, &lt);
        ::FileTimeToSystemTime(&lt, &s);
        std::snprintf(built, sizeof(built), "%04d-%02d-%02d %02d:%02d:%02d", s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);
    }
    char head[128];
    std::snprintf(head, sizeof(head), "wb_serve pid=%lu port=%d token-enforce=%d exe-built=", (unsigned long)::GetCurrentProcessId(), port,
                  1);   //AI(W906-OPLOG-MERGE) 20260929: main has no machine patch 0018 TOKEN-OFF (W906_WEB_TOKEN_ENFORCE; Jimmy to decide), so the token is always enforced here -- restore `(int)(W906_WEB_TOKEN_ENFORCE != 0)` if 0018 is taken
    OpLine("START", std::string(head) + built + "  " + exe);
    webbridge::g_W906OpLogHook = &W906_OpHook;
    { extern void (*g_W906StateRecordNote)(const char*); g_W906StateRecordNote = &W906_OpSrNote; }  { extern void (*g_W906BrakeNote)(const char*); g_W906BrakeNote = &W906_OpBrakeNote; }   //AI(W906-SR-HANG) 20260929; AI(W906-BRAKE-AXIS) 20260929 (2nd fix: the registration sat behind this comment)
    std::printf("oplog: %s\\oplog_<yyyymmdd>.txt\n", g_opDir.c_str());
}

void W906_OpLogMotorRuntime(const std::string& json)
{
    if (!g_opOn) return;
    cJSON* root = cJSON_Parse(json.c_str());
    if (!root) return;
    const cJSON* ms = OpGet(root, "motors");
    if (ms && cJSON_IsArray(ms)) {
        for (const cJSON* m = ms->child; m; m = m->next) {
            const cJSON* idj = OpGet(m, "motorId");
            if (!idj || !cJSON_IsString(idj) || !idj->valuestring) continue;
            const std::string id = idj->valuestring;
            const cJSON* st  = OpGet(m, "state");
            const cJSON* mo  = OpGet(m, "motion");
            const cJSON* dg  = OpGet(m, "diag");
            const cJSON* led = OpGet(st, "led");
            const cJSON* pos = OpGet(m, "position");
            OpFields f;
            OpAdd(f, "svo", OpGet(st, "servoOn"));
            OpAdd(f, "alm", OpGet(st, "alarm"));
            OpAdd(f, "st", OpGet(dg, "stateText"));
            OpAdd(f, "busy", OpGet(mo, "busy"));
            OpAdd(f, "homing", OpGet(mo, "homeBusy"));
            OpAdd(f, "homeJob", OpGet(mo, "homeJob"));
            OpAdd(f, "loopJob", OpGet(mo, "loopJob"));
            OpAdd(f, "homeFlag", OpGet(dg, "homeFlag"));
            if (led && cJSON_IsObject(led)) {
                OpAdd(f, "cw", OpGet(led, "cw"));
                OpAdd(f, "home", OpGet(led, "home"));
                OpAdd(f, "ccw", OpGet(led, "ccw"));
                OpAdd(f, "emg", OpGet(led, "emg"));
                OpAdd(f, "ledAlm", OpGet(led, "alarm"));
                OpAdd(f, "softCw", OpGet(led, "softCw"));
                OpAdd(f, "softCcw", OpGet(led, "softCcw"));
            } else {
                f.push_back(std::make_pair(std::string("led"), std::string("null")));
            }
            OpAdd(f, "q", OpGet(st, "quality"));
            OpAdd(f, "err", OpGet(dg, "errText"));
#ifdef INSTALL_1203_MONITOR
            {   //AI(W906-OPLOG) 20260929: WHY an axis went ERROR_STOP -- EastSun's JOG+ on MInArmX (09:26:10) logged only "alm 0->1 st
                //  READY->ERROR_STOP". The monitor already reads the card's own reason (Acm_GetLastError + the vendor's text, e.g.
                //  0x80005111 "Positive hardware limit has been exceeded", Pci1203Monitor.h driveErr) and the drive's 603Fh alarm;
                //  matched to the row by diag.station / diag.stationAxis like ChanMotorPoints FindCardAxis (ambiguous stations refused).
                const cJSON* stn = OpGet(dg, "station");
                const cJSON* sax = OpGet(dg, "stationAxis");
                ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor();
                if (mon && stn && sax && cJSON_IsNumber(stn) && cJSON_IsNumber(sax)) {
                    const int st0 = (int)stn->valuedouble, sa0 = (int)sax->valuedouble;
                    for (int k = 0; k < mon->axisCount(); ++k) {
                        const ht9045::Pci1203AxisSample& a = mon->axis(k);
                        if (!a.opened || a.stationAmbiguous || a.station != st0 || a.stationAxis != sa0) continue;
                        char hx[32];
                        std::snprintf(hx, sizeof(hx), "0x%08lX", (unsigned long)a.driveErr);
                        f.push_back(std::make_pair(std::string("cardErr"), std::string(a.driveErr ? hx : "0")));
                        f.push_back(std::make_pair(std::string("cardErrText"), "\"" + OpClip(a.driveErrText, 160) + "\""));
                        std::snprintf(hx, sizeof(hx), "0x%04X", (unsigned)a.driveAlarm);
                        f.push_back(std::make_pair(std::string("drvAlm"), a.driveAlarmValid
                            ? std::string(a.driveAlarm ? hx : "0") + (a.driveAlarmName.empty() ? std::string() : " " + OpClip(a.driveAlarmName, 80))
                            : std::string("?")));
                        //AI(W906-HOME-VENDOR) 20260929: the drive's own homing objects as the monitor last read them (6099h:1 search-switch
                        //  speed / 6099h:2 search-zero speed / 609Ah acc -- re-read after every home, MarkCfgDue kCfgDrive) and its user units
                        //  (2701h position n/d = electronic gear, 2702h velocity n/d; gearVal[0..3] = Pci1203Gear.h kGearPosNum..kGearVelDen),
                        //  to settle why a home at the Utility's PTP numbers crawls (EastSun 20260929).
                        char hb[96];
                        if (a.driveHomeValid[0] && a.driveHomeValid[1] && a.driveHomeValid[2])
                            std::snprintf(hb, sizeof(hb), "%ld/%ld acc%ld", a.driveHomeVal[0], a.driveHomeVal[1], a.driveHomeVal[2]);
                        else std::snprintf(hb, sizeof(hb), "?");
                        f.push_back(std::make_pair(std::string("drvHome"), std::string(hb)));
                        if (a.gearValid[0] && a.gearValid[1] && a.gearValid[2] && a.gearValid[3])
                            std::snprintf(hb, sizeof(hb), "pos%lu/%lu vel%lu/%lu", a.gearVal[0], a.gearVal[1], a.gearVal[2], a.gearVal[3]);
                        else std::snprintf(hb, sizeof(hb), "?");
                        f.push_back(std::make_pair(std::string("gear"), std::string(hb)));
                        break;
                    }
                }
            }
#endif
            const std::string where = "cmd=" + OpVal(OpGet(pos, "cmdPos")) + " enc=" + OpVal(OpGet(pos, "encPos"));
            std::map<std::string, OpFields>::iterator it = g_opMot.find(id);
            if (it == g_opMot.end()) {
                OpLine("MOT", id + " baseline  " + OpAll(f) + "  | " + where);
                g_opMot[id] = f;
            } else {
                const std::string dd = OpDiff(it->second, f);
                if (!dd.empty()) { OpLine("MOT", id + "  " + dd + "  | " + where); it->second = f; }
            }
        }
    }
    const cJSON* lk = OpGet(root, "lock");
    const cJSON* pw = OpGet(root, "motorPower");
    const cJSON* rt = OpGet(root, "runtime");
    const cJSON* ls = OpGet(root, "lightScale");
    OpFields pg;
    OpAdd(pg, "locked", OpGet(lk, "locked"));
    OpAdd(pg, "lockText", OpGet(lk, "text"));
    OpAdd(pg, "relay", OpGet(pw, "relayOn"));
    OpAdd(pg, "power", OpGet(pw, "motorPowerState"));
    OpAdd(pg, "powerPending", OpGet(pw, "pending"));
    OpAdd(pg, "selected", OpGet(rt, "selectedMotor"));
    OpAdd(pg, "lightScale", OpGet(ls, "active"));
    if (!g_opPageBase) { OpLine("PAGE", "baseline  " + OpAll(pg)); g_opPage = pg; g_opPageBase = true; }
    else {
        const std::string dd = OpDiff(g_opPage, pg);
        if (!dd.empty()) { OpLine("PAGE", dd); g_opPage = pg; }
    }
    cJSON_Delete(root);
}

// =============================================================================
//  AI(W906-JAM-STOP) 20260930: the host of golden ShowMotorErrorMessage's fNote->ShowModal() (note.cpp:1133) -- INBOX 118.
//  Installed as W906_ShowMotorErrorMessage_Hook next to W906_ShowErrorMessage_Hook (main); the caller is the golden body
//  (forms/fNote_ShowError.cpp EOF), after its stop half (StopAllMotor / Galil ST / IndexMotorBreakerOFF, golden
//  :1054-1058) and its record half (MyDBIEvent / ProductionLog / ErrShowToForm / the event-log line, golden :1080-1127).
//
//  DISPLAY PATH: the SAME one a kcode==0 ShowErrorMessage notice takes -- ForwardShowErrorMessage(code, 0, pos): the
//    kShown warning is skipped (kcode 0), and its AI(W906-Q30-KZERO) branch writes the dialog mailbox
//    (Alarm-dialog-request, via DialogMailboxPostAlarm -- the one writer; the unit alias / message ride in on
//    g_w906MotorNoteUnit / g_w906MotorNoteMsg), raises + clears the alarm ring entry (sjson EmitAlarm / ClearAlarm), prints
//    "NOTE, not blocking" and returns 0.  No PostQuery, no wait loop, no DialogMailboxRetire.  g_w906MotorNoteMsg also makes
//    it skip ShowErrorMessage's own stop (W906_AlarmStopLikeGolden, golden :795-801) and record (golden :839-868) -- the motor
//    body did its own.  WHY IT CANNOT BLOCK: golden :1110 sets KeyCode=0 right before this call, and even a non-zero KeyCode
//    is forced to 0 below, so the only code path is that branch, which has no loop (the tick thread -- main for(;;) ->
//    PumpTick -> MainProc, and every web ack -- keeps running).
//  KNOWN OPEN (J-5, FROM_JERRY 0930, its own INBOX item): a kcode==0 notice's mailbox request is never retired and
//    web/page/ht9045_dialog_host.js rejects its answer (no pending query), so the operator cannot close the box; a newer
//    request, or a blocking alarm answered later (DialogMailboxRetire), replaces it.  This host adds no second writer:
//    the J-5 fix on that branch covers the motor note too.   [AI(W906-J5-ACK) 20260930: done -- WS dialog.notifyAck (EOF W906_NoticeAckCommand); the motor note's SoftStop / SoftStart are not applied a second time (ack pause "already-applied")]
//  The note's answer (golden BtnPauseClick KeyCode==0: SoftStop=true / SoftStart=false) is applied by the body right after
//    this returns (the AI(W906-ALARMSTOP) rule, see the body's banner).
//  W906_MotorAccessOnAlarm: as in W906_AlarmStopLikeGolden -- golden's StopAllMotor is every motor, and the EastSun
//    monitor's 1203 axes are not reached by StopAllMotor(true); also golden uMotorTest Timer1Timer's fNote->fShow rule
//    (HOME / LoopMove up).  First, before the note, as there.
// =============================================================================
void ForwardShowMotorErrorMessage(const char* code, int keyCode, int pos, const char* unitName, const char* message)
{
    { extern void W906_MotorAccessOnAlarm(const char*); W906_MotorAccessOnAlarm(code); }
    if (keyCode != 0)
        std::printf("  ⚠ ShowMotorErrorMessage note with KeyCode=%d -- golden note.cpp:1110 always sets 0; posted as a kcode==0 notice (never the blocking wait)\n", keyCode);
    g_w906MotorNoteUnit = unitName ? unitName : "";
    g_w906MotorNoteMsg  = message ? message : "";
    std::printf("  [motor-note] %s unit=%s pos=%d: %s\n", code ? code : "", g_w906MotorNoteUnit, pos, g_w906MotorNoteMsg);
    ForwardShowErrorMessage(code, 0, pos);
    g_w906MotorNoteUnit = 0;
    g_w906MotorNoteMsg  = 0;
}

// =============================================================================
//  AI(W906-J5-ACK) 20260930: WS dialog.notifyAck -- the operator acknowledges a kCode==0 notice (INBOX 119, Jerry J-5).
//  Dispatched from the main loop (next to dialog.auth).  Inside ForwardShowErrorMessage's blocking wait the same frame
//  is answered not-a-notice there; inside the YES/NO and ShowMyMessage waits it gets modal-pending like any command.
//  The logic is w906dlg::NotifyAckHandle (tools/wb_dialog_mailbox.h): the slot must hold THIS notice, golden
//  BtnPauseClick's early returns refuse, DialogMailboxRetire() is the one writer, and golden's close
//  (forms/fNote_ShowError.cpp W906_NoteNoticeAckLikeGolden) runs once, after the file is idle.
//  It never waits: no wait loop, no PostQuery / ClearQuery (a notice never had a query).  The only delay it can have is
//  MailboxPut's own sharing-violation retry (at most 1 s per file, wb_dialog_mailbox.h AtomicWrite), as every post has.
//  Token-exempt (WebBridge/WebBridgeServer.cpp, as dialog.response), double-click exempt (WebCmdGuard.cpp -- a repeat
//  answers no-pending-notice), allowed while closing (FileRW/MainClose.cpp CmdAllowedWhileClosing).
//  ctest: tests/test_notice_ack.cpp (NoticeAck) runs the same NotifyAckHandle with the same refuse / close functions and
//  pins this body.
// =============================================================================
const char* W906_NoteNoticeAckRefusal(const char* requestId);                  // forms/fNote_ShowError.cpp EOF
bool W906_NoteNoticeAckLikeGolden(const char* requestId, int* pause, bool* jamCounted, unsigned long* passTimeSec);
void W906_NoticeAckCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)
{
    const std::string tag = wc.hasTag ? wc.tag : std::string();
    std::string ack;
    const bool ok = w906dlg::NotifyAckHandle(g_alarmSlot, tag,
        [](const std::string& id) { const char* why = W906_NoteNoticeAckRefusal(id.c_str()); return std::string(why ? why : ""); },
        []() { return DialogMailboxRetire(); },
        [](const std::string& id, std::string* okJson) {
            int pause = 3; bool jam = false; unsigned long passSec = 0;
            W906_NoteNoticeAckLikeGolden(id.c_str(), &pause, &jam, &passSec);
            *okJson = w906dlg::NotifyAckOkJson(id, g_dialogSeq, pause, jam, passSec);
        },
        &ack);
    std::printf("dialog.notifyAck tag=%s -> %s %s\n", tag.c_str(), ok ? "ok" : "refused", ack.c_str());
    std::fflush(stdout);
    server.CompleteCommand((unsigned long long)wc.id, ok, ack);
}

// =============================================================================
//  AI(W906-HMI-KEEP) 20260929: 操作畫面一直開著 —— 不是 golden。
//  EastSun 20260929：「你要一直持續偵測 如果html 被關掉 需要再次開啟 除非C++被關閉」。
//  golden 是 VCL 程式，主畫面就是它自己的視窗，關不掉也不會不見；移植樹的畫面在瀏覽器，關掉之後 wb_serve 照跑、沒有人看得到
//  （筆電第 57 包也記了「運轉中把所有瀏覽器關掉超過 15 秒，機台可能停產而且沒有警報」）。
//  既有的 W906-MODAL-WAKE 只在阻塞框等待時開瀏覽器；這裡是主迴圈（pumpBeat，500 ms）一直看：
//    * 「有網頁」＝ W906_PagesPresent()（活著的 WebSocket，或 3 秒內有 HTTP 請求 —— 同 MODAL-WAKE 的定義）。
//    * wb_serve 自己起來後先等 20 秒（F5 先開的 web/boot_wait.html 在這段時間切到操作畫面，不重複開）。
//    * 連續 5 秒沒有網頁 ⇒ 開操作畫面；開過之後 30 秒內不再開（瀏覽器要幾秒才連上）；連開 3 次都沒連上 ⇒ 改成 2 分鐘一次。
//      有網頁連上就全部歸零。
//    * 網址：環境變數 W906_HMI_URL（F5「IOWEB(這台)」設成 background.html?mode=debug&machine=HT9050），沒設 = ModalWakeUrl（正式版畫面）。
//    * 瀏覽器：這台的預設 http 瀏覽器（HKCU ...\UrlAssociations\http\UserChoice → HKCR\<ProgId>\shell\open\command 的 exe；實測
//      ShellExecute／Start-Process 開 .html 檔在這台叫不起瀏覽器，直接叫 exe 才行）；找不到 ⇒ ModalWakeLaunchEdge（HT9045_Web.cmd 的 Edge）。
//    * 關：W906_NO_BROWSER_WAKE（非 0）或 --seconds N（與 MODAL-WAKE 同一個開關 g_modalWakeOff）。
//    * wb_serve 一結束就停（它就在這支程式裡）。每次開都寫進操作 LOG（HMI 行）與主控台。
//  放在檔尾，上面的行號都不動。
// =============================================================================
bool W906_HmiShellLaunch(const std::string& url, std::string& how);  /* AI(W906-HMI-SHELL) 20260930: EOF; same line, no line below moves */  namespace {
bool W906_HmiDefaultBrowserExe(std::string& exe, std::string& why)
{
    HKEY k = 0;
    char prog[128] = "";
    DWORD sz = sizeof(prog) - 1, type = 0;
    if (::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", 0, KEY_READ, &k) != ERROR_SUCCESS) { why = "no http UserChoice"; return false; }
    const LONG r1 = ::RegQueryValueExA(k, "ProgId", 0, &type, (LPBYTE)prog, &sz);
    ::RegCloseKey(k);
    if (r1 != ERROR_SUCCESS || type != REG_SZ || !prog[0]) { why = "no http ProgId"; return false; }
    const std::string sub = std::string(prog) + "\\shell\\open\\command";
    char cmd[1024] = "";
    sz = sizeof(cmd) - 1; type = 0;
    if (::RegOpenKeyExA(HKEY_CLASSES_ROOT, sub.c_str(), 0, KEY_READ, &k) != ERROR_SUCCESS) { why = std::string("no open command for ") + prog; return false; }
    const LONG r2 = ::RegQueryValueExA(k, 0, 0, &type, (LPBYTE)cmd, &sz);
    ::RegCloseKey(k);
    if (r2 != ERROR_SUCCESS || !cmd[0]) { why = std::string("empty open command for ") + prog; return false; }
    std::string c(cmd);
    if (c[0] == '"') { const std::string::size_type e = c.find('"', 1); exe = (e == std::string::npos) ? std::string() : c.substr(1, e - 1); }
    else { const std::string::size_type e = c.find(".exe"); exe = (e == std::string::npos) ? std::string() : c.substr(0, e + 4); }
    const DWORD a = exe.empty() ? INVALID_FILE_ATTRIBUTES : ::GetFileAttributesA(exe.c_str());
    if (a == INVALID_FILE_ATTRIBUTES || (a & FILE_ATTRIBUTE_DIRECTORY)) { why = std::string(prog) + " -> '" + exe + "' not found"; exe.clear(); return false; }
    why = prog;
    return true;
}

bool W906_HmiLaunch(const std::string& url, std::string& how)
{
    std::string exe, why;
    if (::W906_HmiShellLaunch(url, how)) return true;   /* AI(W906-HMI-SHELL) 20260930: the HMI program window first; browser only without it */  if (W906_HmiDefaultBrowserExe(exe, why)) {
        std::string buf = "\"" + exe + "\" \"" + url + "\"";
        STARTUPINFOA si; ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
        PROCESS_INFORMATION pi; ZeroMemory(&pi, sizeof(pi));
        if (::CreateProcessA(NULL, &buf[0], NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
            ::CloseHandle(pi.hThread); ::CloseHandle(pi.hProcess);
            how = exe + " [" + why + "]";
            return true;
        }
        char e[64]; std::snprintf(e, sizeof(e), "CreateProcess failed %lu", (unsigned long)::GetLastError()); why = e;
    }
    std::string ew;
    if (ht9045::ModalWakeLaunchEdge(url, ew)) { how = "Edge (default browser: " + why + ")"; return true; }
    how = "default browser: " + why + "; Edge: " + ew;
    return false;
}

int                g_hmiKeepPort = 8055;
unsigned long long g_hmiKeepStart = 0, g_hmiAbsentSince = 0, g_hmiLastLaunch = 0;
int                g_hmiFailStreak = 0;
bool               g_hmiKeepAnnounced = false;
}  // namespace

void W906_HmiKeeperTick()
{
    if (!g_modalServer) return;
    if (!g_hmiKeepAnnounced) {
        g_hmiKeepAnnounced = true;
        g_hmiKeepPort = (int)g_modalServer->BoundPort();
        std::printf("[hmi-keep] 操作畫面被關掉就重開：%s\n", g_modalWakeOff ? "關（W906_NO_BROWSER_WAKE 或 --seconds）" : "開（先等 20 s；沒有網頁 5 s 就開；30 s／3 次後 2 min）");
    }
    if (g_modalWakeOff) return;
    const unsigned long long now = W906_TickMs64();
    if (g_hmiKeepStart == 0) { g_hmiKeepStart = now; return; }
    if (now - g_hmiKeepStart < 20000ULL) return;                                // boot_wait.html / the first page gets 20 s to connect
    if (W906_PagesPresent() > 0) { g_hmiAbsentSince = 0; g_hmiFailStreak = 0; return; }
    if (g_hmiAbsentSince == 0) { g_hmiAbsentSince = now; return; }
    if (now - g_hmiAbsentSince < 5000ULL) return;
    const unsigned long long gap = (g_hmiFailStreak >= 3) ? 120000ULL : 30000ULL;
    if (g_hmiLastLaunch != 0 && now - g_hmiLastLaunch < gap) return;
    g_hmiLastLaunch = now;
    ++g_hmiFailStreak;
    const char* u = std::getenv("W906_HMI_URL");
    const std::string url = (u && *u) ? std::string(u) : ht9045::ModalWakeUrl(g_hmiKeepPort);
    std::string how;
    const bool ok = W906_HmiLaunch(url, how);
    char head[96];
    std::snprintf(head, sizeof(head), "no web page for %.0f s -> open #%d: %s ", (now - g_hmiAbsentSince) / 1000.0, g_hmiFailStreak, ok ? "OK" : "FAILED");
    std::printf("[hmi-keep] %s%s  %s\n", head, url.c_str(), how.c_str());
    OpLine("HMI", std::string(head) + url + "  " + how);
}

// ===========================================================================
//  AI(W906-STOPBTN) 20260929: EastSun「目前不是debug模式 但是你也要有按鈕可以給我直接關閉程式停止」.
//  Without gdb (F5 = "noDebug", BOOTSPEED-2) the red stop button ends only VS Code's session, not this program. Now the
//  stop button (postDebugTask) and the task "IOWEB(這台): 關閉程式（正常關站）" run tools/stop_wb_serve.ps1, which sets
//  the named event Local\HT9045_wb_serve_quit_<pid>. The main loop asks here once per pass (a zero-timeout wait) and
//  leaves exactly like the main screen's Exit / --seconds: W906_ProdCloseSave + W906_ProdCloseShutdown (StopAllMotor +
//  1203 axes, brakes held, heater relays off ...), Program Close=1, server.Stop(). While a modal box waits the main loop
//  is inside W906_ModalWaitTick and does not get here: the script then waits, and ends the process after its timeout.
//  Unlike the main Exit this does NOT ask "Sure To Exit?" nor refuse while IC are in the machine (the old gdb stop
//  button killed the process without any of it; this is the gentler of the two).
// ===========================================================================
bool W906_ExternalQuitDue()
{
    static HANDLE s_ev = 0;
    static bool   s_made = false, s_said = false;
    if (!s_made) {
        s_made = true;
        char name[64];
        std::snprintf(name, sizeof(name), "Local\\HT9045_wb_serve_quit_%lu", (unsigned long)::GetCurrentProcessId());
        s_ev = ::CreateEventA(NULL, TRUE, FALSE, name);
        std::printf("[stop] VS Code stop button / 關閉程式 task: %s (%s)\n", s_ev ? "armed" : "NOT armed", name);
    }
    if (s_ev == 0 || ::WaitForSingleObject(s_ev, 0) != WAIT_OBJECT_0) return false;
    if (!s_said) {
        s_said = true;
        std::printf("[stop] stop requested from VS Code (tools/stop_wb_serve.ps1) -> normal close (same path as the main Exit / --seconds)\n");
        std::fflush(stdout);
        OpLine("STOP", "stop requested from VS Code (tools/stop_wb_serve.ps1) -> normal close: W906_ProdCloseSave + W906_ProdCloseShutdown, Program Close=1");
    }
    return true;
}

// =============================================================================
//  AI(W906-HMI-SHELL) 20260930: the HMI program window (tools/hmi_shell, ht9045_hmi.exe) instead of a browser.
//  EastSun 20260930「客戶反映 希望可以跟機台一樣 外框不是chrom 不要讓人員可以有額外操作的空間」／「按快捷鍵不會更改到畫面
//  下面工具列 是軟體的圖標」. HMI-KEEP (W906_HmiLaunch) and MODAL-WAKE start this window first; a browser only when it is missing.
//  Where: %W906_HMI_SHELL% (a full path; "0"/"off" = never use it), else next to this exe, else <tree>\build_hmi_shell\
//  (build_hmi_shell.bat's output; this exe lives in build_integ_ship_x86\). It needs WebView2Loader.dll beside it.
//  One window per session: a second start only brings the first to the front, so this is safe to call repeatedly.
//  W906_HmiShellQuit: the normal close (and the console's immediate close) posts "HT9045_HMI_SHELL_QUIT", and the window
//  closes without asking -- golden's main window IS the program, it does not stay behind.
// =============================================================================
static bool W906_FileExistsA(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static std::string W906_HmiShellExe(std::string& why)
{
    const char* e = std::getenv("W906_HMI_SHELL");
    if (e && (std::strcmp(e, "0") == 0 || ::lstrcmpiA(e, "off") == 0)) { why = "W906_HMI_SHELL=" + std::string(e) + " (off)"; return std::string(); }   //AI(W906-HMI-SHELL-MINGW) 20260930: was _stricmp -- MinGW.org 6.3 (the laptop oracle) does not declare it under -std=c++17 (__STRICT_ANSI__); ::lstrcmpiA = the tree's idiom (database.cpp:3346), same answer for "off"
    char me[MAX_PATH] = "";
    ::GetModuleFileNameA(0, me, MAX_PATH);
    std::string dir(me);
    const std::string::size_type k = dir.find_last_of("\\/");
    dir = (k == std::string::npos) ? std::string(".") : dir.substr(0, k);
    std::vector<std::string> c;
    if (e && *e) c.push_back(e);
    c.push_back(dir + "\\ht9045_hmi.exe");
    c.push_back(dir + "\\..\\build_hmi_shell\\ht9045_hmi.exe");
    for (std::size_t i = 0; i < c.size(); ++i) {
        const std::string::size_type s = c[i].find_last_of("\\/");
        const std::string loader = (s == std::string::npos ? std::string(".") : c[i].substr(0, s)) + "\\WebView2Loader.dll";
        if (W906_FileExistsA(c[i]) && W906_FileExistsA(loader)) return c[i];
    }
    why = "ht9045_hmi.exe (with WebView2Loader.dll) not found: " + c.back();
    return std::string();
}

bool W906_HmiShellLaunch(const std::string& url, std::string& how)
{
    std::string why;
    const std::string exe = W906_HmiShellExe(why);
    if (exe.empty()) { how = "HMI window: " + why; return false; }
    std::string buf = "\"" + exe + "\" \"" + url + "\"";
    STARTUPINFOA si; ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    PROCESS_INFORMATION pi; ZeroMemory(&pi, sizeof(pi));
    if (!::CreateProcessA(NULL, &buf[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        char b[64]; std::snprintf(b, sizeof(b), "CreateProcess failed %lu", (unsigned long)::GetLastError());
        how = "HMI window " + exe + ": " + b;
        return false;
    }
    ::CloseHandle(pi.hThread); ::CloseHandle(pi.hProcess);
    how = "HMI window " + exe;
    return true;
}

void W906_HmiShellQuit()
{
    HWND h = ::FindWindowW(L"HT9045HmiShell", NULL);
    if (!h) return;
    const UINT m = ::RegisterWindowMessageW(L"HT9045_HMI_SHELL_QUIT");
    const BOOL ok = ::PostMessageW(h, m, 0, 0);
    std::printf("[hmi-shell] program closing -> HMI window asked to close (%s)\n", ok ? "posted" : "PostMessage failed");
}