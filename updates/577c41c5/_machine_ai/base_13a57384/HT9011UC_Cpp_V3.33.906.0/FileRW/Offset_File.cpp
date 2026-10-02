// ===========================================================================
//  FileRW/Offset_File_C.cpp -- 結構 Offset_File（Position Offset.Data ＋ Hot／cool 變體；Offset_File、
//  InArmOffSet_File[]、OutArmOffSet_File[]、SortArmOffSet_File[]）的讀寫檔（C 形狀：具名替身，選取式編輯器）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/Offset_File.py（每條 replace 附原因）。
//  檔名帶 _C：A 形狀 FileRW/Offset_File.cpp（tools/formbridge/TfOffSet.py，可顯示不可存）還佔著 Offset_File.cpp；
//  整合者退役 A 形狀後把本檔改名成 FileRW/Offset_File.cpp（gen_editlist.py 的 _editlist_sources.cmake 用的是那個名字）。
//
//  golden TfOffSet（cOffSet.cpp，912）由 tools/gen_editlist.py 轉成 FileRW/Offset_File.gen.inc。
//  TfOffSet 是「選取式編輯器」：畫面上同一組 edit 依按鈕（SpBotSelClick :2476，iNowOffsetSel=Ptr->Tag；
//  IndexOffSetBT2Click :2594，iSpecialOffSetSel=Ptr->Tag）顯示不同部位，存檔鈕 spbSaveClick（:2807）只存目前選取。
//  Steven 20260925 定案：「可以直接全部 offset 透過 JSON 整包傳輸，但是點到某個按鈕才顯示指定的項目」。
//
//    開頁（FileRW_Offset_Page）＝ golden FormShow（:496，含 ReadFile）後，對每一個選取各跑一次 golden 按鈕事件
//      （OS_SpBotSelClick(Tag)＝ShowOneByOneOffSet＋DoIniDataToForm(sel, iSaveStander)；
//       OS_IndexOffSetBT2Click(Tag)＝DoIniDataToForm(tag, iSaveSpecial)＋Timer1Timer 的 shuttle 欄位互鎖），
//      把那次畫面上「存檔會讀的元件」收成一組：offsets["<sel>"]／offsets["<tag>:2"]。
//    存檔（FileRW_Offset_Save）＝ 對頁面送來的每一組：同一個按鈕事件（含 golden 的 ReadFile；非頁面欄位＝檔案現值）→ 套頁面值（只收可改的）
//      → golden spbSaveClick 的 SaveFile 段（只存這一組）；全部組存完後 spbSaveClick 的尾端（ReadFile、ASE_CL scale、
//      fMain->Pause、SetWorkParameter）只跑一次。
//
//  讀檔器是移植樹 fOffSet->ReadFile()（cOffSet.cpp:367）；開機讀檔仍由移植樹既有序列負責。本檔的 FileRW_Offset_Boot()
//  只做 golden 建構子（替身與指標陣列），必須在 LoadMachineConfig／IniConfig 開機之後（建構子讀 USE_* 與 CUSTOMER_CODE）。
// ===========================================================================
#include "FileRW/Offset_File.gen.inc"

