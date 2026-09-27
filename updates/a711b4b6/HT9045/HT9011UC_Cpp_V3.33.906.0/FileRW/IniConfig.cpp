// ===========================================================================
//  FileRW/IniConfig.cpp -- 結構 IniConfig 的讀寫檔（C 類 HTEditList）。
//
//  Steven 20260924.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md 一之二、
//  decisions.md 二之三。
//
//  檔案：config.ini（elConfig）、LastSet.ini（cbLastSet）、configByRecipe.ini（elConfig_byRecipe）。
//  golden 的註冊碼與讀寫流程在 IniConfig.gen.inc（tools/gen_editlist.py 由 golden 原檔產生，元件改成
//  具名替身 EL<T>("名稱")）；本檔只放開機順序與 JSON 入口。
//
//  開機順序照 golden（使用者 20260924：「參考 BCB 的，不要看 CPP」）：
//    1. TfMain 建構子 main.cpp:1532-1543：new elLaser／elConfig／elContact／elUdUld／elTrayForm／elTeach／
//       elConfig_byRecipe／cbLastSet／elVacuumUnit；elTrayForm->iBarcodeReadType；elConfig_byRecipe->SetFontBlue()
//    2. HT9045.cpp 的 CreateForm 順序：TfLd_ULd（:191，註冊 elUdUld）在 TfConfiguration（:207）之前
//       —— HTEditList 同 (區段,鍵) 第一筆生效，順序不能反。
//    3. TfConfiguration 建構子 cConfiguration.cpp:114-117：ReadLockByFile → InitConfigEdtList
//       （→ ChangeCBListProperty）→ ReadConfigStandard → ReadLastSetIni。
//       :117 的 ReadLastSetIni 就在本檔 FileRW_IniConfig_Boot 尾端呼叫 —— 這是 golden 第一次「有效」讀
//       config.ini（之前 HSys 靜態建構子那次 elConfig 還是 NULL），必須在 InitialHandler（golden FormShow
//       main.cpp:9990）之前，否則 InitialHandler／SetWorkParameter 用到的 IniConfig 全是預設值
//       （Isaac 20191105 把 ReadLastSetIni 搬進建構子就是為了這個；審查 20260924 H1）。
//       FormShow :9995 那次仍由 wb_serve 既有的開機序列呼叫。
//       cprod.cpp 裡讀 cbLastSet／elConfig／elConfig_byRecipe 的 GATE GA1-B2 已同日退役。
// ===========================================================================
#include "FileRW/IniConfig.gen.inc"

#include <algorithm>
#include <cstdio>

#include "Public/cJSON.h"
#include "cmydef.h"                 // AccessLevel
#include "WebBridge/JsonWriter.h"

void ReadLastSetIni();             // cprod.cpp（golden cprod.cpp；cprod.h:3274）
extern void W906_LdUldInitOnce();   // cSpeed.cpp：golden TfLd_ULd 建構子那段註冊（只做一次）


