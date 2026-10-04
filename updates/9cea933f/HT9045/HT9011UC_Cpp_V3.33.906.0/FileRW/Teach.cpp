// ===========================================================================
//  FileRW/Teach.cpp -- 結構 Teach（System\teach.ini／tech.dat）的讀寫檔（C 路：D＝TECH_* ＋ C＝elTeach）。
//
//  AI(W906-W5-TEACH) 20260925.  使用者 20260924 晚：「MotorTest畫面功能完善化後，接著要把Teach畫面完善化，
//  可存讀參數，這些參數也必須和C++裡的變數是同步的」；做法定案「採用 Steven 的 C 路（golden 表單橋），不另外發明」。
//  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md 一之二（具名替身）與 §四
//        （「Teach | teach.ini | D（TECH_* SaveToFile）＋C（elTeach）… → Teach.cpp」）。
//
//  ---------------------------------------------------------------------------
//  資料從哪來、存到哪去（golden uteach.cpp）
//  ---------------------------------------------------------------------------
//    D 類：TECH_PARA（登錄表 forms/fTeachRegistry.cpp）、TECH_TWOPARA、TECH_SUCKPARA —— 每筆 = 一個 Tech.* 變數
//          ＋ teach.ini 的 [馬達名] Key。讀：TfTeach::ReadFile（forms/fTeachPara.cpp，TEACH-W1 已活）。
//    C 類：elTeach（golden InitialTeachEditList :3131-3358，203 筆，本檔由 Teach.gen.inc 註冊成具名替身）。
//    另外：Gerneral.ini 的 [Shuttle] CHECK_RANGE／iInShtZRange、teach.ini 的旋轉背隙兩鍵。
//
//  開頁（WS editlist.get tag=Teach）= golden FormShow 的資料部分：ReadFile（→ elTeach 從檔讀進替身與變數），
//    再把 D 類變數鏡像到具名替身（golden ReadFromFile 尾端的 `SetEdit->Text=*Parameter`；移植樹 SetEdit 是 NULL）。
//  存檔（WS editlist.save tag=Teach）= golden btnSaveClick（:2261-2389）：頁面值套進替身 → UpdateTempTech（替身 → 變數）
//    → 客戶碼夾限 → 寫 Gerneral.ini 兩鍵 → SaveFile（tech.dat＋各 TECH_* 寫 teach.ini＋elTeach＋背隙）→ ReadFile
//    → InitShuttleThreadParameter。**存完一定重讀**，所以檔案與 C++ 變數同步是 golden 流程本身保證的。
//
//  ⚠ 偏離與限制（逐條）：
//    1. golden 的確認框是 MessageDlg（只在 fAllMotorHome==false 時問）→ filerw::ELAsk，題目用 golden 原字串。
//    2. btnSaveClick 的 D63（Index Y 尋相，IniConfig.bD63…）與 AOI（USE_Scanner_AOI_Inspection）兩段沒有移植的相依
//       （FrmAOI、SetIndexYPhasePosition）→ 條件成立時 ELTodo 回報，不假裝做了。
//    3. 移植樹 TECH_*::SaveToFile 寫 *Parameter 而不是元件文字（使用者 20260919 裁決 A4=(b)）；本檔先做 UpdateTempTech
//       （替身 → *Parameter），結果與 golden 寫元件文字相同。
//    4. tech.dat：TECH 版面依版本不同（V899 3792／906 3872／V912 3872 但兩欄搬到結尾，NB2 R24）⇒ 一律不寫、回報 ELTodo（IC_SaveFile 註解；
//       待使用者裁決，NIGHT_REPORT §0）。golden ReadFile 只在 teach.ini 缺 Update2 鍵時才讀它，所以 teach.ini 仍是唯一的真實來源。
// ===========================================================================
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

