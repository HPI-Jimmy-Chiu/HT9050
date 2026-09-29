// ===========================================================================
//  FileRW/StartCondition.cpp -- golden TfStartCondition（cStartCondition.cpp，912）的 C 路入口（C 形狀：具名替身）。
//  頁面：web/page/Data.StartCondition.html（頁面補件 web/page/ht9045_startcondition_c.js）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/StartCondition.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 StartCondition.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    建構子（:94）＝ SocketID／SocketContactCnt／列欄標籤／btnClear 的替身陣列；
//    FormShow（:160）＝開頁（fSetup->ReadFile()＋ReadWriteStartCondition(true)＋權限／顯示）；
//    存檔 ＝ 頁面按的那一顆 golden 按鈕（W906_scButton，見下）。沒有 HTEditList：存檔流程讀的替身全部是 mustSend。
//
//  ---- 網頁一顆 golden 按鈕 → 伺服器跑那顆的 golden 處理器（SaveFlow）----------------------------------------
//    sbHeadCondition1Save  Life Time 的 Save（:1091）。A02 擋下時 golden Close() → modal 關窗 → FormClose（:619）
//    sbSave                Contact count 的 Save（:708 sbSaveClick）
//    spbExit               Exit（:751 spbExitClick）→ golden Close() 是 ShowModal 表單的 ModalResult，處理器回傳後
//                          modal 迴圈才跑 FormClose（:619）→ 這裡照 VCL 順序接著呼叫
//    （沒帶按鈕）          非頁面用戶端（例 s12c_page_probe.py）：Life Time Save → Exit（＝操作員依序按這兩顆）
//  頁面在 editlist.save 的 widgets 裡多帶三個非 golden 的鍵（web/page/ht9045_startcondition_c.js 注入），由
//  PageDesc::beforeApply（BeforeApply）讀掉、不當元件套值：
//    W906_scButton     {"text":"<按鈕>"}
//    W906_scActivePage {"text":"<pgLifeTime 目前分頁>"}  golden pgLifeTime->ActivePage（存檔 ReadWriteStartCondition 依它
//                      決定寫 HeadContactSet[n]／套哪一頁的 grid）。VCL 屬性跨開頁保留 → 本 TU 的 SC_pgLifeTimeActivePage。
//    W906_scActions    {"text":"[\"btnInArmC\",\"sbHeadCondition1Clear@tsPickerLifeTime\",…]"}  開頁後點過的清除類按鈕（依序）
//  清除類按鈕（sbSameAsHead1／sbClearCount／sbHeadCondition1Clear／btnClear*／btnInArm*／btnOutArm*／btnArm1*／btnArm2*）
//  golden 是「按下去就改元件（有的也直接改 LastSet／IniConfig 記憶體）」：頁面同時做畫面效果並記下點擊，存檔時
//  BeforeApply 在套頁面值「之前」依序重放 golden 處理器 ——
//    * 可改的元件：接著被頁面值蓋過（頁面已做同樣的畫面效果＋之後的編輯＝頁面最後狀態）；
//    * 不可改的元件（In/Out/Index arm picker 計數框，golden DFM Enabled=False，只能被清除鈕歸零）：保留重放結果；
//    * 記憶體副作用照 golden：sbClearCount → LastSet.iContactCT=0；Socket ID 頁清除 → LastSet.iSocketContactCount[iMaxRow×iMaxCol]=0；
//      Vibrator 頁清除 → IniConfig.iVibrator*=0＋RecordProcess("Clear all vibrator time")。
//    和 golden 的差別：golden 點下去就生效，網頁要按存檔鈕才重放（點了不存 → 沒有記憶體副作用）。
//
//  ---- 未移植（頁面上停用並註明）-------------------------------------------------------------------------------
//    Cylinder 頁（strngrdCylinderView、pmCylinder 彈出選單 mniResetOnOffCountClick → SaveCylinderLife → MachineLife.ini、
//      UpdateCylinderScreen 由 golden 主計時器每 10 次刷新 sCylinderData、LoadCylinderLife 開機讀檔＝移植樹 cinitial.cpp N1-G2e gate）；
//    btnSetOffsetLimitClick（寫 Security_new.def [Input Limit]；gbSetOffsetLimit 只有 CosFunction.bSetOffsetLimitToAll（ASE-CL）才顯示）；
//    cbStartModeOnlyFTClick（cbStartModeOnlyFT 只有 CC_SIGURD_HUKOU 顯示）；sb_Maintenance_SmartDiagnosticFunctionClick（開 fSmartDiagnostic）；
//    SocketIDLog（atester.cpp:882 生產端呼叫，不在頁面）—— AI(W906-W11) 20260927：本體已照翻成檔尾的 W906_SC_SocketIDLog()，
//      呼叫點（atester.cpp 的函式指標）與 wb_serve 開機安裝由 Steven02／St01 另做，見那支函式的註解。
//    客戶專屬條件：Steven 20260925 決定先跳過（golden 程式碼照轉，不測）。
//
//  開機順序（整合者）：golden HT9045.cpp:205 CreateForm(TfStartCondition)，在 TfLotInfo（:171）、TfSetup（:184）之後、
//    TfConfiguration（:207）之前。本檔沒有 HTEditList，順序只要求：FileRW_StartCondition_Boot() 在第一次 editlist.get 之前、
//    且若要解開移植樹 cSetUp.cpp:1190 GATE(G-SU-StartCond)（改呼叫 FileRW_StartCondition_ReadWriteStartCondition(true)），
//    要在開機第一次 fSetup->ReadFile() 之前。
// ===========================================================================
#include "FileRW/StartCondition.gen.inc"