// 設定檔 members 的巨集只給 golden 轉出來的程式用
#undef ReadFile
#undef InArmSuck
#undef OutArmSuck
#undef LastFileName
#undef GetOffsetPath
#undef Tri_Position_Offset

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"
#include "FileRW/_EditPage.h"      // AI(W906-FRW-S165) 20260927 [W906]: filerw::OpenEnterRecord（開頁記 golden 的 "Enter ..."，RULINGS_20260926 S165＝R101）；佔用原本的空行
namespace {

const char* const kForm = "TfOffSet";
bool g_booted = false;
bool g_shown = false;          // 審查 H-A（IniConfig 同規則）：存檔前要在同一個 AccessLevel 下開過頁
int  g_shownLevel = -1;

// golden 建構子 :252-273 tempSpPtr[] 的順序（OffSetSelBot[i]->Tag=i，:323-328）
const char* const kSelButton[OfsTotal] = {
    "sbLoader", "sbHp1", "sbHp2", "sbInSh1", "sbInSh2",
    "sbOutSh1", "sbOutSh2",
    "sbAuto1", "sbAuto2", "sbAuto3", "sbAuto4", "sbAuto5", "sbAuto6",
    "sbFix1", "sbFix2", "sbFix3", "sbFix4", "sbFix5", "sbFix6",
    "sbAutoClean", "sbOCR",
    "sbInRotate", "sbOutRotate",
    "sbTopView", "sbPADView", "sbBGAView",
    "sbLoaderB",
    "sbInSh1LB", "sbInSh1RA", "sbInSh1RB",
    "sbInSh2LB", "sbInSh2RA", "sbInSh2RB",
    "sbInSh1_AutoClean", "sbInSh1LB_AutoClean", "sbInSh1RA_AutoClean", "sbInSh1RB_AutoClean",
    "sbInSh2_AutoClean", "sbInSh2LB_AutoClean", "sbInSh2RA_AutoClean", "sbInSh2RB_AutoClean",
    "sbAutoSh1", "sbAutoSh2",
    "sbPreciser",
    "sbOutSh1LB", "sbOutSh1RA", "sbOutSh1RB",
    "sbOutSh2LB", "sbOutSh2RA", "sbOutSh2RB",
    "sbScanAOI",
    "sbInPlacement",
    "btnBottom2D",
};
// golden 建構子 :274-280 SpecOffSetPtr[]（OfffSetSpecSelBot[i]->Tag=i，:364-369）
const char* const kSpecButton[trayOfsTotal] = {
    "btnTrayOfsLoader", "btnTrayOfsEmpty", "btnTrayOfsColor",
    "btnTrayOfsAuto1",  "btnTrayOfsAuto2", "btnTrayOfsAuto3",
    "btnTrayOfsAuto4",  "btnTrayOfsAuto5", "btnTrayOfsAuto6",
    "IndexOffSetBT1",   "IndexOffSetBT2",
};
// golden 建構子 :281-321 的元件指標陣列（:330-361 指派）（[row][col]）
const char* const kPickEdit[2][8] = {
    {"EditPickA", "EditPickC", "EditPickE", "EditPickG", "EditPickI", "EditPickK", "EditPickM", "EditPickO"},
    {"EditPickB", "EditPickD", "EditPickF", "EditPickH", "EditPickJ", "EditPickL", "EditPickN", "EditPickP"}};
const char* const kRelsEdit[2][8] = {
    {"EdtRelsA", "EdtRelsC", "EdtRelsE", "EdtRelsG", "EdtRelsI", "EdtRelsK", "EdtRelsM", "EdtRelsO"},
    {"EdtRelsB", "EdtRelsD", "EdtRelsF", "EdtRelsH", "EdtRelsJ", "EdtRelsL", "EdtRelsN", "EdtRelsP"}};
const char* const kSingleX[2][8] = {
    {"EdtOffsetAX", "EdtOffsetCX", "EdtOffsetEX", "EdtOffsetGX", "EdtOffsetIX", "EdtOffsetKX", "EdtOffsetMX", "EdtOffsetOX"},
    {"EdtOffsetBX", "EdtOffsetDX", "EdtOffsetFX", "EdtOffsetHX", "EdtOffsetJX", "EdtOffsetLX", "EdtOffsetNX", "EdtOffsetPX"}};
const char* const kSingleY[2][8] = {
    {"EdtOffsetAY", "EdtOffsetCY", "EdtOffsetEY", "EdtOffsetGY", "EdtOffsetIY", "EdtOffsetKY", "EdtOffsetMY", "EdtOffsetOY"},
    {"EdtOffsetBY", "EdtOffsetDY", "EdtOffsetFY", "EdtOffsetHY", "EdtOffsetJY", "EdtOffsetLY", "EdtOffsetNY", "EdtOffsetPY"}};
const char* const kPickLab[2][8] = {
    {"LabPickA", "LabPickC", "LabPickE", "LabPickG", "LabPickI", "LabPickK", "LabPickM", "LabPickO"},
    {"LabPickB", "LabPickD", "LabPickF", "LabPickH", "LabPickJ", "LabPickL", "LabPickN", "LabPickP"}};
const char* const kRelsLab[2][8] = {
    {"LabRelsA", "LabRelsC", "LabRelsE", "LabRelsG", "LabRelsI", "LabRelsK", "LabRelsM", "LabRelsO"},
    {"LabRelsB", "LabRelsD", "LabRelsF", "LabRelsH", "LabRelsJ", "LabRelsL", "LabRelsN", "LabRelsP"}};

// 每一組顯示用的標籤／面板（Visible＋Caption；不存檔）
const char* const kStanderDisplay[] = {
    "palOffsetParts", "lblReleaseRange", "lblPickUpRange", "labWarningForZ", "lblOutArm",
    "lblRelease", "lblPickUp", "lblPitchX1", "lblPitchX1Range", "lblPitchX2", "lblPitchX2Range",
    "lblPitchX3", "lblPitchX4", "lblPitchY", "lblPitchYRange", "palPreciser", "sbZcalibration"};
const char* const kSpecialDisplay[] = {
    "pnlIndexOffset", "Panel2", "Panel3", "pnlTrayXART", "lblLoadZ", "lblShtRight", "lblShtLeft"};
// tsScale 分頁（golden FormShow :547-566 填、spbSaveClick 尾端 CC_ASE_CL 讀）
const char* const kCommon[] = {
    "chkInArmScakeEnable", "edLodX", "edLodY", "edt_HP1X", "edt_HP1Y", "edt_HP2X", "edt_HP2Y",
    "ed_IS1X", "ed_IS1Y", "ed_IS2X", "ed_IS2Y",
    "chkOutArmScakeEnable", "ed_OS1X", "ed_OS1Y", "ed_OS2X", "ed_OS2Y"};

// 可改＝golden 使用者碰得到（自己與祖先 Enabled＋Visible，不是 ReadOnly；filerw::ELEditable）。
// 例外：tsInOutArmOffset／tsIndexOffset 兩頁 golden 建構子 :192 把 tsIndexOffset->TabVisible=false —— 只藏頁籤，
// 內容由 btnToIndexOffset／btnToArmOffset（ActivePage=…）切換可達，所以判斷時這兩頁視為可達。其他分頁照 TabVisible。
bool Editable(const char* name)
{
    TTabSheet* a = EL<TTabSheet>(kForm, "tsInOutArmOffset");
    TTabSheet* b = EL<TTabSheet>(kForm, "tsIndexOffset");
    const bool va = a->TabVisible, vb = b->TabVisible;
    a->TabVisible = true;
    b->TabVisible = true;
    const bool r = filerw::ELEditable(kForm, name);
    a->TabVisible = va;
    b->TabVisible = vb;
    return r;
}

bool IsUnusedAutoFix(int sel)   // golden SaveSetupFile :1635-1640（AUTO_EMPTY_COLOR<3 的 Auto4-6／Fix4-6 不存）
{
    return AUTO_EMPTY_COLOR < 3 &&
           (sel == OfsAuto4 || sel == OfsAuto5 || sel == OfsAuto6 || sel == OfsFix4 || sel == OfsFix5 || sel == OfsFix6);
}

// golden SaveSetupFile 的 iSaveStander 段（:1633-1807）讀的元件
std::vector<std::string> StanderWidgets(int sel)
{
    std::vector<std::string> v;
    if (IsUnusedAutoFix(sel)) return v;
    const char* base[] = {"edArmX", "edArmY", "edPickUp", "edRelease", "edPitchY", "edPitchX4", "edPitchX3", "edPitchX2", "edPitchX1"};
    for (const char* n : base) v.push_back(n);
    if (sel == OfsPreciser) { v.push_back("edPreciserOpen"); v.push_back("edPreciserClose"); }   // :1726-1730
    int row = 0, col = 0;
    FileRW_ArmSuckDims(0, &row, &col);                                  // :1732-1734 InArmSuck.iMotCol／iMotRow
    for (int i = 0; i < col && i < 8; ++i)
        for (int j = 0; j < row && j < 2; ++j) {
            if (IniConfig.bE33InOutArmZOffsetSameOne == false ||          // :1736-1740
                (IniConfig.bE33InOutArmZOffsetSameOne == true &&
                 (sel == OfsLoader || sel == OfsAuto1 || sel == OfsPADView || sel == OfsBGAView ||
                  sel == OfsScanAOI || sel == OfsBottom2DID))) {
                v.push_back(kPickEdit[j][i]);
                v.push_back(kRelsEdit[j][i]);
                v.push_back(kSingleX[j][i]);
                v.push_back(kSingleY[j][i]);
            }
        }
    return v;
}

// golden SaveSetupFile 的 iSaveSpecial 段（:1570-1629）讀的元件
std::vector<std::string> SpecialWidgets(int tag)
{
    std::vector<std::string> v;
    if (tag <= tOfsAuto6) {                                              // :1572-1586 Tray Arm
        v.push_back("edtTrayArmX"); v.push_back("edtARTPlace"); v.push_back("edlLoadZ");
    } else if (tag == tOfsIndex1 || tag == tOfsIndex2) {                 // :1589-1628 Test Arm1／2
        const char* n[] = {"IndexArmOffSet1", "IndexArmOffSet2", "IndexArmOffSet3", "IndexArmOffSet4",
                           "IndexArmOffSet5", "IndexArmOffSet6", "edShtFor2D"};
        for (const char* s : n) v.push_back(s);
    }
    return v;
}

void PutValue(webbridge::JsonWriter& w, TControl* c)
{
    if (TEdit* e = dynamic_cast<TEdit*>(c)) w.Key("text").String(e->Text.c_str());
    else if (TCheckBox* k = dynamic_cast<TCheckBox*>(c)) w.Key("checked").Bool(k->Checked);
}

void PutWidgets(webbridge::JsonWriter& w, const char* key, const std::vector<std::string>& names)
{
    w.Key(key).BeginObject();
    for (const std::string& n : names) {
        TControl* c = filerw::ELFind(kForm, n.c_str());
        if (!c) continue;
        w.Key(n).BeginObject();
        PutValue(w, c);
        w.Key("visible").Bool(c->Visible);
        w.Key("editable").Bool(Editable(n.c_str()));
        w.EndObject();
    }
    w.EndObject();
}

void PutDisplay(webbridge::JsonWriter& w, const std::vector<std::string>& names)
{
    w.Key("display").BeginObject();
    for (const std::string& n : names) {
        TControl* c = filerw::ELFind(kForm, n.c_str());
        if (!c) continue;
        w.Key(n).BeginObject();
        w.Key("visible").Bool(c->Visible);
        if (TLabel* l = dynamic_cast<TLabel*>(c)) w.Key("caption").String(l->Caption.c_str());
        else if (TPanel* p = dynamic_cast<TPanel*>(c)) w.Key("caption").String(p->Caption.c_str());
        else if (TSpeedButton* s = dynamic_cast<TSpeedButton*>(c)) w.Key("caption").String(s->Caption.c_str());
        w.EndObject();
    }
    w.EndObject();
}

std::string GroupKey(int sel, int mode)
{
    return mode == iSaveSpecial ? std::to_string(sel) + ":2" : std::to_string(sel);
}

// 一組：golden 按鈕事件跑完之後的畫面
void PutGroup(webbridge::JsonWriter& w, int sel, int mode)
{
    const char* button = mode == iSaveSpecial ? kSpecButton[sel] : kSelButton[sel];
    TControl* b = filerw::ELFind(kForm, button);
    w.Key(GroupKey(sel, mode)).BeginObject();
    w.Key("sel").Number((wb_int64)sel);
    w.Key("mode").Number((wb_int64)mode);
    w.Key("part").String(mode == iSaveSpecial ? SpecialOffSetName[sel].c_str() : CapStr[sel].c_str());
    w.Key("button").String(button);
    w.Key("buttonVisible").Bool(b && b->Visible);
    w.Key("clickable").Bool(Editable(button));
    const bool refused = mode == iSaveStander && OS_bSelRefused;
    w.Key("refused").Bool(refused);
    std::vector<std::string> disp;
    if (mode == iSaveSpecial) {
        for (const char* n : kSpecialDisplay) disp.push_back(n);
        PutDisplay(w, disp);
        PutWidgets(w, "widgets", SpecialWidgets(sel));
    } else {
        for (const char* n : kStanderDisplay) disp.push_back(n);
        for (int j = 0; j < 2; ++j)
            for (int i = 0; i < 8; ++i) {
                disp.push_back(kPickLab[j][i]);
                disp.push_back(kRelsLab[j][i]);
            }
        PutDisplay(w, disp);
        w.Key("savesNothing").Bool(IsUnusedAutoFix(sel));
        PutWidgets(w, "widgets", StanderWidgets(sel));
    }
    w.EndObject();
}

// 頁面「點部位鈕」：golden 按鈕事件（顯示用；按鈕事件裡的 ReadFile 由呼叫端決定是否略過）
void Select(int sel, int mode)
{
    if (mode == iSaveSpecial) {
        OS_IndexOffSetBT2Click(sel);
        OS_Timer1Timer();              // golden Timer1（FormShow :779 啟動）：Shuttle Right／Left 依 shuttle 位置／SystemStart 開關
    } else {
        OS_bSelRefused = false;  OS_bSavePrev = false;   //AI(W906-D013) 20260929 [W906]：開頁／存檔的逐組切換不照 golden :2487 先存上一組（R127 只給 form.event 的部位鈕，檔尾 OS_EvSelect）；接在同一行
        OS_SpBotSelClick(sel);
    }
}

bool ParseKey(const std::string& k, int* sel, int* mode)
{
    if (k.empty()) return false;
    char* end = nullptr;
    const long v = std::strtol(k.c_str(), &end, 10);
    if (end == k.c_str()) return false;
    int m = iSaveStander;
    if (*end == ':') {
        const std::string rest(end + 1);
        if (rest == "1") m = iSaveStander;
        else if (rest == "2") m = iSaveSpecial;
        else return false;
    } else if (*end != '\0') {
        return false;
    }
    if (v < 0 || v >= (m == iSaveSpecial ? (long)trayOfsTotal : (long)OfsTotal)) return false;
    *sel = (int)v;
    *mode = m;
    return true;
}

bool Contains(const std::vector<std::string>& v, const char* s)
{
    for (const std::string& x : v) if (x == s) return true;
    return false;
}

// 頁面送的一組 {id:{text|checked}}：不在清單的 → unknown、不可改的 → ignored，其餘交給 ELApplyProxies
bool ApplyGroup(cJSON* obj, const std::vector<std::string>& names, std::vector<std::string>* applied,
                std::vector<std::string>* ignored, std::vector<std::string>* unknown, std::string* err)
{
    cJSON* keep = cJSON_CreateObject();
    for (cJSON* it = obj ? obj->child : nullptr; it; it = it->next) {
        if (!Contains(names, it->string)) { unknown->push_back(it->string); continue; }
        if (!Editable(it->string)) { ignored->push_back(it->string); continue; }
        cJSON_AddItemToObject(keep, it->string, cJSON_Duplicate(it, 1));
    }
    char* s = cJSON_PrintUnformatted(keep);
    const std::string filtered = s ? s : "{}";
    if (s) cJSON_free(s);
    cJSON_Delete(keep);
    std::vector<std::string> unk2;
    const bool ok = filerw::ELApplyProxies(kForm, filtered, applied, &unk2, err);
    for (const std::string& u : unk2) unknown->push_back(u);
    return ok;
}

void PutList(webbridge::JsonWriter& w, const char* key, const std::vector<std::string>& v)
{
    w.Key(key).BeginArray();
    for (const std::string& s : v) w.String(s);
    w.EndArray();
}

}  // namespace
void OS_EvBootProxies(); void OS_EvReset(); std::string OS_EvAfterPage();   //AI(W906-EVB3) 20260928 [W906]：OS-1 form.event（檔尾）；佔用原本的空行
// golden TfOffSet 建構（HT9045.cpp CreateForm）：DFM 設計期狀態 → 建構子（iOffsetMap、按鈕／元件指標陣列、Tag）→ 替身。
// 前提：fOffSet（移植樹門面）已建、EnsureArmOffsetObjects() 已跑、機台設定已讀。
void FileRW_Offset_Boot()
{
    if (g_booted) return;
    if (!fOffSet) {
        std::printf("FileRW Offset_File: fOffSet is null -- TfOffSet proxies NOT ready\n");
        return;
    }
    OS_DfmItems();
    OS_DfmState();
    OS_TfOffSet();
    OS_CreateSaveProxies();
    OS_CreateContainerProxies(); OS_EvBootProxies();   //AI(W906-EVB3) 20260928 [W906]：OS-1 微調鈕替身＋DFM 父層（檔尾）；同一行附加
    // 名稱表與 golden 建構子的指標陣列必須是同一批替身
    int bad = 0;
    for (int i = 0; i < OfsTotal; ++i) if (filerw::ELFind(kForm, kSelButton[i]) != OffSetSelBot[i]) ++bad;
    for (int i = 0; i < trayOfsTotal; ++i) if (filerw::ELFind(kForm, kSpecButton[i]) != OfffSetSpecSelBot[i]) ++bad;
    for (int j = 0; j < 2; ++j)
        for (int i = 0; i < 8; ++i) {
            if (filerw::ELFind(kForm, kPickEdit[j][i]) != MyPickEdit[j][i]) ++bad;
            if (filerw::ELFind(kForm, kRelsEdit[j][i]) != MyRelsEdit[j][i]) ++bad;
            if (filerw::ELFind(kForm, kSingleX[j][i]) != MySingleOffsetTEditX[j][i]) ++bad;
            if (filerw::ELFind(kForm, kSingleY[j][i]) != MySingleOffsetTEditY[j][i]) ++bad;
        }
    if (bad) {
        std::printf("FileRW Offset_File: %d name-table mismatches vs golden ctor pointer arrays -- NOT booted\n", bad);
        return;
    }
    std::printf("FileRW Offset_File: TfOffSet proxies ready (%d selections + %d tray/index) -- golden cOffSet.cpp ctor :186\n",
                (int)OfsTotal, (int)trayOfsTotal);
    g_booted = true;
}