#include "FileRW/_EditList.h"
#include "Public/HTEditList.h"
#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"
#include "forms/fTeach.h"          // fTeach、TfTeach::ReadFile、TechPara／TechTwoPara／TechSuckPara
#include "forms/fTeachPara.h"
#include "LastSet.h"               // Tech（TECH）
#include "cprod.h"                 // Teach、IniConfig
#include "cmydef.h"                // fAllMotorHome、CHECK_RANGE、iInShtZRange、CUSTOMER_CODE、USE_OUT_SORT_ARM …
#include "common.h"                // asTeachPath、WriteIniData、WriteIniDataGeneral、CheckAndReadIniDataGeneral、WriteData
#include "cinitial.h"              // InitShuttleThreadParameter
#include "Config.h"
#include "Motor/mymotor.h"         // MOT[] / .Alias（TECH_PARA 的 teach.ini 區段）
#include "FileRW/_EditPage.h"      // AI(W906-FRW-S165) 20260927 [W906]: filerw::OpenEnterRecord（開頁記 golden 的 "Enter ..."，RULINGS_20260926 S165＝R101）；佔用原本的空行
#include "FileRW/Teach.gen.inc"    // IC_InitialTeachEditList、kTeachKeyWidget、kTeInArm／kTeOutArm／kTeSortArm

namespace {

const char* const kForm = "TfTeach";
bool g_teachInit  = false;
bool g_formShown  = false;
const char* const kSaveQuestion = "Sure to Save? (確定要存檔?)";   // golden uteach.cpp:2267 MessageDlg 原字串

TEdit* P(const char* name) { return filerw::EL<TEdit>(kForm, name); }

const char* WidgetOfKey(const AnsiString& key)
{
    for (int i = 0; i < kTeachKeyWidgetCount; ++i)
        if (key == kTeachKeyWidget[i][0]) return kTeachKeyWidget[i][1];
    return key.c_str();                                   // golden 的 Key 幾乎都等於元件名（fTeachPara.h 檔頭）
}

// elTeach 裡有的元件（同一個元件也是 TECH_PARA 時，golden ReadFile 先 ReadFromFile 再 elTeach 讀檔 ⇒ 元件最後是 elTeach 的值）
std::set<std::string> ElTeachNames()
{
    std::set<std::string> s;
    if (!elTeach) return s;
    for (int i = 0; i < elTeach->FEditList->Count; ++i) {
        THTEdit* it = static_cast<THTEdit*>(elTeach->FEditList->Items[i]);
        if (!it->ControlName.IsEmpty()) s.insert(it->ControlName.c_str());
    }
    return s;
}

// golden ReadFromFile 尾端 `SetEdit->Text=(int)(*Parameter)` 的鏡像；elTeach 的元件不蓋（它在 golden 裡最後被 elTeach 寫）。
void MirrorToProxies(bool formShowOpen)   //AI(W906-KB-GOLDEN) 20261004: split in two halves -- golden ReadFile's mirror (TECH_* ReadFromFile uteach.cpp:115 / :187-188 / :229 + backlash ReadFile :4870-4871) runs on EVERY call (after every Save, every reload); golden FormShow's own two texts (:1753 edShtCheckRange, :1956-1961 InSHZDownRange) only when formShowOpen = a real window open (FileRW_Teach_Page's first-open edge :256). Before, every Save (:195) and reload (:258) wrote the stale runtime CHECK_RANGE back into edShtCheckRange (the Save writes only Gerneral.ini, :190; CHECK_RANGE is re-read only by ReadTechData, cinitial.cpp:16139) and the next Save wrote that old value back to Gerneral.ini; golden keeps the typed text until the form is reopened (btnSaveClick :2283 ReadFile, no FormShow)
{
    if (!fTeach) return;
    const std::set<std::string> el = ElTeachNames();
    for (std::size_t i = 0; i < fTeach->TechPara.size(); ++i) {
        TECH_PARA* t = fTeach->TechPara[i];
        if (!t || !t->Parameter) continue;
        const char* w = WidgetOfKey(t->Key);
        if (el.count(w)) continue;
        P(w)->Text = AnsiString(*t->Parameter);
    }
    for (std::size_t i = 0; i < fTeach->TechTwoPara.size(); ++i) {
        TECH_TWOPARA* t = fTeach->TechTwoPara[i];
        if (!t) continue;
        for (int j = 0; j < 2; ++j) {
            if (!t->Parameter[j]) continue;
            const char* w = WidgetOfKey(t->Key[j]);
            if (el.count(w)) continue;
            P(w)->Text = AnsiString(*t->Parameter[j]);
        }
    }
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j) {
            if (fTeach->TechSuckPara[0].Parameter[i][j]) P(kTeInArm[i][j])->Text  = AnsiString(*fTeach->TechSuckPara[0].Parameter[i][j]);
            if (fTeach->TechSuckPara[1].Parameter[i][j]) P(kTeOutArm[i][j])->Text = AnsiString(*fTeach->TechSuckPara[1].Parameter[i][j]);
        }
    if (USE_OUT_SORT_ARM != eartUninstall) {                                    // golden :367-380（RogerYang 20250416）
        if (fTeach->TechSuckPara[2].Parameter[0][0]) P(kTeSortArm[0])->Text = AnsiString(*fTeach->TechSuckPara[2].Parameter[0][0]);
        if (fTeach->TechSuckPara[2].Parameter[0][1]) P(kTeSortArm[1])->Text = AnsiString(*fTeach->TechSuckPara[2].Parameter[0][1]);
    }
    P("edtEditRotateInBacklash")->Text  = AnsiString(Tech.M_In_iRotateA_Backlash);   // golden ReadFile :4870-4871（RogerYang 20260113）
    P("edtEditRotateOutBacklash")->Text = AnsiString(Tech.M_Out_iRotateA_Backlash);
    if (!formShowOpen) return;                                                  // ↓ golden FormShow 才寫、ReadFile 不碰的兩個元件：只在真的開窗時（見 :84）
    P("edShtCheckRange")->Text = AnsiString(CHECK_RANGE);                       // golden FormShow :1753（Steven 20160108 : 改去Teaching調整）
    if (In_Shuttle_Auto_Latch == eInSHAutoLtc)                                  // golden :1956（KenHsieh 20250722）
        P("InSHZDownRange")->Text = AnsiString(iInShtZRange);                   // golden :1961（不成立時保持原字：之前打的、上次開窗填的或 .dfm 預設空字串 —— golden 同）
}