#include <cstdio>
#include <cstdlib>   // AI(W906-W11) 20260927: std::getenv（W906_SC_SocketIDLog 的 W906_SOCKETIDLOG_ROOT）
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"
#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"

namespace {
const char* const kForm = "TfStartCondition";
bool g_booted = false;
bool Booted() { return g_booted; }

// pgLifeTime 的頁（golden DFM 順序，cStartCondition.dfm:687-4113）
const char* const kLifeTabs[] = {"tsCondition01", "tsCondition02", "tsCondition03", "TabsSocketID", "tsVibration",
                                 "tsSmartDiagnostic", "tsSocketCount", "tsPickerLifeTime", "tsCylinderView"};
// golden pgLifeTimeChange（:1199-1221）會改 editContactCountAlarm 的頁
const char* const kAlarmTabs[] = {"tsCondition01", "tsCondition02", "tsCondition03", "TabsSocketID", "tsSocketCount"};

std::string TabName(TTabSheet* t)
{
    for (const char* n : kLifeTabs)
        if (t && EL<TTabSheet>(kForm, n) == t) return n;
    return "";
}
TTabSheet* TabByName(const std::string& n)
{
    for (const char* x : kLifeTabs)
        if (n == x) return EL<TTabSheet>(kForm, x);
    return nullptr;
}

// ---- 存檔流程讀的替身：產生器掃出的（kSC_SaveReads）＋ 經陣列讀的（產生器的掃描只認 `EL<>(…)->屬性`）：
//   sgHeadCondition1..3（ReadWriteStartCondition :855 StrGrid[]）、editSocketAa..Dh／PanelAa..Dh（sbHeadCondition1SaveClick
//   :1110-1126 SocketID[][]／SocketContactCnt[][]）、picker 計數框（ReadWriteStartCondition :857-875 edt*PickLifeCnt[][]）。
std::vector<std::string> g_readNames;
std::vector<const char*> g_reads;
const char kRows[] = "ABCD", kCols[] = "abcdefgh", kArm8[] = "ABCDEFGH";

void BuildReads()
{
    for (const char* n : kSC_SaveReads) g_readNames.push_back(n);
    for (const char* g : {"sgHeadCondition1", "sgHeadCondition2", "sgHeadCondition3"}) g_readNames.push_back(g);
    for (int r = 0; r < MAX_SOCKET_ROW; ++r)
        for (int c = 0; c < MAX_SOCKET_COL; ++c) {
            g_readNames.push_back(std::string("editSocket") + kRows[r] + kCols[c]);
            g_readNames.push_back(std::string("Panel") + kRows[r] + kCols[c]);
        }
    for (int i = 0; i < 8; ++i) {
        g_readNames.push_back(std::string("edtInArm") + kArm8[i]);
        g_readNames.push_back(std::string("edtOutArm") + kArm8[i]);
    }
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 8; ++c) {
            g_readNames.push_back(std::string("edtArm1") + kRows[r] + kCols[c]);
            g_readNames.push_back(std::string("edtArm2") + kRows[r] + kCols[c]);
        }
    for (const std::string& s : g_readNames) g_reads.push_back(s.c_str());
}

