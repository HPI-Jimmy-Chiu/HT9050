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
#include "FileRW/_EditPage.h"      // AI(W906-FRW-S165) 20260927 [W906]: filerw::OpenEnterRecord（開頁記 golden 的 "Enter ..."，RULINGS_20260926 S165＝R101）；佔用原本的空行
void ReadLastSetIni();             // cprod.cpp（golden cprod.cpp；cprod.h:3274）
extern void W906_LdUldInitOnce();   // cSpeed.cpp：golden TfLd_ULd 建構子那段註冊（只做一次）
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbConfigurationClickHead／Tail（FileRW/MainClick.cpp；Q41 第 3 項 CC-E15；只有宣告）；佔用原本的空行
#include "WebReauth.h"             //AI(W906-Q45-B5) 20260930: W906_ReauthHasAnswer／W906_ReauthControls／W906_ReauthConfigM01／W906_ReauthOpenJson（本體 WebLogin.cpp 檔尾）；佔用原本的空行
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
void IC_EvCreateProxies();   //AI(W906-EVB4) 20260928 [W906]：form.event 事件表上、產生器沒建替身的元件（定義在檔尾 (6)）
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
    IC_DfmState();  { void FileRW_IniConfig_EvB10BBoot(); FileRW_IniConfig_EvB10BBoot(); }   // 審查 #2：DFM 設計期 Enabled=False／ReadOnly=True／Visible=False  //AI(W906-EVB10B) 20260929 [W906]：HTEditList 勾選 hook（X-3，Add 之前；OnClick 登記在 IC_DfmState）＋PageControl1／pcConfig 頁序（CC-L1）；本體在檔尾；同一行

    // HT9045.cpp:191 TfLd_ULd 在 :207 TfConfiguration 之前建立（elUdUld 的註冊順序）
    W906_LdUldInitOnce();

    // cConfiguration.cpp:112-113 TfConfiguration 建構子
    IC_ReadLockByFile();
    IC_InitConfigEdtList();
    IC_CreateSaveProxies();   // 審查 C1：存檔流程讀的替身先建好（見 IniConfig.gen.inc）
    IC_CreateContainerProxies();   // 審查 H-A：權限判斷要的容器替身與 DFM 父子表
    CreateOCRTrayLotProxies();  IC_EvCreateProxies();   // //AI(W906-FRW-Q19) 20260927: 建構子 :206-224 P26 OCR Tray Lot 10 格（見本檔上方）  //AI(W906-EVB4) 20260928 [W906]：btnRecordJamRateByTimeClear 替身＋DFM 父（檔尾 (6)）；同一行  //AI(W906-EVB4-FIX) 20260929: IC_EvCreateProxies() 原本寫在 // 後面、從來沒跑（St02 10:41 查到），移到註解前面
    // :116 ReadConfigStandard()（開機時把 config_Standard.ini 複製成 config.ini，會寫檔）—— 尚未轉，列入待辦
    //      （只在 CosFunction.bConfigStandard 時有作用）
    ReadLastSetIni();   { void FileRW_IniConfig_N04Ctor(); FileRW_IniConfig_N04Ctor(); }   // :117 golden 第一次有效讀 config.ini／LastSet.ini／configByRecipe.ini（見檔頭 3.）  //AI(W906-SETUPA-N04) 20261002: 接著 golden 建構子 :121-147 [N04] edtN04_Host／mmoN04_IP（本機名稱、IP；FileRW/IniConfig_N04.cpp）；同一行
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
//   不照翻 golden 的 InitialOK 守衛（:6777、:6827、:6863 `if(InitialOK==false) return;`＝程式還沒開完不問）。⛔ 20260927 更正（decisions-pending R74）：原本寫「移植樹
//   wb_serve 沒有把 InitialOK 設成 true、照翻就永遠不檢查」不對——開機 PumpInit 成功時會設 true（移植樹 WebBridgeTags.cpp:563，tools/wb_serve.cpp:4212 呼叫；cmydef.cpp:285
//   預設 false）。照翻只差在 PumpInit 失敗（例：sim canary 拒絕）或開機完成前：golden 那時不問就讓改，這裡一律檢查（比 golden 嚴；R74＝A 待 Steven 決定，行為不變）。
//   CC-L5 :7274 Image1DblClick（輸入 SG_PW.ini 的密碼才讓 grpA32_1 看得見）不用在這裡擋：移植樹 grpA32_1 一直是 Visible=false
//   （IniConfig.gen.inc 的 DFM 設計期值、golden InitConfigEdtList_ItemA :743），沒有任何程式把它打開 → 底下 cbA32_01～10 的
//   頁面值已經被「不可改的丟掉」那一段丟掉、沿用檔案值。
//   只看「不可改的丟掉」之後還留著的值：golden 停用／看不見的格子使用者點不到，也就不會問密碼。
//   不寫、不印任何密碼（RULINGS_20260926 S41／S55）。
// ---------------------------------------------------------------------------
namespace { bool IC_EvShownChecked(const char* id, bool now); AnsiString IC_EvShownText(const char* id, const AnsiString& now); bool IC_EvBeforeApply(const std::string& widgetsJson, std::string* filtered, std::vector<std::string>* ignored, std::vector<std::string>* replayed, std::string* err); void IC_EvRestore(); void IC_EvAfterFormShow(); std::string IC_EvKeepShown(std::string proxiesJson); std::string IC_EvPageJson();  void IC_ReauthM01();   //AI(W906-FRW-S158) 20260927 [W906]: Q41 CC-E2／CC-E7 form.event 段的前置宣告（定義在檔尾）；接在同一行  //AI(W906-Q45-B5) 20260930: + IC_ReauthM01（[M01] 重新登入，檔尾），註解之前、同一行
// 頁面送來、且沒被丟掉的勾選值；沒送或不是布林（ELApplyProxies 會整批拒）→ false
bool IC_PageChecked(const cJSON* root, const char* id, bool* v)
{
    const cJSON* it = cJSON_GetObjectItemCaseSensitive(root, id);
    const cJSON* c = (it && cJSON_IsObject(it)) ? cJSON_GetObjectItemCaseSensitive(it, "checked") : nullptr;
    if (!c || !cJSON_IsBool(c)) return false;
    *v = cJSON_IsTrue(c) != 0;
    return true;
}
bool IC_OpenChecked(const char* id)   // 開頁（golden FormShow）時的值（⛔ 20260927 更正：不再等於「替身現在的值」，見下一行）
{
    TCheckBox* c = dynamic_cast<TCheckBox*>(filerw::ELFind("TfConfiguration", id));
    return c ? IC_EvShownChecked(id, c->Checked) : false;   //AI(W906-FRW-S158) 20260927 [W906]: 改看開頁時送給頁面的那一份 proxies（檔尾 IC_EvKeepShown）——form.event 的 state 會先把頁面值套進替身，替身現值就不是開頁值（不改的話：送一個 E30 事件、state 帶 cbA27=true，存檔時 cbA27 就不算改過、不問密碼）；沒有快照（沒開過頁）才用現值；同一行
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
                why.push_back("C12（PE 模式，cbC12）打勾：這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改（golden cConfiguration.cpp:6775 cbC12Click；Q45 #1＝C）");   //AI(W906-Q45-B5) 20260930: 說明改成設計 3.2.1 的字（本行與下面 5 行）
            else if (!others.empty())
                why.push_back("C12 勾著的時候改 " + others + "：這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改（golden :6775 cbC12Click 綁在這幾格上、只看 C12；"
                              "密碼錯 golden 會把 C12 取消；Q45 #1＝C）");
        }
    }
    // CC-L3 golden :6824 cbA27Click、:6860 cbN07_EnableEmployeeCheakClick
    if (IC_Changed(root, "cbA27"))
        why.push_back("A27（cbA27）勾或取消：這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改（golden :6824 cbA27Click；Q45 #2＝C）");
    if (IC_Changed(root, "cbN07_EnableEmployeeCheak"))
        why.push_back("N07-5（cbN07_EnableEmployeeCheak）勾或取消：這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改（golden :6860 cbN07_EnableEmployeeCheakClick；Q45 #3＝C）");
    // CC-L4 golden :6531 cbM01Click → :6482 DoPassword（AccessLevel[92]==0 或沒有 REAL_TIME_CCD 時 golden 不問）  //AI(W906-Q45-B5) 20260930: 這次存檔帶了重新登入（editlist.save 的 reauth，W906_ReauthHasAnswer）就不在這裡拒 —— 套值之後 FileRW_IniConfig_Save 照 golden DoPassword 問（檔尾 IC_ReauthM01）；沒帶照舊整次拒存（設計 3.1 第 2 條）
    if (LevelSet.AccessLevel[92] != 0 && REAL_TIME_CCD && !W906_ReauthHasAnswer("IniConfig")) {
        int nM01 = 0;   // dfm:21162-21305 OnClick = cbM01Click（cbM01、cbM01_01～15 共 16 格）
        const char* const* kM01 = W906_ReauthControls("m01", &nM01);   //AI(W906-Q45-B5) 20260930: 名單改用 WebLogin.cpp 檔尾那一份（檔尾 IC_ReauthM01、開頁 extra.auth 用同一份）
        std::string m;
        for (int i = 0; i < nM01; ++i)
            if (IC_Changed(root, kM01[i])) m += std::string(m.empty() ? "" : "、") + kM01[i];
        if (!m.empty())
            why.push_back("M01 群組（" + m + "）要重新登入到等級 " + std::to_string((int)LevelSet.AccessLevel[92]) +
                          " 以上（LevelSet.AccessLevel[92]；golden :6531 cbM01Click → :6482 DoPassword）：這次存檔沒有帶重新登入的帳號密碼"
                          "（Configuration 頁補件 ht9045_iniconfig_auth_c.js 按存檔時會跳登入框；請重讀頁面再存）");
    }
    cJSON_Delete(root);
    if (why.empty()) return std::string();
    std::string s = "這次沒有存檔：下面這幾項在 golden 要先輸入密碼（或重新登入）才能改（Q45：廠商密碼的幾項網頁版不提供；"
                    "M01 監控功能要在按存檔時重新登入）。請把廠商密碼的幾項改回原值再存，其他設定可以照常存：";
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
    std::vector<std::string> evReplayed; if (!IC_EvBeforeApply(widgetsJson, &filtered, &ignored, &evReplayed, err)) return 400;   //AI(W906-FRW-S158) 20260927 [W906]: Q41 CC-E2／CC-E7 —— 頁面沒送 form.event 就直接存時，照 VCL 補點一次那幾格並跑 golden 處理器（D46 上下鍵、E30／E39／D36 勾選、Timer1 顯示段），再用重播後的可見／可改重算上面的「不可改的丟掉」（同 FileRW/TrayForm.cpp BeforeApply，R94／R95）；沒有要重播的＝什麼都不動；在密碼守衛之後（重播不碰要密碼的格子，檔尾另外再查一次）；佔用原本的空行
    // 1. 套用
    AnsiString ocrOpen[kOCRTrayLotCount];   // //AI(W906-FRW-Q19) 20260927: 套值前的 P26 OCR Tray Lot（＝開頁 FormShow 填的 LastSet.TrayCount）
    for (int i = 0; i < kOCRTrayLotCount; ++i) ocrOpen[i] = IC_EvShownText(OCRTrayLotName(i, false).c_str(), OCRTrayLotEdit(i)->Text);   //AI(W906-FRW-S158) 20260927 [W906]: 開頁值改看開頁時送給頁面的那一份（檔尾 IC_EvKeepShown；form.event 的 state 可能已把頁面值套進替身，不改的話頁面改過的格子會被當成沒改、不寫 lastdata.dat）；同一行
    std::vector<std::string> applied, unknown;
    if (!filerw::ELApplyProxies("TfConfiguration", filtered, &applied, &unknown, err)) { IC_EvRestore(); return 400; }   //AI(W906-FRW-S158) 20260927 [W906]: 先還原存檔前重播改過的替身（400＝伺服器端沒被頁面值改動；沒有重播時什麼都不做）；同一行
    for (std::size_t i = 0; i < unknown.size(); ++i)
        for (std::size_t j = 0; j < sizeof(kIC_SaveReads) / sizeof(kIC_SaveReads[0]); ++j)
            if (unknown[i] == kIC_SaveReads[j]) {   // 送了但型別不歸頁面管（例 TPanel）→ 值沒套上 → 不可存
                IC_EvRestore(); ReadLastSetIni();    // 把已套的替身還原成檔案值（//AI(W906-FRW-S158) 20260927 [W906]: 先還原存檔前重播的 golden 事件改過的顯示，見檔尾；同一行）
                *err = "refused: save reads " + unknown[i] + " but its value kind cannot be applied";
                return 400;
            }

    // 3. golden 存檔流程
    filerw::SessionBegin(answersJson);  IC_ReauthM01();   //AI(W906-Q45-B5) 20260930: Q45 甲 #4 —— golden cbM01Click（cConfiguration.cpp:6531-6544）→ DoPassword（:6482-6529）：golden 在表單開著、點下去的當下問，網頁在套值之後、golden FormClose 之前問一次（[W906] Q45-2＝A）；有密碼本時問完一律登出成 Operator，登出在 FormClose 之前（Q45-3＝A：開了 [A01_2] 的機台 FormClose 照 golden 什麼都不存）；本體在檔尾；同一行
    // //AI(W906-FRW-Q19) 20260927: golden edOCRTrayLotChange（:5522，P26 OCR Tray Lot 各格的 OnClick）在表單開著時就跑、按下即寫
    //   lastdata.dat —— 網頁送的是最後狀態：值和開頁時不同＝使用者改過 → 依格序重播（golden 小鍵盤夾 0～20 → LastSet.TrayCount[Tag]
    //   → WriteLastDataFile()），在 FormClose 之前，所以答 NO 也已經寫了（同 golden）。沒送／被丟掉（gbP26_OCRCheck 看不見＝
    //   CosFunction.bTrayOCR 關、或停用）的格子不重播。WriteLastDataFile 寫的是整份 LastSet（golden 同）；SaveConfiguration :8251 答 YES
    //   時會再寫一次同樣內容。沒存成時下面的 ReadLastSetIni → ReadLastDataFile 讀回的就是剛寫的檔。
    std::vector<std::string> events(evReplayed);   //AI(W906-FRW-S158) 20260927 [W906]: ack.events 先列存檔前重播的 form.event 控制項（IC_EvBeforeApply），再列 P26 重播；同一行
    for (int i = 0; i < kOCRTrayLotCount; ++i) {
        const std::string id = OCRTrayLotName(i, false).c_str();
        if (std::find(applied.begin(), applied.end(), id) == applied.end()) continue;
        TEdit* e = OCRTrayLotEdit(i);
        if (e->Text == ocrOpen[i]) continue;                         // golden 沒有 OnChange 的判斷；網頁只能從值看出「改過」
        filerw::ELMark(("edOCRTrayLotChange " + id + " (golden cConfiguration.cpp:5522, replayed before FormClose)").c_str());
        IC_edOCRTrayLotChange(e);
        events.push_back(id);
    }
    { void FileRW_IniConfig_EvB10AExitHead(const char* from); FileRW_IniConfig_EvB10AExitHead("editlist.save"); }  IC_FormClose();   //AI(W906-EVB10A) 20260929 [W906]: CC-E10 golden btnSaveClick（cConfiguration.cpp:7899-7903）＝sbExitClick(btnSave)：先跑 sbExitClick 前三句（ShowSpeed、PE 鈕、[C12] 關掉時自動關 PE，本檔檔尾）再 Close()→FormClose；接在同一行
    g_formShown = false;   // 審查 #6：golden FormClose 就是關頁；下一次存檔前要重新開頁（editlist.get）
    const bool saved = filerw::ELMarked("SaveConfiguration");
    // 審查 M3：沒存（答 NO、A01_2 權限…）時，golden 只把 elConfig 刷回（:5845），cbLastSet／elConfig_byRecipe
    // 的替身留著頁面值；golden 下次 FormShow 會 ReadLastSetIni（:4625）重讀，這裡沒有下次開頁 → 立刻重讀。
    if (!saved) { ReadLastSetIni(); }  if (const char* w = W906_Main_sbConfigurationClickTail()) filerw::ELTodo(w);   // AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 CC-E15（原本 `if (!saved) ReadLastSetIni();` 加大括號只為了 -Wmisleading-indentation，行為不變） —— golden TfMain::sbConfigurationClick 關窗尾段 V912 main.cpp:28626-28657（本體 FileRW/MainClick.cpp；R85／R86）：網頁存檔＝golden FormClose（上面 IC_FormClose），golden 關窗不論存不存都跑 → 不看 saved；放在「沒存成就重讀」之後，IniConfig 才是 golden 關窗後的值（答 NO＝檔案值，舊值比對＝沒變）；密碼守衛與其他 4xx 在 FormClose 之前 return（golden 視窗還開著）→ 不跑；接在同一行

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
    W906_Main_sbConfigurationClickHead(); IC_FormShow();   // AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 CC-E15 —— golden main.cpp:28615-28622 在 ShowModal（→ FormShow 的 ReadLastSetIni）之前記 E43／A10／I37／A51／I21／A78 舊值，關窗尾段比對用（FileRW/MainClick.cpp）；接在同一行
    g_formShown = true;  IC_EvAfterFormShow();  { void FileRW_IniConfig_EvB10AShown(); FileRW_IniConfig_EvB10AShown(); }   //AI(W906-EVB10A) 20260929 [W906]: CC-E10 記下「這一次開窗 golden FormShow 跑過」（關窗邊緣才跑 sbExitClick 前三句，本檔檔尾）  //AI(W906-FRW-S158) 20260927 [W906]: golden FormShow :5312 起動 Timer1 → 第一拍的顯示段（頁面拿到的是那一拍之後的畫面）＋登記 form.event 別名頁 Config.Configuration「開過頁」（檔尾 (1)）；同一行
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
    w.Key("proxies").RawValue(IC_EvKeepShown(filerw::ProxyStateJson("TfConfiguration")));   //AI(W906-FRW-S158) 20260927 [W906]: 同一份同時記成「開頁值」（密碼守衛 IC_OpenChecked、P26 重播比對用；檔尾 (5)）；回傳的就是原字串；同一行
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
    w.Key("auth").RawValue(W906_ReauthOpenJson("IniConfig", false, false, false, false));  w.EndObject();   //AI(W906-Q45-B5) 20260930: extra.auth（設計 q45-web-password.md 3.2.2-①：m01 重新登入點＋廠商密碼三點、SG_PW 一點 armed:false；頁面 ht9045_iniconfig_auth_c.js 看它）；同一行
    w.Key("events").RawValue(IC_EvPageJson()); w.Key("eventTag").String("Config.Configuration"); w.EndObject();   //AI(W906-FRW-S158) 20260927 [W906]: 同其他 C 路頁的 editlist.get "events"（{控制項:{event, golden, operable}}，檔尾 IC_EvPageJson）；form.event 要送 tag＝eventTag（檔尾 (1)）；同一行
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
    if (!g_booted) { *json = "IniConfig edit lists are not booted"; return 409; }  filerw::OpenEnterRecord("IniConfig", g_formShown);   //AI(W906-FRW-S165) 20260927 [W906]: golden sbConfigurationClick V912 main.cpp:28616 NewRecordProcess("MES2185", "Enter Configuration") 在 :28624 fConfiguration->ShowModal() 之前（下一行 IniConfigPageJson 的 W906_Main_sbConfigurationClickHead 記舊值、IC_FormShow；golden :28615 記 E43 舊值比這一筆早一行，兩者互不相干）；g_formShown 在 IniConfigPageJson 才設 true，存檔（:347 golden FormClose）變回 false ⇒ 存檔後的自動重讀會再記一筆（golden 存檔＝關窗，再改要重按一次）（表與規則在 FileRW/_EditPage.cpp 檔尾）；接在同一行
    return IniConfigPageJson(json);
}