// golden TfTeach::UpdateTempTech（uteach.cpp:2234-2259）：元件 → 變數
void IC_UpdateTempTech()
{
    for (std::size_t i = 0; i < fTeach->TechPara.size(); ++i) {
        TECH_PARA* t = fTeach->TechPara[i];
        if (t && t->Parameter) *t->Parameter = atoi(P(WidgetOfKey(t->Key))->Text.c_str());
    }
    for (std::size_t i = 0; i < fTeach->TechTwoPara.size(); ++i) {
        TECH_TWOPARA* t = fTeach->TechTwoPara[i];
        if (!t) continue;
        for (int j = 0; j < 2; ++j)
            if (t->Parameter[j]) *t->Parameter[j] = atoi(P(WidgetOfKey(t->Key[j]))->Text.c_str());
    }
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j) {
            if (fTeach->TechSuckPara[0].Parameter[i][j]) *fTeach->TechSuckPara[0].Parameter[i][j] = atoi(P(kTeInArm[i][j])->Text.c_str());
            if (fTeach->TechSuckPara[1].Parameter[i][j]) *fTeach->TechSuckPara[1].Parameter[i][j] = atoi(P(kTeOutArm[i][j])->Text.c_str());
        }
    if (USE_OUT_SORT_ARM != eartUninstall) {                                    // RogerYang 20250416 for HT9046AU add
        if (fTeach->TechSuckPara[2].Parameter[0][0]) *fTeach->TechSuckPara[2].Parameter[0][0] = atoi(P(kTeSortArm[0])->Text.c_str());
        if (fTeach->TechSuckPara[2].Parameter[0][1]) *fTeach->TechSuckPara[2].Parameter[0][1] = atoi(P(kTeSortArm[1])->Text.c_str());
    }
}