// ---- 清除類按鈕（W906_scActions 的一項）→ golden 處理器 ------------------------------------------------------
enum class Act { None, SameAsHead1, ClearCount, HeadClear, ClearSocket, InArm, OutArm, Arm1, Arm2 };
bool In(char ch, const char* set) { for (; *set; ++set) if (*set == ch) return true; return false; }
Act KindOf(const std::string& a, std::string* btn, std::string* tab)
{
    *btn = a;
    if (a == "sbSameAsHead1") return Act::SameAsHead1;                     // golden :671
    if (a == "sbClearCount") return Act::ClearCount;                       // golden :688
    if (a.compare(0, 22, "sbHeadCondition1Clear@") == 0) {                 // golden :772（依點擊當時的分頁）
        *btn = "sbHeadCondition1Clear";
        *tab = a.substr(22);
        return TabByName(*tab) ? Act::HeadClear : Act::None;
    }
    if (a.size() == 10 && a.compare(0, 8, "btnClear") == 0 && In(a[8], kRows) && In(a[9], kCols)) return Act::ClearSocket;   // :1390
    if (a.size() == 9 && a.compare(0, 8, "btnInArm") == 0 && In(a[8], kArm8)) return Act::InArm;                              // :1306
    if (a.size() == 10 && a.compare(0, 9, "btnOutArm") == 0 && In(a[9], kArm8)) return Act::OutArm;                           // :1313
    if (a.size() == 9 && a.compare(0, 7, "btnArm1") == 0 && In(a[7], "AB") && In(a[8], kCols)) return Act::Arm1;              // :1327
    if (a.size() == 9 && a.compare(0, 7, "btnArm2") == 0 && In(a[7], "AB") && In(a[8], kCols)) return Act::Arm2;              // :1320
    return Act::None;
}

// BeforeApply → SaveFlow 之間的狀態（同一個 editlist.save 請求內）
struct Pending {
    bool refuse = false;           // W906_scActions 有不認得的項 → 不存
    std::string button;            // W906_scButton
    std::vector<std::string> events;
};
Pending g_p;  bool EvB10CRefuseSave();  void EvB10CFormShow();   //AI(W906-EVB10C) 20260929 [W906]: B2 守衛（開著時跑過生產 ⇒ 拒存）與 FormShow 包一層，本體檔尾；同一行附加

std::string TextOf(const cJSON* root, const char* key, bool* has)
{
    const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, key);
    const cJSON* t = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "text") : nullptr;
    *has = t && cJSON_IsString(t);
    return *has ? std::string(t->valuestring) : std::string();
}

// PageDesc::beforeApply：讀掉頁面注入的三個鍵（列入 handled，不當元件套值），依序重放清除類按鈕的 golden 處理器，
// 最後把 golden pgLifeTime->ActivePage 設成頁面目前的分頁。這時替身還是伺服器端的值（開頁 FormShow 之後），
// 重放完 PageSave 才套頁面值（可改的被頁面最後狀態蓋過；不可改的留重放結果）。見檔頭。
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    g_p = Pending();
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root) return;
    bool hasBtn = false, hasTab = false, hasAct = false;
    g_p.button = TextOf(root, "W906_scButton", &hasBtn);
    const std::string tab = TextOf(root, "W906_scActivePage", &hasTab);
    const std::string acts = TextOf(root, "W906_scActions", &hasAct);
    if (hasBtn) handled->push_back("W906_scButton");
    if (hasTab) handled->push_back("W906_scActivePage");
    if (hasAct) handled->push_back("W906_scActions");
    cJSON_Delete(root);  if (EvB10CRefuseSave()) return;   //AI(W906-EVB10C) 20260929 [W906]: B2 守衛 —— 這一次開窗跑過生產 ⇒ 每一顆存檔鈕都拒存（清除鈕也不重放；g_p.refuse ⇒ SaveFlow 什麼都不做，PageSave 照舊 reload），檔尾；同一行附加

    // (1) 清除類按鈕：先全部驗過（不認得的一項都不重放），再依序跑
    std::vector<std::string> list;
    if (hasAct && !acts.empty()) {
        cJSON* a = cJSON_Parse(acts.c_str());
        if (!a || !cJSON_IsArray(a)) {
            g_p.refuse = true;
        } else {
            for (const cJSON* it = a->child; it; it = it->next) {
                if (!cJSON_IsString(it)) { g_p.refuse = true; break; }
                list.push_back(it->valuestring);
            }
        }
        if (a) cJSON_Delete(a);
    }
    for (const std::string& s : list) {
        std::string b, t;
        if (KindOf(s, &b, &t) == Act::None) { g_p.refuse = true; break; }
    }
    if (g_p.refuse) {
        filerw::ELMessage("W906_scActions is not a list of known Start Condition buttons; nothing was replayed or saved.",
                          "頁面送來的清除按鈕清單看不懂，這次沒有重放也沒有存檔。");
        return;
    }
    TTabSheet* keep = SC_pgLifeTimeActivePage;
    for (const std::string& s : list) {
        std::string b, t;
        const Act k = KindOf(s, &b, &t);
        // golden 使用者按得到才算：按鈕自己＋每一層容器 Enabled／Visible（TabSheet 還要 TabVisible），同 PageSave 丟值的判斷
        if (!filerw::ELEditable(kForm, b.c_str())) {
            filerw::ELTodo((std::string("golden button ") + b + " is disabled or hidden now -- click not replayed").c_str());
            continue;
        }
        switch (k) {
            case Act::SameAsHead1: SC_sbSameAsHead1Click(); break;
            case Act::ClearCount:  SC_sbClearCountClick(); break;
            case Act::HeadClear: {
                TTabSheet* p = TabByName(t);
                if (!p->TabVisible) { filerw::ELTodo((std::string("sbHeadCondition1Clear on hidden tab ") + t + " -- not replayed").c_str()); continue; }
                SC_pgLifeTimeActivePage = p;                // golden 按下去當時的 pgLifeTime->ActivePage
                SC_sbHeadCondition1ClearClick(EL<TSpeedButton>(kForm, "sbHeadCondition1Clear"));
                break;
            }
            case Act::ClearSocket: SC_btnClearAaClick(EL<TButton>(kForm, b.c_str())); break;
            case Act::InArm:       SC_btnInArmAClick(EL<TButton>(kForm, b.c_str())); break;
            case Act::OutArm:      SC_btnOutArmAClick(EL<TButton>(kForm, b.c_str())); break;
            case Act::Arm1:        SC_btnArm1AaClick(EL<TButton>(kForm, b.c_str())); break;
            case Act::Arm2:        SC_btnArm2AaClick(EL<TButton>(kForm, b.c_str())); break;
            default: break;
        }
        g_p.events.push_back(s);
        filerw::ELMark((std::string("replay:") + s).c_str());
    }
    SC_pgLifeTimeActivePage = keep;
    // (2) 頁面目前的分頁（看得見的 pgLifeTime 頁才收；不認得／藏起來 → 沿用伺服器端，VCL 不會停在藏起來的頁）
    if (hasTab) {
        TTabSheet* p = TabByName(tab);
        if (p && p->TabVisible) SC_pgLifeTimeActivePage = p;
        else filerw::ELTodo((std::string("W906_scActivePage '") + tab + "' is not a visible pgLifeTime tab -- server keeps " +
                             TabName(SC_pgLifeTimeActivePage)).c_str());
    }
}