// WS editlist.get tag=Offset_File（主迴圈，呼叫端持 FormLock）
int FileRW_Offset_Page(std::string* json)
{
    if (!g_booted) { *json = "Offset_File edit proxies are not booted"; return 409; }  const bool osB10AEntered = filerw::OpenEnterRecord("Offset_File", g_shown);   //AI(W906-FRW-S165) 20260927 [W906]: golden sbOffsetClick V912 main.cpp:28717 NewRecordProcess("MES2188", "Enter Offset") 在 :28718 fOffSet->Show()（→ 下面 OS_FormShow）之前；g_shown 到本函式尾才設 true（之後不再變回 false）⇒ 存檔後的自動重讀不再記（表與規則在 FileRW/_EditPage.cpp 檔尾）；接在同一行  //AI(W906-EVB10A) 20260929 [W906]: 回傳值留給下面 OS_FormShow 那一行（OS-7：這一次真的開窗才送 SECS Enter Offset）
    filerw::SessionBegin("");
    OS_bSkipReadFile = false;
    OS_bPartOnly = false;
    OS_bTailOnly = false;
    OS_FormShow();  { void FileRW_Offset_EvB10AShown(bool entered); FileRW_Offset_EvB10AShown(osB10AEntered); }   // golden :496（ReadFile＋可見度＋權限）  //AI(W906-EVB10A) 20260929 [W906]: OS-7 golden main.cpp:28718-28722 fOffSet->Show() 之後送 SECS EnterOffset；OS-6 記下「這一次開窗 golden FormShow 跑過」（關窗邊緣才跑 FormClose）。本體本檔檔尾；接在同一行
    OS_bSkipReadFile = true;                           // 逐一切換選取：檔案沒變，按鈕事件裡的 ReadFile 略過

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String("Offset_File");
    w.Key("form").String(kForm);
    w.Key("page").String("Setup.OffSet.html");
    w.Key("booted").Bool(true);
    w.Key("offsetDir").String(fOffSet->GetOffsetPath().c_str());
    w.Key("temperatureHot").Bool(LastSet.iTemperature == Tempture_Hot);
    w.Key("triTempMachine").Bool(Tri_Temp_Machine == 1);
    w.Key("offsets").BeginObject();
    for (int sel = 0; sel < OfsTotal; ++sel) {
        Select(sel, iSaveStander);
        PutGroup(w, sel, iSaveStander);
    }
    for (int tag = 0; tag < trayOfsTotal; ++tag) {
        Select(tag, iSaveSpecial);
        PutGroup(w, tag, iSaveSpecial);
    }
    w.EndObject();
    std::vector<std::string> common(kCommon, kCommon + sizeof(kCommon) / sizeof(kCommon[0]));
    PutWidgets(w, "common", common);
    w.Key("proxies").RawValue(filerw::ProxyStateJson(kForm));
    w.Key("session").RawValue(filerw::SessionJson());
    w.Key("events").RawValue(OS_EvAfterPage()); w.Key("eventTag").String("Setup.OffSet"); w.EndObject();   //AI(W906-EVB3) 20260928 [W906]：OS-1 form.event 的 events（{控制項:{event, golden, operable}}）與送 form.event 用的 tag（檔尾 (1)）；同一行
    OS_bSkipReadFile = false;
    g_shown = true;
    g_shownLevel = AccessLevel;
    *json = w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
    return 200;
}