// golden TfTeach::SaveFile（uteach.cpp:4924-4949）
void IC_SaveFile(bool bSaveByTeach)
{
    // golden 這裡無條件 WriteData(tech.dat, &Tech.iZLoad, sizeof(TECH))。⚠ 不寫，待使用者裁決（NIGHT_REPORT §0 第 6 件）：
    //   NB2 R24（tools/nb2_assist/struct_layout_across_trees.py）量到 TECH 的版面**依版本不同**（不是對齊 —— 20260925 我原本寫錯）：
    //     V899 = 3792 bytes（這台的 tech.dat 就是它寫的）；golden 906／移植 = 3872（V899 ＋ 結尾 20 個 SortArm int ⇒ 對 V899 是前綴相容）；
    //     V912 = 3872 但把 M_In/Out_iRotateA_Backlash 從 byte 724 搬到結尾 ⇒ 大小相同、第 161 個欄位起錯位 8 bytes。
    //   量產 exe 只在 teach.ini 缺 [Teach INI] Update2 時讀 tech.dat；照 golden 906 版面寫，V899 讀得對、V912 會讀錯位。
    //   原本「檔案大小相同才寫」兩邊都錯（V899 的 3792 安全卻不寫；V912 的 3872 不安全卻會寫）⇒ 改成一律不寫並回報。
    filerw::ELTodo(("SaveFile :4926 tech.dat not written: TECH layout differs by version (V899 3792 / 906 3872 / V912 3872 with 2 fields moved), port sizeof(TECH)=" +
                    std::to_string(sizeof(TECH)) + " -- awaiting ruling; teach.ini is written").c_str());
    for (std::size_t i = 0; i < fTeach->TechPara.size(); ++i)
        if (fTeach->TechPara[i]) fTeach->TechPara[i]->SaveToFile(bSaveByTeach);
    for (std::size_t i = 0; i < fTeach->TechTwoPara.size(); ++i)
        if (fTeach->TechTwoPara[i]) fTeach->TechTwoPara[i]->SaveToFile(bSaveByTeach);
    fTeach->TechSuckPara[0].SaveToFile(bSaveByTeach);                          // Steven 20240523 : Teach SUCKPARA改存成ini
    fTeach->TechSuckPara[1].SaveToFile(bSaveByTeach);
    if (USE_OUT_SORT_ARM != eartUninstall)                                      // RogerYang 20250416 for HT9046AU add
        fTeach->TechSuckPara[2].SaveToFile(bSaveByTeach);
    AnsiString FileName = ExtractFileName(asTeachPath);
    AnsiString FilePath = ExtractFilePath(asTeachPath);
    elTeach->SaveEditTextToFile(FilePath, FileName);                            // JerryYang 20241119 : fix AOA
    Tech.M_In_iRotateA_Backlash  = atoi(P("edtEditRotateInBacklash")->Text.c_str());   // RogerYang 20260113 : Rotator新增背隙補償
    Tech.M_Out_iRotateA_Backlash = atoi(P("edtEditRotateOutBacklash")->Text.c_str());
    WriteIniData(asTeachPath, "MInRotate", "edtEditRotateInBacklash", Tech.M_In_iRotateA_Backlash);
    WriteIniData(asTeachPath, "MOutRotate", "edtEditRotateOutBacklash", Tech.M_Out_iRotateA_Backlash);
    filerw::ELMark("SaveFile");
}