// ===========================================================================
//  //AI(W906-FRW-S158) 20260927 [W906]：Configuration 頁的 WS form.event（Q41 盤點 CC-E2「D46 上下鍵」、CC-E7「勾選連動顯示」）。
//  依據：Steven ★ Q40＝A（RULINGS_20260926 S157）；盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 3.19 節；
//        Q14＝B（S136）的延伸見 (3)。規格：D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md §3.0g。
//  golden（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp，綁定 cConfiguration.dfm）：
//    :6094 udD46Click（udD46 dfm:4964，TUpDown Min 5 Max 15 Wrap=False）、:6125 cbE30Click（8 格，dfm:5880 起）、
//    :6369 cbE39Click（dfm:9189）、:6551 cbD36Click（5 格，dfm:4824 起）、:6025 Timer1Timer 的顯示段 :6032-6044
//    （cbD21／cbD47／cbF05 沒有 OnClick，靠 Timer1 每秒一拍；dfm:22667，Interval 是 VCL 預設 1000 ms）。
//  處理器由產生器轉（tools/editlist/IniConfig.py 'events' → IniConfig.gen.inc 檔尾 kIC_Events），這裡做五件事：
//  (1) 掛上 form.event。IniConfig 不是 PageDesc（開頁／存檔走本檔 FileRW_IniConfig_Page／_Save：密碼守衛、P26 重播、關窗尾段），
//      W906_FormEvent（FileRW/_FormEvent.cpp）只認 PageDesc ⇒ 另登記一個只給 form.event 用的 PageDesc kEvPage：
//      tag "Config.Configuration"、page "Config.Configuration.html"、form "TfConfiguration"（同一批替身）。
//      ⚠ tag 不能用 "IniConfig"：tools/wb_serve.cpp 的 editlist.get／editlist.save 先找 PageDesc（:5204、:5300），會把 IniConfig 的
//      開頁／存檔改走 FileRW/_EditPage.cpp 的 PageJson／PageSave（沒有密碼守衛等）。"Config.Configuration" 沒有登記開窗閘
//      （FileRW/_EditPage.cpp kOpenGates）⇒ 拿它送 editlist.get／editlist.save 一律 no-gate 拒絕；它的 saveFlow 也只記 todo、不存。
//      RunPageEvent 要這個 tag「開過頁、同一個等級」（_EditPage.cpp 的開頁紀錄只有 PageJson 會設）⇒ 本檔開頁（IniConfigPageJson）
//      跑完 golden FormShow 之後呼叫一次 filerw::PageJson(kEvPage)（formShow 是空的、回應丟掉；代價是多一次 proxies 序列化），
//      另外每個事件再查本檔自己的 g_formShown（存檔＝golden FormClose 之後、重新開頁之前，golden 不會有事件）。
//      那一次呼叫的 SessionBegin("") 會清掉 FormShow 記的訊息 —— IniConfig 的開頁回應本來就不帶 session，沒有影響。
//  (2) 事件表照抄 kIC_Events，改兩處：
//      * udD46（VCL TUpDown）：golden 使用者按上／下兩個箭頭，VCL 先把 Position 加減 Increment（Wrap=False → 夾在 Min..Max）
//        再呼叫 OnClick(Sender, Button)。form.event 的處理器拿不到頁面送的值（RunPageEvent 只替下拉／單選群組／勾選框／輸入框
//        套自己的值），所以拆成兩個事件名 btNext（上）／btPrev（下）＝golden TUDBtnType 的兩個值；"click" 不登記（沒有方向 → no-handler）。
//      * 每個事件跑完 golden 處理器再補 Timer1 一拍（顯示段）：golden Timer1 每秒都在跑，state 改到 D21／D47／F05 時下一拍就會更新；
//        cbD21／cbD47／cbF05 三列的處理器本身就是那一拍，不重複。
//  (3) D46 的記憶體值（偏離 golden，列 R 題）：golden :6098 點一下就改 IniConfig.iD46WaitIndexDestroyTime（只有記憶體，沒寫檔；
//      檔案在 FormClose 存檔時由 elConfig 讀 edD46 寫）。golden 表單是 modal，關窗一定經過 FormClose 的 YES／NO；網頁關窗不經過伺服器 ⇒
//      照翻的話「按了上下鍵、沒存就關頁」機台會用沒存的值跑到下次重讀。這裡處理器照跑、跑完把記憶體值放回：畫面（udD46／edD46）
//      照 golden 變，記憶體等存檔（SaveConfiguration → elConfig 讀 edD46）才改 —— 跟 Q14＝B「按下不寫、整頁存檔一起寫」同一個方向。
//      改回 golden 只要拿掉 IC_EvUdStep 的 keep 兩行。
//      另：golden edD46 雖然 DFM 是 Enabled=False，開頁 ReadLastSetIni → InitialDataToEdit 會把它設成清單筆的 bEnable（true）
//      （golden Public/HTEditList.cpp:1401；移植樹 Public/HTEditList.cpp:1448，cprod.cpp:3217 → :3064）⇒ golden 操作員也可以直接打字，
//      伺服器端 edD46 是可改的、存檔不會丟（頁面要照 editlist.get 的 proxies.edD46.editable，不是照 DFM）。
//  (4) 存檔補重播（同 FileRW/TrayForm.cpp BeforeApply，R94／R95）：頁面沒送 form.event 就直接存，伺服器端的勾選與面板顯示還是開頁時的 ⇒
//      例：頁面勾了 E30、改了 palE30 底下的值 → 伺服器端 palE30 還是看不見 → 那些值被「不可改的丟掉」丟掉（St02 回報的「存檔、重開才生效」）。
//      FileRW_IniConfig_Save 在密碼守衛之後、套值之前呼叫 IC_EvBeforeApply：頁面值和伺服器端不同、點得到的事件列照 VCL 點一次
//      （勾選框設 Checked；udD46 往目標一格一格按）＋golden 處理器＋Timer1 一拍，然後用重播後的可見／可改重算「不可改的丟掉」、
//      再跑一次密碼守衛（重播不會碰到要密碼的格子；保險）。之後回 400 的路徑先 IC_EvRestore（400＝伺服器端沒被頁面值改動）。
//      頁面有送 form.event 時伺服器端已經等於頁面值 ⇒ 不重播、行為與原本相同。
//      ⚠ 看不到點的先後：golden 可以「先改 palE30 裡的值、再取消 E30」，伺服器只看到最後 E30 是關的 ⇒ 那些值照舊丟掉（R 題）。
//      ⚠ 重播改的勾選值之後會被頁面送的值蓋掉（例：D36 勾了、頁面 D33 還是勾著 → 重播把 D33 取消，接著套頁面值又勾回去）：
//      golden 也到得了（勾 D36 之後再手動勾 D33），伺服器分不出來 ⇒ 以頁面最後狀態為準（同 PageSave）。
//  (5) 開頁值快照：form.event 的 state（頁面其他控制項目前的值）會先套進替身，替身現值就不再是開頁值。密碼守衛（IC_PasswordGuard，
//      a8eca460）和 P26 OCR 重播（Q19）比的是開頁值 ⇒ 改看開頁時送給頁面的那一份 proxies（IC_EvKeepShown）。
//  (6) //AI(W906-EVB4) 20260928 [W906]：批次 B4（Steven 20260928 事件派工「任何畫面的事件, 都是我們做」；
//      D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B4）。事件表多 14 列（tools/editlist/IniConfig.py 'events'）：
//      * CC-E1 btD47 click → :6062 btD47Click（LastSet.iD47SocketTestedCount=0，edD47_3 顯示 0）。照 golden 當下就改記憶體，沒有套 (3) D46 的
//        「跑完放回」：Clear 是動作不是設定值（golden 答 NO 記憶體也已經歸零；移植樹答 NO 由 ReadLastSetIni 讀回檔案值，審查 M3 既有偏離）。
//        存檔由 cbLastSet 讀 edD47_3（golden :1599）寫 LastSet.ini。
//      * CC-E3 btnRecordJamRateByTimeClear click → :6691（兩個計數歸零、bRecordJamRateByTime_Clear=true；DFM Caption 是 'Set All'，golden 怪處）。
//        產生器只替「golden 方法本體用到的元件＋祖先」建替身（tools/gen_editlist.py names_used），這顆鈕處理器沒提到自己 ⇒ 開機
//        IC_EvCreateProxies 照 golden DFM 補（:13898，父 pal_O2 :13882）。讀這個旗標的 golden TfMain::RecordJamRateByTime（V912 main.cpp:33109，
//        :33259 呼叫）移植樹還沒有 ⇒ 計數有歸零、計時起點沒人重設（交給 Jimmy）。
//      * CC-E4 btResume click → :6257（bLockByServer=false＋ShowMyMessage「手動恢復機台動作。」→ ack.messages）。只有 CC_MTI 看得到
//        （FormShow :5303-5310）；只清伺服器鎖機旗標，本身不動機台（讀它的是 ckernel.cpp:865、csystem.cpp:31173）。
//      * CC-E6 cbA09／chkA09_1／cbA14 click → :6453 cbA09Click（golden 三格綁同一支，只看、只改 cbA09）：機台內有 IC 就把 cbA09 改回＋
//        ShowErrorMessage MES1645（常溫，請先 One Cycle）／MES1646（加熱，請先 Clean out）；wb_serve 走 ForwardShowErrorMessage（kcode 0 不阻塞，
//        照 golden note.cpp:795-801 先 StopAllMotor）。互鎖不信任前端：存檔時 IC_EvA09Recheck 一律重查（頁面值≠IniConfig 記憶體值就照 VCL
//        點一次 cbA09、跑處理器；被改回 ⇒ 頁面值不收、ack.ignored 列 cbA09）——比的是記憶體值（開頁讀的檔案值），不看伺服器替身是不是
//        已經等於頁面值（29a13bdb 之後 state 已經改不了事件列的控制項；存檔再查是為了「點完到存檔之間機台多了料」，同 R95 存檔一定重查）。
//        比 golden 嚴：golden 只在點的當下查（R 題）。
//      * CC-E8 8 條 EP 滑桿 change（帶 position，7e1785dc）→ tbD25_*／tbD60_* Change（:6271-:6684）：標籤當場更新；D25 記
//        aEP*OldData／bEP*DataChange（存檔時 CheckConfigurationBeforeSave 寫 EP 變更 log）；D60 當場 RecordProcess。
//        (4) 的存檔補重播原本把每個 ELTrackBar 都當 udD46 一格一格按 ⇒ 改成只有 udD46 這樣按，滑桿走 IC_EvSlide（照 VCL 設 Position 再跑
//        處理器；EP 保護把滑桿拉回 85／115 時再拖一次，最多兩次）。
//      每一列跑完照 (2) 補 Timer1 一拍。
// ===========================================================================
#include <cstring>
#include <stdexcept>

