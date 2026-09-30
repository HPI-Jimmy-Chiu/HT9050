// =============================================================================
//  WebLevelSet.cpp -- Status.Security.html 權限表（system\levelset.dat）：WS system.levels.put 的本體（存檔＋版面）。
//
//  //AI(W906-FRW-S64) 20260926: 新檔（Steven 團隊，S64「levelset.dat：SetLevelSet 的觸發者」）。
//  NOT in golden as a file。golden 本體是 V912 cSecurity.cpp 的 TfSecurity::FormClose（:439-469）與 SetLevelSet（:1511-1515），
//  移植樹逐行翻在本樹 cSecurity.cpp（同名函式）。這裡只把「操作員點 radio、按 Exit」換成 JSON，再照 golden 順序呼叫那些函式。
//  放在 wb_serve 的來源清單（同 WebSecurityJam.cpp）：要用 WebBridge 的 JsonWriter 與 cJSON。
//
//  //AI(W906-FRW-S64F) 20260927: Steven 20260927 對 todo ★ Q25～Q30 的裁決（RULINGS_20260926 S145～S149；Q30 經主 session 轉述）
//    Q25=A 每次存檔前照 golden 重算 Insufficient(29)            → (0)（S64 起就是這樣，這次沒改，只確認）
//    Q26=B 停用格子只擋那一格、其餘照存                          → (2)(3)：擋下的格子列在 ack 的 blocked[]（索引、送來的值、保留的值、原因）
//    Q27=A 這次的備份驗證通過就刪                                → (5)(8)（system\ 既有的舊備份不碰）
//    Q28=B SecurityPalVisible 重排與 5 階 radio 現在做完          → op "layout"：golden FormShow／SecurityPalVisible／TMySecurity::SetParent
//                                                                   算出來的版面給頁面排；(2) 看不到的格子（隱藏／PageControl1 不可見）也照 Q26 擋
//    Q29=A 檔案不在或大小不對拒寫（偏離 golden）                   → 檔案檢查（見下）
//    Q30=B FormClose／SecurityExitClick 接上、先修越界             → (6)(7) 改成呼叫 golden SecurityExitClick → FormClose；
//                                                                   越界保護在 cSecurity.cpp FormClose 迴圈（[W906] 標記）
//
//  為什麼要換掉舊本體（tools/wb_serve.cpp system.levels.put，20260916／0924）
//    舊版＝「讀檔 → 套網頁值 → 三條鉗制 → 稀疏改檔（BinApplyValues，自己 fopen/MoveFileEx）→ GetLevelSet 讀回」。
//    檔案是 wb_serve 自己寫的，不是 golden SetLevelSet 寫的；和 golden 的位元組差在三處：
//      (a) golden 起點是記憶體（FormShow :263 GetLevelSet 讀進來、已做 CheckRange／[163]=3／KYEC 強制值），舊版起點是檔案原值；
//          檔案裡若有 GetLevelSet 會改掉的值（超出 0..3/0..4、[163]≠3），golden 存檔會把它們一起寫成正規化後的值，舊版原樣留著。
//      (b) golden 值域＝radio 選項數（4 階 0..3、bSecurityHave5Level 5 階 0..4），舊版收聯集 0..4 —— 4 階機台送 4 會寫進檔案，
//          而 GetLevelSet 讀回記憶體又夾成 3（檔案≠記憶體）。
//      (c) golden 畫面停用的格子（FormShow :265-278：[163] 一律、CC_KYEC_LEE 的 35/104/114/128）改不動，舊版照寫。
//
//  golden 對照（V912 行號；移植樹 cSecurity.cpp 同名函式）
//    main.cpp:28592 sbPasswordClick   Insufficient(29)==false 就 return（開不了表單）          → (0) 每次存檔前重算
//    cSecurity.cpp:263 FormShow       GetLevelSet()（radio 的起始值＝正規化後的記憶體）       → (1)
//    cSecurity.cpp:265-278            [163]／KYEC 格子 SetEnabled(false)                        → GoldenFormShowPal（停用）
//    cSecurity.cpp:285-287            #ifndef SOFT_SIMULTE SecurityPalVisible()（:471-572）     → GoldenFormShowPal（隱藏）
//    cSecurity.cpp:302-381            依 AccessLevel 決定 PageControl1->Visible（整張權限表）   → GoldenPageControlVisible
//    cSecurity.cpp:383-415            可見的面板在各 ScrollBox 內重排（Top=2 起、每格 52）       → layout 的 pitch／start，頁面排
//    TMySecurity::SetParent :864-891  radio 選項 4 或 5 個（5 階 KYEC_LEE 另一組名稱）          → GoldenRadioItems
//    cSecurity.cpp:814-817 SecurityExitClick  Close() → FormClose                               → (7)
//    cSecurity.cpp:441-446 FormClose  LevelSet.AccessLevel[i]=mySecurityPal[i]->GetLevel()      → (3)（網頁的 radio 就是 LevelSet，見 (7)）
//    cSecurity.cpp:448-458            三條鉗制：[87]≥iDefSupervisorLevel、[129]≤[130]、非 CC_SIGURD_PeiXing 時 [86]≥iDefEngineerLevel
//    cSecurity.cpp:463                SaveJamLevel()   （讀新的 LevelSet.AccessLevel[35]）
//    cSecurity.cpp:464                SavePassword()   → 照 golden 呼叫（Q24=B）；設了任一測試縫（W906_LOGINDAT_PATH／W906_PWBOOK_PATH／W906_LEVELSET_PATH）才跳過，見 (7)   //AI(W906-SEC-Q24) 20260927 (St02-E)
//    cSecurity.cpp:465                SetLevelSet()    （WriteData 整塊 sizeof(LAST_LEVEL_SET)＝1024 bytes）
//    cSecurity.cpp:467                ReadPassword()   → 照 golden 呼叫（同上）
//
//  網頁版多出來的（不改 golden 寫出的位元組）
//    (0) 權限：golden 表單是 modal，開著時登入等級不會變；網頁頁面可以一直開著、中途換人登入 —— 所以每次存檔都重算 golden
//        sbPasswordClick 的 Insufficient(29)（bAlarm=false：不在存檔指令裡跳 WAR1676 停機告警，改回錯誤訊息給頁面）。
//        PageControl1 不可見的等級（golden 看不到任何一組 radio）→ 全部格子照 Q26 擋下（原因 pageControl）。
//    (5) 備份：寫檔前先把原檔 1024 bytes 抄成 levelset.dat.bak_YYYYMMDD_HHMMSS（備份失敗就整個不做）。
//        20260927 起每次真的寫都備份（FormClose 無條件 SetLevelSet；以前只在 changed>0 時備份）。
//    (8) 驗證：寫後重讀檔案，與記憶體 LevelSet 逐位元組比（verified），並確認 golden FormClose 算出來的表＝預演的表。
//        通過就刪**這次的**備份（Q27=A）；不通過就用備份還原檔案、記憶體回到舊值、備份留著、回 ok=false。
//        golden WriteData 開檔失敗會自己 ShowErrorMessage("WAR1682")。
//    dryRun：跑 (0)～(4) 算出結果後把記憶體 LevelSet 還原，什麼都不寫（引擎的「預演 → 確認 → 寫入」要靠它）。
//    檔案不存在或大小≠1024：一律拒寫（Q29=A，偏離 golden）。golden 的行為：
//        不存在 → GetLevelSet（:1477-1480）用**記憶體現值**建新檔再讀回（開機時記憶體是全 0 ＝ 每一格都是 Operator，
//                 只有 [163] 在記憶體被夾成 3；等 FormClose 才把鉗制後的值寫回）；
//        大小不對 → ReadData（cprod.cpp:1340）只讀前 1024 bytes（短檔就只讀到哪算到哪，後面維持記憶體舊值），
//                 WriteData（CREATE_ALWAYS）整塊 1024 bytes 蓋掉。
//        這裡寧可擋：缺檔時建出來的是「全部 Operator」的表；大小不對代表佈局不一致，整塊寫下去會把權限表寫壞。
//        開機 W906_SecurityBoot 仍照 golden（缺檔就建），只有存檔這條路擋。
//    測試縫：沒有。golden GetLevelSet／SetLevelSet 的路徑是字面值（cSecurity.cpp 同名函式），只轉這支的讀檔會讓
//        「備份／驗證」與「golden 寫檔」落在兩個不同的檔 —— 所以不加半套的轉向。
//
//  op（value＝JSON 字串）
//    {"values":{"<AccessLevel 索引>":<等級>,...}, "dryRun":bool}   存檔（沒有 op 或 op="put"）；稀疏：沒列的格子＝畫面上沒動的 radio
//    {"op":"layout"}                                                 唯讀：golden FormShow 會怎麼排這張表（不讀檔進記憶體、不寫任何東西）
//
//  存檔 ack 欄位（引擎 ht9045_wire_engine.js 只用 changed／notFound；其餘給人看）
//    changed    這次寫檔會改到磁碟的格數（256 格逐格比；含鉗制與正規化的效果）
//    identical  網頁送來、但與起始值（正規化後的記憶體）相同的格數
//    notFound   成功時恆為 0（非法索引／值域外會整批拒寫，回 ok=false —— 那是對照表的錯，不是 Q26 的停用格子）
//    blocked    Q26=B 擋下的格子 [{idx, sent, kept, reason, golden}]；reason＝disabled（FormShow :265-278 停用）／
//               hidden（SecurityPalVisible 隱藏，出貨組態）／pageControl（這個登入等級 golden 看不到權限表）
//    clamped    被三條鉗制改掉的索引（逗號分隔）
//    normalized 檔案原值與 GetLevelSet 正規化後不同的索引（golden 存檔會一起寫回正規化後的值）
//    jamSaved   SaveJamLevel 存了哪一筆（"<區>/<碼>"；網頁沒開過 Jam 分頁時 JamArea 空，golden 本身就 return，回 ""）
// =============================================================================
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "WebLevelSet.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fSecurity.h"
#include "cmydef.h"        // AccessLevel, CUSTOMER_CODE, CC_*, iDefSupervisorLevel／iDefEngineerLevel, SecurityPalVisible 用到的機台旗標
#include "MachineType.h"   // SOFT_SIMULTE, eSensorCCLink／eht*／ebctUninstall／eartInstall／eATCHonPrecType
#include "Config.h"        // IniConfig
#include "CosFunction.h"   // CosFunction.bSecurityHave5Level 等
#include "cprod.h"         // LAST_LEVEL_SET, LevelSet
#include "CCLink/MyCCLinkSensor_predicates.h"   // UseCanBusOrEtherCAT()（golden SecurityPalVisible :491）
#include "CMNet.h"         // G9004_M204（golden SecurityPalVisible :492；Motor/vendor/CMNet.h:20，同 cinitial.cpp 的引用方式）