// PageDesc::saveFlow：頁面按的那一顆 golden 按鈕（見檔頭）
void SaveFlow()
{
    if (g_p.refuse) return;
    const std::string b = g_p.button;
    if (b == "sbSave") {
        SC_sbSaveClick();                                                   // golden :708
    } else if (b == "sbHeadCondition1Save") {
        SC_sbHeadCondition1SaveClick(EL<TSpeedButton>(kForm, "sbHeadCondition1Save"));   // golden :1091
        if (filerw::ELMarked("closed")) SC_FormClose();                     // A02：golden Close() → modal 關窗 → FormClose :619
    } else if (b == "spbExit") {
        SC_spbExitClick();                                                  // golden :751（Close() → 回傳後 FormClose）
        SC_FormClose();
    } else if (b.empty()) {
        // 非頁面用戶端：操作員依序按 Life Time Save → Exit
        SC_sbHeadCondition1SaveClick(EL<TSpeedButton>(kForm, "sbHeadCondition1Save"));
        if (!filerw::ELMarked("closed")) SC_spbExitClick();
        SC_FormClose();
    } else {
        filerw::ELMessage(AnsiString("Unknown Start Condition button: ") + AnsiString(b.c_str()) + "; nothing was saved.",
                          AnsiString("不認得的按鈕：") + AnsiString(b.c_str()) + "，這次沒有存檔。");
        return;
    }
    // golden 真的寫了檔：sbSaveClick（WriteLastDataFile）、spbExitClick（WriteLastDataFile）、ReadWriteStartCondition(false)（配方／SocketCount.ini）
    if (filerw::ELMarked("sbSaveClick") || filerw::ELMarked("spbExitClick") || filerw::ELMarked("ReadWriteStartCondition"))
        filerw::ELMark("StartCondition:write");
}

// 沒寫檔時把替身還原：golden 重開表單（FormShow，含 fSetup->ReadFile()／ReadWriteStartCondition(true)）
void Reload()
{
    SC_FormShow();
}

// PageDesc::extraJson：頁面要、但不是元件值的 golden 狀態
//   activePage  golden pgLifeTime->ActivePage（開頁 FormShow 之後）
//   tabAlarm    golden pgLifeTimeChange（:1199）在每一頁會放進 editContactCountAlarm 的值（頁面換分頁時照放 —— golden
//               換頁會觸發 OnChange；FormShow 裡設 ActivePage 不會）。只有 golden 會改的五頁
//   tabCaption  tsCondition01..03 的 Caption（golden FormShow :474-476＝IniConfig.ContactConditionName）
//   amd         IniConfig.bAMDFunction（golden sgHeadCondition1SelectCell :1157：改計數前先問是否歸零）
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("activePage").String(TabName(SC_pgLifeTimeActivePage));
    w.Key("tabAlarm").BeginObject();
    {
        TTabSheet* keep = SC_pgLifeTimeActivePage;
        TEdit* ed = EL<TEdit>(kForm, "editContactCountAlarm");
        const AnsiString text = ed->Text;
        for (const char* n : kAlarmTabs) {
            SC_pgLifeTimeActivePage = EL<TTabSheet>(kForm, n);
            SC_pgLifeTimeChange();
            w.Key(n).String(ed->Text.c_str());
            ed->Text = text;
        }
        SC_pgLifeTimeActivePage = keep;
    }
    w.EndObject();
    w.Key("tabCaption").BeginObject();
    for (const char* n : {"tsCondition01", "tsCondition02", "tsCondition03"}) w.Key(n).String(EL<TTabSheet>(kForm, n)->Caption.c_str());
    w.EndObject();
    w.Key("amd").Bool(IniConfig.bAMDFunction != 0);
    w.EndObject();
    return w.Str();
}