namespace {
// ---- (1) form.event 別名頁 ---------------------------------------------------------------------------------------------
const char kEvTag[] = "Config.Configuration";
bool IC_EvBooted() { return g_booted; }
void IC_EvNoFormShow() {}   // golden FormShow 已經由 IniConfigPageJson 的 IC_FormShow 跑過；這裡只為了登記「開過頁」
void IC_EvRefuseSave()      // 不會走到（wb_serve 對這個 tag 先回 no-gate）；保險：不存，PageSave 接著 reload()
{
    filerw::ELTodo("Config.Configuration is only the form.event alias of IniConfig -- save with WS editlist.save tag=IniConfig (nothing saved)");
}
void IC_EvReload() { ReadLastSetIni(); }   // PageSave 沒存成時的還原（同 FileRW_IniConfig_Save 的 ReadLastSetIni）
HTEditList** const kEvLists[] = {&elConfig, &cbLastSet, &elConfig_byRecipe};   // 同 FileRW_IniConfig_Save 丟值看的三份（bEnable=false 的筆）
const char* const kEvListNames[] = {"elConfig", "cbLastSet", "elConfig_byRecipe"};
const filerw::PageDesc kEvPage = {
    kEvTag, "TfConfiguration", "Config.Configuration.html",
    kEvLists, kEvListNames, 3,
    nullptr, 0,
    &IC_EvNoFormShow, &IC_EvRefuseSave, "SaveConfiguration", &IC_EvReload, &IC_EvBooted,
    nullptr, nullptr,
};
filerw::PageRegistrar g_evPageReg(&kEvPage);

void IC_EvRequireShown()   // golden 的事件只發生在開著的表單上（RunPageEvent 查過別名頁的開頁紀錄；這裡查本檔自己的）
{
    if (!g_formShown || g_formShownLevel != AccessLevel)
        throw std::runtime_error("reload page: the Configuration page is closed (saved = golden FormClose) or the access level changed -- "
                                 "send WS editlist.get tag=IniConfig (golden FormShow) before sending events");
}

// ---- (2)(3) 事件表 -------------------------------------------------------------------------------------------------------
// VCL TUpDown 按一下：Position ±Increment（Wrap=False：SetPosition 夾在 Min..Max）→ golden udD46Click（:6094）
void IC_EvUdStep(filerw::ELTrackBar* ud, int dir)
{
    ud->Position = (int)ud->Position + dir * ud->Increment;
    const int keep = IniConfig.iD46WaitIndexDestroyTime;   // (3)：記憶體值等存檔才改（改回 golden：拿掉這一行和下下一行）
    IC_udD46Click();                                       // golden :6097 edD46->Text=Position；:6098 IniConfig.iD46WaitIndexDestroyTime=Position
    IniConfig.iD46WaitIndexDestroyTime = keep;
}
void IC_EvUd(TControl* Sender, int dir)
{
    IC_EvRequireShown();
    filerw::ELTrackBar* ud = dynamic_cast<filerw::ELTrackBar*>(Sender);
    if (!ud) throw std::runtime_error("internal: udD46 proxy is not a TUpDown stand-in (filerw::ELTrackBar)");
    IC_EvUdStep(ud, dir);
    IC_Timer1Timer();                                      // (2)：Timer1 下一拍
}
void IC_EvUdNext(TControl* Sender) { IC_EvUd(Sender, +1); }   // golden TUDBtnType btNext（上箭頭）
void IC_EvUdPrev(TControl* Sender) { IC_EvUd(Sender, -1); }   // golden TUDBtnType btPrev（下箭頭）
// 勾選框：RunPageEvent 已經照 VCL 設好 Checked；跑 golden 處理器，再補 Timer1 一拍（處理器本身就是 Timer1Timer 的三列不重複）
void IC_EvClick(TControl* Sender)
{
    IC_EvRequireShown();
    for (std::size_t i = 0; i < sizeof(kIC_Events) / sizeof(kIC_Events[0]); ++i) {
        if (filerw::ELFind("TfConfiguration", kIC_Events[i].control) != Sender) continue;
        kIC_Events[i].handler(Sender);
        if (kIC_Events[i].handler != &IC_Ev_Timer1Timer) IC_Timer1Timer();
        return;
    }
    throw std::runtime_error("internal: form.event sender is not in kIC_Events");
}
filerw::PageEvent g_icEvents[sizeof(kIC_Events) / sizeof(kIC_Events[0]) + 1];   // udD46 拆成兩列 → 多一格
int g_icNEvents = 0;
struct IcEvRegistrar {
    IcEvRegistrar()
    {
        const int cap = (int)(sizeof(g_icEvents) / sizeof(g_icEvents[0]));
        int n = 0;
        for (std::size_t i = 0; i < sizeof(kIC_Events) / sizeof(kIC_Events[0]); ++i, ++n) {
            g_icEvents[n] = kIC_Events[i];
            g_icEvents[n].handler = &IC_EvClick;
            if (std::strcmp(kIC_Events[i].control, "udD46") == 0 && n + 1 < cap) {
                g_icEvents[n].event = "btNext";
                g_icEvents[n].handler = &IC_EvUdNext;
                ++n;
                g_icEvents[n] = kIC_Events[i];
                g_icEvents[n].event = "btPrev";
                g_icEvents[n].handler = &IC_EvUdPrev;
            }
        }
        g_icNEvents = n;
        filerw::RegisterPageEvents(kEvTag, g_icEvents, n);
    }
} g_evreg;

// editlist.get IniConfig 的 "events"（格式同 FileRW/_EditPage.cpp EvPageJson：{控制項:{event, golden, operable}}；
// 同一個控制項有兩個事件名（udD46）時 event 是第一個、另帶 "events":[全部]）
std::string IC_EvPageJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    for (int i = 0; i < g_icNEvents; ++i) {
        const filerw::PageEvent& e = g_icEvents[i];
        if (i > 0 && std::strcmp(g_icEvents[i - 1].control, e.control) == 0) continue;
        w.Key(e.control).BeginObject();
        w.Key("event").String(e.event);
        if (i + 1 < g_icNEvents && std::strcmp(g_icEvents[i + 1].control, e.control) == 0) {
            w.Key("events").BeginArray();
            for (int k = i; k < g_icNEvents && std::strcmp(g_icEvents[k].control, e.control) == 0; ++k) w.String(g_icEvents[k].event);
            w.EndArray();
        }
        w.Key("golden").String(e.golden);
        w.Key("operable").Bool(filerw::ELFind("TfConfiguration", e.control) != nullptr && filerw::ELOperable("TfConfiguration", e.control));
        w.EndObject();
    }
    w.EndObject();
    return w.Str();
}