extern int iMaxLevelItem;                                           // cSecurity.cpp:47（golden cSecurity.cpp:19 檔案層全域）
std::string W906_SecurityClampLevels(int* lv);   extern bool authMainForm[12]; static bool LvOpenGate(std::string* why);   // cSecurity.cpp 檔尾：golden FormClose :448-458 三條鉗制原句   //AI(W906-LVGATE) 20260930: authMainForm＝cAuthority.cpp:119（開機 GetMainAuth）；不 include cAuthority.h（會帶進 language.h，同 St01 _EditPage.cpp）；LvOpenGate＝檔尾
extern bool W906_FormCloseSkipPassword;                             // cSecurity.cpp 檔尾：FormClose 的 SavePassword／ReadPassword 開關（Q24 排後）

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp

// LAST_LEVEL_SET 佈局（檔案位元組＝記憶體位元組，golden WriteData/ReadData 沒有表頭）：
//   BCB6：int 32 位元、單一成員 int AccessLevel[256]、無填充 → 1024 bytes、AccessLevel 在位移 0（小端）。
//   移植樹（MinGW g++／MSVC）必須一樣，否則同一份 levelset.dat 兩邊讀出來會錯格 —— 編不過比靜默錯格好。
static_assert(sizeof(int) == 4, "LAST_LEVEL_SET: BCB6 int is 32-bit");
static_assert(sizeof(LAST_LEVEL_SET) == 1024, "LAST_LEVEL_SET must be 1024 bytes (BCB6 layout of int AccessLevel[256])");
static_assert(offsetof(LAST_LEVEL_SET, AccessLevel) == 0, "LAST_LEVEL_SET::AccessLevel must sit at offset 0");
static_assert(sizeof(LevelSet.AccessLevel) / sizeof(LevelSet.AccessLevel[0]) == 256, "AccessLevel[256]");

const char* W906_LevelSetPath(); namespace {   // W906_LevelSetPath：cSecurity.cpp 檔尾（R66 測試縫）

const char* const kLevelSetPath = W906_LevelSetPath();   // golden cSecurity.cpp:1476 / :1513（寫死 "d:\\HT9045\\system\\levelset.dat"）；//[W906] 20260927 測試縫 W906_LEVELSET_PATH（decisions R66），跟 cSecurity.cpp GetLevelSet／SetLevelSet 同一支，三處落在同一個檔

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

bool W906EnvSet(const char* n) { const char* e = std::getenv(n); return e != 0 && e[0] != 0; }   // 同 cSecurity.cpp 檔尾 W906_LevelSetPath() 的判斷   // Q30／Q24：skip＝true 時 golden FormClose 跳過 SavePassword／ReadPassword；離開時一定還原（例外也一樣）   //AI(W906-SEC-Q24) 20260927 (St02-E)
struct SkipPasswordGuard {
    bool old;
    explicit SkipPasswordGuard(bool skip) : old(W906_FormCloseSkipPassword) { if (skip) W906_FormCloseSkipPassword = true; }   //AI(W906-SEC-Q24) 20260927 (St02-E)
    ~SkipPasswordGuard() { W906_FormCloseSkipPassword = old; }
};

bool ReadRaw(std::string* out)
{
    FILE* fp = std::fopen(kLevelSetPath, "rb");
    if (!fp) return false;
    out->clear();
    char buf[2048];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), fp)) > 0) out->append(buf, n);
    std::fclose(fp);
    return true;
}