// WS editlist.save tag=Offset_File（主迴圈，呼叫端持 FormLock）。
// widgetsJson＝{"offsets":{"<sel>"|"<tag>:2":{id:{text|checked}}…}, "common":{id:{…}}}
int FileRW_Offset_Save(const std::string& widgetsJson, const std::string& answersJson, std::string* ack, std::string* err)
{
    if (!g_booted) { *err = "Offset_File edit proxies are not booted"; return 409; }  OS_EvReset();   //AI(W906-EVB3) 20260928 [W906]：存檔會逐組重跑 golden 按鈕事件 → form.event 的「目前部位」作廢（檔尾 (2)）；接在同一行
    if (!g_shown || g_shownLevel != AccessLevel) {
        *err = "reload page: open the page (editlist.get Offset_File = golden FormShow) with the current access level before saving";
        return 409;
    }
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        *err = "widgets is not a JSON object";
        return 400;
    }
    cJSON* offsets = cJSON_GetObjectItemCaseSensitive(root, "offsets");
    cJSON* common = cJSON_GetObjectItemCaseSensitive(root, "common");
    if ((offsets && !cJSON_IsObject(offsets)) || (common && !cJSON_IsObject(common))) {
        cJSON_Delete(root);
        *err = "offsets / common must be JSON objects";
        return 400;
    }
    // 先全部檢查鍵（一個都不對就整包拒）
    std::string badKeys;
    for (cJSON* g = offsets ? offsets->child : nullptr; g; g = g->next) {
        int s = 0, m = 0;
        if (!ParseKey(g->string, &s, &m) || !cJSON_IsObject(g)) badKeys += std::string(badKeys.empty() ? "" : ", ") + g->string;
    }
    if (!badKeys.empty()) {
        cJSON_Delete(root);
        *err = "refused: bad group keys (\"<sel 0..OfsTotal-1>\" or \"<tag 0..trayOfsTotal-1>:2\", value an object): " + badKeys;
        return 400;
    }

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String("Offset_File");
    w.Key("groups").BeginObject();
    bool anySaved = false;
    int lastPage = 0;
    for (cJSON* g = offsets ? offsets->child : nullptr; g; g = g->next) {
        int sel = 0, mode = 0;
        ParseKey(g->string, &sel, &mode);
        const char* button = mode == iSaveSpecial ? kSpecButton[sel] : kSelButton[sel];
        w.Key(g->string).BeginObject();
        filerw::SessionBegin(answersJson);
        // 存檔時照 golden SpBotSelClick／IndexOffSetBT2Click 在 DoIniDataToForm 之前 ReadFile：非頁面欄位是「現在檔案」的值，
        // 且前一組剛寫的檔案也會被重讀（只有開頁的逐一切換才略過 ReadFile）
        OS_bSkipReadFile = false;
        OS_bPartOnly = true;
        OS_bTailOnly = false;
        std::string refused;
        if (!Editable(button)) refused = std::string("golden button ") + button + " is not visible/enabled (the user cannot select this part)";
        if (refused.empty()) {
            Select(sel, mode);                          // 非頁面欄位＝golden DoIniDataToForm 的值
            if (mode == iSaveStander && OS_bSelRefused) refused = "golden SpBotSelClick refused (KYEC barcode login)";
        }
        bool saved = false;
        if (refused.empty()) {
            OS_iActivePage = mode == iSaveSpecial ? 1 : 0;
            std::vector<std::string> applied, ignored, unknown;
            std::string e;
            const std::vector<std::string> names = mode == iSaveSpecial ? SpecialWidgets(sel) : StanderWidgets(sel);
            if (!ApplyGroup(g, names, &applied, &ignored, &unknown, &e)) {
                w.Key("error").String(e);
            } else {
                OS_spbSaveClick();                      // golden :2807 → SaveFile(目前選取) 為止（OS_bPartOnly）
                saved = filerw::ELMarked("SaveSetupFile");
                if (saved) { anySaved = true; lastPage = OS_iActivePage; }
            }
            PutList(w, "applied", applied);
            PutList(w, "ignored", ignored);
            PutList(w, "unknown", unknown);
        } else {
            w.Key("refused").String(refused);
        }
        w.Key("saved").Bool(saved);
        w.Key("session").RawValue(filerw::SessionJson());
        w.EndObject();
    }
    w.EndObject();

    // spbSaveClick 尾端：一次（ReadFile → ASE_CL scale → bUseUpdate → fMain->Pause → SetWorkParameter）
    const bool commonSent = common && common->child;
    bool tailRan = false;
    std::vector<std::string> cApplied, cIgnored, cUnknown;
    std::string cErr;
    if (anySaved || commonSent) {
        std::vector<std::string> names(kCommon, kCommon + sizeof(kCommon) / sizeof(kCommon[0]));
        if (commonSent && !ApplyGroup(common, names, &cApplied, &cIgnored, &cUnknown, &cErr)) {
            w.Key("commonError").String(cErr);
        }
        filerw::SessionBegin(answersJson);
        OS_bSkipReadFile = false;
        OS_bPartOnly = false;
        OS_bTailOnly = true;
        OS_iActivePage = lastPage;
        OS_spbSaveClick();
        OS_bTailOnly = false;
        tailRan = true;
    }
    OS_bSkipReadFile = false;
    OS_bPartOnly = false;
    w.Key("tail").BeginObject();
    w.Key("ran").Bool(tailRan);
    if (commonSent) {
        PutList(w, "applied", cApplied);
        PutList(w, "ignored", cIgnored);
        PutList(w, "unknown", cUnknown);
    }
    if (tailRan) w.Key("session").RawValue(filerw::SessionJson());
    w.EndObject();
    w.Key("saved").Bool(anySaved);
    w.EndObject();
    cJSON_Delete(root);
    *ack = w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
    return 200;
}