void IC_EvAfterFormShow()
{
    IC_Timer1Timer();                        // golden FormShow :5312 Timer1->Enabled=true → 第一拍（fShow 已由 FormShow 設成 true）
    std::string discard;
    filerw::PageJson(kEvPage, &discard);     // (1)：登記別名頁「開過頁、這個等級」
}

// ---- (5) 開頁值快照 ------------------------------------------------------------------------------------------------------
cJSON* g_shown = nullptr;   // IniConfigPageJson 送給頁面的 proxies（每次開頁換新）
std::string IC_EvKeepShown(std::string proxiesJson)
{
    if (g_shown) cJSON_Delete(g_shown);
    g_shown = cJSON_Parse(proxiesJson.c_str());
    return proxiesJson;
}
const cJSON* IC_EvShownItem(const char* id, const char* key)
{
    const cJSON* o = g_shown ? cJSON_GetObjectItemCaseSensitive(g_shown, id) : nullptr;
    return (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
}
bool IC_EvShownChecked(const char* id, bool now)
{
    const cJSON* v = IC_EvShownItem(id, "checked");
    return (v && cJSON_IsBool(v)) ? cJSON_IsTrue(v) != 0 : now;
}
AnsiString IC_EvShownText(const char* id, const AnsiString& now)
{
    const cJSON* v = IC_EvShownItem(id, "text");
    return (v && cJSON_IsString(v) && v->valuestring) ? AnsiString(v->valuestring) : now;
}

// ---- (4) 存檔補重播 ------------------------------------------------------------------------------------------------------
cJSON* g_saveBefore = nullptr;   // 重播前的 TfConfiguration 替身狀態（IC_EvRestore 用）；沒有重播＝nullptr

// //AI(W906-EVB4) 20260928 [W906] (6) CC-E6：這一列的 golden 處理器是 cbA09Click 嗎（cbA09／chkA09_1／cbA14，dfm:579／:781／:936）
bool IC_EvIsA09Row(const char* ctl)
{
    for (std::size_t i = 0; i < sizeof(kIC_Events) / sizeof(kIC_Events[0]); ++i)
        if (std::strcmp(kIC_Events[i].control, ctl) == 0) return kIC_Events[i].handler == &IC_Ev_cbA09Click;
    return false;
}
// 頁面送的 cbA09 跟 IniConfig 記憶體值不同嗎（＝golden cbA09Click :6455-6456 會往下查的條件）
bool IC_EvA09Pending(const cJSON* root)
{
    const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, "cbA09");
    const cJSON* v = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr;
    return v && cJSON_IsBool(v) && (cJSON_IsTrue(v) != 0) != IniConfig.bA09_ByArmCloseSite;
}
// 存檔前重查 A09 互鎖：照 VCL 點一次 cbA09（Checked＝頁面值 → OnClick＝cbA09Click），機台內有 IC 時 golden 會改回 ⇒ 頁面值不收
void IC_EvA09Recheck(cJSON* root, std::vector<std::string>* ignored, std::vector<std::string>* replayed)
{
    if (!IC_EvA09Pending(root)) return;      // 沒送、型別不對（交給 ELApplyProxies）、或跟記憶體值一樣（golden 不查）
    TCheckBox* cb = dynamic_cast<TCheckBox*>(filerw::ELFind("TfConfiguration", "cbA09"));
    if (!cb) return;
    const bool want = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(root, "cbA09"), "checked")) != 0;
    cb->Checked = want;
    IC_EvClick(cb);                          // golden :6453 cbA09Click（MES1645／MES1646）＋Timer1 一拍
    replayed->push_back("cbA09");
    if (cb->Checked != want) {               // golden 改回了：不信任前端，頁面值丟掉（同 FileRW/TrayForm.cpp R95）
        cJSON_DeleteItemFromObjectCaseSensitive(root, "cbA09");
        ignored->push_back("cbA09");
    }
}
// CC-E8：照 VCL 拖滑桿（Position 夾在 Min..Max → OnChange＝tb*Change）。處理器的 EP 保護把位置拉回時，golden 使用者看得到、可以再拖一次 ⇒ 最多兩次
void IC_EvSlide(filerw::ELTrackBar* tb, TControl* c, int want)
{
    if (want < (int)tb->Min) want = tb->Min;
    if (want > (int)tb->Max) want = tb->Max;
    for (int k = 0; k < 2 && (int)tb->Position != want; ++k) {
        tb->SetPosition(want, false);        // 同 FileRW/_EditPage.cpp RunPageEvent 第 5 步（不從替身的 OnChange 指標再觸發一次）
        IC_EvClick(c);                       // golden tb*Change ＋ Timer1 一拍
    }
}