filerw::PageDesc g_page = {
    "StartCondition", "TfStartCondition", "Data.StartCondition.html",
    nullptr, nullptr, 0,
    kSC_SaveReads, (int)(sizeof(kSC_SaveReads) / sizeof(kSC_SaveReads[0])),
    &EvB10CFormShow, &SaveFlow, "StartCondition:write", &Reload, &Booted,   //AI(W906-EVB10C) 20260929 [W906]: formShow &SC_FormShow → &EvB10CFormShow（檔尾：golden FormShow 之後清「這一次開窗跑過生產」；reload 仍是 SC_FormShow、不清 —— 頁面上的格子要重讀才是新的）；同一行改寫
    &BeforeApply, &ExtraJson,
};
filerw::PageRegistrar g_reg(&g_page);
}  // namespace

// golden TfStartCondition 建構（HT9045.cpp:205 CreateForm）：DFM 設計期（Items／狀態／Tag／grid 大小／ActivePage）→ 建構子
// → 存檔流程讀的替身 → 容器替身與父子。
void FileRW_StartCondition_Boot()
{
    if (g_booted) return;
    // SC_CapEdit（TPanel 的 Caption 由頁面收送）要最先建：同名替身第一次 EL<> 決定型別（FileRW/_EditList.h）
    for (const char* n : {"palVibratorHP1", "palVibratorSht1", "palVibratorSht2", "palVibratorUnloader"}) EL<SC_CapEdit>(kForm, n);
    SC_DfmItems();
    SC_DfmState();
    SC_DfmStatic();
    SC_TfStartCondition();
    SC_CreateSaveProxies();
    SC_CreateContainerProxies();
    // 清除類按鈕（BeforeApply 重放時的 Sender；golden DFM Tag 已由 SC_DfmStatic 設好，Tag=0 的在這裡補建）
    EL<TSpeedButton>(kForm, "sbSameAsHead1");
    EL<TSpeedButton>(kForm, "sbClearCount");
    EL<TSpeedButton>(kForm, "sbHeadCondition1Clear");
    EL<TSpeedButton>(kForm, "sbHeadCondition1Save");
    for (int i = 0; i < 8; ++i) {
        EL<TButton>(kForm, (std::string("btnInArm") + kArm8[i]).c_str());
        EL<TButton>(kForm, (std::string("btnOutArm") + kArm8[i]).c_str());
    }
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 8; ++c) {
            EL<TButton>(kForm, (std::string("btnArm1") + kRows[r] + kCols[c]).c_str());
            EL<TButton>(kForm, (std::string("btnArm2") + kRows[r] + kCols[c]).c_str());
        }
    BuildReads();
    g_page.saveReads = g_reads.data();
    g_page.nSaveReads = (int)g_reads.size();
    std::printf("FileRW StartCondition: TfStartCondition proxies ready (%d save reads) -- golden cStartCondition.cpp ctor :94\n",
                g_page.nSaveReads);
    g_booted = true;
}

// ---- 給整合者：golden 其他地方呼叫 fStartCondition 的點（移植樹目前都是 gate／沒接）-------------------------------
// golden cSetUp.cpp:2954 TfSetup::ReadFile 內 fStartCondition->ReadWriteStartCondition(true)（移植樹 cSetUp.cpp:1190 GATE(G-SU-StartCond)）
void FileRW_StartCondition_ReadWriteStartCondition(bool bRead)
{
    if (g_booted) SC_ReadWriteStartCondition(bRead);
}
// golden main.cpp:9436 TfMain::DoReadLastData 的 fStartCondition->DoIniDataToForm()（Socket ID → 替身＋fLotInfo->edSocket）
void FileRW_StartCondition_DoIniDataToForm()
{
    if (g_booted) SC_DoIniDataToForm();
}
// golden main.cpp:6619 fStartCondition->WritePickerCount()（O20 吸嘴計數寫回配方／SocketCount.ini）
void FileRW_StartCondition_WritePickerCount()
{
    if (g_booted) SC_WritePickerCount();
}