// golden TfTeach::btnSaveClick（uteach.cpp:2261-2389）
void IC_btnSaveClick()
{
    int ret;
    if (fAllMotorHome == false)
    {
        ret = filerw::ELAsk(kSaveQuestion, "確定要存檔?");                      // golden MessageDlg(mbYes|mbNo)
        if (ret != 1)                                                           // golden: if(ret==mrNo) return;
            return;
    }
    IC_UpdateTempTech();
    if ((CUSTOMER_CODE == CC_ASE_KaohSiung ||
         CUSTOMER_CODE == CC_ASE_KaohSiung_K12) &&
        Tech.iTestZDown < -1000)                                                // kevin 20190930 add 保護
        Tech.iTestZDown = -1000;
    WriteIniDataGeneral("Shuttle", "CHECK_RANGE", atoi(P("edShtCheckRange")->Text.c_str()));   // Steven 20160108 : 改去Teaching調整
    WriteIniDataGeneral("Shuttle", "iInShtZRange", atoi(P("InSHZDownRange")->Text.c_str()));   // KenHsieh 20251107（元件沒被 FormShow 填時寫 0 —— golden 同）
    iInShtZRange = CheckAndReadIniDataGeneral("Shuttle", "iInShtZRange", 250);                 // KenHsieh 20251223
    IC_SaveFile(false);                                                         // Steven 20240501 : Teach改存成ini
    fTeach->ReadFile();
    MirrorToProxies(false);                                                     // golden ReadFile 的元件鏡像（移植樹 SetEdit 是 NULL）  //AI(W906-KB-GOLDEN) 20261004: false = the ReadFile half only -- golden btnSaveClick :2283 ReadFile() leaves edShtCheckRange / InSHZDownRange as typed (only FormShow :1753 / :1961 writes them); this line used to put the stale runtime CHECK_RANGE back on the proxy (:190 wrote Gerneral.ini only)
    InitShuttleThreadParameter();                                               // Steven 20120921 : Teach做完要重新Init一次
    fAllMotorHome = false;
    if (IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange == true)
        filerw::ELTodo("btnSaveClick :2283-2352 IniConfig.bD63 Index Y 尋相檢查（SetIndexYPhasePosition／lblFindPhaseNotes 未移植）");
    if (USE_Scanner_AOI_Inspection == (int)eBtnAOI_TopBottomInstall)
        filerw::ELTodo("btnSaveClick :2354-2385 AOI 上下檢查座標抄到 FrmAOI（FrmAOI 未移植）");
}

void EnsureInit()
{
    if (g_teachInit) return;
    if (!elTeach) elTeach = new HTEditList;                                     // golden main.cpp:1532-1543（IniConfig 開機通常已建）
    IC_InitialTeachEditList();                                                  // golden TfTeach 建構子
    g_teachInit = true;
}

struct Row { std::string id, group, key, text; };

Row MkRow(const char* id, const std::string& group, const std::string& key)
{
    Row r;
    r.id = id; r.group = group; r.key = key; r.text = P(id)->Text.c_str();
    return r;
}

// TECH_PARA 的 teach.ini 區段 = MOT[MotorSelect].Alias（MInShutte1／MInShuttle1 兩種拼法見 TECH_PARA::ReadFromFile）
std::string MotorGroup(int m)
{
    return (m >= 0 && m < MAX_TRAY_MOTOR) ? std::string(MOT[m].Alias.c_str()) : std::string();
}

void EntryList(webbridge::JsonWriter& w, const char* name, const std::vector<Row>& rows)
{
    w.Key(name).BeginObject();
    w.Key("entries").BeginArray();
    for (std::size_t i = 0; i < rows.size(); ++i) {
        w.BeginObject();
        w.Key("id").String(rows[i].id);
        w.Key("group").String(rows[i].group);
        w.Key("key").String(rows[i].key);
        w.Key("text").String(rows[i].text);
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
}

}  // namespace

// 開機：golden TfTeach 建構子在 CreateForm 時就註冊 elTeach —— 開機的 ReadTechData→fTeach->ReadFile 才讀得到 Teach.*
//   與 AOA 那幾組。wb_serve 在 FileRW_IniConfig_Boot 之後、InitialHandler 之前呼叫。
void FileRW_Teach_Boot()
{
    EnsureInit();
}