// 小端 int32（與 BCB6 x86 相同）；手動組裝，不依賴本機端序。
int LeI32(const std::string& b, std::size_t i)
{
    return (int)((unsigned int)(unsigned char)b[i]
               | ((unsigned int)(unsigned char)b[i + 1] << 8)
               | ((unsigned int)(unsigned char)b[i + 2] << 16)
               | ((unsigned int)(unsigned char)b[i + 3] << 24));
}

bool StrIEq(const std::string& a, const char* b)
{
    const std::size_t n = std::strlen(b);
    if (a.size() != n) return false;
    for (std::size_t i = 0; i < n; ++i) {
        char x = a[i], y = b[i];
        if (x >= 'A' && x <= 'Z') x = (char)(x - 'A' + 'a');
        if (y >= 'A' && y <= 'Z') y = (char)(y - 'A' + 'a');
        if (x != y) return false;
    }
    return true;
}

void AppendIdx(std::string* s, int i)
{
    char b[16];
    std::snprintf(b, sizeof(b), "%s%d", s->empty() ? "" : ",", i);
    *s += b;
}

// ---- golden TfSecurity 表單狀態的替身 ------------------------------------------------------------------
// 移植樹 mySecurityPal 在 GATE (SEC1)／(SEC10) 裡是空的（TMySecurity 要 TPanel／TRadioGroup／TSpeedButton）。
// 網頁只需要 golden 對每一組 radio 設的兩個屬性：看不看得到、能不能點。用同名成員函式做替身，
// 讓下面 golden 的原句（mySecurityPal[N]->SetVisible(...)／->SetEnabled(...)）一字不改就能編。
struct PalState {
    bool Visible = true;    // golden 建構子 :211-214 全部 SetVisible(true)
    bool Enabled = true;    // VCL TRadioGroup 預設 Enabled=true（FormClose :444-445 關表單時也設回 true）
    void SetVisible(bool bShow) { Visible = bShow; }            // golden cSecurity.h TMySecurity::SetVisible
    void SetEnabled(bool Enabled_) { Enabled = Enabled_; }      // golden cSecurity.h TMySecurity::SetEnabled
};
struct PalTable {
    PalState p[256];
    PalState spare;                                             // 範圍外的索引落這裡（不會發生：golden 最大 [179]）—— 不越界
    PalState* operator[](int i) { return (i >= 0 && i < 256) ? &p[i] : &spare; }
};