// ---- AI(W906-W11) 20260927 (Steven 團隊)：golden TfStartCondition::SocketIDLog（V912 cStartCondition.cpp:1435-1492）--------
//  Steven 20260927 W11＝B：本體放在這裡（St01 的 FileRW/StartCondition.cpp），逐行照翻成自由函式（golden 是 TfStartCondition
//  成員、沒有參數；本體不讀 TfStartCondition 的任何成員或元件 —— aSiteName 是區域變數 —— 所以不需要 EL<> 替身，也不需要
//  FileRW_StartCondition_Boot() 先跑過）。golden 呼叫點：atester.cpp:882 GetTesterResult 的 case 1（每次測試開始寫一次）。
//  分工：Steven02 在 atester.cpp 定義 `void (*W906_SocketIDLogBody)() = 0;` 與呼叫點（gate T02，移植樹 atester.cpp:944-946）；
//    St01 等那個指標進來後，才在 wb_serve 開機（tools/wb_serve.cpp:4111 那一行行尾）裝 W906_SocketIDLogBody = &W906_SC_SocketIDLog;
//    本函式現在沒有呼叫者。
//  讀的東西（全部是 golden 同名的移植樹全域／門面，沒有替身）：
//    asSocketIDLogPath（common.cpp:267 ＝ golden common.cpp:78 "D:\\HT9045_Log\\SocketIDLog\\"）、SystemYear／Month／Date
//    （GetTimeInfo 更新）、iRunStartMode／FT、FTestSuck.iMaxRow／iMaxCol（本檔巨集 → FileRW_KitSuckDims(1)）、
//    LastSet.strSocketID／iSocketContactCount、TestIF_File.iContactAlarmCount[3]、fLotInfo->edtASECL_LotID->Text、
//    fMain->cbSetupFileName->Text（wb_serve.cpp:4111 開機設、換配方時 WebRecipeChange.cpp 設）。
//  ctest 隔離接縫（不是 golden）：環境變數 W906_SOCKETIDLOG_ROOT 有設（非空）就用它當根目錄取代 asSocketIDLogPath；
//    沒設＝golden 原字面。只換根目錄，其後 "\\YYYYMM\\" 子目錄與檔名照 golden。
//  照翻、但看起來不對的地方（沒有改，行為要改由 Steven 決定）：
//    (1) 行尾只寫 "\r"（golden 原文就是 \r，不是 \r\n）：fopen 的文字模式只轉 \n，所以檔案裡每行結尾是單獨的 CR。
//    (2) 檔名只帶日期：同一天每次測試開始都 FileExists → DeleteFile → 重寫，檔案永遠只有最後一次的快照（"a+" 等於 "w"）。
//    (3) tmps1（FT／RT）算了沒用；asStr／str2／str3 宣告了沒用 —— golden 原樣。
//    (4) 路徑重複分隔：asSocketIDLogPath 已經以 '\\' 結尾，sprintf 又加一個 → "D:\HT9045_Log\SocketIDLog\\YYYYMM\"。
//        Windows 會把中間的 "\\" 當成一個分隔，golden 在 BCB6 上也是這樣跑；照翻。
//    (5) 迴圈上限是 FTestSuck.iMaxRow／iMaxCol，aSiteName 與 LastSet 的陣列是 [4][8]；golden 沒有另外檢查
//        （mykitsuck.h _MAX_SUCK_ROW_ITEM 4／_MAX_SUCK_COL_ITEM 8 限住了），照翻。
//    (6) 每一列的 Alarm Count 都是 TestIF_File.iContactAlarmCount[3]（Socket ID 頁的警報值，不分 site），golden 原樣。
void W906_SC_SocketIDLog()
{
    FILE * pFile;
    AnsiString asStr, asPath, asFileName="";
    AnsiString tmps, tmps1,str1, str2, str3;
//    int iRow, iCol;
    AnsiString aSiteName[4][8]={{"Aa", "Ab", "Ac", "Ad", "Ae", "Af", "Ag", "Ah"},
                                {"Ba", "Bb", "Bc", "Bd", "Be", "Bf", "Bg", "Bh"},
                                {"Ca", "Cb", "Cc", "Cd", "Ce", "Cf", "Cg", "Ch"},
                                {"Da", "Db", "Dc", "Dd", "De", "Df", "Dg", "Dh"},};
    GetTimeInfo();
//    GetComputerName(PcName, &PcNameLen); //Steven 20110131
    const char* pW906LogRoot=std::getenv("W906_SOCKETIDLOG_ROOT");              //AI(W906-W11) 20260927: ctest 隔離接縫（見上）；沒設＝golden asSocketIDLogPath
    const AnsiString asW906LogRoot=(pW906LogRoot!=NULL && *pW906LogRoot!='\0') ? AnsiString(pW906LogRoot) : asSocketIDLogPath;
    asPath.sprintf("%s\\%04d%02d\\", asW906LogRoot, SystemYear, SystemMonth);   //golden: asSocketIDLogPath
    if(iRunStartMode==FT)
    {
        tmps1="FT";
    }
    else
    {
        tmps1="RT";
    }

    str1.sprintf("SocketID_Lifetime_%04d%02d%02d.csv", SystemYear, SystemMonth, SystemDate);                            //Steven 20170123 (Jou) : 修改檔案命名格式
//    asPath+=asFileName;
//
//    str1.sprintf("SocketID_Lifetime.csv");    //Steven 20210517 : 檔名加上時間戳記

    if(DirectoryExists(asPath)==false)
    {
        ForceDirectories(asPath);
    }

    asFileName = asPath+ str1;

    if(FileExists(asFileName))
    {
        DeleteFile(asFileName);
    }

    pFile=fopen(asFileName.c_str() ,"a+");
    if(pFile!=NULL)
    {
        tmps.sprintf("Socket ID,  Real time count, Alarm Count, Site Number, LOT AO, Setup File(LB Name)\r");
        fputs(tmps.c_str(), pFile);

        for(int i=0; i<FTestSuck.iMaxRow && i<4; i++)                                //AI(W906-W11) 20260927 [W906] 偏離 golden（decisions R68 C，Steven 沒反對就加）：陣列是 [4][8]（aSiteName、LastSet.strSocketID／iSocketContactCount，LastSet.h:468-469），golden 只看 FTestSuck 列數；範圍內行為不變
        {
            for(int j=0; j<FTestSuck.iMaxCol && j<8; j++)                            //AI(W906-W11) 20260927 [W906] 同上（欄數上限 8）
            {
                aSiteName[i][j].sprintf("SITE %c%c", 'A'+i, 'a'+j);
                tmps.sprintf("%s, %d, %d, %s, %s, %s\r"  , LastSet.strSocketID[i][j], LastSet.iSocketContactCount[i][j], TestIF_File.iContactAlarmCount[3], aSiteName[i][j], fLotInfo->edtASECL_LotID->Text.c_str(), fMain->cbSetupFileName->Text.c_str());
                fputs(tmps.c_str(), pFile);
            }
        }

        fclose(pFile);
    }
}

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 18 列；ST01-E 20260929 B2 放行「照 BCB」）。
//  golden TfStartCondition::FormClose（V912 cStartCondition.cpp:619-669）＝產生檔的 SC_FormClose：
//    sbSaveClick(this)（:708：LastSet.ContactSet／iContactCT ← sgContactCount、bContactAlmNeedOneCycle 時寫 HandContactWarningSet、
//    WriteLastDataFile）→ 沒料時 LastSet.iStartMode ← rbStartMode1..3 → fShow=false → fLotInfo->edSocket[][] ← LastSet.strSocketID →
//    TestIF_File.iContactAlarmCount[0..3] ← CheckAndReadIniData（HandlerCondition.Data 或 D05_1 的 SocketCount.ini，缺鍵補寫 6000／0）
//    → DoIniDataToForm。⇒ golden 關這個視窗就會存檔（golden BorderIcons=[] 沒有 ✕，關法只有 spbExit／A02 的 Close()）。
//  網頁：Exit 鈕（spbExit）與 Life Time Save 的 A02 已經經 editlist.save 跑過 SC_FormClose（上面 SaveFlow；golden Close() 記 "closed"
//    ⇒ PageSave 記 closeRan）⇒ 關窗邊緣不跑第二次；外框的 ✕（沒送存檔就關）⇒ 頁面表的關窗邊緣（FileRW/MainClick.cpp
//    W906_EvB10A_WindowEdge）呼叫這裡：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過（filerw::PageCloseEdgeRefused）才跑。
//    運轉中不跑（ShowModal main.cpp:28553；開窗閘 GStartMode 運轉中也打不開）。
//  跑的是伺服器端替身（＝開頁 FormShow 的值；網頁沒存的改動伺服器看不到，form.event 重播過的除外）⇒ 大多是同值重寫。不是同值的：
//    * LastSet.ContactSet／iContactCT 寫回的是「開窗那一刻」的 sgContactCount。iContactCT 是生產中會加的計數（atester_ProcessCount.cpp:2236）：
//      golden 模態，開著時點不到 START，不會發生；網頁非模態，「開窗 → START → 生產 → 停機 → ✕」會把計數拉回開窗值（運轉中按 ✕ 不跑，
//      skipWhileRunning）。⛔ 20260929 更正：ST01-E 裁決加 B2 守衛（下面第二段），這種情況現在不跑、也不准存；
//    * LastSet.iStartMode 只在 HasICUnderMachine()==false 時改；fLotInfo 的站號格重填（主畫面 Lot Info）；
//    * WriteLastDataFile 寫真的 lastdata（cprod.cpp 硬編路徑）、HandlerCondition.Data／SocketCount.ini 可能補寫缺鍵。
//  主畫面尾段 sbStartModeClick :28554-28555（沒料時 SetStartModeData）移植樹任何一條路都還沒接（不在本批；交件列出）。
//
//  ---- B2 守衛（ST01-E 20260929「guard B2」；規則與訊息在 FileRW/WindowEdgeTails.h evb10a::RanDuringOpen）----
//    golden ShowModal 表單開著時點不到 START ⇒「這一次開窗裡跑過生產」在 golden 不存在；網頁走得到時，格子（頁面送回的值與伺服器端
//    替身）都還是開窗那一刻的計數，照跑 golden 會把生產中加上的計數拉回去。守衛讓結果＝golden（golden 不會有這一筆寫入）：
//    * 記：FileRW_StartCondition_EvB10CSample —— wb_serve 主迴圈每一拍（FileRW/MainRecord.cpp W906_MainRecordTimer1Tick，約 500 ms）
//      看 SystemStart||SoftStart，這一頁開著（filerw::PageShownNow("StartCondition")：editlist.get 之後、關窗或 golden Close() 之前）就記上。
//    * 清：EvB10CFormShow（PageDesc::formShow＝editlist.get＝golden FormShow：開窗、重讀鈕、存檔後引擎自動重讀）⇒ 格子是新的。
//      PageDesc::reload（存檔沒寫檔時的還原）不清：頁面畫面上的格子還是舊的，要頁面重讀才算。
//    * 擋：(1) 關窗邊緣（本檔下面）不跑 SC_FormClose（它的 sbSaveClick）；(2) editlist.save 的每一顆鈕 —— sbSave（sbSaveClick）、
//      sbHeadCondition1Save（ReadWriteStartCondition(false)＋LastSet.iSocketContactCount／IniConfig.SocketContactCount，含它的 A02 Close()
//      → FormClose）、spbExit（spbExitClick＋FormClose）、沒帶按鈕（Life Time Save → Exit）—— 在 BeforeApply 一開頭就拒（清除鈕也不重放），
//      頁面收到 ELMessage。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