// WS editlist.get tag=Teach（主迴圈，呼叫端持 FormLock）：golden FormShow 的資料部分。
int FileRW_Teach_Page(std::string* json)
{
    EnsureInit();
    if (!fTeach) { *json = "fTeach is not constructed in this process"; return 409; }  const bool teOpen = filerw::OpenEnterRecord("Teach", g_formShown);   //AI(W906-FRW-S165) 20260927 [W906]: golden sbTeachingClick V912 main.cpp:28845 NewRecordProcess("MES2189", "Enter Teach Form") 在 :28846 fTeach->ShowModal()（→ FormShow → 下一行 ReadFile）之前；g_formShown 到 :259 才設 true（之後不再變回 false）⇒ 存檔後的自動重讀、重讀鈕都不再記（表與規則在 FileRW/_EditPage.cpp 檔尾）；接在同一行，:259（J1）行號不動
    fTeach->ReadFile();                                                         // golden FormShow → ReadFile（elTeach 讀檔寫進替身）
    MirrorToProxies(teOpen);                                                    //AI(W906-KB-GOLDEN) 20261004: golden FormShow's own texts (:1753 edShtCheckRange, :1956-1961 InSHZDownRange) only on a real window open = the edge OpenEnterRecord returned at :256 (true = it logged golden sbTeachingClick's "Enter Teach Form": the first editlist.get after the page table's close edge FileRW/_EditPage.cpp W906_EditPageWindowClosed("fTeach"); without the edges armed (ctest, wb_publish) only the first open of the process); the Save's automatic reload and the reload button get the ReadFile half only, so a typed and saved CHECK_RANGE stays on the page as in golden
    g_formShown = true;  if (SystemStart == false) fAllMotorHome = false;      // AI(W906-W5-b) 20260925: 覆核 W5B-R3 golden FormShow uteach.cpp:1606（進教導頁就清 ⇒ 下一次 START 先全部歸零）  AI(W906-FLOW-1) 20260927（St01 J1）：golden 只能從 sbTeachingClick 開 Teach，它第一行 if(SystemStart) return;（906 main.cpp:27827-27828）⇒ 運轉中永遠不會清；移植樹的 Teach iframe 開站就載入（web/background.html:460／:906），運轉中重新整理會清掉旗標、DoAllProcess 每拍 return（csystem.cpp:1629）而且沒有警報

    std::vector<Row> tp, tw, ts, gen;
    const std::set<std::string> el = ElTeachNames();
    for (std::size_t i = 0; i < fTeach->TechPara.size(); ++i) {
        TECH_PARA* t = fTeach->TechPara[i];
        if (!t) continue;
        const char* w = WidgetOfKey(t->Key);
        if (el.count(w)) continue;                                              // elTeach 那一份清單會帶
        tp.push_back(MkRow(w, MotorGroup(t->MotorSelect), t->Key.c_str()));
    }
    for (std::size_t i = 0; i < fTeach->TechTwoPara.size(); ++i) {
        TECH_TWOPARA* t = fTeach->TechTwoPara[i];
        if (!t) continue;
        for (int j = 0; j < 2; ++j) {
            const char* w = WidgetOfKey(t->Key[j]);
            if (el.count(w)) continue;
            tw.push_back(MkRow(w, MotorGroup(t->MotorSelect[j]), t->Key[j].c_str()));
        }
    }
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j) {
            ts.push_back(MkRow(kTeInArm[i][j], "InArmZSub", fTeach->TechSuckPara[0].Key[i][j].c_str()));
            ts.push_back(MkRow(kTeOutArm[i][j], "OutArmZSub", fTeach->TechSuckPara[1].Key[i][j].c_str()));
        }
    if (USE_OUT_SORT_ARM != eartUninstall)
        for (int j = 0; j < 2; ++j)
            ts.push_back(MkRow(kTeSortArm[j], "SortArmZSub", fTeach->TechSuckPara[2].Key[0][j].c_str()));
    gen.push_back(MkRow("edShtCheckRange", "Gerneral.ini/Shuttle", "CHECK_RANGE"));
    gen.push_back(MkRow("InSHZDownRange", "Gerneral.ini/Shuttle", "iInShtZRange"));
    gen.push_back(MkRow("edtEditRotateInBacklash", "MInRotate", "edtEditRotateInBacklash"));
    gen.push_back(MkRow("edtEditRotateOutBacklash", "MOutRotate", "edtEditRotateOutBacklash"));

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String("Teach");
    w.Key("form").String(kForm);
    w.Key("lists").BeginObject();
    EntryList(w, "techPara", tp);
    EntryList(w, "techTwoPara", tw);
    EntryList(w, "techSuckPara", ts);
    EntryList(w, "general", gen);
    w.Key("elTeach").RawValue(filerw::EditListToJson(elTeach));
    w.EndObject();
    w.Key("proxies").BeginObject().EndObject();
    w.Key("mustSend").BeginArray().EndArray();       // 沒送的元件沿用開頁時從檔案鏡像的值（開頁是存檔的前提）
    w.EndObject();
    *json = w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
    return 200;
}

