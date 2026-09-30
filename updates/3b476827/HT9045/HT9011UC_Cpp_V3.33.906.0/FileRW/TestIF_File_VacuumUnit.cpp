// ===========================================================================
//  FileRW/TestIF_File_VacuumUnit.cpp -- golden TfVacuumUnit（VacuumUnit\VacuumUnit.cpp，V912）的 C 路入口（C 形狀：HTEditList elVacuumUnit
//  → <recipe>\HandlerCondition.Data [Vacuum Threshold]／[Tray]）。頁面：web/page/HW.VacuumUnit.html（頁面補件 web/page/ht9045_vacuumunit_c.js）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_VacuumUnit.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 TestIF_File_VacuumUnit.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    Initial（:32）＝ elVacuumUnit 的註冊（面板 edSV 34 筆：HT9045 4 欄；HT9046／HT9046LS／HT1032／USE_46_SUCKER_DB 8 欄 → 50 筆）；
//    FormShow（:210）＝開頁（ReadFile → SetPanelPos → ShowSuckMode 依 FTestSuck 顯示／隱藏 Index 面板）；
//    spbSaveClick（:378）＝存檔鈕（A02 守衛 → SaveSetupFile → elVacuumUnit->SaveEditTextToFile → BackupSetupFile → ReadFile → SECS）。
//  mustSend＝0：存檔流程讀的只有 elVacuumUnit 登記的元件，頁面沒送的沿用開頁時從檔案讀到的值。
//  reload＝golden FormClose（:304，ReadFile＋DoIniDataToForm）：golden A02 路徑 Close() 之後跑的就是它。
//
//  ---- 開機與換配方（golden 呼叫點，D:\HT9045 V912 行號）-----------------------------------------------------
//    main.cpp:1542  TfMain 建構子  elVacuumUnit=new HTEditList        → 移植樹 FileRW_IniConfig_Boot 已做（本檔再防一次 NULL）
//    main.cpp:10933 TfMain::FormShow fVacuumUnit->Initial()          → FileRW_VacuumUnit_Boot()（註冊；不讀檔）
//    main.cpp:10934 TfMain::FormShow SetIOTableByECAT_VC8_Sucker()   → 不在本檔（TMySucker 的 VC8 位址複製到面板；移植樹 VacuumUnit.cpp GATE 1）
//    ReadFile：golden 只在 FormShow（:214）、FormClose（:307）、spbSaveClick（:394）呼叫。DoReadLastData（main.cpp:9264-9420，
//      開程式 :9995、換工作檔 cbSetupFileNameChange :25724／:25772）**沒有** fVacuumUnit->ReadFile()；全 golden 樹
//      `grep -rn "fVacuumUnit\|elVacuumUnit"`（20260925）只有上面三個 main.cpp 呼叫點＋ delete（:12555）＋ Show（:35560）。
//    ⇒ 照 V912：開機只註冊、不讀檔；換配方不讀。FileRW_VacuumUnit_ReadFile() 給整合者／Steven 決定要不要偏離 golden 開機讀。
//  ---- 讀檔之後誰用這些值 ----------------------------------------------------------------------------------
//    TestIF_File.iVaccumThrdIndexArm1／IndexArm2／InArm／OutArm（cprod.h:2564-2567）：全 golden 樹除了 VacuumUnit.cpp:121-133 的
//    elVacuumUnit->Add 之外 0 個讀者（grep "iVaccumThrd"，20260925）；也沒有 TestIF=TestIF_File 之後的讀者。
//    真空判斷用的是 ECAT-VC8 模組裡的閥值：只有 btnSV（MyVacuumPanel.cpp:385 WriteVaccumThreshold(atof(edSV->Text))）與
//    Set All（VacuumUnit.cpp:551 btnSetInArmClick）會經 MyLaneIO.SetIOValueThread 寫進硬體 —— 開機／存檔都不寫。
//    ⇒ 開始讀檔（或開機註冊）不改變任何執行期行為；頁面的 Set／Set All／吸破真空鈕是硬體指令，網頁停用（ht9045_vacuumunit_c.js）。
//    AI(W906-VACUNIT-1203) 20260930：不再停用 —— 硬體那一半在檔尾（WS vacuum.*：golden tmr1Timer 即時畫面＋那幾顆鈕，
//      經 ECAT-VC8 路由 VacuumUnit/Vc8Route.h；不是 ECAT-VC8 的站一律拒絕並說原因）。
//
//  ⚠ 移植樹 VacuumUnit/VacuumUnit.cpp 的 TfVacuumUnit::Initial() 以真元件註冊（頁面拿不到名稱）；**不要**再呼叫它，
//    否則 elVacuumUnit 同一組鍵會有兩筆（同 FileRW/Ld_UldDelayTime.cpp 的 fLd_ULd->Init() 說明）。
// ===========================================================================
#include "FileRW/TestIF_File_VacuumUnit.gen.inc"
#include "MachineType.h"   //AI(W906-VACUNIT-1203) 20260930: the gen.inc already reaches it (:14); spelled here so tools/macro_order_gate.ps1 (which follows only .h includes) sees W906_VC8_SUCKER_REMAP (:86, :284) is tested after MachineType.h
#include <cstdio>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