namespace {
evb10a::RanDuringOpen g_b10cProd;   // 這一次開窗裡跑過生產（見上）

void EvB10CFormShow()
{
    SC_FormShow();                                                              // golden cStartCondition.cpp:160
    g_b10cProd.FormShown();                                                     // 格子重讀了 ⇒ 重新算
}

bool EvB10CRefuseSave()
{
    const char* no = evb10a::StartConditionProductionGuard(g_b10cProd, true);
    if (!no) return false;
    g_p.refuse = true;                                                          // SaveFlow 開頭就 return（不跑任何一顆 golden 鈕）
    filerw::ELMessage(no, evb10a::kStartConditionProductionGuardZh);
    std::printf("[EVB10C] fStartCondition editlist.save refused: production ran during this open (button \"%s\") -- stale grid counters not written back\n",
                g_p.button.c_str());
    std::fflush(stdout);
    return true;
}
}  // namespace

// wb_serve 主迴圈每一拍（FileRW/MainRecord.cpp W906_MainRecordTimer1Tick 開頭呼叫）。
void FileRW_StartCondition_EvB10CSample()
{
    if (g_b10cProd.Sample(SystemStart || SoftStart, g_booted && filerw::PageShownNow("StartCondition"))) {
        std::printf("[EVB10C] fStartCondition: production running while the page is open (SystemStart=%d SoftStart=%d) -- "
                    "closing it will not run golden FormClose and its saves are refused until the page is re-read\n",
                    (int)SystemStart, (int)SoftStart);
        std::fflush(stdout);
    }
}

const char* FileRW_StartCondition_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfStartCondition proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("StartCondition")) return no;
    if (const char* no = evb10a::StartConditionProductionGuard(g_b10cProd, false)) {   // B2 守衛
        std::printf("%s\n", no);
        g_b10cProd.FormShown();                                                 // 這一次開窗結束（下一次開窗 FormShow 也會清）
        return "not run: production ran during this open (B2 guard) -- golden FormClose / sbSaveClick skipped";
    }
    SC_FormClose();                                                             // golden cStartCondition.cpp:619
    return "ran golden TfStartCondition::FormClose (cStartCondition.cpp:619-669) with the server-side values read at open: sbSaveClick (WriteLastDataFile), "
           "LastSet.iStartMode (no IC under machine), fLotInfo socket IDs, iContactAlarmCount re-read, DoIniDataToForm";
}

#undef TestSocket
#undef FTestSuck