// ---- golden fContact->IsRun2DCheck()：移植樹 fContact 是 TfContactShim（atester_shims.h，離線回 false），
//      真表單是 fContactForm（forms/fContact.h:1573 IsRun2DCheck ACTIVE、:1634 fContactForm）。
#include "forms/fContact.h"

bool FileRW_Offset_IsRun2DCheck() { return fContactForm ? fContactForm->IsRun2DCheck() : false; }   // golden cOffSet.cpp:1173／:1261
#include <stdexcept>
#include "FileRW/_FormEventCtx.h"      //AI(W906-D013) 20260929 [W906]：R128 formevent::CurrentValueJson（value 的 "noPart"）

// ===========================================================================
//  //AI(W906-EVB3) 20260928 [W906]：OS-1（事件移植批次 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B3；
//  Steven 20260928「任何畫面的事件, 都是我們做」「如果沒有移植的, 我們直接實作」）—— WS form.event 接 Offset 頁。
//  規格：D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md §3.0g（別名頁的做法同 §3.0g-7 IniConfig）。
//
//  golden（V912 cOffSet.cpp）：sb_AutoOffsetUp／Down／Right／Left Click（:2892／:2905／:2918／:2931）＝ SystemStart 時 return；
//    edArmY／edArmX ±0.1；spbSaveClick(this)（:2807，存「目前選的部位」iNowOffsetSel）；勾了 cb_AutoOffsetPositionCheck → fMain->Start
//    （OS-1b，會讓機台動 → 不做，產生器 replace 記 todo，B8／Jimmy）。處理器由產生器轉（tools/editlist/Offset_File.py，kOS_Events）。
//  (1) 別名頁：Offset_File 不是 PageDesc（開頁／存檔是本檔 FileRW_Offset_Page／FileRW_Offset_Save，JSON 形狀不同），W906_FormEvent
//      （FileRW/_FormEvent.cpp）只認 PageDesc ⇒ 另登記一個只給 form.event 用的 kEvPage：tag "Setup.OffSet"、page "Setup.OffSet.html"、
//      form "TfOffSet"（同一批替身）。tag 不能用 "Offset_File"：tools/wb_serve.cpp 的 editlist.get／save 先找 PageDesc，會改走
//      _EditPage.cpp 的 PageJson／PageSave。"Setup.OffSet" 沒有開窗閘（_EditPage.cpp kOpenGates）⇒ 拿它送 editlist.get／save 一律 no-gate；
//      沒有 kOpenEnters 列 ⇒ 不多記 "Enter Offset"。RunPageEvent 要這個 tag「開過頁、同一個等級」⇒ FileRW_Offset_Page 尾端呼叫一次
//      filerw::PageJson(kEvPage)（formShow 是空的；它回應裡的 "events" 轉給頁面，另帶 "eventTag"）。
//  (2) 選取式編輯器：頁面點部位鈕只在頁面換顯示（ht9045_offset_wire.js；開頁時伺服器已把每一組都跑過一次），伺服器端的 iNowOffsetSel
//      停在開頁迴圈的最後一組 —— 照原樣跑 golden 處理器會把 ±0.1 存到錯的部位。所以：
//      * 53 顆部位鈕（kSelButton，golden 建構子 :326 OnClick＝SpBotSelClick）也登記成 form.event click → golden SpBotSelClick(Tag)
//        （含 ReadFile＋DoIniDataToForm，golden 點部位鈕就是這樣；換部位前存上一組那一行照原本的 replace 不做）。
//        產生器的 events 收不了（處理器參數是 int iPtrTag）→ 這 53 列在這裡手寫。
//      * 四顆微調鈕的處理器包一層：伺服器端「最近一次 form.event 選的部位」要等於 iNowOffsetSel，而且之後沒有重新開頁／存檔
//        （兩者都會逐組重跑、改掉 iNowOffsetSel），否則回 handler-failed「先點一次部位鈕」—— 不猜頁面在哪一組。
//      * 頁面的送法（D:\HT9045\web\page\ht9045_offset_ev.js）：先送目前部位鈕的 click（不套回覆的 changed，頁面照自己的整包顯示），
//        再送微調鈕的 click＋state（這一組畫面上的值，RunPageEvent 先套上去）＝golden「畫面上的值 ±0.1 後整組存」。
//  (3) 跟 golden 不同：golden 點部位鈕時會先存上一組（SpBotSelClick :2487）；C 路頁面是「切組記住、按 Save 一起送」，
//      所以微調鈕存的只有目前這一組，別組沒按 Save 的修改還留在頁面上（頁面照舊按 Save 才寫）。
//      ⛔ 更正 //AI(W906-D013) 20260929 [W906]：R127＝照 BCB（decisions-decided R127「20260929 結果」：換部位時先存上一組，選項 B）——
//      OS_EvSelect 照 golden :2487 先存上一組（產生器 OS_bSavePrev，tools/editlist/Offset_File.py）；「上一組」＝這一頁 form.event 上一次選的
//      部位 g_evSel（伺服器替身現在就是那一組；RunPageEvent 已先把頁面帶的 state＝那一組的畫面值套上去）。頁面按微調時把「改過還沒存」的
//      stander 組依序點一遍（點下一組時帶上一組的值），最後點目前這一組＝golden 操作員一路切過來、每切一次存上一次（ht9045_offset_ev.js）。
//      開頁／存檔之後 g_evSel＝-1（golden 還沒選過＝iNowOffsetSel -1）⇒ 第一下不存。special 組（Index／Tray，IndexOffSetBT2Click :2598）沒做：
//      那些鈕在 golden 藏起來的 tsIndexOffset 頁上（R129 同一個前提），照舊按 Save 才寫。
//  (5) //AI(W906-D013) 20260929 [W906]：R128＝照 BCB（選項 B）—— golden 開窗後還沒選部位（iNowOffsetSel=-1，建構子 :371）就按微調：
//      SaveFile(-1) 什麼都不存（:1408），spbSaveClick 尾端照跑（ReadFile、bEnterOffset、ASE_CL、bUseUpdate、fMain->Pause("Save Offset")、
//      labWarningForStop、SetWorkParameter）。頁面沒有選部位時微調鈕照送、value 帶 "noPart":true（不先點部位鈕）⇒ OS_EvAuto 把 iNowOffsetSel
//      設成 -1 再跑 golden 處理器（FileRW/_FormEventCtx.h CurrentValueJson）。沒帶 noPart、又沒先點部位鈕＝照舊 select-first。
//  (4) OS-2（IndexArmOffSet3Change :3263，iIndexChange=2）不接：golden 全樹只有寫、沒有讀這個旗標；控制項在 tsIndexOffset
//      （golden 建構子 :192 TabVisible=false）→ filerw::ELOperable 點不到。見交件 R 題。
// ===========================================================================
namespace {
const char kEvTag[] = "Setup.OffSet";
int g_evSel = -1;               // 最近一次 form.event 部位鈕選的 iNowOffsetSel；開頁／存檔後歸 -1（兩者都會逐組重跑）
bool OS_EvBooted() { return g_booted; }
void OS_EvNoFormShow() {}       // golden FormShow 已經由 FileRW_Offset_Page 跑過；這裡只為了登記「開過頁」
void OS_EvRefuseSave()          // 不會走到（wb_serve 對這個 tag 先回 no-gate）；保險：不存
{
    filerw::ELTodo("Setup.OffSet is only the form.event alias of Offset_File -- save with WS editlist.save tag=Offset_File (nothing saved)");
}
void OS_EvNoReload() {}
const filerw::PageDesc kEvPage = {
    kEvTag, "TfOffSet", "Setup.OffSet.html",
    nullptr, nullptr, 0,
    nullptr, 0,
    &OS_EvNoFormShow, &OS_EvRefuseSave, "SaveSetupFile", &OS_EvNoReload, &OS_EvBooted,
    nullptr, nullptr,
};
filerw::PageRegistrar g_evPageReg(&kEvPage);

void OS_EvRequireShown()        // 本檔自己的開頁紀錄（RunPageEvent 查過別名頁的；這裡再查 Offset_File 的）
{
    if (!g_shown || g_shownLevel != AccessLevel)
        throw std::runtime_error("reload page: open the page (WS editlist.get tag=Offset_File = golden FormShow) with the current access level first");
}
//AI(W906-D013) 20260929 [W906]：R128 —— value 的 "noPart"（true＝頁面沒有選部位，本段檔頭 (5)）。沒帶／null／false＝照舊；別的型別 ⇒ 例外（handler-failed）
bool OS_EvNoPart()
{
    cJSON* v = cJSON_Parse(formevent::CurrentValueJson().c_str());
    const cJSON* n = v ? cJSON_GetObjectItemCaseSensitive(v, "noPart") : nullptr;
    const bool bad = n && !cJSON_IsNull(n) && !cJSON_IsBool(n);
    const bool on = n && cJSON_IsTrue(n);
    if (v) cJSON_Delete(v);
    if (bad) throw std::runtime_error("bad-payload: noPart must be true, false or null");
    return on;
}
// 部位鈕：golden SpBotSelClick（:2476，Sender＝部位鈕，產生檔的參數是它的 Tag＝建構子 :326 OffSetSelBot[i]->Tag=i）
void OS_EvSelect(TControl* Sender)
{
    OS_EvRequireShown();
    for (int i = 0; i < OfsTotal; ++i) {
        if (OffSetSelBot[i] != Sender) continue;
        OS_bSkipReadFile = false;                                               // golden 點部位鈕照讀檔（:2521 前）
        OS_bPartOnly = false;
        OS_bTailOnly = false;
        OS_bSelRefused = false;
        //AI(W906-D013) 20260929 [W906]：R127 照 BCB（見本段檔頭 (3) ⛔）—— golden :2487 換部位前先存上一組；上一組＝這一頁 form.event 上一次
        //   選的部位，而且伺服器現在就停在那一組（開頁／存檔會逐組重跑、g_evSel 歸 -1 ⇒ 不存，golden 還沒選過也是 SaveFile(-1) 不存）。
        //   OS_bSavePrev 只在這一支處理器期間是 true（例外也收回：開頁／存檔的逐組切換絕對不能照 :2487 寫檔）。
        struct SavePrevOff { ~SavePrevOff() { OS_bSavePrev = false; } } savePrevOff;
        const int prev = (g_evSel >= 0 && g_evSel == iNowOffsetSel) ? iNowOffsetSel : -1;
        OS_bSavePrev = prev >= 0;
        g_evSel = -1;
        OS_SpBotSelClick(i);
        OS_bSavePrev = false;
        if (!OS_bSelRefused) g_evSel = iNowOffsetSel;                           // KYEC barcode 拒絕（golden return）＝沒選到
        if (prev >= 0)
            std::printf("FileRW Offset_File: form.event %s -> golden SpBotSelClick saved the previous part %s (%d) first (cOffSet.cpp:2487, R127)\n",
                        kSelButton[i], kSelButton[prev], prev);
        if (!OS_bSelRefused && iNowOffsetSel != i) {                            // golden :2488-2493 bResult（SaveSetupFile 回 true：CC_TFME_CHINA 超出上下限被重設）⇒ 停在上一組
            g_evSel = -1;
            filerw::ELTodo((std::string("golden SpBotSelClick kept part ") + (iNowOffsetSel >= 0 && iNowOffsetSel < OfsTotal ? kSelButton[iNowOffsetSel] : "-1") +
                            " (SaveFile of the previous part returned true: values outside the limits were reset, cOffSet.cpp:2488) -- reload page to see the file values").c_str());
        }
        return;
    }
    throw std::runtime_error("internal: form.event sender is not one of the golden OffSetSelBot[] part buttons");
}
// 微調鈕：先確定伺服器選的就是頁面那一組，再跑 golden 處理器
void OS_EvAuto(TControl* Sender)
{
    OS_EvRequireShown();
    if (OS_EvNoPart()) {                                                        //AI(W906-D013) 20260929 [W906]：R128 照 BCB（本段檔頭 (5)）：頁面沒有選部位 ⇒ golden iNowOffsetSel=-1（建構子 :371）
        iNowOffsetSel = -1;                                                     //   → SaveFile(-1) 不存、spbSaveClick 尾端照跑
        g_evSel = -1;
    } else
    if (g_evSel < 0 || g_evSel != iNowOffsetSel)
        throw std::runtime_error("select-first: send form.event click on the current part button (golden SpBotSelClick) right before this one "
                                 "-- the server does not know which part the page shows (page open / save re-runs every part)");
    for (std::size_t i = 0; i < sizeof(kOS_Events) / sizeof(kOS_Events[0]); ++i) {
        if (filerw::ELFind(kForm, kOS_Events[i].control) != Sender) continue;
        kOS_Events[i].handler(Sender);                                          // golden :2892／:2905／:2918／:2931
        return;
    }
    throw std::runtime_error("internal: form.event sender is not in kOS_Events");
}
filerw::PageEvent g_osEvents[sizeof(kOS_Events) / sizeof(kOS_Events[0]) + OfsTotal];  bool OS_EvIsSort(const char* control); void OS_EvSort(TControl* Sender); void OS_EvB8BootProxies();   //AI(W906-B8-OS5) 20260930 [W906]: OS-5 排序鈕的跳板與替身父層（本體檔尾）；同一行附加
int g_osNEvents = 0;
struct OsEvRegistrar {
    OsEvRegistrar()
    {
        int n = 0;
        for (std::size_t i = 0; i < sizeof(kOS_Events) / sizeof(kOS_Events[0]); ++i, ++n) {
            g_osEvents[n] = kOS_Events[i];
            g_osEvents[n].handler = OS_EvIsSort(kOS_Events[i].control) ? &OS_EvSort : &OS_EvAuto;   //AI(W906-B8-OS5) 20260930 [W906]: OS-5 的 12 顆排序鈕不走微調鈕的「先選部位」檢查（golden btnSortAuto1Click 不看部位，檔尾）；同一行改寫
        }
        for (int i = 0; i < OfsTotal; ++i, ++n) {
            g_osEvents[n].control = kSelButton[i];
            g_osEvents[n].event = "click";
            g_osEvents[n].golden = "cOffSet.cpp:2476 TfOffSet::SpBotSelClick";
            g_osEvents[n].handler = &OS_EvSelect;
        }
        g_osNEvents = n;
        filerw::RegisterPageEvents(kEvTag, g_osEvents, n);
    }
} g_evreg;
}  // namespace