// WS editlist.save tag=Teach（主迴圈，呼叫端持 FormLock）：golden btnSaveClick。
int FileRW_Teach_Save(const std::string& widgetsJson, const std::string& answersJson, std::string* ack, std::string* err)
{
    EnsureInit();
    if (!fTeach) { *err = "fTeach is not constructed in this process"; return 409; }
    // 沒開過頁 ⇒ 替身不是檔案的值（新建的替身是空字串，UpdateTempTech 會把 0 寫進每個教導點）⇒ 一律拒絕
    if (!g_formShown) { *err = "reload page (editlist.get tag=Teach) before save"; return 409; }
    std::vector<std::string> applied, unknown;  const AnsiString shtWas = P("edShtCheckRange")->Text, zWas = P("InSHZDownRange")->Text;   //AI(W906-KB-GOLDEN) 20261004: the two FormShow-only texts before the page's values are applied -- the NO path below puts them back (ReadFile does not re-read them)
    if (!filerw::ELApplyProxies(kForm, widgetsJson, &applied, &unknown, err)) return 400;
    filerw::SessionBegin(answersJson);
    IC_btnSaveClick();
    const bool saved = filerw::ELMarked("SaveFile");
    if (!saved) { fTeach->ReadFile(); MirrorToProxies(false); P("edShtCheckRange")->Text = shtWas; P("InSHZDownRange")->Text = zWas; }   // 沒存（答 NO）：替身回到檔案的值  //AI(W906-KB-GOLDEN) 20261004: the ReadFile half reverts TECH_* / elTeach; edShtCheckRange / InSHZDownRange go back to their text before this save (was: the runtime CHECK_RANGE, stale after an earlier Save in the same window-open); golden NO returns before touching any edit (uteach.cpp:2268-2270)
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String("Teach");
    w.Key("saved").Bool(saved);
    w.Key("applied").Number((wb_int64)applied.size());
    w.Key("unknown").BeginArray();
    for (std::size_t i = 0; i < unknown.size(); ++i) w.String(unknown[i]);
    w.EndArray();
    w.Key("session").RawValue(filerw::SessionJson());
    w.EndObject();
    *ack = w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
    return 200;
}
// Steven 團隊 20260926（S12-C ShuttleMove）：golden 其他表單呼叫的 fTeach->SaveFile(bSaveByTeach)（uteach.cpp:4924-4949）。
//   呼叫端：FileRW/ShuttleMove.gen.inc（golden TfShuttleMove::sbUpdateClick ShuttleMove.cpp:1991 fTeach->SaveFile(true)）。
//   bSaveByTeach==true：golden TECH_PARA／TECH_TWOPARA／TECH_SUCKPARA::SaveToFile 先 SetEdit->Text=*Parameter（uteach.cpp:146-147、
//   :197-200、:247）再寫檔 —— 表單已經把新值寫進 Tech.*，要存的是變數，不是 Teach 頁元件上的舊字。移植樹 SaveToFile 本來就寫
//   *Parameter（裁決 A4=(b)），這裡把「元件＝變數」補到具名替身上（每一筆都補，含 elTeach 也登記的元件：golden 同一個元件先被
//   SaveToFile 設成變數值、再由 elTeach->SaveEditTextToFile 寫出）。背隙兩鍵：golden SaveFile :4944-4945 讀 Teach 頁元件，golden 元件
//   開機由 ReadFile :4870-4871 填成 Tech 值；移植樹 Teach 頁沒開過時替身是空字串（atoi＝0 會把背隙寫成 0）→ 先照 Tech 補上。
void FileRW_Teach_SaveFile(bool bSaveByTeach)
{
    EnsureInit();
    if (!fTeach) { filerw::ELTodo("fTeach->SaveFile: fTeach is not constructed in this process -- teach.ini not written"); return; }
    if (bSaveByTeach) {
        for (std::size_t i = 0; i < fTeach->TechPara.size(); ++i) {
            TECH_PARA* t = fTeach->TechPara[i];
            if (t && t->Parameter) P(WidgetOfKey(t->Key))->Text = AnsiString(*t->Parameter);
        }
        for (std::size_t i = 0; i < fTeach->TechTwoPara.size(); ++i) {
            TECH_TWOPARA* t = fTeach->TechTwoPara[i];
            if (!t) continue;
            for (int j = 0; j < 2; ++j)
                if (t->Parameter[j]) P(WidgetOfKey(t->Key[j]))->Text = AnsiString(*t->Parameter[j]);
        }
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 8; ++j) {
                if (fTeach->TechSuckPara[0].Parameter[i][j]) P(kTeInArm[i][j])->Text  = AnsiString(*fTeach->TechSuckPara[0].Parameter[i][j]);
                if (fTeach->TechSuckPara[1].Parameter[i][j]) P(kTeOutArm[i][j])->Text = AnsiString(*fTeach->TechSuckPara[1].Parameter[i][j]);
            }
        if (USE_OUT_SORT_ARM != eartUninstall) {
            if (fTeach->TechSuckPara[2].Parameter[0][0]) P(kTeSortArm[0])->Text = AnsiString(*fTeach->TechSuckPara[2].Parameter[0][0]);
            if (fTeach->TechSuckPara[2].Parameter[0][1]) P(kTeSortArm[1])->Text = AnsiString(*fTeach->TechSuckPara[2].Parameter[0][1]);
        }
        P("edtEditRotateInBacklash")->Text  = AnsiString(Tech.M_In_iRotateA_Backlash);    // golden ReadFile :4870
        P("edtEditRotateOutBacklash")->Text = AnsiString(Tech.M_Out_iRotateA_Backlash);   // golden ReadFile :4871
    }
    IC_SaveFile(bSaveByTeach);
}

//AI(W906-GEARRATIO) 20261002: the Motor Test Gear Ratio tab (RULINGS_20261002 #22; GearRatioLive.cpp) -- golden ReadFile's
//  `SetEdit->Text=(int)(*Parameter)` mirror (MirrorToProxies above) after its own write + fTeach->ReadFile(), as IC_btnSaveClick
//  :195 does: a later Teach save that does not send a widget then keeps the variable, not a stale proxy text.
void FileRW_Teach_MirrorProxies()
{
    EnsureInit();
    if (fTeach) MirrorToProxies(false);   //AI(W906-KB-GOLDEN) 20261004: the ReadFile half only (as IC_btnSaveClick :195) -- a Gear Ratio save is not a Teach window open; golden ReadFile never writes edShtCheckRange / InSHZDownRange (FormShow :1753 / :1961 does)
}