HTEditList** const kLists[] = {&elVacuumUnit};
const char* const kListNames[] = {"elVacuumUnit"};

const filerw::PageDesc kPage = {
    "TestIF_File_VacuumUnit", "TfVacuumUnit", "HW.VacuumUnit.html",
    kLists, kListNames, 1,
    nullptr, 0,                      // golden 存檔流程只讀 elVacuumUnit 登記的元件 → 沒有 mustSend
    &VU_FormShow, &VU_spbSaveClick, "SaveSetupFile", &VU_FormClose, &Booted,
    nullptr, nullptr,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden main.cpp:10933 TfMain::FormShow 的 fVacuumUnit->Initial()（前提：golden HT9045.cpp:275 CreateForm(TfVacuumUnit) ＝
// DFM 設計期狀態＋建構子 :26 iCount=5；main.cpp:1542 elVacuumUnit=new HTEditList）。不讀檔（golden 開機不讀，見檔頭）。
// 要在 SetMyKitSuckItemAmount 之後、第一次 editlist.get 之前（開頁 ShowSuckMode 讀 FTestSuck）。冪等。
void FileRW_VacuumUnit_Boot()
{
    if (g_booted) return;
    if (!elVacuumUnit) elVacuumUnit = new HTEditList;                           // golden main.cpp:1542（FileRW_IniConfig_Boot 通常已建）
    VU_DfmItems();
    VU_DfmState();
    VU_TfVacuumUnit();
    VU_Initial();
    VU_CreateSaveProxies();
    VU_CreateContainerProxies();
    // 偏離（port-only）：Set All Value 的三個輸入框只給 btnSetInArmClick（:551，寫 ECAT-VC8 硬體閥值）用，那顆鈕網頁不做
    // （指令通道未設計）→ 標唯讀，頁面停用、存檔不收。golden DFM 沒有 ReadOnly；它們的替身是產生器從 FormShow :248-252
    // 那段 golden 註解裡掃到才建的，不在任何清單、存檔流程也不讀。
    //AI(W906-VACUNIT-1203) 20260930: 不再標唯讀 —— 那顆鈕現在有了（檔尾 vacuum.setAll → golden btnSetInArmClick），
    //  golden DFM 本來就沒有 ReadOnly。存檔照舊不收（不在 elVacuumUnit）；可不可以按由頁面依 vacuum.* 快照決定
    //  （沒有可寫的 ECAT-VC8 就鎖住並說原因，web/page/ht9045_vacuumunit_c.js）。
    std::printf("FileRW TestIF_File_VacuumUnit: elVacuumUnit registered by name (%d entries, index cols %d, in/out cols %d) -- golden VacuumUnit.cpp Initial :32\n",
                elVacuumUnit->FEditList->Count, iIndexColMax, iInOutColMax);
    g_booted = true;
    //AI(W906-VACUNIT-1203) 20260930: golden main.cpp:10497-10498 -- right after fVacuumUnit->Initial() golden runs
    //  SetIOTableByECAT_VC8_Sucker() (EastSun ruling R3 20260930: follow golden). Its panels are the LIVE golden
    //  objects (VacuumUnit/VacuumUnitLive.h), so they are built here -- Initial's panel half, without a second
    //  elVacuumUnit registration -- and the remap copies the sucker table into them and hands the suckers to the
    //  ECAT-VC8 (fail closed: VacuumUnit/Vc8Route.h W906_Vc8SuckerGate). InitialHandler's InitSucker has already
    //  run (tools/wb_serve.cpp, before this boot line), as in golden. MachineType.h W906_VC8_SUCKER_REMAP commented
    //  out = neither happens here (the panels are then built when the page first opens).
#ifdef W906_VC8_SUCKER_REMAP
    {
        extern bool W906_VacuumLiveBuild(char* why, int whyLen);
        extern void SetIOTableByECAT_VC8_Sucker();
        char why[200] = "";
        if (W906_VacuumLiveBuild(why, (int)sizeof(why))) SetIOTableByECAT_VC8_Sucker();   // golden main.cpp:10498
        else std::printf("VacuumUnit: live panels not built (%s) -- SetIOTableByECAT_VC8_Sucker not run\n", why);
    }
#endif
}

// golden TfVacuumUnit::ReadFile（:351）。golden 開機／換配方都沒有呼叫（見檔頭）——給整合者選用，預設不接。
void FileRW_VacuumUnit_ReadFile()
{
    if (g_booted) VU_ReadFile();
}

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 14 列；Steven 20260928「如果已經有移植, 就接上」、
//    20260929「照 BCB 的邏輯」）。golden TfVacuumUnit::FormClose（V912 VacuumUnit\VacuumUnit.cpp:304-311）：ReadFile();
//    tmr1->Enabled=false;（即時真空顯示，產生檔已閘，同 FormShow :222）fShow=false; DoIniDataToForm(); ＝產生檔的 VU_FormClose
//    （也是本頁的 PageDesc::reload）。重讀 <配方>\HandlerCondition.Data（elVacuumUnit）＝丟掉這一次開窗裡沒存的改動。不碰真空閥。
//  頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過
//    （filerw::PageCloseEdgeRefused；A02 的 Close() 之後 PageSave 已經用 reload＝VU_FormClose ⇒ 不跑第二次）才跑。
//    運轉中照跑（skipWhileRunning＝false）：golden 是非模態 fVacuumUnit->Show()（main.cpp:35560）。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

static void W906_VacuumLiveStop_();   //AI(W906-VACUNIT-1203) 20260930: the live view's close half, defined below

const char* FileRW_VacuumUnit_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfVacuumUnit proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("TestIF_File_VacuumUnit")) return no;
    VU_FormClose();                                                             // golden VacuumUnit\VacuumUnit.cpp:304
    W906_VacuumLiveStop_();                                                     //AI(W906-VACUNIT-1203) 20260930: golden FormClose :308-309 tmr1->Enabled=false; fShow=false -- the live half
    return "ran golden TfVacuumUnit::FormClose (VacuumUnit.cpp:304-311): ReadFile (HandlerCondition.Data), fShow=false, DoIniDataToForm, tmr1 (the live vacuum view) stopped";
}