// FileRW_Offset_Boot 呼叫（OS_CreateContainerProxies 同一行）：四顆微調鈕的替身（產生器只替有 DFM Tag 的三顆建了）＋DFM 父層
void OS_EvBootProxies()
{
    static const char* const kBtn[] = {"sb_AutoOffsetUp", "sb_AutoOffsetDown", "sb_AutoOffsetRight", "sb_AutoOffsetLeft"};   // golden cOffSet.h:97-100 TSpeedButton
    static const char* const kPar[][2] = {{"sb_AutoOffsetUp", "pan_AutoOffsetMove"}, {"sb_AutoOffsetDown", "pan_AutoOffsetMove"},
                                          {"sb_AutoOffsetRight", "pan_AutoOffsetMove"}, {"sb_AutoOffsetLeft", "pan_AutoOffsetMove"}};   // golden cOffSet.dfm:8772-9241
    for (std::size_t i = 0; i < sizeof(kBtn) / sizeof(kBtn[0]); ++i) EL<TSpeedButton>(kForm, kBtn[i]);
    filerw::ELSetParents(kForm, kPar, (int)(sizeof(kPar) / sizeof(kPar[0])));  OS_EvB8BootProxies();   //AI(W906-B8-OS5) 20260930 [W906]: OS-5 btnSortAuto4／5、btnSortFix4／5／6 的 DFM 父層 grpOutArm456（檔尾）；同一行附加
    for (int i = 0; i < g_osNEvents; ++i)
        if (!filerw::ELFind(kForm, g_osEvents[i].control))
            std::printf("FileRW Offset_File: WARNING form.event control %s has no proxy (add it to OS_EvBootProxies)\n", g_osEvents[i].control);
}
void OS_EvReset() { g_evSel = -1; }
// FileRW_Offset_Page 尾端（golden FormShow 之後）：登記別名頁「開過頁、這個等級」，回傳它的 "events"（{控制項:{event, golden, operable}}）
std::string OS_EvAfterPage()
{
    g_evSel = -1;
    std::string page;
    if (filerw::PageJson(kEvPage, &page) != 200) return "{}";
    cJSON* j = cJSON_Parse(page.c_str());
    const cJSON* ev = j ? cJSON_GetObjectItemCaseSensitive(j, "events") : nullptr;
    char* s = ev ? cJSON_PrintUnformatted(ev) : nullptr;
    const std::string out = s ? s : "{}";
    if (s) cJSON_free(s);
    if (j) cJSON_Delete(j);
    return out;
}