// golden V912 cSecurity.cpp:471-572 TfSecurity::SecurityPalVisible() —— 本體逐字（由 V912 原檔貼上，未改任何一行）
void SecurityPalVisible(PalTable& mySecurityPal)
{
    mySecurityPal[ 9]->SetVisible(true);                                        //kevin 20180917
    mySecurityPal[27]->SetVisible(CUSTOMER_CODE==CC_Greatek);                   //JimmyChiu 20211228 : InterFace Type
    mySecurityPal[33]->SetVisible(false);                                       //沒用到隱藏
    mySecurityPal[34]->SetVisible(false);                                       //沒用到隱藏
    mySecurityPal[35]->SetVisible(IniConfig.bSPILFunction==false &&             //Steven 20140430 : 矽品要求全自定  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                                  CUSTOMER_CODE!=CC_SCS);                       //jou 2015-08-27 SCS 要求 Jam Level 要可以自己選擇
    mySecurityPal[36]->SetVisible(false);                                       //沒用到隱藏
    mySecurityPal[37]->SetVisible(IniConfig.bEnableCCDUSETCPIP ||               //CCD
                                  REAL_TIME_CCD ||                              //RTC權限
                                  USE_Scanner_AOI_Inspection ||                 //Ifor 20200813 add: Scal AOI 權限
                                  USE_Top_Scanner_AOI_Inspection);              //Ifor 20200902 add: TFAMD Top AOI
    mySecurityPal[40]->SetVisible(IniConfig.bShuttleModeAccseeLevel);           //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com] //請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
    mySecurityPal[41]->SetVisible(CosFunction.bSetupFileNameControlByLevel);    //JerryYang 20160427 add 矽格北興    //Steven 20131108 Add Singapore  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction  //Steven 20150505 : Add SPIL_CHINA_SUZHOU 要求Setup File更換權限 //jou 20171011 (wei) : bSetupFileName Control By Level
    mySecurityPal[42]->SetVisible(IniConfig.bEnable_SECS_GEM);
    mySecurityPal[43]->SetVisible(IniConfig.bEnableAutoCleanFunction);
    mySecurityPal[44]->SetVisible((ATC_SYSTEM==eATCHonPrecType));
    mySecurityPal[45]->SetVisible(SHUTTLE_SENSOR_TYPE==eSensorCCLink ||
                                  SHUTTLE_SENSOR_TYPE==eSensorCCLink3 ||        //Steven 20131008 : for HT9046AH
                                  UseCanBusOrEtherCAT());                       //Sam 20230707 : EtherCAT Shuttle sensor //Steven 20191126 : 補上Canbus
    mySecurityPal[46]->SetVisible((SYN_TEK_MOTION_MODULE==G9004_M204));
    mySecurityPal[47]->SetVisible((USE_16_HEATER==eht16HeaterEJ1N ||
                                   USE_16_HEATER==eht32HeaterEJ1N ||
                                   USE_16_HEATER==eht16HeaterDTME08 ||
                                   USE_16_HEATER==eht32HeaterDTME08));          //Steven 20140923 : Index使用EJ1N版32組加熱器
    mySecurityPal[48]->SetVisible(INSTALL_OCR);                                 //Steven 20120716 : OCR
    mySecurityPal[49]->SetVisible(CosFunction.bAutoKTemp);                      //Steven 20120719 : 自動K溫
    mySecurityPal[80]->SetVisible(IniConfig.bC04EnableTestTempIC);              //Steven 20120807 : 動態溫度
    mySecurityPal[83]->SetVisible(CUSTOMER_CODE!=CC_ASE_CL);                    //Alick 20160616 ASE CL時隱藏
    mySecurityPal[84]->SetVisible(false);                                       //沒用到隱藏
    mySecurityPal[85]->SetVisible(IniConfig.bQAMode);                           //Steven 20121016 : QA Mode
    mySecurityPal[88]->SetVisible(BAR_CODE_INSTALL!=ebctUninstall);             //Steven 20121009 : Bar Code
    mySecurityPal[89]->SetVisible(USE_ROTATE_KIT==1);                           //Steven 20130626 : Rotate權限
    mySecurityPal[90]->SetVisible(USE_AIR_CONDITIONER==2);                      //Steven 20131011 : 冷氣機
    mySecurityPal[95]->SetVisible(false);                                       //Steven 20140428 : 重複的砍掉
    mySecurityPal[96]->SetVisible(CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[97]->SetVisible(IniConfig.bEnableAutoCleanFunction);          //Steven 20140519
    mySecurityPal[98]->SetVisible(IniConfig.bQAMode);                           //Steven 20121016 : QA Mode
    mySecurityPal[99]->SetVisible(IniConfig.bSocketCommunication);
    mySecurityPal[100]->SetVisible(USE_LASER_DISTANCE);
    mySecurityPal[116]->SetVisible(CosFunction.bKnockerSetBySetupFile);         //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
    mySecurityPal[110]->SetVisible(CosFunction.bUsePMAlarmFunction);            //wei 20160225 PMAlarmFunction
    mySecurityPal[111]->SetVisible(CosFunction.bUsePMAlarmFunction);            //wei 20160225 PMAlarmFunction
    mySecurityPal[112]->SetVisible(CosFunction.bUsePMAlarmFunction);            //wei 20160225 PMAlarmFunction
    mySecurityPal[113]->SetVisible(CosFunction.bUsePMAlarmFunction);            //wei 20160225 PMAlarmFunction
    mySecurityPal[117]->SetVisible(CosFunction.bYieldAlarm4);                   //wei 20160406 Yield Alarm4
    mySecurityPal[118]->SetVisible(CUSTOMER_CODE==CC_Greatek);                  //wei 20160406 Yield Alarm4
    mySecurityPal[119]->SetVisible(CUSTOMER_CODE==CC_Greatek);                  //wei 20160406 Yield Alarm4
    mySecurityPal[120]->SetVisible(CUSTOMER_CODE==CC_Greatek);                  //wei 20160406 Yield Alarm4
    mySecurityPal[121]->SetVisible(CosFunction.bYieldAlarm4);                   //Sam 20191109 YieldAlarm4 Fix //wei 20160406 Yield Alarm4
    mySecurityPal[122]->SetVisible(CUSTOMER_CODE==CC_Greatek);                  //wei 20160406 Yield Alarm4
    mySecurityPal[125]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //Alick 20160616 ASE CL 將啟用功能與計數分開,125:ON/OFF 126:Count
    mySecurityPal[126]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //Alick 20160616 ASE CL 將啟用功能與計數分開,125:ON/OFF 126:Count
    mySecurityPal[128]->SetVisible(CosFunction.bUsePEModelFunction);            //Ifor 20160822 PE Model 權限設定顯示 By CosFunction
    mySecurityPal[129]->SetVisible(CosFunction.bUseSCKART &&
                                   USE_AUTO_RETEST==eartInstall &&
                                   IniConfig.bA10_AutoReTest);                  //Steven 20161201 : For SCK 93K ART
    mySecurityPal[130]->SetVisible(CosFunction.bUseSCKART &&
                                   USE_AUTO_RETEST==eartInstall &&
                                   IniConfig.bA10_AutoReTest);                  //Steven 20161201 : For SCK 93K ART
    mySecurityPal[131]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[132]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[133]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[134]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[135]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[136]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[137]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[138]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[139]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[140]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[141]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[142]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[143]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[144]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[145]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[146]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //JerryYang 20170303 (wei) ASE_CL權限控制
    mySecurityPal[147]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);                   //Isaac 20170327 (jou) : AutoSkip權限控制
    mySecurityPal[148]->SetVisible(CosFunction.bUseYieldControlFunction);       //Steven 20170603 (wei) : Add for Korea //Ifor 20180731 (wei) : IniConfig.bKoreaFunction => CosFunction.bUseYieldControlFunction
    mySecurityPal[149]->SetVisible(IniConfig.bKoreaFunction);                   //Steven 20170603 (wei) : Add for Korea
    mySecurityPal[150]->SetVisible(CosFunction.bRecipeParameterDefault);        //Isaac 20170630 (Steven) Recipe Parameter Default權限控制
    mySecurityPal[151]->SetVisible(IniConfig.bEnableErms);                      //Steven 20170829 (wei) : ERMS Selsection新增權限控管
    mySecurityPal[152]->SetVisible(WEIGHT_CALIBRATION ||                        //Steven 20220208 : 壓力校正的權限
                                   CosFunction.bUseDynamicKitDiameter);         //JerryYang 20171102 (wei) EP壓力對應電壓校正權限
    mySecurityPal[158]->SetVisible(CUSTOMER_CODE==CC_Microchip_Phil);           //JerryYang 20181214 : 新增ATC權限管控
    mySecurityPal[159]->SetVisible(IniConfig.bC08_SocketSensor);                //Steven 20191129 : Socket sensor加入權限控制
    mySecurityPal[160]->SetVisible(CosFunction.bUnloaderEditTrayLevelSet);      //Steven 20191218 : Hana Micron要求unloaderTray編輯權限要不一樣 //Steven 20191224 : Unloader編輯Tray改用另外一組權限
    mySecurityPal[161]->SetVisible(CUSTOMER_CODE==CC_SCK);                      //Steven 20200225 : SCK想要獨立控管部分2DID功能
    mySecurityPal[162]->SetVisible(CUSTOMER_CODE==CC_TERAPOWER &&
                                   CosFunction.bUseSCKART &&
                                   USE_AUTO_RETEST==eartInstall &&
                                   IniConfig.bA10_AutoReTest);                  //Sam 20202015 : TCP ART 增加 OneCycle 可手動強制中斷流程
    mySecurityPal[163]->SetVisible(CosFunction.bUseHeadContactCount);           //Ifor 20200114 add:Life Time Edit Permission
    mySecurityPal[164]->SetVisible(CUSTOMER_CODE==CC_Murata);                   //Steven 20200629 : Murata要求可以按按鈕後停止Server功能
    mySecurityPal[165]->SetVisible(IniConfig.bD58UseArm1PickPlaceArm2Test);
    mySecurityPal[166]->SetVisible(CosFunction.bConAlarmInTimeLevelUp);         //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    mySecurityPal[167]->SetVisible(CosFunction.bI21EnableASMByRecipe);          //Isaac 20210714 : JECT要求獨立權限設定
    mySecurityPal[168]->SetVisible(CosFunction.bRecipeParameterDefault);        //Sam 20210414 : Recipe Parameter Default 功能新增權限控制
    mySecurityPal[169]->SetVisible(CosFunction.bRecipeParameterDefault);        //Sam 20210414 : Recipe Parameter Default 功能新增權限控制
    mySecurityPal[170]->SetVisible(USE_AOI_Inspection ||
                                   USE_Scanner_AOI_Inspection);                 //CCD || RTC權限
}

// golden FormShow 的權限表那兩段（V912 cSecurity.cpp:265-287）
void GoldenFormShowPal(PalTable& mySecurityPal)
{
    for(int i=0; i<iMaxLevelItem; i++)
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE &&
           (i==35  ||                                                           //Ifor 20160914 京元喬智要求 Trouble Shooting強制設定為2不可修改
            i==104 ||                                                           //Tray Edit
            i==114 ||                                                           //Ifor 20170203 (Steven) 京元喬智要求ATC Control強制設定為2不可修改
            i==128))                                                            //Ifor 20160825 add PE模式強制設定為2不可修改
        {
            mySecurityPal[i]->SetEnabled(false);
        }
        else if(i==163)                                                         //Contact Count Alarm Edit Permission
        {
            mySecurityPal[i]->SetEnabled(false);
        }
        // :279 mySecurityPal[i]->SetLevel(LevelSet.AccessLevel[i]) —— 網頁的 radio 值由引擎讀檔填，這裡不需要
    }

    #ifndef SOFT_SIMULTE
    SecurityPalVisible(mySecurityPal);                                          //Steven 20250430 : 包起來
    #else
    (void)&SecurityPalVisible;   // 模擬組態 golden 不呼叫（上一行在 #ifndef 裡）；只取位址、不執行 —— 免得 MinGW 6.3 出 -Wunused-function（它不認 [[maybe_unused]]）
    #endif
}