// 頁面送的這一格和伺服器端不同嗎（勾選框看 checked、udD46 看 position）；*want＝頁面值。型別不對＝不同也不算（交給 ELApplyProxies 整批 400）
bool IC_EvPageDiffers(const cJSON* root, const char* ctl, int* want)
{
    const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, ctl);
    if (!o || !cJSON_IsObject(o)) return false;
    TControl* c = filerw::ELFind("TfConfiguration", ctl);
    if (filerw::ELTrackBar* ud = dynamic_cast<filerw::ELTrackBar*>(c)) {
        const cJSON* p = cJSON_GetObjectItemCaseSensitive(o, "position");
        if (!p || !cJSON_IsNumber(p)) return false;
        *want = p->valueint;
        return *want != (int)ud->Position;
    }
    if (TCheckBox* cb = dynamic_cast<TCheckBox*>(c)) {
        const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, "checked");
        if (!v || !cJSON_IsBool(v)) return false;
        *want = cJSON_IsTrue(v) ? 1 : 0;
        return (*want != 0) != cb->Checked;
    }
    return false;
}

bool IC_EvBeforeApply(const std::string& widgetsJson, std::string* filtered, std::vector<std::string>* ignored,
                      std::vector<std::string>* replayed, std::string* err)
{
    if (g_saveBefore) { cJSON_Delete(g_saveBefore); g_saveBefore = nullptr; }   // 上一次存檔的快照作廢
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {   // 呼叫端已驗過（不會發生）
        if (root) cJSON_Delete(root);
        return true;
    }
    int want = 0;
    bool any = false;
    for (int i = 0; i < g_icNEvents && !any; ++i) any = !IC_EvIsA09Row(g_icEvents[i].control) && IC_EvPageDiffers(root, g_icEvents[i].control, &want);   //AI(W906-EVB4) 20260928 [W906]：A09 三列不在這裡重播（下面 IC_EvA09Recheck）；同一行
    if (!any && !IC_EvA09Pending(root)) {   // 頁面有送 form.event（或沒改這幾格）：filtered／ignored 不動，行為與原本相同（AI(W906-EVB4) 20260928：A09 互鎖要重查時也往下走）
        cJSON_Delete(root);
        return true;
    }
    g_saveBefore = cJSON_Parse(filerw::ProxyStateJson("TfConfiguration").c_str());
    // 同一格只點一次；點不到的（自己或上層看不見／停用）不點，交給下面的「不可改的丟掉」。多跑幾輪：前一格點了之後
    // 下一格才點得到的情形（今天這幾格互不包含，保險）
    for (int pass = 0; pass < g_icNEvents; ++pass) {
        bool progress = false;
        for (int i = 0; i < g_icNEvents; ++i) {
            const filerw::PageEvent& e = g_icEvents[i];
            if (IC_EvIsA09Row(e.control) || std::find(replayed->begin(), replayed->end(), std::string(e.control)) != replayed->end()) continue;   //AI(W906-EVB4) 20260928 [W906]：A09 三列見 IC_EvA09Recheck；同一行
            if (!IC_EvPageDiffers(root, e.control, &want) || !filerw::ELOperable("TfConfiguration", e.control)) continue;
            TControl* c = filerw::ELFind("TfConfiguration", e.control);
            filerw::ELTrackBar* const ud = dynamic_cast<filerw::ELTrackBar*>(c); if (ud && std::strcmp(e.control, "udD46") == 0) {   //AI(W906-EVB4) 20260928 [W906]：CC-E8 之後表上也有滑桿（ELTrackBar），只有 udD46 一格一格按；同一行
                const int dir = want > (int)ud->Position ? +1 : -1;
                for (int k = 0; k < 64 && (int)ud->Position != want; ++k) {   // golden 使用者一格一格按
                    const int before = ud->Position;
                    IC_EvUdStep(ud, dir);
                    if ((int)ud->Position == before) break;                    // 到 Min／Max 了
                }
                IC_Timer1Timer();
            } else if (ud) {                                                   //AI(W906-EVB4) 20260928 [W906]：CC-E8 EP 滑桿（見 IC_EvSlide）
                IC_EvSlide(ud, c, want);
            } else if (TCheckBox* cb = dynamic_cast<TCheckBox*>(c)) {
                cb->Checked = want != 0;                                       // VCL：使用者點勾選框 ⇒ Checked 變 → OnClick
                IC_EvClick(c);
            }
            replayed->push_back(e.control);
            progress = true;
        }
        if (!progress) break;
    }
    // 重播後的可見／可改重算「不可改的丟掉」（同 FileRW_IniConfig_Save 上面那一段）
    ignored->clear();
    HTEditList* const lists[] = {elConfig, cbLastSet, elConfig_byRecipe};
    for (cJSON* it = root->child; it;) {
        cJSON* next = it->next;
        bool drop = !filerw::ELEditable("TfConfiguration", it->string);
        if (!drop) {
            TControl* c = filerw::ELFind("TfConfiguration", it->string);
            for (int k = 0; k < 3 && c && !drop; ++k)
                for (int i = 0; lists[k] && i < lists[k]->FEditList->Count; ++i) {
                    THTEdit* he = static_cast<THTEdit*>(lists[k]->FEditList->Items[i]);
                    if (he->SourceControl == c && !he->bEnable) { drop = true; break; }
                }
        }
        if (drop) {
            ignored->push_back(it->string);
            cJSON_DeleteItemFromObjectCaseSensitive(root, it->string);
        }
        it = next;
    }
    IC_EvA09Recheck(root, ignored, replayed);   //AI(W906-EVB4) 20260928 [W906]：CC-E6 A09 互鎖存檔一定重查（在「不可改的丟掉」之後：點不到的格子本來就不收）
    char* s = cJSON_PrintUnformatted(root);
    *filtered = s ? s : "{}";
    if (s) cJSON_free(s);
    cJSON_Delete(root);
    const std::string why = IC_PasswordGuard(*filtered);   // 保險：重播改了可見／可改，用新的 filtered 再查一次
    if (!why.empty()) {
        IC_EvRestore();
        *err = why;
        return false;
    }
    return true;
}