// ===========================================================================
//  AI(W906-VACUNIT-1203) 20260930: THE HARDWARE HALF OF HW.VacuumUnit -- WS vacuum.*.
//
//  EastSun 20260930: 「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」
//                    「針對1203 開啟 vacuunit 時，如果不符合現在程式碼就新創一個1203分支」
//  The file half above (elVacuumUnit, HandlerCondition.Data) is unchanged. What this adds is golden's tmr1Timer
//  live view and its hardware buttons, on golden's OWN TfVacuumUnit / TMyVacuumPanel objects
//  (VacuumUnit/VacuumUnitLive.h -- they cannot live in this TU, see there), reaching the card only through the
//  ECAT-VC8 route (VacuumUnit/Vc8Route.h -> EtherCAT/Pci1203Vc8Route.inc), which fails CLOSED unless the station
//  really is an ECAT-VC8. With no VC8 on the ring every panel shows golden's failure values (999.0 / Error3) and
//  every hardware button stays locked with the station's own reason.
//
//  WIRE (tools/wb_serve.cpp dispatch, `vacuum.` prefix; value = a JSON string; every ack carries the snapshot)
//      vacuum.open    {}                                  golden FormShow's live half (after editlist.get = FormShow)
//      vacuum.get     {"act":"timer"|"snap"}              timer = golden tmr1Timer (at most once per 900 ms), then the snapshot
//      vacuum.close   {}                                  golden FormClose's live half (tmr1 off)
//      vacuum.setSV   {"panel":"myPalInArm_0_0","kPa":-60}            golden btnSVClick
//      vacuum.do      {"panel":...,"which":"on"|"off","value":0|1}    golden btnVaccumOnOffOnClick (value = the new Down)
//      vacuum.setAll  {"arm":"in"|"index"|"out","kPa":-60}            golden btnSetInArmClick (Tag from the DFM table)
//      vacuum.reset   {}                                              golden sbResetClick
//    vacuum.open / get / close are READS (token-exempt, WebBridge/WebBridgeServer.cpp); the four others are
//    presses and go through the operator token like every button.
//
//  A PRESS IS REFUSED -- before any golden code runs -- unless ALL of these hold (each refusal says which):
//      the page is open (golden FormShow ran, FormClose has not) and vacuum.open started the live view
//      the VC8 route is installed, VacuUnitType == 1, SystemStart == false, SoftStart == false,
//      the 1203 control is armed (a DRY RUN is reported as such: nothing written)
//      setSV / do: the panel exists and is visible, and its station passes the route's write check
//                  (an ECAT-VC8 by identity, in OP, its bytes in the card maps)
//      setAll: the arm is one of the table's three names, and at least one of its panels' stations passes
//      setSV / setAll: kPa is a number in golden's keyboard range -116..148 (checked here, in C++)
//    The route then checks every single write again (EtherCAT/Pci1203Vc8Route.inc) and so does Pci1203Control.
//    Every write is in the ack (result.writes), on the console, in the oplog and in Pci1203Control's audit.
//
//  RULINGS (EastSun 20260930): R1 no VC8 on the ring today -> everything fails closed, reads identity-checked too;
//  R2 follow golden (tmr1Timer writes the threshold MODE from the page's first refreshing tick, through the
//  fail-closed route); R3 follow golden (SetIOTableByECAT_VC8_Sucker at boot, MachineType.h W906_VC8_SUCKER_REMAP).
//  Defaults that still stand: R4 only visible panels are read, R5 SDO DataSize 2 -- see VacuumUnit/VacuumUnitLive.inc.
// ===========================================================================
#include "VacuumUnit/VacuumUnitLive.h"
#include "VacuumUnit/Vc8Route.h"
#include "EtherCAT/Pci1203Control.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace {

bool          g_vuLive = false;      // vacuum.open ran for this opening of the window
std::uint64_t g_vuLastTickMs = 0;
const int     kVuThrBudget = 8;       // threshold SDO reads per tick (D1); the monitor caps the process at 16/s

std::uint64_t VuNowMs_()
{
    return (std::uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool VuPageOpen_() { return g_booted && fShow; }   // the gen.inc's golden fShow: FormShow ran (editlist.get), FormClose not yet

VU_TMyVacuumPanel* VuProxy_(int arm, int col, int row)
{
    if (row < 0 || row > 1 || col < 0) return 0;
    if (arm == kVuArmIndex1) return col < iIndexColMax && col < TOTAL_VACUUM_UNIT ? myPalArm1[col][row] : 0;
    if (arm == kVuArmIndex2) return col < iIndexColMax && col < TOTAL_VACUUM_UNIT ? myPalArm2[col][row] : 0;
    if (arm == kVuArmIn)     return col < iInOutColMax && col < TOTAL_VACUUM_UNIT / 2 ? myPalInArm[col][row] : 0;
    if (arm == kVuArmOut)    return col < iInOutColMax && col < TOTAL_VACUUM_UNIT / 2 ? myPalOutArm[col][row] : 0;
    return 0;
}

// golden SetPanelPos / ShowSuckMode ran on the page's proxies (VU_FormShow): copy each panel's Visible across.
// A live panel the page has no proxy for (a different column count) stays invisible -- it is then not read (R4).
void VuSyncVisible_()
{
    TVuLivePanel lp;
    for (int i = 0; i < W906_VacuumLiveCount(); ++i) {
        if (!W906_VacuumLivePanel(i, &lp)) continue;
        VU_TMyVacuumPanel* px = VuProxy_(lp.arm, lp.col, lp.row);
        W906_VacuumLiveSetVisible(i, px && px->GroupBox && px->GroupBox->Visible);
    }
}

std::string VuColor_(unsigned long bgr)   // TColor (0x00BBGGRR) -> "#rrggbb"
{
    char b[16];
    std::snprintf(b, sizeof(b), "#%02x%02x%02x", (unsigned)(bgr & 0xFF), (unsigned)((bgr >> 8) & 0xFF), (unsigned)((bgr >> 16) & 0xFF));
    return b;
}

// The page-level reason a press cannot be sent at all ("" = none).
std::string VuWriteBlock_()
{
    if (!VuPageOpen_() || !g_vuLive) return "Vacuum Unit 視窗沒有開著（或即時畫面還沒開始）——不送";
    if (!Vc8Route())
        return "這個 wb_serve 沒有 ECAT-VC8 路由（SOFT_SIMULTE 組態、或沒有 INSTALL_1203_MONITOR / WB_PUMP_1203_CONTROL）——不送";
    if (VCCU_UNIT_TYPE != 1) {
        char b[160];
        std::snprintf(b, sizeof(b), "Gerneral.ini VacuUnitType=%d，不是 1（ECAT-VC8_ODM1）——不送", VCCU_UNIT_TYPE);
        return b;
    }
    if (SystemStart) return "機台運轉中（SystemStart）——真空單元不從網頁寫";
    if (SoftStart)   return "機台正要啟動／回原點（SoftStart）——真空單元不從網頁寫";
    if (ht9045::Pci1203Control() == 0) return "1203 命令面沒有武裝（Pci1203ControlEnable 沒有成功）——不送";
    return "";
}

struct VuStation_ { int ring, ip; bool readOk; std::string readWhy; bool writeOk; std::string writeWhy; };

int VuStationOf_(std::vector<VuStation_>& st, int ring, int ip)
{
    for (std::size_t k = 0; k < st.size(); ++k) if (st[k].ring == ring && st[k].ip == ip) return (int)k;
    VuStation_ s;
    s.ring = ring; s.ip = ip;
    char why[320] = "";
    s.readOk = (W906_Vc8Check(ring, ip, 0, why, (int)sizeof(why)) == 0);
    s.readWhy = why;
    why[0] = '\0';
    s.writeOk = s.readOk && (W906_Vc8Check(ring, ip, 1, why, (int)sizeof(why)) == 0);
    s.writeWhy = s.readOk ? std::string(why) : s.readWhy;
    st.push_back(s);
    return (int)st.size() - 1;
}

void VuTri_(webbridge::JsonWriter& j, const char* k, int v)
{
    j.Key(k);
    if (v < 0) j.Null(); else j.Bool(v != 0);
}

void VuSnapshot_(webbridge::JsonWriter& j)
{
    TVuLiveState ls;
    W906_VacuumLiveState(&ls);
    const std::string block = VuWriteBlock_();
    ht9045::TPci1203Control* ctl = ht9045::Pci1203Control();
    j.Key("open").Bool(VuPageOpen_());
    j.Key("live").Bool(g_vuLive && ls.show);
    j.Key("built").Bool(ls.built);
    j.Key("initialOK").Bool(ls.initialOK);
    j.Key("iCount").Number((wb_int64)ls.iCount);
    j.Key("ticks").Number((wb_int64)ls.ticks);
    j.Key("thrReads").Number((wb_int64)ls.thrReadsLastTick);
    j.Key("thrDeferred").Number((wb_int64)ls.thrDeferredLastTick);
    j.Key("route").Bool(Vc8Route() != 0);
    j.Key("control").Bool(ctl != 0);
    j.Key("dryRun"); if (ctl) j.Bool(ctl->IsDryRun()); else j.Null();
    j.Key("vacuUnitType").Number((wb_int64)VCCU_UNIT_TYPE);
    j.Key("systemStart").Bool(SystemStart);
    j.Key("writeBlock").String(block);
    j.Key("rulings").String("R1: no ECAT-VC8 on the ring today; every station is identity-checked before any read or write. "
                            "R2: as golden, tmr1Timer writes the threshold MODE from the first refreshing tick (fail-closed). "
                            "R3: as golden, SetIOTableByECAT_VC8_Sucker at boot (MachineType.h W906_VC8_SUCKER_REMAP). "
                            "R4: only visible panels are read. R5: SDO DataSize 2 (golden 128).");
#ifdef W906_VC8_SUCKER_REMAP
    j.Key("suckerRemap").Bool(true);
#else
    j.Key("suckerRemap").Bool(false);
#endif
    std::vector<VuStation_> st;
    j.Key("panels").BeginArray();
    TVuLivePanel lp;
    for (int i = 0; i < W906_VacuumLiveCount(); ++i) {
        if (!W906_VacuumLivePanel(i, &lp)) continue;
        const int k = VuStationOf_(st, lp.ring, lp.ip);
        j.BeginObject();
        j.Key("id").String(lp.id);
        j.Key("vis").Bool(lp.visible);
        j.Key("st").Number((wb_int64)k);
        j.Key("vc").Number((wb_int64)lp.vc);
        j.Key("onCh").Number((wb_int64)lp.onPort);
        j.Key("offCh").Number((wb_int64)lp.offPort);
        j.Key("diCh").Number((wb_int64)lp.senPort);
        j.Key("cur").String(lp.cur);
        j.Key("thr").String(lp.thr);
        j.Key("evt").String(lp.evt);
        j.Key("evtColor").String(VuColor_(lp.evtColor));
        VuTri_(j, "led", lp.led);
        VuTri_(j, "on", lp.onDown);
        VuTri_(j, "off", lp.offDown);
        j.Key("modeOk").Bool(lp.modeOk);
        j.Key("available").Bool(lp.visible && block.empty() && st[(std::size_t)k].writeOk);
        j.Key("reason").String(!lp.visible ? std::string("這個面板在目前的測試模式不顯示（golden ShowSuckMode）")
                               : !block.empty() ? block : st[(std::size_t)k].writeOk ? std::string() : st[(std::size_t)k].writeWhy);
        j.EndObject();
    }
    j.EndArray();
    j.Key("stations").BeginArray();
    for (std::size_t k = 0; k < st.size(); ++k) {
        j.BeginObject();
        j.Key("ring").Number((wb_int64)st[k].ring);
        j.Key("station").Number((wb_int64)st[k].ip);
        j.Key("vc8").Bool(st[k].readOk);
        j.Key("writable").Bool(st[k].writeOk);
        j.Key("why").String(st[k].writeOk ? std::string() : st[k].writeWhy);
        j.EndObject();
    }
    j.EndArray();
}

std::string VuAck_(const std::string& op, unsigned long seq0, const std::string& note)
{
    webbridge::JsonWriter j;
    j.BeginObject();
    j.Key("op").String(op);
    if (!note.empty()) j.Key("note").String(note);
    if (seq0 != (unsigned long)-1) {
        TVc8Write w[64];
        const int n = W906_Vc8WritesSince(seq0, w, 64);
        int issued = 0, refused = 0, dry = 0, failed = 0;
        j.Key("writes").BeginArray();
        for (int i = 0; i < n; ++i) {
            j.BeginObject();
            j.Key("kind").String(w[i].kind == 0 ? "do" : "sdo");
            j.Key("ring").Number((wb_int64)w[i].ring);
            j.Key("station").Number((wb_int64)w[i].ip);
            j.Key(w[i].kind == 0 ? "chan" : "index").Number((wb_int64)w[i].chan);
            if (w[i].kind == 1) j.Key("sub").Number((wb_int64)w[i].sub);
            j.Key("value").Number((wb_int64)w[i].value);
            j.Key("rc").Number((wb_int64)(unsigned long)w[i].rc);
            j.Key("issued").Bool(w[i].issued);
            j.Key("dryRun").Bool(w[i].dryRun && w[i].reached && !w[i].issued);
            j.Key("why").String(w[i].why);
            j.Key("call").String(w[i].call);
            j.EndObject();
            if (w[i].issued && w[i].rc == 0) ++issued;
            else if (w[i].issued) ++failed;
            else if (w[i].reached && w[i].dryRun) ++dry;
            else ++refused;
        }
        j.EndArray();
        j.Key("issued").Number((wb_int64)issued);
        j.Key("failed").Number((wb_int64)failed);
        j.Key("dry").Number((wb_int64)dry);
        j.Key("refused").Number((wb_int64)refused);
    }
    VuSnapshot_(j);
    j.EndObject();
    return j.Str();
}

const cJSON* VuGet_(const cJSON* o, const char* k) { return o ? cJSON_GetObjectItemCaseSensitive(o, k) : 0; }

bool VuNum_(const cJSON* o, const char* k, double* v)
{
    const cJSON* x = VuGet_(o, k);
    if (!x || !cJSON_IsNumber(x)) return false;
    *v = x->valuedouble;
    return true;
}

std::string VuStr_(const cJSON* o, const char* k)
{
    const cJSON* x = VuGet_(o, k);
    return (x && cJSON_IsString(x) && x->valuestring) ? std::string(x->valuestring) : std::string();
}

bool VuRefuse_(const std::string& op, const std::string& why, std::string& ack)
{
    std::printf("vacuum.%s REFUSED -- %s\n", op.c_str(), why.c_str());
    ack = why;
    return false;
}

}  // namespace

static void W906_VacuumLiveStop_()
{
    W906_VacuumLiveClose();
    g_vuLive = false;
}

// tools/wb_serve.cpp, on the tick thread, under FormLock. false + `ack` = a plain-text refusal (nothing was sent).
bool W906_VacuumWire(const std::string& cmd, const std::string& value, std::string& ack)
{
    const std::string op = cmd.compare(0, 7, "vacuum.") == 0 ? cmd.substr(7) : cmd;
    cJSON* root = value.empty() ? 0 : cJSON_Parse(value.c_str());
    if (!value.empty() && !root) return VuRefuse_(op, "value 不是 JSON", ack);
    struct Del_ { cJSON* r; ~Del_() { if (r) cJSON_Delete(r); } } del = { root };

    if (op == "open") {
        if (!VuPageOpen_()) return VuRefuse_(op, "Vacuum Unit 還沒開頁（editlist.get = golden FormShow 沒有跑）", ack);
        char why[200] = "";
        if (!W906_VacuumLiveBuild(why, (int)sizeof(why))) return VuRefuse_(op, why, ack);
        VuSyncVisible_();
        W906_VacuumLiveShow();                          // golden FormShow :221-247
        g_vuLive = true;
        g_vuLastTickMs = VuNowMs_();
        ack = VuAck_(op, (unsigned long)-1, "");
        std::printf("vacuum.open -- live view started (golden tmr1Timer every 1 s while the page polls); %s\n",
                    VuWriteBlock_().empty() ? "presses allowed where a station passes" : VuWriteBlock_().c_str());
        return true;
    }
    if (op == "close") {
        W906_VacuumLiveStop_();
        ack = VuAck_(op, (unsigned long)-1, "");
        return true;
    }
    if (op == "get") {
        const std::string act = VuStr_(root, "act");
        std::string note;
        if (act.empty() || act == "timer") {
            if (!g_vuLive || !VuPageOpen_()) note = "即時畫面沒有在跑（vacuum.open 之後才有）";
            else {
                const std::uint64_t now = VuNowMs_();
                if (now - g_vuLastTickMs >= 900) {           // golden tmr1 Interval 1000 ms (DFM default); polls jitter
                    g_vuLastTickMs = now;
                    VuSyncVisible_();
                    W906_VacuumLiveTick(kVuThrBudget);        // golden tmr1Timer :255-302
                }
            }
        } else if (act != "snap") {
            return VuRefuse_(op, "act 只收 timer / snap", ack);
        }
        ack = VuAck_(op, (unsigned long)-1, note);
        return true;
    }

    // ---- presses -----------------------------------------------------------------------------------------
    if (op != "setSV" && op != "do" && op != "setAll" && op != "reset")
        return VuRefuse_(op, "不認得的 vacuum 指令（open / get / close / setSV / do / setAll / reset）", ack);
    const std::string block = VuWriteBlock_();
    if (!block.empty()) return VuRefuse_(op, block, ack);
    const unsigned long seq0 = W906_Vc8LastSeq();
    char why[320] = "";

    if (op == "reset") {
        W906_VacuumLiveReset(why, (int)sizeof(why));   // golden sbResetClick :408-429
        ack = VuAck_(op, seq0, "golden sbResetClick：面板事件清掉；下一個 tick 照 golden 重讀並重寫閥值模式");
        return true;
    }
    if (op == "setAll") {
        const std::string arm = VuStr_(root, "arm");
        const int tag = W906_VuSetAllTag(arm.c_str());
        if (tag < 0) return VuRefuse_(op, "arm 只收 in / index / out，收到「" + arm + "」——不送", ack);
        double kPa = 0.0;
        if (!VuNum_(root, "kPa", &kPa) || !W906_VuKpaOk(kPa))
            return VuRefuse_(op, "kPa 要是 -116 ~ 148 的數字（golden 小鍵盤範圍）——不送", ack);
        bool anyOk = false;
        std::string firstWhy;
        std::vector<VuStation_> st;
        TVuLivePanel lp;
        for (int i = 0; i < W906_VacuumLiveCount(); ++i) {
            if (!W906_VacuumLivePanel(i, &lp)) continue;
            const bool mine = (tag == 0 && lp.arm == kVuArmIn) || (tag == 1 && (lp.arm == kVuArmIndex1 || lp.arm == kVuArmIndex2)) ||
                              (tag == 2 && lp.arm == kVuArmOut);
            if (!mine) continue;
            const int k = VuStationOf_(st, lp.ring, lp.ip);
            if (st[(std::size_t)k].writeOk) anyOk = true;
            else if (firstWhy.empty()) firstWhy = st[(std::size_t)k].writeWhy;
        }
        if (!anyOk) return VuRefuse_(op, "這一臂沒有任何一個 ECAT-VC8 可寫：" + firstWhy, ack);
        W906_VacuumLiveSetAll(tag, kPa, why, (int)sizeof(why));   // golden btnSetInArmClick, Tag from the DFM table
        ack = VuAck_(op, seq0, why);
        return true;
    }
    // setSV / do: one panel
    const std::string id = VuStr_(root, "panel");
    const int i = W906_VacuumLiveFind(id.c_str());
    TVuLivePanel lp;
    if (i < 0 || !W906_VacuumLivePanel(i, &lp)) return VuRefuse_(op, "沒有面板「" + id + "」——不送", ack);
    if (!lp.visible) return VuRefuse_(op, "面板「" + id + "」在目前的測試模式不顯示——不送", ack);
    if (W906_Vc8Check(lp.ring, lp.ip, 1, why, (int)sizeof(why)) != 0) return VuRefuse_(op, why, ack);
    if (op == "setSV") {
        double kPa = 0.0;
        if (!VuNum_(root, "kPa", &kPa) || !W906_VuKpaOk(kPa))
            return VuRefuse_(op, "kPa 要是 -116 ~ 148 的數字（golden 小鍵盤範圍）——不送", ack);
        W906_VacuumLiveSetSV(i, kPa, why, (int)sizeof(why));     // golden btnSVClick
        ack = VuAck_(op, seq0, why);
        return true;
    }
    // do
    const std::string which = VuStr_(root, "which");
    if (which != "on" && which != "off") return VuRefuse_(op, "which 只收 on（吸真空 ^）/ off（破真空 v）——不送", ack);
    double v = -1.0;
    const cJSON* jv = VuGet_(root, "value");
    if (jv && cJSON_IsBool(jv)) v = cJSON_IsTrue(jv) ? 1.0 : 0.0;
    else if (jv && cJSON_IsNumber(jv)) v = jv->valuedouble;
    if (v != 0.0 && v != 1.0) return VuRefuse_(op, "value 要是 0 或 1（按下之後的 Down）——不送", ack);
    const int ret = W906_VacuumLiveDo(i, which == "on", v != 0.0, why, (int)sizeof(why));   // golden btnVaccumOnOffOnClick
    if (ret == (int)kVc8RcRefused && W906_Vc8LastSeq() == seq0) return VuRefuse_(op, why, ack);   // D3, refused before any write
    ack = VuAck_(op, seq0, why);
    return true;
}

#undef FTestSuck
