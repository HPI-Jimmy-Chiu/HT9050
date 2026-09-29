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
        OS_bSelRefused = false;
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
    OS_CreateContainerProxies();
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
    if (!g_booted) { *json = "Offset_File edit proxies are not booted"; return 409; }  filerw::OpenEnterRecord("Offset_File", g_shown);   //AI(W906-FRW-S165) 20260927 [W906]: golden sbOffsetClick V912 main.cpp:28717 NewRecordProcess("MES2188", "Enter Offset") 在 :28718 fOffSet->Show()（→ 下面 OS_FormShow）之前；g_shown 到本函式尾才設 true（之後不再變回 false）⇒ 存檔後的自動重讀不再記（表與規則在 FileRW/_EditPage.cpp 檔尾）；接在同一行
    filerw::SessionBegin("");
    OS_bSkipReadFile = false;
    OS_bPartOnly = false;
    OS_bTailOnly = false;
    OS_FormShow();                                     // golden :496（ReadFile＋可見度＋權限）
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
    w.EndObject();
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
    if (!g_booted) { *err = "Offset_File edit proxies are not booted"; return 409; }
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