// 把 TfConfiguration 替身還原成重播前（g_saveBefore）：可見、可用、分頁可見、勾選、位置、選項、文字（只改有變的）
void IC_EvRestore()
{
    if (!g_saveBefore) return;
    for (const cJSON* it = g_saveBefore->child; it; it = it->next) {
        TControl* c = it->string ? filerw::ELFind("TfConfiguration", it->string) : nullptr;
        if (!c) continue;
        const cJSON* vis = cJSON_GetObjectItemCaseSensitive(it, "visible");
        const cJSON* ena = cJSON_GetObjectItemCaseSensitive(it, "enabled");
        const cJSON* tab = cJSON_GetObjectItemCaseSensitive(it, "tabVisible");
        const cJSON* ck = cJSON_GetObjectItemCaseSensitive(it, "checked");
        const cJSON* ps = cJSON_GetObjectItemCaseSensitive(it, "position");
        const cJSON* ix = cJSON_GetObjectItemCaseSensitive(it, "itemIndex");
        const cJSON* tx = cJSON_GetObjectItemCaseSensitive(it, "text");
        if (vis && cJSON_IsBool(vis)) c->Visible = cJSON_IsTrue(vis) != 0;
        if (ena && cJSON_IsBool(ena)) c->Enabled = cJSON_IsTrue(ena) != 0;
        if (TTabSheet* t = dynamic_cast<TTabSheet*>(c)) { if (tab && cJSON_IsBool(tab)) t->TabVisible = cJSON_IsTrue(tab) != 0; }
        if (TCheckBox* x = dynamic_cast<TCheckBox*>(c)) { if (ck && cJSON_IsBool(ck)) x->Checked = cJSON_IsTrue(ck) != 0; }
        else if (TRadioButton* x = dynamic_cast<TRadioButton*>(c)) { if (ck && cJSON_IsBool(ck)) x->Checked = cJSON_IsTrue(ck) != 0; }
        else if (filerw::ELTrackBar* x = dynamic_cast<filerw::ELTrackBar*>(c)) { if (ps && cJSON_IsNumber(ps)) x->SetPosition(ps->valueint, false); }
        else if (TComboBox* x = dynamic_cast<TComboBox*>(c)) {
            if (ix && cJSON_IsNumber(ix) && x->ItemIndex != ix->valueint) x->ItemIndex = ix->valueint;
            if (tx && cJSON_IsString(tx) && tx->valuestring && std::strcmp(x->Text.c_str(), tx->valuestring) != 0) x->Text = AnsiString(tx->valuestring);
        }
        else if (TRadioGroup* x = dynamic_cast<TRadioGroup*>(c)) { if (ix && cJSON_IsNumber(ix) && x->ItemIndex != ix->valueint) x->ItemIndex = ix->valueint; }
        else if (TCustomEdit* x = dynamic_cast<TCustomEdit*>(c)) {
            if (tx && cJSON_IsString(tx) && tx->valuestring && std::strcmp(x->Text.c_str(), tx->valuestring) != 0) x->Text = AnsiString(tx->valuestring);
        }
    }
    cJSON_Delete(g_saveBefore);
    g_saveBefore = nullptr;
}