namespace {
bool g_booted = false;
// 審查 H-A／M-B：golden 的 Enabled（權限）是開頁 FormShow 時依當下 AccessLevel 算的；存檔必須在同一個
// AccessLevel 下開過頁（FormShow）之後，否則 409 要頁面重開。
bool g_formShown = false;
int  g_formShownLevel = -1;

// ---------------------------------------------------------------------------
// //AI(W906-FRW-Q19) 20260927（Steven 團隊 St01）：Steven 20260927 Q19（RULINGS_20260926 S140）「P26 OCR Tray Lot 可以補」。
//   golden（HT9011UC_Code_V3.33.912.0_20260908_Jimmy cConfiguration.cpp，wei 20151117 OCR Lot check）：
//     建構子 :206-224  for(i<10) new TEdit(gbP26_OCRCheck)（Name "OCRTrayLot"+i、Left 29+(i%5)*75、Top 37+(i/5)*25、Width 45、
//                      OnClick=edOCRTrayLotChange、Tag=i）＋ new TLabel（Name "labOCRTrayLot"+i、Left 4+(i%5)*75、Top 41+(i/5)*25、
//                      Width 19、Caption "<i+1>："）；
//     FormShow :5289-5294  gbP26_OCRCheck->Visible=CosFunction.bTrayOCR；edOCRTrayLot[i]->Text=LastSet.TrayCount[i]、Tag=i；
//     edOCRTrayLotChange :5522-5531  小鍵盤（N_INTEGER 0～20）→ LastSet.TrayCount[Tag]=atoi(Text) → WriteLastDataFile()（lastdata.dat，
//                      按下就寫，與「Config data save to define?」無關）。
//   移植樹：FormShow／edOCRTrayLotChange 由產生器轉（tools/editlist/IniConfig.py）；建構子那段在這裡（下面），
//   版面放 editlist.get 的 extra.ocrTrayLot（頁面 web/page/ht9045_iniconfig_p26_c.js 照著建元件），
//   事件在 FileRW_IniConfig_Save 重播（頁面值與開頁時不同＝使用者改過；在 golden FormClose 之前，同 golden 表單開著時就寫）。
// ---------------------------------------------------------------------------
const int kOCRTrayLotCount = 10;   // golden cConfiguration.h:2380 TEdit *edOCRTrayLot[10]
AnsiString OCRTrayLotName(int i, bool bLabel)                                    // golden :209／:218 Name
{
    return AnsiString(bLabel ? "labOCRTrayLot" : "OCRTrayLot") + AnsiString(i);
}
TEdit* OCRTrayLotEdit(int i) { return filerw::EL<TEdit>("TfConfiguration", OCRTrayLotName(i, false).c_str()); }

// golden 建構子 :206-224 的 C 路版（具名替身；Width 只給頁面，vclcompat TControl 沒有 Width）
void CreateOCRTrayLotProxies()
{
    for (int i = 0; i < kOCRTrayLotCount; i++) {
        const AnsiString asEd = OCRTrayLotName(i, false);
        const AnsiString asLb = OCRTrayLotName(i, true);
        TEdit*  ed = filerw::EL<TEdit>("TfConfiguration", asEd.c_str());      // golden :208 new TEdit(gbP26_OCRCheck)
        TLabel* lb = filerw::EL<TLabel>("TfConfiguration", asLb.c_str());     // golden :217 new TLabel(gbP26_OCRCheck)
        {                                                                      // golden :210／:219 Parent=gbP26_OCRCheck
            const char* const pr[2][2] = { { asEd.c_str(), "gbP26_OCRCheck" }, { asLb.c_str(), "gbP26_OCRCheck" } };
            filerw::ELSetParents("TfConfiguration", pr, 2);
        }
        ed->Left = 29 + (i % 5) * 75;                                          // golden :211
        ed->Top  = 37 + (i / 5) * 25;                                          // golden :212
        ed->Tag  = i;                                                          // golden :215（:214 OnClick → 存檔時重播）
        lb->Left = 4 + (i % 5) * 75;                                           // golden :220
        lb->Top  = 41 + (i / 5) * 25;                                          // golden :221
        lb->Caption = AnsiString(i + 1) + "：";                                // golden :223
    }
}
}

// golden 開機：main.cpp:1532-1543 ＋ CreateForm 順序 ＋ TfConfiguration 建構子的註冊段
void FileRW_IniConfig_Boot()
{
    if (g_booted) return;
    // main.cpp:1532-1543（移植樹其他地方已經建立的就沿用，不重建 —— cLd_ULd／cTrayForm 會自己 new）
    if (!elLaser)           elLaser           = new HTEditList;
    if (!elConfig)          elConfig          = new HTEditList;
    if (!elContact)         elContact         = new HTEditList;
    if (!elUdUld)           elUdUld           = new HTEditList;
    if (!elTrayForm)        elTrayForm        = new HTEditList;
    if (!elTeach)           elTeach           = new HTEditList;
    if (!elConfig_byRecipe) elConfig_byRecipe = new HTEditList;                 //Sam 20220921 : config儲存跟隨recipe
    elTrayForm->iBarcodeReadType = (int)bcTrayForm;
    if (!cbLastSet)         cbLastSet         = new HTEditList;
    if (!elVacuumUnit)      elVacuumUnit      = new HTEditList;                 //Sam 20230210 : 新增 VacuumUnit 通訊模組
    elConfig_byRecipe->SetFontBlue();                                           //Sam 20220921 : config儲存跟隨recipe

    // golden cConfiguration.dfm 的設計期 Items（TfConfiguration 建構時 VCL 從 DFM 載入，早於建構子本體）
    IC_DfmItems();
    IC_DfmState();   // 審查 #2：DFM 設計期 Enabled=False／ReadOnly=True／Visible=False

    // HT9045.cpp:191 TfLd_ULd 在 :207 TfConfiguration 之前建立（elUdUld 的註冊順序）
    W906_LdUldInitOnce();

    // cConfiguration.cpp:112-113 TfConfiguration 建構子
    IC_ReadLockByFile();
    IC_InitConfigEdtList();
    IC_CreateSaveProxies();   // 審查 C1：存檔流程讀的替身先建好（見 IniConfig.gen.inc）
    IC_CreateContainerProxies();   // 審查 H-A：權限判斷要的容器替身與 DFM 父子表
    CreateOCRTrayLotProxies();     // //AI(W906-FRW-Q19) 20260927: 建構子 :206-224 P26 OCR Tray Lot 10 格（見本檔上方）
    // :116 ReadConfigStandard()（開機時把 config_Standard.ini 複製成 config.ini，會寫檔）—— 尚未轉，列入待辦
    //      （只在 CosFunction.bConfigStandard 時有作用）
    ReadLastSetIni();   // :117 golden 第一次有效讀 config.ini／LastSet.ini／configByRecipe.ini（見檔頭 3.）
    std::printf("FileRW IniConfig: edit lists registered (elConfig=%d cbLastSet=%d elConfig_byRecipe=%d "
                "elUdUld=%d) -- golden main.cpp:1532-1543 + TfConfiguration ctor; ReadConfigStandard NOT run (todo)\n",
                elConfig->FEditList->Count, cbLastSet->FEditList->Count,
                elConfig_byRecipe->FEditList->Count, elUdUld->FEditList->Count);
    g_booted = true;
}