// ===========================================================================
//  AI(W906-EVB10A) 20260929 [W906]：事件批次 B10 part a（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
//    B10 表 OS-6、OS-7；Steven 20260928「任何畫面的事件, 都是我們做」「如果沒有移植的, 我們直接實作」、20260929「照 BCB 的邏輯」）。
//  OS-7　golden TfMain::sbOffsetClick（V912 main.cpp:28709-28722）：:28717 NewRecordProcess("MES2188","Enter Offset")（上面 FileRW_Offset_Page
//    開頭那一行，S165）→ :28718 fOffSet->Show()（非模態；→ FormShow＝OS_FormShow）→ :28720-28721 bEnable_SECS_GEM 時 EventReport(SECS_EVENT.EnterOffset)。
//    ⇒ 在 OS_FormShow 之後、而且只有「這一次開窗真的記了 Enter」（OpenEnterRecord 回 true：頁面表的關窗邊緣清過之後的第一次 editlist.get）
//    才送；存檔後的自動重讀不送（golden 沒有再按一次 sbOffset）。移植樹的 EventReport 是模擬計數（SECSGEM/SecsEventReport.h），不送 MES。
//  OS-6　golden TfOffSet::FormClose（cOffSet.cpp:835-857，產生檔 FileRW/Offset_File.gen.inc 檔尾 OS_FormClose）：fShow=false、
//    cb_AutoOffsetPositionCheck 取消、bOffsetEnterBarcode=false、ReadFile（丟掉沒存的改動）、Timer1／TimerSetupTeach 停；
//    bDoInZTeach||bDoOutZTeach（Z 教導按了還沒做完）⇒ 清掉、fAllMotorHome=false、iHome=1（下一次 START 先全部回原點）＋
//    NewRecordProcess("", "ZCalibration not finish, do homing!", "OffSet->FormClose")。
//    golden 的 Offset 沒有 ✕（cOffSet.dfm:4 BorderIcons=[]），只能按 Exit（sbtExitClick → Close → FormClose）；網頁的 Exit／✕ 都只是外框關視窗
//    ⇒ 由頁面表的關窗邊緣呼叫（FileRW/MainClick.cpp W906_EvB10A_WindowEdge，表在 FileRW/WindowEdgeTails.h）。
//    ⚠ 會改機台狀態：fAllMotorHome=false／iHome=1 只在 Z 教導做到一半時（golden 同）；運轉中也照跑（golden Offset 是非模態、開窗沒有
//    SystemStart 守衛，關掉就跑 FormClose）。golden sbtExitClick（:2801）另外的內容這裡沒做（見交件）。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"
#include "SECSGEM/SecsEventType.h"     // SECS_EVENT.EnterOffset（OS-7）
#include "SECSGEM/SecsEventReport.h"   // EventReport（OS-7；同 FileRW/MainClick.cpp SP-6）

namespace {
evb10a::OpenLatch g_b10aOpen;   // 這一次開窗 golden FormShow 跑過（OS-6 關窗時才跑 FormClose）
std::string       g_b10aWhat;
}  // namespace

// FileRW_Offset_Page 的 OS_FormShow 那一行呼叫（golden FormShow 剛跑完）。
void FileRW_Offset_EvB10AShown(bool entered)
{
    g_b10aOpen.Shown();
    if (!entered) return;                                                       // 同一次開窗裡的重讀（存檔後、重讀鈕）：golden 沒有再按 sbOffset
    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem   // golden main.cpp:28720
        EventReport(SECS_EVENT.EnterOffset);                                    // golden main.cpp:28721
    std::printf("[EVB10A] fOffSet opened -> %s (golden V912 main.cpp:28720-28721 sbOffsetClick, after fOffSet->Show())\n",
                IniConfig.bEnable_SECS_GEM ? "EventReport(SECS_EVENT.EnterOffset)" : "bEnable_SECS_GEM off, no EnterOffset");
}