// ---- (6) 事件表上、產生器沒建替身的元件 ---------------------------------------------------------------------------------
// //AI(W906-EVB4) 20260928 [W906]：tools/gen_editlist.py 只替「golden 方法本體用到的元件＋每一層祖先」建替身與 DFM 父子；
//   btnRecordJamRateByTimeClearClick（:6691）本體沒提到自己的按鈕 ⇒ 沒有替身，RunPageEvent 會回 handler-failed
//   "internal: event proxy not created"。照 golden 補：型別 cConfiguration.h:391 TButton、DFM :13898 在 pal_O2（:13882）底下
//   （pal_O2 的祖先鏈產生器已建：pal_O2 → tsO_11 → pcO00 …）；DFM 沒有 Enabled／Visible=False ⇒ 用 VCL 預設（ELKeep 設 true）。
//   開機 FileRW_IniConfig_Boot 呼叫（IC_CreateContainerProxies 之後）。
void IC_EvCreateProxies()
{
    filerw::EL<TButton>("TfConfiguration", "btnRecordJamRateByTimeClear");
    static const char* const kPar[][2] = { {"btnRecordJamRateByTimeClear", "pal_O2"} };
    filerw::ELSetParents("TfConfiguration", kPar, 1);
    for (int i = 0; i < g_icNEvents; ++i)    // 保險：以後加列又沒有替身時，開機就講（不然要等操作員點了才知道）
        if (!filerw::ELFind("TfConfiguration", g_icEvents[i].control))
            std::printf("  !! FileRW IniConfig: form.event row %s \"%s\" has no proxy -- RunPageEvent will answer handler-failed "
                        "(add it to IC_EvCreateProxies, FileRW/IniConfig.cpp)\n", g_icEvents[i].control, g_icEvents[i].event);
}
}  // namespace

// ===========================================================================
//  AI(W906-EVB10A) 20260929 [W906]：事件批次 B10 part a CC-E10（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；
//    Steven 20260928「如果沒有移植的, 我們直接實作」、20260929「照 BCB 的邏輯」）。
//  golden TfConfiguration::sbExitClick（V912 cConfiguration.cpp:6383-6390）：
//      fShowMessage->ShowSpeed(cbG05->Checked);                    主畫面的速度訊息（[G05]）
//      fMain->sbPEModel->Visible   =cbC12->Checked;                主畫面 PE 鈕看不看得見（[C12]）
//      if(cbC12->Checked==false && bEnablePEModel==true)           [C12] 關掉而 PE 模式還開著 ⇒ 按一下 PE 鈕把它關掉
//          fMain->sbPEModelClick(this);
//      Close();                                                    → OnClose＝FormClose（:5816，YES/NO 之後存檔）
//    golden 的 Save 鈕 btnSaveClick（:7899-7903）也是 bSave=true; sbExitClick(btnSave); ⇒ 兩條路都先跑上面三句。
//  網頁：Save＝editlist.save（FileRW_IniConfig_Save 的 IC_FormClose＝golden FormClose）→ 那一行之前呼叫 FileRW_IniConfig_EvB10AExitHead；
//        Exit／✕＝外框直接關視窗（不存；golden Configuration 沒有 ✕，cConfiguration.dfm:4 BorderIcons=[]）→ 頁面表的關窗邊緣
//        （FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫 FileRW_IniConfig_WindowEdge，只跑三句（運轉中不跑：golden 模態、運轉中打不開）。
//  ⚠ 跟 golden 不同（寫明）：
//    1. 關窗時看的是伺服器端 cbG05／cbC12 替身（＝開頁或上一次存檔的值）；golden 看畫面上的勾選（Exit 之後 FormClose 問要不要存，答 NO 時
//       PE 鈕與 PE 模式已經照沒存的勾選改了）。網頁沒存的勾選伺服器看不到。
//    2. 存完再按 Exit：三句會再跑一次（golden 存檔＝關窗，不會有第二次）；三句都是冪等的（ShowSpeed 在移植樹是空殼、Visible 同值、
//       PE 已經關了就不會再按）。
//    3. fShowMessage->ShowSpeed：移植樹 forms/fShowMessage.cpp:10 是空殼（沒有畫面），網頁主畫面也沒有這一塊 —— 照呼叫。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"
#include "forms/fShowMessage.h"   // fShowMessage->ShowSpeed（golden :6385；移植樹空殼）

namespace {
evb10a::OpenLatch g_b10aOpen;     // 這一次開窗 golden FormShow 跑過（IniConfigPageJson 那一行設）
}  // namespace

void FileRW_IniConfig_EvB10AShown() { g_b10aOpen.Shown(); }

void FileRW_IniConfig_EvB10AExitHead(const char* from)
{
    const bool c12 = EL<TCheckBox>("TfConfiguration", "cbC12")->Checked;
    fShowMessage->ShowSpeed(EL<TCheckBox>("TfConfiguration", "cbG05")->Checked);   // golden cConfiguration.cpp:6385
    W906_Main_SetPEModelVisible(c12);                                          //Ifor 20170125 (Steven) add C12關閉後要將主畫面PE Mode開關 隱藏   // golden :6386
    const char* pe = "PE model untouched";
    if(c12==false && bEnablePEModel==true)                                     //Ifor 20170809 (wei) 避免PE模式開啟時被關閉功能導致異常   // golden :6387
        pe = W906_Main_PEModelClickFromConfig();                               // golden :6388 fMain->sbPEModelClick(this)
    std::printf("[EVB10A] fConfiguration sbExitClick head (%s): ShowSpeed(G05=%d), sbPEModel->Visible=%d, %s (golden V912 cConfiguration.cpp:6385-6388)\n",
                from ? from : "", (int)EL<TCheckBox>("TfConfiguration", "cbG05")->Checked, (int)c12, pe);
}

const char* FileRW_IniConfig_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: IniConfig edit lists are not booted";
    if (!g_b10aOpen.TakeForClose())
        return "not run: golden FormShow did not run in this window-open (open gate refused or the page never read) -- golden: the form never opened";
    FileRW_IniConfig_EvB10AExitHead("window closed (Exit / x)");
    return "ran golden TfConfiguration::sbExitClick head (cConfiguration.cpp:6385-6388); Close()->FormClose not run -- the web Exit does not save (Save = editlist.save)";
}

