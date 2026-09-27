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

    // 1. 套用
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