// 頁面表的邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge；呼叫端持 FormLock、已 SessionBegin）。
const char* FileRW_Offset_WindowEdge(bool open)
{
    if (open) return "no open-edge action (OS-7 EnterOffset is sent by the C-route page open, FileRW_Offset_Page)";
    if (!g_booted) return "not run: Offset_File edit proxies are not booted";
    if (!g_b10aOpen.TakeForClose())
        return "not run: golden FormShow did not run in this window-open (open gate refused or the page never read) -- golden: the form never opened, no FormClose";
    const bool zTeach = bDoInZTeach || bDoOutZTeach;
    OS_bSkipReadFile = false;                                                   // FormClose 的 ReadFile 照 golden 讀檔（丟掉沒存的改動）
    OS_bPartOnly = false;
    OS_bTailOnly = false;
    OS_FormClose();                                                             // golden cOffSet.cpp:835
    OS_EvReset();                                                               // form.event 的「目前部位」作廢（下次開窗重新選）
    g_b10aWhat = zTeach
        ? "ran golden TfOffSet::FormClose (cOffSet.cpp:835): Z teach was not finished -> bDoInZTeach/bDoOutZTeach=false, fAllMotorHome=false, iHome=1 "
          "(next START homes all), NewRecordProcess(\"ZCalibration not finish, do homing!\")"
        : "ran golden TfOffSet::FormClose (cOffSet.cpp:835): ReadFile, cb_AutoOffsetPositionCheck off, timers off (no Z teach in progress)";
    return g_b10aWhat.c_str();
}

// ===========================================================================
//  //AI(W906-B8-OS5) 20260930 [W906]：B8 OS-5（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「OS-5」；Steven 20260928
//    「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）—— A30 Setup Teach 的 12 顆排序鈕（Setup Teaching 分頁）。
//  golden V912 cOffSet.cpp:3211-3221 btnSortAuto1Click（12 顆共用；cOffSet.dfm grpSetupTeach 裡每一顆 OnClick = btnSortAuto1Click）：
//    IniConfig.bA30SetupTeachFunction && LastSet.iTester==OFF_LINE && LastSet.bNeedSetupTeach ⇒ iSortUnloadT6=Sender->Tag
//    （Tag：Auto1～6＝0～5、Fix1～6＝6～11；DFM 設計期值由產生器寫進 OS_DfmState，建構子 :425-430 再設 Auto1～3／Fix1～3＝eAuto1…eFix3，同值）。
//    按下當下機台不動：讀 iSortUnloadT6 的是運轉中 Out Arm 放料（golden aoutarm.cpp:3641-3643 → OutArmSuck.iWhichAuto，同三條件＋iSortUnloadT6>=0；
//    移植樹 aoutarm.cpp:4172）。處理器由產生器轉（tools/editlist/Offset_File.py 的 _OS5_EVENTS → FileRW/Offset_File.gen.inc 的 OS_btnSortAuto1Click）。
//  跟 OS-1 微調鈕同一個別名頁（tag "Setup.OffSet"，本檔 (1)）；不同的地方：
//    * 不做「先選部位」檢查（OS_EvAuto 的 select-first）：golden btnSortAuto1Click 不看 iNowOffsetSel，也不存檔。
//    * 不另查本檔的 g_shown：RunPageEvent 第 1 步已查別名頁「開過頁、同一個等級」—— 別名頁只在 FileRW_Offset_Page 尾端（跟 g_shown 同一次）登記，
//      關窗邊緣會清（FileRW/_EditPage.cpp PageWindowClosed），所以比 g_shown（設了就不清）嚴。
//    * 點不點得到照 golden 畫面：RunPageEvent 第 2 步 ELOperable 沿 DFM 父層查 —— grpSetupTeach 只在 A30＋離線時看得見（FormShow :712-721，
//      DFM Visible=False）、grpOutArm456 要 AUTO_EMPTY_COLOR>=3、btnSortAuto6 要 >=4（FormShow :592-594）。golden 方法沒提到的五顆
//      （btnSortAuto4／5、btnSortFix4／5／6）產生器沒有登記父層 ⇒ 這裡照 golden DFM 補（cOffSet.dfm:13153-13212，父＝grpOutArm456）。
//  ⚠ 跟 golden 不同（寫明，不在這一列改）：golden Offset 是非模態（main.cpp:28718 fOffSet->Show()），btnSortAuto1Click 沒有 SystemStart 檢查，
//    而且 A30＋bNeedSetupTeach 時 golden TfMain::Start 會自己把 Offset 打開（main.cpp:5094-5112）——排序鈕本來就是給「運轉中指定下一顆放哪一盤」用的。
//    移植樹 form.event 運轉中一律拒收（FileRW/_FormEvent.cpp:77-81，RULINGS_20260927 第 2 條第 7 題 A）⇒ 網頁只能在停機時按。跟 B8 P-3（Contact）同一種形狀，
//    要不要開例外交 ST01-E（共用守衛，不在這一列動）。⛔ 20260930 更正 //AI(W906-FE-RUNEXC)：已開例外——FileRW/_FormEvent.cpp 檔尾的運轉中例外表列了這 12 顆，SystemStart／SoftStart 時照 golden 放行（頁面表要說 fOffSet 開著）；本檔不用改。
//  TimerSetupTeachTimer（:3104-3209）在 V912 除了開頭 A30 檢查全被註解掉（JerryYang 20230523「沒在用」）＝一拍什麼都不做 ⇒ 沒有轉，也不需要拍子；
//    FormShow／FormClose 設 TimerSetupTeach->Enabled 的產生碼照舊。
//  測試：ctest B8_Os5_SortButtons（tests/test_b8_os5_sortbuttons.cpp）。
// ===========================================================================
#include <cstring>

namespace {
bool OS_EvIsSort(const char* control)
{
    return std::strncmp(control, "btnSortAuto", 11) == 0 || std::strncmp(control, "btnSortFix", 10) == 0;
}
void OS_EvSort(TControl* Sender)
{
    for (std::size_t i = 0; i < sizeof(kOS_Events) / sizeof(kOS_Events[0]); ++i) {
        if (!OS_EvIsSort(kOS_Events[i].control) || filerw::ELFind(kForm, kOS_Events[i].control) != Sender) continue;
        const int before = iSortUnloadT6;
        kOS_Events[i].handler(Sender);                                          // golden :3211
        std::printf("FileRW Offset_File: form.event %s (Tag %d) -> golden btnSortAuto1Click: A30=%d iTester=%s bNeedSetupTeach=%d -> iSortUnloadT6 %d -> %d%s\n",
                    kOS_Events[i].control, Sender ? Sender->Tag : -1, (int)IniConfig.bA30SetupTeachFunction,
                    LastSet.iTester == OFF_LINE ? "OFF_LINE" : "ON_LINE", (int)LastSet.bNeedSetupTeach, before, iSortUnloadT6,
                    before == iSortUnloadT6 ? " (unchanged)" : "");
        return;
    }
    throw std::runtime_error("internal: form.event sender is not one of the golden btnSort* buttons in kOS_Events");
}
void OS_EvB8BootProxies()
{
    static const char* const kPar[][2] = {{"btnSortAuto4", "grpOutArm456"}, {"btnSortAuto5", "grpOutArm456"},   // golden cOffSet.dfm:13153／:13163
                                          {"btnSortFix4", "grpOutArm456"}, {"btnSortFix5", "grpOutArm456"},     // :13183／:13193
                                          {"btnSortFix6", "grpOutArm456"}};                                     // :13203
    for (std::size_t i = 0; i < sizeof(kPar) / sizeof(kPar[0]); ++i) EL<TButton>(kForm, kPar[i][0]);          // 產生器的 OS_DfmState 已建（DFM Tag）；保險
    filerw::ELSetParents(kForm, kPar, (int)(sizeof(kPar) / sizeof(kPar[0])));
}
}  // namespace