// ===========================================================================
//  AI(W906-EVB10B) 20260929 [W906]：事件批次 B10 part b（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表
//    X-3、X-5、CC-L1；Steven 20260928「任何畫面的事件, 都是我們做」、20260929「照 BCB 的邏輯」）。本體在 FileRW_IniConfig_Boot 的
//    IC_DfmState 那一行呼叫（Add 之前）。
//  (1) X-3（R118＝照 BCB）：golden HTEditList 的 Add／ReadEditTextFromFile／InitialDataToEdit 設 TCheckBox::Checked，VCL 當場跑那一格的
//      OnClick（golden Public/HTEditList.cpp:1390 一類；TCustomCheckBox.SetState）。裝上 Public/HTEditList.cpp 的 hook
//      （HTEditList_SetClickHooks）→ filerw::ELHTEditSetChecked／ELHTEditSetItemIndex：只替有登記 OnClick 的替身觸發（登記在產生的
//      IC_DfmState：cbE30 等 8 格→cbE30Click、cbE39→cbE39Click、cbD36 等 5 格→cbD36Click、cbA09／chkA09_1／cbA14→cbA09Click），
//      其他替身照舊純設值。hook 是整個 wb_serve 共用的（每一份 HTEditList），只有登記過的替身會觸發 —— 目前只有 TfConfiguration 這 17 格。
//      ⚠ 照 VCL 跑出來的結果（寫明，不是偏離）：R118 的例子（config.ini D36 與 D33 都勾）在 golden 開頁時「看得到的」畫面仍是 D33 勾著——
//        golden cprod.cpp SetCustomerLimitationForConfig 先 elConfig->ReadEditTextFromFile（:2837；逐筆「設元件→元件值寫回變數」，D36 由不勾
//        變勾時 cbD36Click 把畫面 D33 取消、D35 勾上，但 D33／D35 的變數在前幾筆已經寫好＝檔案值），同一次讀檔的 :2922 elConfig->InitialDataToEdit
//        再把 D33／D35 設回變數值（D33 沒有 OnClick）。所以 golden 開頁（FormShow :4625 ReadLastSetIni）看到的 D33 是檔案值；這裡跑的是同一串，
//        結果相同。連動只在「D36 這一次讀檔由不勾變勾」時發生，而且當場就被同一次讀檔蓋回。
//  (2) CC-L1（Q46＝照 BCB 翻切頁程式）＋X-5：PageControl1Change／pcConfigChange 讀 `->ActivePage` ⇒ 兩排分頁的頁序照 golden
//      cConfiguration.dfm（DFM 子物件順序＝VCL PageIndex）登記給 filerw::ELActivePage；同時讓 form.event 的 activePageIndex 範圍變精確
//      （ELPageIndexRefused）。
// ===========================================================================
void FileRW_IniConfig_EvB10BBoot()
{
    HTEditList_SetClickHooks(&filerw::ELHTEditSetChecked, &filerw::ELHTEditSetItemIndex);   // (1)
    static const char* const kPc1[] = {"tsSoftSimu", "tsTempComm", "tsConfig", "tsTrayData", "tsHPData"};   // golden cConfiguration.dfm:44／:126／:449／:21346／:22005（funConfig：Soft／Comm／Config／Tray／Hot Plate）
    static const char* const kPcConfig[] = {"tsA00", "tsb00", "tsC00", "tsD00", "tsE00", "tsF00", "tsG00", "tsI00", "tsL00", "tsO00",
                                            "tsN00", "tsP00", "tsM00", "tsSearchFunction"};   // golden cConfiguration.dfm:471 起（authConf[0..12]＝funConf 13 項，cAuthority.cpp:188）
    filerw::ELSetPageOrder("TfConfiguration", "PageControl1", kPc1, (int)(sizeof(kPc1) / sizeof(kPc1[0])));
    filerw::ELSetPageOrder("TfConfiguration", "pcConfig", kPcConfig, (int)(sizeof(kPcConfig) / sizeof(kPcConfig[0])));
    std::printf("[EVB10B] FileRW IniConfig: HTEditList OnClick hooks installed (VCL programmatic-set OnClick); PageControl1 / pcConfig tab order registered (CC-L1)\n");
}

// ===========================================================================
//  AI(W906-Q45-B5) 20260930 (St01)：Q45 甲 #4 [M01] 監控功能 16 格的重新登入（事件批次 B5 CC-L4，
//    D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md；設計
//    D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\q45-web-password.md §2.4、§3.2.3；
//    Steven Q45「按照你的建議執行」、20260929「請按照bcb的邏輯處理」）。
//  golden V912 cConfiguration.cpp:6531-6544 cbM01Click（16 格都綁它，cConfiguration.dfm:21162-21305）：
//      if(bM01Enter==true || fShow==false) return;   bM01Enter=true;
//      if(DoPassword()!=true) ptr->Checked=!ptr->Checked;       ← 錯了：點的那一格改回
//      gbM01->Visible=cbM01->Checked;                           bM01Enter=false;
//    DoPassword（:6482-6529）＝WebLogin.cpp 檔尾 W906_Reauth：[92]＝0 直接過、沒裝 REAL_TIME_CCD 直接過；問的時候有密碼本走
//    cbUserSelectChange 的密碼本分支、沒有走 stOperatorClick（只比下拉目前那一級）；AccessLevel < [92] 算失敗；有密碼本時問完一律
//    登出成 Operator（:6518-6525）。
//  網頁（FileRW_IniConfig_Save 在 SessionBegin 那一行呼叫：套值之後、IC_FormClose 之前）：
//    * 改過的格子＝替身現值 ≠ 開頁值（IC_OpenChecked，開頁送給頁面的那一份 proxies）。一格都沒改 ⇒ golden 沒點就沒有 DoPassword，
//      不問（頁面帶了 reauth 也不用，回應 reauth.asked=false）。
//    * [W906] Q45-2＝A：golden 每點一格問一次、每次問完都登出；這裡按存檔時問一次，涵蓋這次改過的全部格子；錯了全部改回開頁值。
//    * Q45-3＝A：登出在 golden FormClose 之前 ⇒ 開了 [A01_2]（IniConfig.bA02DisableSaveParsWhenSwitchToOp）的機台，IC_FormClose
//      （IniConfig.gen.inc「[A01_2]目前已切換到Operator權限」）照 golden 什麼都不存 —— golden 怪處（由程式推得，設計 §2.4），照翻，要通報 Jimmy。
//    * 這次存檔沒帶 reauth、golden 又會問：IC_PasswordGuard 在套值之前就整次拒存，走不到這裡；保險：W906_ReauthConfigM01 在沒答案時
//      回 false（bHandled=false），一樣把改過的格子改回（往擋得住的那側）並記 todo。
//    * gbM01->Visible=cbM01->Checked（:6542）照做。這次存檔「不可改的丟掉」在套值前已經算完：開頁時 cbM01 沒勾（gbM01 看不見），
//      同一次存檔又勾子格 ⇒ 子格這次照通用規則被丟（ack.ignored）；存完重讀 gbM01 看得見再改 —— 同 golden 要先點 cbM01 才看得到子格。
// ===========================================================================
namespace {
void IC_ReauthM01()
{
    int n = 0;
    const char* const* ids = W906_ReauthControls("m01", &n);
    std::vector<std::string> changed;
    for (int i = 0; i < n; ++i) {
        TCheckBox* c = dynamic_cast<TCheckBox*>(filerw::ELFind("TfConfiguration", ids[i]));
        if (c && c->Checked != IC_OpenChecked(ids[i])) changed.push_back(ids[i]);
    }
    if (changed.empty()) return;                                                // golden：沒點 M01 就沒有 DoPassword
    const auto revert = [](const std::string& id) {                             // golden :6540 ptr->Checked=!ptr->Checked（一格點一次＝翻一次 ⇒ 回到開頁值）
        TCheckBox* c = dynamic_cast<TCheckBox*>(filerw::ELFind("TfConfiguration", id.c_str()));
        if (c) c->Checked = IC_OpenChecked(id.c_str());
    };
    bool handled = true;
    W906_ReauthConfigM01(changed, revert, &handled);                           // golden :6538 if(DoPassword()!=true) ...
    if (!handled)
        filerw::ELTodo("golden cbM01Click DoPassword: this save carried no reauth answer, so the changed M01 cells were put back "
                       "(IC_PasswordGuard should have refused the save first)");
    EL<TGroupBox>("TfConfiguration", "gbM01")->Visible = EL<TCheckBox>("TfConfiguration", "cbM01")->Checked;   // golden :6542
}
}  // namespace