// GET /api/editlist/<name>：golden ReadEditTextFromFile＋InitialDataToEdit 之後的值（依 ControlName）
// ---------------------------------------------------------------------------
//  存檔：golden 關 TfConfiguration 表單就存（cConfiguration.cpp:5816 FormClose）。
//    1. 頁面值依名稱套進 TfConfiguration 的替身（先全部檢查再套，錯一個就整批不動）。
//    2. 存檔流程讀、但不在任何 HTEditList 裡的替身（kIC_SaveReads 裡那些，例 tbD25_*->Position、
//       dtO06_LastDate->Date —— golden 由 FormShow 填，移植樹還沒有這段）頁面沒送 → 拒存。
//       在 HTEditList 裡的替身頁面沒送 → 沿用（它的值就是開機／上次存檔讀進來的檔案值），ack.kept 列出。
//    3. 跑 golden FormClose（→ CheckConfigurationBeforeSave → SaveConfiguration → LoadConfiguration）。
//       golden 的「Config data save to define?」由頁面在 answers 答（1＝YES）；沒答＝NO＝不存。
//  呼叫端要持 FormLock。回 HTTP 式狀態碼；*ack 是 JSON。
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// //AI(W906-FRW-S158) 20260927 [W906] Q41 盤點 CC-L2～CC-L4（Steven 20260927 Q41，RULINGS_20260926 S158；盤點
//   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 3.19 節）：golden 要先輸入密碼、或重新登入到夠高
//   的等級才能改的勾選框。網頁版還沒有密碼框（盤點 C-3；做法待 Steven 決定 ★ Q45）→ 這幾個值跟開頁時（golden FormShow
//   填的＝檔案值）不同就**整次拒存**（fail-closed，同 FileRW/TestIF_File_SetUp 的 ELPasswordRefused 精神：網頁上打不開
//   需要密碼的設定）；沒改到這幾個的存檔照常。偏離 golden：golden 是點下去當場問、錯了改回再照存，這裡是存檔時整次拒絕
//   並說明原因（網頁沒有「點下去當場問」的入口，盤點 C-4／Q40 form.event）。
//   golden 出處全部是 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp（DFM 綁定 cConfiguration.dfm）：
//   CC-L2 :6775 cbC12Click（綁在 cbC12 dfm:3178、cbC13 :3161、chkC14 :3238、cbC17 :3533、cbC24 :3903）：點完之後 cbC12 是勾著的
//         才問密碼（:6779-6787 比對寫死的密碼），錯了把 **cbC12** 取消勾。golden 怪處照判：綁在 C13／C14／C17／C24 上也只看、
//         只改 cbC12 ⇒ C12 勾著的時候改那四個也要密碼（錯了是 C12 被取消，不是那一格被改回）。取消 C12 本身不問。
//         ⇒ 最後 C12 勾著，而且（C12 原本沒勾，或那四格有任何一格跟原值不同）＝golden 一定問過密碼 → 拒存；
//            最後 C12 沒勾 → golden 不用密碼也到得了（先取消 C12 再改其他格）→ 放行。
//   CC-L3 :6824 cbA27Click（dfm:1323）、:6860 cbN07_EnableEmployeeCheakClick（dfm:15080）：勾、取消都問（比對寫死的密碼），
//         錯了改回 → 值不同就拒存。（A27 另外改主畫面 Label2 的 "L"／"X"（:6848-6855），網頁主畫面沒有這個標記，不在這裡。）
//   CC-L4 :6531 cbM01Click（cbM01 與 cbM01_01～15 共 16 格，dfm:21162-21305）→ :6482 DoPassword：LevelSet.AccessLevel[92]==0
//         不檢查（:6488-6490）；REAL_TIME_CCD 才要重新登入、登入後等級 < AccessLevel[92] 算失敗（:6491-6516），失敗改回
//         （:6538-6541）；沒有 REAL_TIME_CCD 直接過。勾、取消都算。:6493 的 fInput->fShow 移植樹沒有 fInput 門面，照 golden
//         「沒開」處理。golden DoPassword 問完之後還會把使用者登出成 Operator（:6518-6525，有密碼本時）—— 網頁一律拒，沒有這一步。
//   不照翻 golden 的 InitialOK 守衛（:6777、:6827、:6863 `if(InitialOK==false) return;`＝程式還沒開完不問）：網頁存檔一定在
//   wb_serve 開機序列之後，但移植樹 wb_serve 沒有把 InitialOK 設成 true（golden V912 main.cpp:10900 那行沒翻，移植樹 cmydef.cpp:285
//   預設 false）—— 照翻會讓這道檢查永遠不生效，變得比 golden 寬鬆。
//   CC-L5 :7274 Image1DblClick（輸入 SG_PW.ini 的密碼才讓 grpA32_1 看得見）不用在這裡擋：移植樹 grpA32_1 一直是 Visible=false
//   （IniConfig.gen.inc 的 DFM 設計期值、golden InitConfigEdtList_ItemA :743），沒有任何程式把它打開 → 底下 cbA32_01～10 的
//   頁面值已經被「不可改的丟掉」那一段丟掉、沿用檔案值。
//   只看「不可改的丟掉」之後還留著的值：golden 停用／看不見的格子使用者點不到，也就不會問密碼。
//   不寫、不印任何密碼（RULINGS_20260926 S41／S55）。
// ---------------------------------------------------------------------------
namespace {
// 頁面送來、且沒被丟掉的勾選值；沒送或不是布林（ELApplyProxies 會整批拒）→ false
bool IC_PageChecked(const cJSON* root, const char* id, bool* v)
{
    const cJSON* it = cJSON_GetObjectItemCaseSensitive(root, id);
    const cJSON* c = (it && cJSON_IsObject(it)) ? cJSON_GetObjectItemCaseSensitive(it, "checked") : nullptr;
    if (!c || !cJSON_IsBool(c)) return false;
    *v = cJSON_IsTrue(c) != 0;
    return true;
}
bool IC_OpenChecked(const char* id)   // 開頁（golden FormShow）時的值＝替身現在的值（還沒套頁面值）
{
    TCheckBox* c = dynamic_cast<TCheckBox*>(filerw::ELFind("TfConfiguration", id));
    return c ? c->Checked : false;
}
bool IC_Changed(const cJSON* root, const char* id)
{
    bool v = false;
    return IC_PageChecked(root, id, &v) && v != IC_OpenChecked(id);
}
// 回空字串＝可以存；否則是拒存原因（中文，給操作員看；wb_serve 會照樣印在主控台：editlist.save IniConfig -> 400 …）
std::string IC_PasswordGuard(const std::string& filteredJson)
{
    cJSON* root = cJSON_Parse(filteredJson.c_str());
    if (!root) return std::string();                       // 格式錯由 ELApplyProxies 回
    std::vector<std::string> why;
    // CC-L2 golden :6775 cbC12Click
    {
        static const char* const kC12Group[] = {"cbC13", "chkC14", "cbC17", "cbC24"};   // 同一支處理器（dfm:3161／:3238／:3533／:3903）
        bool c12 = IC_OpenChecked("cbC12");
        IC_PageChecked(root, "cbC12", &c12);               // 沒送／被丟掉 → 沿用開頁值
        if (c12) {
            std::string others;
            for (std::size_t i = 0; i < sizeof(kC12Group) / sizeof(kC12Group[0]); ++i)
                if (IC_Changed(root, kC12Group[i])) others += std::string(others.empty() ? "" : "、") + kC12Group[i];
            if (!IC_OpenChecked("cbC12"))
                why.push_back("C12（PE 模式，cbC12）打勾要密碼（golden cConfiguration.cpp:6775 cbC12Click）");
            else if (!others.empty())
                why.push_back("C12 勾著的時候改 " + others + " 也要密碼（golden :6775 cbC12Click 綁在這幾格上、只看 C12；"
                              "密碼錯 golden 會把 C12 取消）");
        }
    }
    // CC-L3 golden :6824 cbA27Click、:6860 cbN07_EnableEmployeeCheakClick
    if (IC_Changed(root, "cbA27"))
        why.push_back("A27（cbA27）勾或取消都要密碼（golden :6824 cbA27Click）");
    if (IC_Changed(root, "cbN07_EnableEmployeeCheak"))
        why.push_back("N07-5（cbN07_EnableEmployeeCheak）勾或取消都要密碼（golden :6860 cbN07_EnableEmployeeCheakClick）");
    // CC-L4 golden :6531 cbM01Click → :6482 DoPassword（AccessLevel[92]==0 或沒有 REAL_TIME_CCD 時 golden 不問）
    if (LevelSet.AccessLevel[92] != 0 && REAL_TIME_CCD) {
        static const char* const kM01[] = {"cbM01", "cbM01_01", "cbM01_02", "cbM01_03", "cbM01_04", "cbM01_05", "cbM01_06",
                                           "cbM01_07", "cbM01_08", "cbM01_09", "cbM01_10", "cbM01_11", "cbM01_12", "cbM01_13",
                                           "cbM01_14", "cbM01_15"};   // dfm:21162-21305 OnClick = cbM01Click
        std::string m;
        for (std::size_t i = 0; i < sizeof(kM01) / sizeof(kM01[0]); ++i)
            if (IC_Changed(root, kM01[i])) m += std::string(m.empty() ? "" : "、") + kM01[i];
        if (!m.empty())
            why.push_back("M01 群組（" + m + "）要重新登入到等級 " + std::to_string((int)LevelSet.AccessLevel[92]) +
                          " 以上（LevelSet.AccessLevel[92]；golden :6531 cbM01Click → :6482 DoPassword）");
    }
    cJSON_Delete(root);
    if (why.empty()) return std::string();
    std::string s = "這次沒有存檔：下面這幾項在 golden 要先輸入密碼（或重新登入）才能改，網頁版還沒有密碼確認"
                    "（Q41 盤點 C-3，做法待 Steven 決定 ★Q45）。請把它們改回原值再存，其他設定可以照常存：";
    for (std::size_t i = 0; i < why.size(); ++i) s += "\n  " + std::to_string(i + 1) + ". " + why[i];
    return s;
}
}  // namespace