// golden FormShow :302-381 的 PageControl1->Visible（權限表 10 個分頁＋Jam 兩頁都在 PageControl1 裡；看不到＝一組 radio 都點不到）
// switch 以外的等級（不會出現）golden 保留上一次的值；這裡取 false，與 WebSecurityJam.cpp JamTabAllowed 一致。
bool GoldenPageControlVisible()
{
    bool Visible = false;
    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
    {
        switch(AccessLevel)
        {
            case 4:                                                             //HonPrec
                Visible=true;
                break;
            case 3:                                                             //Supervisor
                if(IniConfig.bSPILFunction==true ||                             //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                   CUSTOMER_CODE==CC_SCS)                                       //jou 2015-08-27 SCS 要求 Jam Level 要可以自己選擇
                    Visible=false;
                else
                    Visible=true;
                break;
            case 2:                                                             //Engineer
                Visible=false;
                break;
            case 1:                                                             //OP
            case 0:                                                             //Open
                Visible=false;
                break;
        }
    }
    else
    {
        switch(AccessLevel)
        {
            case 3:                                                             //HonPrec
                Visible=true;
                break;
            case 2:                                                             //Supervisor
                if(IniConfig.bSPILFunction==true ||                             //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                   CUSTOMER_CODE==CC_SCS)                                       //jou 2015-08-27 SCS 要求 Jam Level 要可以自己選擇
                    Visible=false;
                else
                    Visible=true;
                break;
            case 1:                                                             //Engineer
                Visible=false;
                break;
            case 0:                                                             //OP
                Visible=false;
                break;
        }
    }
    return Visible;
}

// golden TMySecurity::SetParent :864-891 —— 每一組 radio 的選項（Columns＝選項數；ItemIndex＝等級）
struct RadioItemsProxy { std::vector<std::string> v; void Add(const char* s) { v.push_back(s); } };
struct RadioGroupProxy { int Columns = 0; RadioItemsProxy items; RadioItemsProxy* Items = &items; };
void GoldenRadioItems(RadioGroupProxy* RadioGroup)
{
    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
    {
        RadioGroup->Columns=5;
        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //wei 20160505 增加PE權限
        {
            RadioGroup->Items->Add("Operator");
            RadioGroup->Items->Add("Engineer");
            RadioGroup->Items->Add("PEngineer");
            RadioGroup->Items->Add("Supervisor");
            RadioGroup->Items->Add("HonPrec");
        }
        else
        {
            RadioGroup->Items->Add("Open");
            RadioGroup->Items->Add("Operator");
            RadioGroup->Items->Add("Engineer");
            RadioGroup->Items->Add("Supervisor");
            RadioGroup->Items->Add("HonPrec");
        }
    }
    else
    {
        RadioGroup->Columns=4;
        RadioGroup->Items->Add("Operator");
        RadioGroup->Items->Add("Engineer");
        RadioGroup->Items->Add("Supervisor");
        RadioGroup->Items->Add("HonPrec");
    }
}

// Q26／Q28：golden 畫面上改不到這一格的原因（nullptr＝改得到）
const char* BlockReason(PalTable& pal, bool pageControlVisible, int i, const char** goldenLine)
{
    if (!pageControlVisible) { *goldenLine = "cSecurity.cpp:302-381 FormShow PageControl1->Visible=false at this AccessLevel"; return "pageControl"; }
    if (!pal[i]->Visible)    { *goldenLine = "cSecurity.cpp:471-572 SecurityPalVisible SetVisible(false) (shipping build)";   return "hidden"; }
    if (!pal[i]->Enabled)    { *goldenLine = "cSecurity.cpp:265-278 FormShow SetEnabled(false)";                               return "disabled"; }
    return nullptr;
}

struct Blocked { int idx, sent, kept; const char* reason; const char* golden; };

// op "layout"：golden FormShow 會怎麼排這張表。唯讀（不呼叫 GetLevelSet —— 它在缺檔時會建檔）。
std::string LevelSetLayout(bool* ok)
{
    PalTable pal;
    GoldenFormShowPal(pal);
    const bool pcVisible = GoldenPageControlVisible();
    RadioGroupProxy rg;
    GoldenRadioItems(&rg);
    std::string raw;
    const bool exists = ReadRaw(&raw);

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String("layout");
    w.Key("levels5").Bool(CosFunction.bSecurityHave5Level == true);
    w.Key("columns").Number((wb_int64)rg.Columns);
    w.Key("items").BeginArray();
    for (std::size_t k = 0; k < rg.items.v.size(); ++k) w.String(rg.items.v[k]);
    w.EndArray();
    w.Key("maxLevel").Number((wb_int64)rg.items.v.size() - 1);
    w.Key("levelItems").Number((wb_int64)iMaxLevelItem);
#ifndef SOFT_SIMULTE
    w.Key("palVisibleApplied").Bool(true);                          // 出貨組態：golden FormShow :285-287 會跑 SecurityPalVisible
#else
    w.Key("palVisibleApplied").Bool(false);                         // 模擬組態（SOFT_SIMULTE）：golden 不跑，全部看得到
#endif
    w.Key("hidden").BeginArray();
    for (int i = 0; i < iMaxLevelItem && i < 256; ++i) if (!pal[i]->Visible) w.Number((wb_int64)i);
    w.EndArray();
    w.Key("disabled").BeginArray();
    for (int i = 0; i < iMaxLevelItem && i < 256; ++i) if (!pal[i]->Enabled) w.Number((wb_int64)i);
    w.EndArray();
    w.Key("pageControlVisible").Bool(pcVisible);
    w.Key("allowed").Bool(fSecurity->Insufficient(29, false));   { std::string g; const bool og = LvOpenGate(&g); w.Key("openGate").Bool(og); w.Key("openGateWhy").String(g); }   // golden main.cpp:28592（存檔時會再算一次）   //AI(W906-LVGATE) 20260930: openGate＝golden 開表單的整條路（設定選單：SystemStart／[Main] Maintance／Insufficient(1)，檔尾 LvOpenGate）；allowed 照舊只看 [29]
    w.Key("accessLevel").Number((wb_int64)AccessLevel);
    w.Key("level29").Number((wb_int64)LevelSet.AccessLevel[29]);
    w.Key("pitch").Number((wb_int64)52);                             // golden FormShow :384 iPitch=52
    w.Key("start").Number((wb_int64)2);                              // golden FormShow :384 iStart=2
    w.Key("fileExists").Bool(exists);
    w.Key("fileBytes").Number((wb_int64)(exists ? raw.size() : 0));
    w.Key("fileOk").Bool(exists && raw.size() == sizeof(LAST_LEVEL_SET));
    w.Key("path").String(std::string(kLevelSetPath));
    w.Key("goldenLine").String(std::string("cSecurity.cpp:261-437 FormShow (:265-287 pal, :302-381 PageControl1, :383-415 arrange), :861-903 SetParent"));
    w.EndObject();
    if (!w.Ok()) return "json-writer-misuse";
    if (ok) *ok = true;
    return w.Str();
}

}  // namespace

std::string W906_LevelSetPut(const std::string& tag, const std::string& payloadJson, bool allowSystemWrite, bool* ok)
{
    if (ok) *ok = false;
    if (!StrIEq(tag, "levelset") && !StrIEq(tag, "levelset.dat"))
        return "not a binary system projection (try levelset)";

    // ---- payload（驗證規則沿用舊版：索引鍵純十進位、值必須是整數，不四捨五入、不 atoi 猜） ----
    cJSON* root = cJSON_Parse(payloadJson.empty() ? "" : payloadJson.c_str());
    if (!root || !cJSON_IsObject(root)) { if (root) cJSON_Delete(root); return "value is not parseable JSON"; }
    struct RootGuard { cJSON* r; ~RootGuard() { cJSON_Delete(r); } } rg{root};

    std::string op;
    const cJSON* jop = cJSON_GetObjectItemCaseSensitive(root, "op");
    if (jop && cJSON_IsString(jop) && jop->valuestring) op = jop->valuestring;
    if (op == "layout") {                                            // Q28：唯讀，不需要 --allow-system-write
        FormLockGuard lock;
        if (iMaxLevelItem <= 0)
            return "security boot has not run (iMaxLevelItem=0): W906_SecurityBoot sets golden V912 cSecurity.cpp:210";
        return LevelSetLayout(ok);
    }
    if (!op.empty() && op != "put") return "unknown op (expected put or layout): " + op.substr(0, 40);

    bool dry = false;
    const cJSON* jdry = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
    if (jdry && cJSON_IsBool(jdry)) dry = (cJSON_IsTrue(jdry) != 0);
    std::map<int, int> edits;                                       // 索引 → 網頁 radio 值
    int nOverlong = 0;                                              // 鍵超過 4 位數（atoi 會溢位）→ 算越界，整批拒寫
    const cJSON* vals = cJSON_GetObjectItemCaseSensitive(root, "values");
    if (vals && cJSON_IsObject(vals)) {
        for (const cJSON* v = vals->child; v; v = v->next) {
            if (!v->string) continue;
            const char* s = v->string;
            bool bad = (*s == 0);
            for (const char* q = s; *q && !bad; ++q) if (*q < '0' || *q > '9') bad = true;
            if (!bad && !cJSON_IsNumber(v)) bad = true;
            const double d = bad ? 0.0 : v->valuedouble;
            if (!bad && (d < -1.0e9 || d > 1.0e9)) bad = true;          // (int)d 在這範圍外是 UB
            if (!bad && d != (double)(int)d) bad = true;                // 非整數拒收
            if (bad) return "values keys must be decimal indices and values whole numbers";
            if (std::strlen(s) > 4) { ++nOverlong; continue; }
            edits[std::atoi(s)] = (int)d;
        }
    }
    if (edits.empty() && nOverlong == 0) return "no values/<index> entries in the payload";
    if (!dry && !allowSystemWrite)
        return "system writes need --allow-system-write (system\\ is shared production configuration)";

    FormLockGuard lock;
    if (iMaxLevelItem <= 0)                                          // W906_SecurityBoot 沒跑（golden 建構子 :210）
        return "security boot has not run (iMaxLevelItem=0): W906_SecurityBoot sets golden V912 cSecurity.cpp:210";

    // (0) Q25=A：golden main.cpp:28592 sbPasswordClick —— Insufficient(29)==false 就開不了表單。每次存檔（含 dryRun）當下重算，   AI(W906-LVGATE) 20260930: 下面 :519 先查 golden 開表單的整條路（檔尾 LvOpenGate），再查 [29]
    //     對的是「目前記憶體」的 AccessLevel 與 LevelSet[29]（同 golden 開表單的時間點；換人登入後下一次存檔就用新的等級）。
    { std::string g; if (!LvOpenGate(&g)) return g; }   if (fSecurity->Insufficient(29, false) == false) {   //AI(W906-LVGATE) 20260930: 前一句＝設定選單閘（運轉中／Maintance 關／等級不到 [1] 都拒存，理由回頁面）
        char b[220];
        std::snprintf(b, sizeof(b),
                      "not-authorized: golden sbPasswordClick (main.cpp:28592) needs AccessLevel >= LevelSet[29]=%d, current AccessLevel=%d; nothing written",
                      LevelSet.AccessLevel[29], AccessLevel);
        return b;
    }

    // Q29=A（偏離 golden）：檔案必須在、而且剛好 sizeof(LAST_LEVEL_SET)。在 GetLevelSet 之前檢查 —— GetLevelSet 缺檔時會自己建檔。
    std::string raw;
    if (!ReadRaw(&raw)) {
        return std::string("refused: levelset.dat 不存在或讀不到（") + kLevelSetPath + "），什麼都沒寫。"
               "golden GetLevelSet（cSecurity.cpp:1477-1480）會用記憶體現值建一份新檔；Steven 20260927 Q29=A 改成拒寫。"
               "請先把檔案放回（從備份還原）再存檔。";
    }
    if (raw.size() != sizeof(LAST_LEVEL_SET)) {
        char b[520];
        std::snprintf(b, sizeof(b),
                      "refused: levelset.dat 大小 %u bytes，golden 的結構 LAST_LEVEL_SET（int AccessLevel[256]）是 %u bytes，什麼都沒寫（%s）。"
                      "golden ReadData 會照讀前 %u bytes、WriteData 整塊蓋掉；Steven 20260927 Q29=A 改成拒寫。請先確認檔案再存檔。",
                      (unsigned)raw.size(), (unsigned)sizeof(LAST_LEVEL_SET), kLevelSetPath, (unsigned)sizeof(LAST_LEVEL_SET));
        return b;
    }

    const LAST_LEVEL_SET saved = LevelSet;                           // dryRun／拒寫時還原

    // (1) golden FormShow :263 GetLevelSet()：radio 起始值＝正規化後的記憶體
    fSecurity->GetLevelSet();
    const LAST_LEVEL_SET base = LevelSet;

    // (2) radio 能表達的東西：索引 0..iMaxLevelItem-1、值 0..(4 階 3 | 5 階 4)。
    //     超出這些＝對照表錯（整批拒寫，notFound）；golden 畫面改不到的格子（停用／隱藏／PageControl1 看不到）＝Q26=B 只擋那一格。
    PalTable pal;
    GoldenFormShowPal(pal);                                          // golden FormShow :265-287（每次存檔當下重算）
    const bool pcVisible = GoldenPageControlVisible();               // golden FormShow :302-381（登入等級可能在開頁之後才變）
    const int maxLv = (CosFunction.bSecurityHave5Level == true) ? 4 : 3;   // TMySecurity::SetParent :864-891 的選項數-1
    int nNotFound = nOverlong, nSame = 0;
    std::string badList, blockedList;
    std::vector<Blocked> blocked;
    std::vector<std::pair<int, int> > apply;
    for (std::map<int, int>::const_iterator it = edits.begin(); it != edits.end(); ++it) {
        const int i = it->first, v = it->second;
        if (i < 0 || i >= iMaxLevelItem || i >= 256 || v < 0 || v > maxLv) { ++nNotFound; if (badList.size() < 120) AppendIdx(&badList, i); continue; }
        if (v == base.AccessLevel[i]) { ++nSame; continue; }        // 沒改（含停用格子送回原值）＝不算擋
        const char* gl = "";
        const char* why = BlockReason(pal, pcVisible, i, &gl);
        if (why) { Blocked b = { i, v, base.AccessLevel[i], why, gl }; blocked.push_back(b); AppendIdx(&blockedList, i); continue; }
        apply.push_back(std::make_pair(i, v));
    }
    if (nNotFound > 0) {
        LevelSet = saved;
        char b[480];
        std::snprintf(b, sizeof(b),
                      "refused: %d value(s) out of range or index without a radio (golden TfSecurity: %d radios, levels 0..%d) [%s], nothing written",
                      nNotFound, iMaxLevelItem, maxLv, badList.c_str());
        return b;
    }

    // (3) golden FormClose :441-446 的「radio → LevelSet」：只套改得到的格子；沒送的格子＝畫面沒動＝起始值（已在 LevelSet 裡）；
    //     擋下的格子維持 base（Q26=B）。
    for (std::size_t k = 0; k < apply.size(); ++k)
        LevelSet.AccessLevel[apply[k].first] = apply[k].second;

    // (4) 預演 golden FormClose :448-458 的三條鉗制（原句在 cSecurity.cpp 檔尾 W906_SecurityClampLevels），算出這次會寫下的整張表
    LAST_LEVEL_SET preview = LevelSet;
    const std::string clamped = W906_SecurityClampLevels(&preview.AccessLevel[0]);

    // 會改到磁碟的格數（SetLevelSet 寫整塊 → 256 格逐格比）與正規化清單
    int nChanged = 0;
    std::string normalized;
    for (int i = 0; i < 256; ++i) {
        const int fileV = LeI32(raw, (std::size_t)i * 4u);
        if (fileV != preview.AccessLevel[i]) ++nChanged;
        if (fileV != base.AccessLevel[i]) AppendIdx(&normalized, i);
    }

    std::string bpath, jamSaved, restoreNote;
    bool verified = false, backupDeleted = false, formCloseMatches = false;
    if (dry) {
        LevelSet = saved;                                            // 預演：不動記憶體、不寫檔
    } else {
        // (5) 備份（網頁版多的；golden 沒有）—— 在任何 golden 寫檔之前，備份失敗就整個不做
        {
            char stamp[32] = "00000000_000000";
            const std::time_t now = std::time(0);
            const std::tm* lt = std::localtime(&now);
            if (lt) std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", lt);
            const std::string bak = std::string(kLevelSetPath) + ".bak_" + stamp;
            FILE* b = std::fopen(bak.c_str(), "wb");
            if (!b || std::fwrite(raw.data(), 1, raw.size(), b) != raw.size()) {
                if (b) std::fclose(b);
                LevelSet = saved;
                return "cannot write backup " + bak + "; nothing written";
            }
            std::fclose(b);
            bpath = bak;
        }
        if (fSecurity->JamArea != "" && fSecurity->JamCode != "")
            jamSaved = std::string(fSecurity->JamArea.c_str()) + "/" + fSecurity->JamCode.c_str();
        // (6)(7) Q30=B：golden SecurityExit 的 OnClick —— SecurityExitClick（:814-817）→ Close() → FormClose（:439-469）：
        //     :441-446 radio→LevelSet 的迴圈（移植樹 mySecurityPal 是空的 → 越界保護後 0 次；網頁的值已在 (3) 放進 LevelSet）
        //     → :448-458 三條鉗制 → :463 SaveJamLevel（用新的 [35]）→ :464 SavePassword → :465 SetLevelSet → :467 ReadPassword（Q24=B：照 golden）。
        //     //AI(W906-SEC-Q24) 20260927 (St02-E)：只有設了測試縫才跳過 SavePassword／ReadPassword —— SavePassword 用記憶體的 USER 蓋掉 d:\HT9045\system\login.dat
        //     （字面值，測試縫轉不到它），只轉走 levelset.dat 或密碼本的探測也不能碰真檔。Q9 寫 login.dat 後已把 USER 重載，這裡寫回的是同一份。
        {
            SkipPasswordGuard skipPw(W906EnvSet("W906_LOGINDAT_PATH") || W906EnvSet("W906_PWBOOK_PATH") || W906EnvSet("W906_LEVELSET_PATH"));   //AI(W906-SEC-Q24) 20260927 (St02-E) Q24=B
            // golden FormShow :279 SetLevel 的對等動作：mySecurityPal 若哪天被填了（GATE SEC1 解開），FormClose 迴圈會拿 radio 的值
            // 蓋掉 LevelSet —— 先把 radio 設成這次要存的值。今天 mySecurityPal 是空的，0 次。
            for (int i = 0; i < iMaxLevelItem && i < (int)fSecurity->mySecurityPal.size(); ++i)
                fSecurity->mySecurityPal[i]->SetLevel(LevelSet.AccessLevel[i]);
            fSecurity->SecurityExitClick(nullptr);
        }
        formCloseMatches = (std::memcmp(&LevelSet.AccessLevel[0], &preview.AccessLevel[0], sizeof(LAST_LEVEL_SET)) == 0);
        // (8) 驗證：檔案＝記憶體、而且 golden FormClose 算出來的表＝預演（操作員確認過的）。
        //     通過就刪這次的備份（Q27=A；system\ 既有的舊備份不碰）；不通過就用備份還原檔案、記憶體回到正規化後的舊值（base），備份留著給人查。
        std::string after;
        verified = formCloseMatches && ReadRaw(&after) && after.size() == sizeof(LAST_LEVEL_SET) &&
                   std::memcmp(after.data(), &LevelSet.AccessLevel[0], sizeof(LAST_LEVEL_SET)) == 0;
        if (verified) {
            backupDeleted = (std::remove(bpath.c_str()) == 0);
        } else {
            FILE* rf = std::fopen(kLevelSetPath, "wb");
            const bool restored = rf && std::fwrite(raw.data(), 1, raw.size(), rf) == raw.size();
            if (rf) std::fclose(rf);
            if (restored) LevelSet = base;
            restoreNote = restored ? "; file restored from backup, memory reverted" : "; RESTORE FAILED";
        }
    }

    std::printf("system.levels.put levelset%s: changed=%d identical=%d blocked=[%s] clamped=[%s] normalized=[%s] jam=%s verified=%d%s%s%s\n",
                dry ? " (dry)" : "", nChanged, nSame, blockedList.c_str(), clamped.c_str(), normalized.c_str(),
                jamSaved.empty() ? "-" : jamSaved.c_str(), verified ? 1 : 0,
                bpath.empty() ? "" : "  backup=", bpath.c_str(),
                bpath.empty() ? "" : (backupDeleted ? " (deleted after verify)" : " (kept)"));

    if (!dry && !verified)
        return std::string(formCloseMatches ? "SetLevelSet wrote but " : "golden FormClose produced a table different from the preview; ") +
               kLevelSetPath + (formCloseMatches ? " does not match memory (golden WriteData failure shows WAR1682)" : "") + restoreNote +
               "; backup kept at " + bpath;

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("changed").Number((wb_int64)nChanged);
    w.Key("identical").Number((wb_int64)nSame);
    w.Key("notFound").Number((wb_int64)0);
    w.Key("blockedCount").Number((wb_int64)blocked.size());
    w.Key("blocked").BeginArray();
    for (std::size_t k = 0; k < blocked.size(); ++k) {
        w.BeginObject();
        w.Key("idx").Number((wb_int64)blocked[k].idx);
        w.Key("sent").Number((wb_int64)blocked[k].sent);
        w.Key("kept").Number((wb_int64)blocked[k].kept);
        w.Key("reason").String(std::string(blocked[k].reason));
        w.Key("golden").String(std::string(blocked[k].golden));
        w.EndObject();
    }
    w.EndArray();
    w.Key("pageControlVisible").Bool(pcVisible);
    if (!bpath.empty()) w.Key("backup").String(bpath);              // 驗證通過後已刪（backupDeleted），只留路徑給記錄
    w.Key("backupDeleted").Bool(backupDeleted);
    w.Key("clamped").String(clamped);
    w.Key("normalized").String(normalized);
    w.Key("jamSaved").String(jamSaved);
    w.Key("dryRun").Bool(dry);
    w.Key("verified").Bool(verified);
    w.Key("path").String(std::string(kLevelSetPath));
    w.Key("bytes").Number((wb_int64)sizeof(LAST_LEVEL_SET));
    w.Key("levelItems").Number((wb_int64)iMaxLevelItem);
    w.Key("maxLevel").Number((wb_int64)maxLv);
    w.Key("goldenLine").String(std::string("cSecurity.cpp:814-817 SecurityExitClick -> :439-469 FormClose -> :1511 SetLevelSet"));
    w.EndObject();
    if (!w.Ok()) return "json-writer-misuse";
    if (ok) *ok = true;
    return w.Str();
}
//------------------------------------------------------------------------------
// AI(W906-LVGATE) 20260930 (St02-E): golden 開這張表單要走的整條路，存檔（含 dryRun，:519）與版面（:448 openGate）都重查。
//   同 St01 FileRW/_EditPage.cpp 的 GConfigMenu（50be588e；它在 filerw 的匿名 namespace 裡、這裡叫不到，照抄順序與文字）
//   與 C 路存檔前的 OpenGateRefused（Q41 C-1／C-2「C++ 不信任前端」），也是 Steven Q25=A（存檔前照 golden 重算）的延伸。
//   golden V912：
//     main.cpp:12952       ChangeLevelAttr  sbConfig->Enabled=(AccessLevel>=LevelSet.AccessLevel[1] && authMainForm[1])
//                          （Timer2Timer 每拍重算；運轉中 :12937-12940 設定選單鈕一律灰）
//     main.cpp:29009-29016 sbConfigClick    if(SystemStart) return;  if(fSecurity->Insufficient(1)==false) return;  → palConfig
//     main.dfm             sbPassword 在 palConfig 上；main.cpp:28589-28596 sbPasswordClick Insufficient(29) → fSecurity->ShowModal()
//                          （[29] 這一道呼叫端原本就有：:448 allowed、:519）
//     cSecurity.cpp:439-469 FormClose       存檔時什麼都不再查（golden 的閘只在開表單）
//   偏離 golden（刻意）：golden 的 modal 表單開著時機台若被啟動，關表單照樣會存；網頁頁面可以一直開著、中途換人登入或按 START，
//     所以這裡每次存檔都重查、運轉中一律拒存。   AI(W906-LVGATE-SOFTSTART) 20260930: 「運轉中」＝SystemStart || SoftStart（RULINGS_20260927 §2 Q7 A，同 FileRW/_FormEvent.cpp:77-81 的設定畫面規則；SoftStart＝正要啟動或回原點，golden 設定畫面開著時不會進入這個狀態，同一個偏離）
//   bAlarm 一律 false：不在存檔指令裡跳 WAR1676（同 :519 的 Q25、St01 _EditPage.cpp 的「偏離 1」），理由回給頁面。
//   authMainForm＝Security_new.def [Main]（golden cAuthority.cpp:366，鍵 "Maintance"，缺鍵預設 1；移植樹 cAuthority.cpp:119，
//     開機 GetMainAuth tools/wb_serve.cpp:4051）。
static bool LvOpenGate(std::string* why)
{
    if (SystemStart || SoftStart) {                                                          // golden V912 main.cpp:29012-29013   // AI(W906-LVGATE-SOFTSTART) 20260930: + SoftStart（Q7 A）
        if (why) *why = SystemStart ? "running: 機台運轉中（SystemStart）不能開設定選單（golden V912 main.cpp:29012-29013 sbConfigClick if(SystemStart) return;），權限表不能存；什麼都沒寫" : "running: 機台正要啟動或回原點（SoftStart）——這時不能存權限表；golden 在設定畫面開著時根本不會進入這個狀態（同 FileRW/_FormEvent.cpp:77-81，RULINGS_20260927 §2 Q7 A）；什麼都沒寫";
        return false;
    }
    if (!authMainForm[1]) {                                                     // golden V912 main.cpp:12952
        if (why) *why = "disabled: 設定選單的鈕在 golden 主畫面是灰的、按不到（D:\\HT9045\\config\\Security_new.def [Main] Maintance=0 => authMainForm[1] 關；golden V912 main.cpp:12952，開機 GetMainAuth 讀，缺鍵預設 1）；什麼都沒寫";
        return false;
    }
    if (fSecurity == 0 || fSecurity->Insufficient(1, false) == false) {         // golden V912 main.cpp:29015
        char b[512];
        std::snprintf(b, sizeof(b),
                      "not-authorized: 等級不足：設定選單需要 AccessLevel >= LevelSet[1]=%d，目前 AccessLevel=%d（golden V912 main.cpp:29015 sbConfigClick fSecurity->Insufficient(1)）；什麼都沒寫",
                      LevelSet.AccessLevel[1], AccessLevel);
        if (why) *why = b;
        return false;
    }
    if (why) why->clear();
    return true;
}