int FileRW_IniConfig_Save(const std::string& widgetsJson, const std::string& answersJson, std::string* ack,
                          std::string* err)
{
    if (!g_booted) { *err = "IniConfig edit lists are not booted"; return 409; }
    if (!g_formShown || g_formShownLevel != AccessLevel) {
        *err = "reload page: open the Configuration page (editlist.get IniConfig = golden FormShow) with the current "
               "access level before saving";
        return 409;
    }
    HTEditList* const lists[] = {elConfig, cbLastSet, elConfig_byRecipe, elUdUld};

    // 2. 必送檢查（先於套用）
    std::vector<std::string> need;
    {
        cJSON* root = cJSON_Parse(widgetsJson.c_str());
        for (std::size_t i = 0; i < sizeof(kIC_SaveReads) / sizeof(kIC_SaveReads[0]); ++i) {
            TControl* c = filerw::ELFind("TfConfiguration", kIC_SaveReads[i]);
            // 還沒建立的替身（只有存檔流程才第一次 EL<>）一定不在任何 HTEditList 裡 → 也是必送
            if (c && filerw::ELInAnyList(c, lists, 4)) continue;
            if (!root || !cJSON_GetObjectItemCaseSensitive(root, kIC_SaveReads[i])) need.push_back(kIC_SaveReads[i]);
        }
        if (root) cJSON_Delete(root);
    }
    if (!need.empty()) {
        *err = "refused: golden save reads these widgets, they are not in any HTEditList (golden FormShow fills them), "
               "and the page did not send them:";
        for (std::size_t i = 0; i < need.size(); ++i) *err += (i ? ", " : " ") + need[i];
        return 400;
    }

    // 審查 C1：必送的替身一定已經存在（開機 IC_CreateSaveProxies）；不在就是內部錯，不可存
    for (std::size_t i = 0; i < sizeof(kIC_SaveReads) / sizeof(kIC_SaveReads[0]); ++i)
        if (!filerw::ELFind("TfConfiguration", kIC_SaveReads[i])) {
            *err = std::string("internal: save proxy not created: ") + kIC_SaveReads[i];
            return 500;
        }

    // 審查 H-A：golden 停用的元件（自己或任一層容器 Enabled=false，或 HTEditList 筆 bEnable=false）使用者改不到，
    //   golden 存的是它原本的值 → 頁面送的值丟掉、沿用伺服器端的值，ack.ignored 列出。
    std::vector<std::string> ignored;
    std::string filtered;
    {
        cJSON* root = cJSON_Parse(widgetsJson.c_str());
        if (!root || !cJSON_IsObject(root)) {
            if (root) cJSON_Delete(root);
            *err = "widgets is not a JSON object";
            return 400;
        }
        for (cJSON* it = root->child; it;) {
            cJSON* next = it->next;
            bool drop = !filerw::ELEditable("TfConfiguration", it->string);
            if (!drop) {
                TControl* c = filerw::ELFind("TfConfiguration", it->string);
                for (int k = 0; k < 3 && c && !drop; ++k)
                    for (int i = 0; lists[k] && i < lists[k]->FEditList->Count; ++i) {
                        THTEdit* e = static_cast<THTEdit*>(lists[k]->FEditList->Items[i]);
                        if (e->SourceControl == c && !e->bEnable) { drop = true; break; }
                    }
            }
            if (drop) {
                ignored.push_back(it->string);
                cJSON_DeleteItemFromObjectCaseSensitive(root, it->string);
            }
            it = next;
        }
        char* s = cJSON_PrintUnformatted(root);
        filtered = s ? s : "{}";
        if (s) cJSON_free(s);
        cJSON_Delete(root);
    }

    // //AI(W906-FRW-S158) 20260927 [W906] Q41 CC-L2～CC-L4：golden 要密碼的勾選框改了 → 整次拒存（見上方 IC_PasswordGuard）。
    //   在套值之前：替身還是開頁值、什麼都沒動，不用還原；g_formShown 不動，操作員改回之後可以直接再存。
    {
        const std::string why = IC_PasswordGuard(filtered);
        if (!why.empty()) {
            *err = why;
            return 400;
        }
    }

    // 1. 套用
    AnsiString ocrOpen[kOCRTrayLotCount];   // //AI(W906-FRW-Q19) 20260927: 套值前的 P26 OCR Tray Lot（＝開頁 FormShow 填的 LastSet.TrayCount）
    for (int i = 0; i < kOCRTrayLotCount; ++i) ocrOpen[i] = OCRTrayLotEdit(i)->Text;
    std::vector<std::string> applied, unknown;
    if (!filerw::ELApplyProxies("TfConfiguration", filtered, &applied, &unknown, err)) return 400;
    for (std::size_t i = 0; i < unknown.size(); ++i)
        for (std::size_t j = 0; j < sizeof(kIC_SaveReads) / sizeof(kIC_SaveReads[0]); ++j)
            if (unknown[i] == kIC_SaveReads[j]) {   // 送了但型別不歸頁面管（例 TPanel）→ 值沒套上 → 不可存
                ReadLastSetIni();                    // 把已套的替身還原成檔案值
                *err = "refused: save reads " + unknown[i] + " but its value kind cannot be applied";
                return 400;
            }

    // 3. golden 存檔流程
    filerw::SessionBegin(answersJson);
    // //AI(W906-FRW-Q19) 20260927: golden edOCRTrayLotChange（:5522，P26 OCR Tray Lot 各格的 OnClick）在表單開著時就跑、按下即寫
    //   lastdata.dat —— 網頁送的是最後狀態：值和開頁時不同＝使用者改過 → 依格序重播（golden 小鍵盤夾 0～20 → LastSet.TrayCount[Tag]
    //   → WriteLastDataFile()），在 FormClose 之前，所以答 NO 也已經寫了（同 golden）。沒送／被丟掉（gbP26_OCRCheck 看不見＝
    //   CosFunction.bTrayOCR 關、或停用）的格子不重播。WriteLastDataFile 寫的是整份 LastSet（golden 同）；SaveConfiguration :8251 答 YES
    //   時會再寫一次同樣內容。沒存成時下面的 ReadLastSetIni → ReadLastDataFile 讀回的就是剛寫的檔。
    std::vector<std::string> events;
    for (int i = 0; i < kOCRTrayLotCount; ++i) {
        const std::string id = OCRTrayLotName(i, false).c_str();
        if (std::find(applied.begin(), applied.end(), id) == applied.end()) continue;
        TEdit* e = OCRTrayLotEdit(i);
        if (e->Text == ocrOpen[i]) continue;                         // golden 沒有 OnChange 的判斷；網頁只能從值看出「改過」
        filerw::ELMark(("edOCRTrayLotChange " + id + " (golden cConfiguration.cpp:5522, replayed before FormClose)").c_str());
        IC_edOCRTrayLotChange(e);
        events.push_back(id);
    }
    IC_FormClose();
    g_formShown = false;   // 審查 #6：golden FormClose 就是關頁；下一次存檔前要重新開頁（editlist.get）
    const bool saved = filerw::ELMarked("SaveConfiguration");
    // 審查 M3：沒存（答 NO、A01_2 權限…）時，golden 只把 elConfig 刷回（:5845），cbLastSet／elConfig_byRecipe
    // 的替身留著頁面值；golden 下次 FormShow 會 ReadLastSetIni（:4625）重讀，這裡沒有下次開頁 → 立刻重讀。
    if (!saved) ReadLastSetIni();

    // 沒送、沿用檔案值的 HTEditList 筆（只算 TfConfiguration 的三份；elUdUld 屬 TfLd_ULd）
    std::vector<std::string> kept;
    for (int k = 0; k < 3; ++k) {
        if (!lists[k]) continue;
        for (int i = 0; i < lists[k]->FEditList->Count; ++i) {
            THTEdit* it = static_cast<THTEdit*>(lists[k]->FEditList->Items[i]);
            if (it->ControlName.IsEmpty()) continue;
            if (std::find(applied.begin(), applied.end(), std::string(it->ControlName.c_str())) == applied.end() &&
                std::find(ignored.begin(), ignored.end(), std::string(it->ControlName.c_str())) == ignored.end() &&
                std::find(kept.begin(), kept.end(), std::string(it->ControlName.c_str())) == kept.end())
                kept.push_back(it->ControlName.c_str());
        }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String("IniConfig");
    w.Key("saved").Bool(saved);
    w.Key("applied").Number((wb_int64)applied.size());
    w.Key("ignored").BeginArray();
    for (std::size_t i = 0; i < ignored.size(); ++i) w.String(ignored[i]);
    w.EndArray();
    w.Key("kept").BeginArray();
    for (std::size_t i = 0; i < kept.size(); ++i) w.String(kept[i]);
    w.EndArray();
    w.Key("unknown").BeginArray();
    for (std::size_t i = 0; i < unknown.size(); ++i) w.String(unknown[i]);
    w.EndArray();
    w.Key("events").BeginArray();   // //AI(W906-FRW-Q19) 20260927: 存檔前重播了哪些 golden 事件（同 FileRW/_EditPage.cpp 的 ack.events）
    for (std::size_t i = 0; i < events.size(); ++i) w.String(events[i]);
    w.EndArray();
    w.Key("session").RawValue(filerw::SessionJson());
    w.EndObject();
    *ack = w.Str();
    return 200;
}

// golden cprod.cpp ReadLastSetIni：`if(fConfiguration!=NULL) fConfiguration->ChangeCBListProperty();`
// fConfiguration 在 golden 是 CreateForm 建構完才指派 → 本檔的 g_booted（開機序列跑完才設）
void FileRW_IniConfig_ChangeCBListProperty()
{
    if (g_booted) IC_ChangeCBListProperty();
}

// GET /api/editlist/IniConfig —— golden 開 TfConfiguration 頁面：FormShow（:4614，含 :4625 ReadLastSetIni 重讀檔、
// 依客戶碼／權限設 Visible／Enabled、把 LastSet／IniConfig 的值填進不在 HTEditList 裡的元件）。
// 回三份清單＋ TfConfiguration 全部替身的值與狀態。存檔（editlist.save）要送回的就是這些值。
// 呼叫端要持 FormLock（FormShow 會讀檔、改替身）。
static int IniConfigPageJson(std::string* json)
{
    IC_FormShow();
    g_formShown = true;
    g_formShownLevel = AccessLevel;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String("IniConfig");
    w.Key("form").String("TfConfiguration");
    w.Key("booted").Bool(g_booted);
    w.Key("lists").BeginObject();
    w.Key("elConfig").RawValue(filerw::EditListToJson(elConfig));
    w.Key("cbLastSet").RawValue(filerw::EditListToJson(cbLastSet));
    w.Key("elConfig_byRecipe").RawValue(filerw::EditListToJson(elConfig_byRecipe));
    w.EndObject();
    w.Key("proxies").RawValue(filerw::ProxyStateJson("TfConfiguration"));
    // 存檔時頁面必須送回的（不在任何清單裡、值只從 FormShow 來的）
    HTEditList* const lists[] = {elConfig, cbLastSet, elConfig_byRecipe, elUdUld};
    w.Key("mustSend").BeginArray();
    for (std::size_t i = 0; i < sizeof(kIC_SaveReads) / sizeof(kIC_SaveReads[0]); ++i) {
        TControl* c = filerw::ELFind("TfConfiguration", kIC_SaveReads[i]);
        if (!c || !filerw::ELInAnyList(c, lists, 4)) w.String(kIC_SaveReads[i]);
    }
    w.EndArray();
    // //AI(W906-FRW-Q19) 20260927: P26 OCR Tray Lot 10 格是 golden 建構子 :206-224 動態建的（DFM 沒有、頁面產生器畫不出來）→ 版面給頁面，
    //   web/page/ht9045_iniconfig_p26_c.js 照著在 gbP26_OCRCheck 裡建 Label／輸入框（id＝替身名稱），值／可見／可改走上面的 proxies。
    //   位置＝golden 建構子算的 Left／Top；Width 是 golden 常數（vclcompat TControl 沒有 Width）；kb＝golden :5526 ShowQwertyKey 的參數。純讀。
    w.Key("extra").BeginObject();
    w.Key("ocrTrayLot").BeginObject();
    w.Key("golden").String("V912 cConfiguration.cpp:206-224 ctor / :5289-5294 FormShow / :5522-5531 edOCRTrayLotChange "
                           "(LastSet.TrayCount[10] -> lastdata.dat, written when a box is changed)");
    w.Key("parent").String("gbP26_OCRCheck");
    w.Key("kb").BeginObject();
    w.Key("flag").String("INTEGER");
    w.Key("dp").Number((wb_int64)0);
    w.Key("checkRange").Bool(true);
    w.Key("min").Number((wb_int64)0);
    w.Key("max").Number((wb_int64)20);
    w.EndObject();
    w.Key("cells").BeginArray();
    for (int i = 0; i < kOCRTrayLotCount; ++i) {
        TEdit*  ed = OCRTrayLotEdit(i);
        TLabel* lb = filerw::EL<TLabel>("TfConfiguration", OCRTrayLotName(i, true).c_str());
        w.BeginObject();
        w.Key("ed").String(OCRTrayLotName(i, false).c_str());
        w.Key("lb").String(OCRTrayLotName(i, true).c_str());
        w.Key("left").Number((wb_int64)ed->Left);
        w.Key("top").Number((wb_int64)ed->Top);
        w.Key("width").Number((wb_int64)45);          // golden :213
        w.Key("lbLeft").Number((wb_int64)lb->Left);
        w.Key("lbTop").Number((wb_int64)lb->Top);
        w.Key("lbWidth").Number((wb_int64)19);        // golden :222
        w.Key("caption").String(lb->Caption.c_str());
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    w.EndObject();
    w.EndObject();
    *json = w.Str();
    return 200;
}

int FileRW_IniConfig_Json(const std::string& name, std::string* json)
{
    if (name == "IniConfig") {
        // 審查 C-A：golden FormShow 會 ReadLastSetIni → ReadLastDataFile 整塊覆蓋 LastSet、改 IniConfig／CosFunction；
        // HTTP 在 socket 執行緒，會與 MainProc 搶。只能從主迴圈的 WS 命令 editlist.get 跑。
        *json = "{\"error\":\"use WS editlist.get tag=IniConfig (golden FormShow must run on the main loop)\"}";
        return 405;
    }
    HTEditList* el = name == "elConfig" ? elConfig : name == "cbLastSet" ? cbLastSet
                   : name == "elConfig_byRecipe" ? elConfig_byRecipe : name == "elUdUld" ? elUdUld : nullptr;
    if (!el && name != "elConfig" && name != "cbLastSet" && name != "elConfig_byRecipe" && name != "elUdUld") {
        *json = "{\"error\":\"unknown edit list (IniConfig / elConfig / cbLastSet / elConfig_byRecipe / elUdUld)\"}";
        return 404;
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("list").String(name);
    w.Key("struct").String("IniConfig");
    w.Key("booted").Bool(g_booted);
    w.Key("data").RawValue(filerw::EditListToJson(el));
    // 審查 M-B：這裡沒跑 FormShow，FormShow 才填的替身值不可信 → 不給 proxies（開頁請用 editlist.get）
    w.Key("formShown").Bool(g_formShown);
    w.EndObject();
    *json = w.Str();
    return 200;
}

// WS editlist.get（主迴圈）：golden 開頁 = FormShow。呼叫端持 FormLock。
int FileRW_IniConfig_Page(std::string* json)
{
    if (!g_booted) { *json = "IniConfig edit lists are not booted"; return 409; }
    return IniConfigPageJson(json);
}
