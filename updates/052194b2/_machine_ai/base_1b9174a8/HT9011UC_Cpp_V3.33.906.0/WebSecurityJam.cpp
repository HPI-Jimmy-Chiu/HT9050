// =============================================================================
//  WebSecurityJam.cpp -- Status.Security.html Jam 分頁的 WS 指令 security.jam（golden V912 TfSecurity，cSecurity.cpp）。
//
//  Steven 團隊 20260926。NOT in golden as a file：golden 本體是 cSecurity.cpp 的 ChangeJamMessage（:977-1061）、
//  SaveJamLevel（:1063-1203）、spbImportClick（:1517-1585）、spbExportClick（:1587-1665）、AddAlarmList（:1672），
//  移植樹逐行翻在本樹 cSecurity.cpp（同名函式）。這裡只把「操作員點下拉選單／勾選／按鈕」換成 JSON，再呼叫那些 golden 函式。
//  放在 wb_serve 的來源清單（同 WebLotInfo.cpp）：要用 WebBridge 的 JsonWriter／Base64 與 cJSON。
//
//  為什麼不是 C 路 editlist：editlist 是「一個 struct 的欄位 ↔ 固定 ini 鍵」；Jam 分頁的 ini section／key 由 cbJamArea／cbJamCode
//  動態決定（JAM0000.dat 有 31 區 × 上千碼），外加 GetJamLevel／SaveJamLevel 的客戶碼鉗制、逐筆瀏覽、匯入匯出，沒有 struct 可綁。
//
//  op（value＝JSON 字串）
//    open    golden FormShow 的 Jam 那一半（:289-311、:417-433，見下方 op=="open" 為什麼不整支呼叫）→ 選第 0 區第 0 碼 English。
//    select  {"from":{"area":a,"code":c,"lang":l},"to":{...},"values":{...}}
//            ＝ golden「操作員改完勾選後換區／換碼／換語言」：先把 from 那筆從檔案讀回（ChangeJamMessage(false)，跟網頁看到的一樣），
//              套用 values（只套 Visible 且 Enabled 的 widget），再照 golden 事件：
//                area 不同 → cbJamArea->ItemIndex=a → cbJamAreaChange（:952，碼與語言歸 0，ChangeJamMessage() 先 SaveJamLevel 存 from）
//                code 不同 → cbJamCodeChange（:965，語言歸 0）
//                lang 不同 → cbJamLangChange（:972）
//                都一樣   → 不存（golden 下拉選同一項不觸發 OnChange）
//    save    {"from":{...},"values":{...}}＝Exit（golden FormClose :463 SaveJamLevel）。**不**跑 FormClose 其餘部分：
//              :464 SavePassword／:467 ReadPassword（login.dat，不經網頁）、:441-458/:465 LevelSet（網頁走 system.levels.put）。
//    import  {"from":{...},"csvBase64":"..."}：瀏覽器選的檔（替代 OpenDialog1，dfm:1094）原位元組寫成暫存檔，武裝 OpenDialog1，
//              呼叫 golden spbImportClick。每列寫 5 鍵：<code>、Silent、Red、Bit8（非 SCK）、UnlockPassWord（bUseAlarmUnlockPassWord）。
//    export  {"from":{...}}（from 可省）：//AI(W906-SEC-S54) 20260926: Steven S54「如果是獨立的 cpu 處理就沒關係」→ 改成背景執行緒。
//              主執行緒只做「快照」（JAM0000.dat／AlarmCodeList.txt 複製到 %TEMP%、旗標與 LevelSet[35] 抄一份），立刻回
//              {"started":true,"job":N}；背景執行緒用快照逐行照 golden spbExportClick（V912 cSecurity.cpp:1587-1665）＋
//              ChangeJamMessage（:977-1061）＋各 getter 產生同樣的 CSV（見檔尾「S54 背景匯出」為什麼不能直接呼叫 golden 函式）。
//              同時只跑一個；第二個請求回 guard "export-in-progress"。
//    exportStatus {"job":N}           進度（state running|done|failed|cancelled、areasDone／areas、rows、bytes）
//    exportChunk  {"job":N,"offset":o} done 之後分段取 CSV 原位元組（每段 96 KB raw → base64；一個 ack 塞整份會超過
//                                      WebBridgeServer maxSendBacklog 256 KiB 被斷線，且頁面 15 s ack 逾時）
//    exportRelease {"job":N}          頁面下載完通知釋放記憶體（不送也行，下一次 export 會覆蓋）
//              ⚠ 與舊版（主執行緒直接呼叫 golden spbExportClick）的差異：fSecurity 的選單**不再**停在最後一區最後一碼、
//                JAM0000.dat 缺鍵的補寫只寫到快照（不寫真檔）—— 報告決策題 S54-1／S54-2。
//              //AI(W906-SEC-S128) 20260927 (St02): Steven Q6＝B —— 匯出記下它在快照裡補的缺鍵，完成後主執行緒在 exportStatus
//                裡分段（每次最多 200 鍵）合回真的 JAM0000.dat：先備份（<檔>.<時間>.s128.bak）、只寫真檔還缺的、寫完驗證、失敗還原。
//                狀態在 exportStatus 的 export.merge。JamIniMerge.h 有做法與鎖。
//    stats   tsStatisticsJam：sgStatisticsJam 全表（唯讀；只在記憶體，golden AddJamCount 由 note.cpp:1819 累加，移植樹 fNote 尚未翻）。
//
//  權限：golden FormShow :302-381 依登入等級決定 PageControl1->Visible（整組分頁看不到＝不能改 Jam）。
//        select／save／import／export 在看不到時回 guard "not-authorized"（open／stats 照回狀態，網頁顯示原因）。
//  訊息說明（RichEditJamCode）：網頁只顯示（textarea 唯讀）；存檔時 SaveJamLevel :1201 照 golden 把「載入時的行」寫回訊息檔。
//  寫入的檔：D:\HT9045\Error\English\JAM0000.dat、D:\HT9045\Error\<語言>\<碼>.dat；暫存 %TEMP%\ht9045_jam_import.csv（用完刪）。
//  //AI(W906-SEC-S128) 20260927 (St02): open／select／save／import 與 S128 合回都在 JamIniMerge.h 的具名鎖
//  "Local\HT9045_JAM0000_dat" 裡寫 JAM0000.dat（ELA worker 也拿同一把，EventLogAnalysis/ElaCore.cpp）；最多等 2 秒，等不到回 guard "busy"。
//  //AI(W906-SEC-S54) 20260926: 匯出暫存 %TEMP%\ht9045_jam_export_<pid>.{dat,codes.txt,csv}（背景執行緒結束時刪；Ctrl-C 砍行程會留下，
//  內容只是 JAM0000.dat／AlarmCodeList.txt 的複本與 CSV，不含密碼）。
// =============================================================================
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <windows.h>
#include "WebBridge/Sync.h"   // WbThread（MinGW 6.3 win32 執行緒模型沒有 std::thread）；要在機台標頭之前（它們另外宣告了 ::Sleep 多載，放後面 WbSleepMs 會 ambiguous）   //AI(W906-SEC-S54) 20260926

#include "WebSecurityJam.h"
#include "WebBridge/JsonWriter.h"
#include "WebBridge/Base64.h"
#include "Public/cJSON.h"
#include "forms/fSecurity.h"
#include "cmydef.h"        // AccessLevel, CUSTOMER_CODE, CC_*
#include "Config.h"        // IniConfig
#include "CosFunction.h"   // CosFunction
#include "cprod.h"         // LevelSet（S54 快照抄 AccessLevel[35]）   //AI(W906-SEC-S54) 20260926
#include "vclcompat/IniFiles.h"    // worker 私有的 TIniFile（不走 common.cpp 的全域 INIFile 單例）   //AI(W906-SEC-S54) 20260926
#include "WebSecurityJamW45.h"    // S128 合回＋JAM0000.dat 具名鎖   //AI(W906-SEC-S128) 20260927   //AI(W906-ELA-W45) 20260928 (St02-E helper): includes JamIniMerge.h + the ★W45 second box

void GetJameCodeOfAxis(int iAxis, TComboBox *ComboBox);   // cMyDB.h:114（不 include cMyDB.h，理由同 cSecurity.cpp:16-28）

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

struct Box { const char* name; TCheckBox* TfSecurity::*member; };
const Box kBox[] = {                                                            // cSecurity.dfm:812-1012（id＝dfm 元件名）
    { "cbJamNeedRed",          &TfSecurity::cbJamNeedRed },
    { "cbSilentMode",          &TfSecurity::cbSilentMode },
    { "cbUnlockPassWord",      &TfSecurity::cbUnlockPassWord },
    { "cbIncludeMTBA",         &TfSecurity::cbIncludeMTBA },
    { "chkCheckContAlarm",     &TfSecurity::chkCheckContAlarm },
    { "chkO17",                &TfSecurity::chkO17 },
    { "cbAddAlarmLog",         &TfSecurity::cbAddAlarmLog },
    { "cbN27AddBoard",         &TfSecurity::cbN27AddBoard },
    { "cbN27AlarmSel",         &TfSecurity::cbN27AlarmSel },
    { "cbN27AlarmSelByArea",   &TfSecurity::cbN27AlarmSelByArea },
    { "chkTCPAlarm",           &TfSecurity::chkTCPAlarm },
    { "cbContAlarmNotUpload",  &TfSecurity::cbContAlarmNotUpload },
    { "chkAlarmAfterFullTray", &TfSecurity::chkAlarmAfterFullTray },
};
const std::size_t kBoxCount = sizeof(kBox) / sizeof(kBox[0]);

std::string S(const AnsiString& a) { return std::string(a.c_str()); }

// golden FormShow :302-381 的 PageControl1->Visible（每個寫入指令當下重算：登入等級可能在 open 之後才變）
bool JamTabAllowed()
{
    const bool spil = (IniConfig.bSPILFunction==true || CUSTOMER_CODE==CC_SCS);
    if (CosFunction.bSecurityHave5Level==true) {
        switch (AccessLevel) { case 4: return true; case 3: return !spil; default: return false; }
    }
    switch (AccessLevel) { case 3: return true; case 2: return !spil; default: return false; }
}

// 選單定位＝操作員點下拉（ItemIndex＋VCL 同步 Text；fSecurity.h TSecurityComboBox::Refresh）
bool Select(int a, int c, int l, std::string* err)
{
    TfSecurity* f = fSecurity;
    if (a < 0 || a >= f->cbJamArea->Items->Count) { *err = "area index out of range"; return false; }
    f->cbJamArea->ItemIndex = a; f->cbJamArea->Refresh();
    GetJameCodeOfAxis(a + 1, f->cbJamCode);
    if (c < 0 || c >= f->cbJamCode->Items->Count) { *err = "code index out of range"; return false; }
    f->cbJamCode->ItemIndex = c; f->cbJamCode->Refresh();
    if (l < 0 || l > 3) { *err = "lang index out of range"; return false; }
    f->cbJamLang->ItemIndex = l; f->cbJamLang->Refresh();
    return true;
}

bool ReadSel(const cJSON* o, int* a, int* c, int* l)
{
    if (!o || !cJSON_IsObject(o)) return false;
    const cJSON* ja = cJSON_GetObjectItemCaseSensitive(o, "area");
    const cJSON* jc = cJSON_GetObjectItemCaseSensitive(o, "code");
    const cJSON* jl = cJSON_GetObjectItemCaseSensitive(o, "lang");
    if (!cJSON_IsNumber(ja) || !cJSON_IsNumber(jc) || !cJSON_IsNumber(jl)) return false;
    *a = ja->valueint; *c = jc->valueint; *l = jl->valueint;
    return true;
}

// 網頁勾選 → widget（golden 操作員只能動看得見且沒反灰的元件）
void ApplyValues(const cJSON* jv, webbridge::JsonWriter& w)
{
    TfSecurity* f = fSecurity;
    w.Key("applied").BeginArray();
    std::string ignored;
    if (jv && cJSON_IsObject(jv)) {
        for (const cJSON* it = jv->child; it; it = it->next) {
            const std::string k = it->string ? it->string : "";
            bool done = false;
            for (std::size_t i = 0; i < kBoxCount; ++i) {
                if (k != kBox[i].name) continue;
                TCheckBox* b = f->*(kBox[i].member);
                if (cJSON_IsBool(it) && b->Visible && b->Enabled) { b->Checked = cJSON_IsTrue(it) ? true : false; w.String(k); done = true; }
                break;
            }
            if (!done && k == "rgJamLevel" && cJSON_IsNumber(it) && f->rgJamLevel->Visible &&
                it->valueint >= 0 && it->valueint < f->rgJamLevel->Items->Count) {
                f->rgJamLevel->ItemIndex = it->valueint; w.String(k); done = true;
            }
            if (!done && k == "rgMachineStatusBit8" && cJSON_IsNumber(it) && f->rgMachineStatusBit8->Visible &&
                it->valueint >= 0 && it->valueint <= 1) {
                f->rgMachineStatusBit8->ItemIndex = it->valueint; w.String(k); done = true;
            }
            if (!done && jamw45::Apply(k, it)) { w.String(k); done = true; }   if (!done) { if (!ignored.empty()) ignored += ","; ignored += k; }   //AI(W906-ELA-W45) 20260928: + the second box cbIncludeMTBF
        }
    }
    w.EndArray();
    w.Key("ignored").String(ignored);
}

// FileNameJam000／JamArea／JamCode 是 TfSecurity 的 private，只有 friend W906_SecurityJamOp 讀得到 —— 由它填進來
struct Priv { std::string file, jamArea, jamCode; };

void WriteExportState(webbridge::JsonWriter& w);   //AI(W906-SEC-S54) 20260926: 背景匯出的狀態（定義在檔尾 S54 區塊）

void WriteState(webbridge::JsonWriter& w, const Priv& pv)
{
    TfSecurity* f = fSecurity;
    w.Key("file").String(pv.file);
    w.Key("pageControlVisible").Bool(JamTabAllowed());
    w.Key("statisticsTabVisible").Bool(f->tsStatisticsJam->TabVisible);
    w.Key("sel").BeginObject();
    w.Key("area").Number((wb_int64)f->cbJamArea->ItemIndex);
    w.Key("code").Number((wb_int64)f->cbJamCode->ItemIndex);
    w.Key("lang").Number((wb_int64)(f->cbJamLang->ItemIndex < 0 ? 0 : f->cbJamLang->ItemIndex));
    w.Key("jamArea").String(pv.jamArea);
    w.Key("jamCode").String(pv.jamCode);
    w.EndObject();
    w.Key("codes").BeginArray();
    for (int i = 0; i < f->cbJamCode->Items->Count; ++i) w.String(S(f->cbJamCode->Items->Strings[i]));
    w.EndArray();
    w.Key("rgJamLevel").BeginObject();
    w.Key("itemIndex").Number((wb_int64)f->rgJamLevel->ItemIndex);
    w.Key("visible").Bool(f->rgJamLevel->Visible);
    w.Key("items").BeginArray();
    for (int i = 0; i < f->rgJamLevel->Items->Count; ++i) w.String(S(f->rgJamLevel->Items->Strings[i]));
    w.EndArray();
    w.EndObject();
    w.Key("rgMachineStatusBit8").BeginObject();
    w.Key("itemIndex").Number((wb_int64)f->rgMachineStatusBit8->ItemIndex);
    w.Key("visible").Bool(f->rgMachineStatusBit8->Visible);
    w.EndObject();
    w.Key("labMustCheck_35").Bool(f->labMustCheck_35->Visible);
    w.Key("btnOperator").Bool(f->btnOperator->Visible);
    w.Key("boxes").BeginObject();
    for (std::size_t i = 0; i < kBoxCount; ++i) {
        TCheckBox* b = f->*(kBox[i].member);
        w.Key(kBox[i].name).BeginObject();
        w.Key("checked").Bool(b->Checked);
        w.Key("visible").Bool(b->Visible);
        w.Key("enabled").Bool(b->Enabled);
        w.EndObject();
    }
    jamw45::WriteBox(w, pv); w.EndObject();                                    //AI(W906-ELA-W45) 20260928: + boxes.cbIncludeMTBF (the second box)
    std::string msg;
    for (int i = 0; i < f->RichEditJamCode->Lines->Count; ++i) { if (i) msg += "\n"; msg += S(f->RichEditJamCode->Lines->Strings[i]); }
    w.Key("message").String(msg);                                               // JsonQuote 會把 Big5／cp1252 轉 UTF-8（JsonWriter.cpp SanitizeToUtf8）
    w.Key("messageIsRtf").Bool(msg.compare(0, 5, "{\\rtf") == 0);
    WriteExportState(w);                                                        //AI(W906-SEC-S54) 20260926: 頁面重新整理後也知道有沒有匯出在跑（按鈕要停用）
}

std::string TempPath(const char* name)
{
    char buf[MAX_PATH + 1] = {0};
    DWORD n = ::GetTempPathA(MAX_PATH, buf);
    std::string p = (n > 0 && n < MAX_PATH) ? std::string(buf) : std::string("C:\\Windows\\Temp\\");
    return p + name;
}

bool ReadFileBytes(const std::string& path, std::string* out)
{
    FILE* fp = std::fopen(path.c_str(), "rb");
    if (!fp) return false;
    char buf[65536]; std::size_t n;
    out->clear();
    while ((n = std::fread(buf, 1, sizeof(buf), fp)) > 0) out->append(buf, n);
    std::fclose(fp);
    return true;
}

std::string Guard(const std::string& op, const char* guard, const char* golden, const std::string& detail, const Priv& pv)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("op").String(op);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(golden);
    w.Key("detail").String(detail);
    WriteState(w, pv);
    w.EndObject();
    return w.Str();
}

// =============================================================================
//  //AI(W906-SEC-S54) 20260926: S54 背景匯出（Steven「如果是獨立的 cpu 處理就沒關係」）
//
//  golden spbExportClick（V912 cSecurity.cpp:1587-1665）做的事（逐區逐碼，本機 31 區 × AlarmCodeList.txt 3079 碼）：
//    讀  D:\HT9045\Error\AlarmCodeList.txt   —— 每一區一次（GetJameCodeOfAxis，V912 cMyDB.cpp:1586-1611）
//    讀  D:\HT9045\Error\English\JAM0000.dat —— 每一碼 5～15 次（ChangeJamMessage(false) :977-1061 → 各 getter 的
//        CheckAndReadIniData；**缺鍵會補寫預設**，所以它也會寫這個檔）
//    讀  D:\HT9045\Error\English\<碼>.dat    —— 每一碼一次（RichEditJamCode 的訊息說明；CSV 用不到）
//    寫  SaveDialog1->FileName（CSV）
//    碰  fSecurity 的 cbJamArea／cbJamCode／cbJamLang／JamArea／JamCode／RichEditJamCode／13 個勾選／rgJamLevel／
//        rgMachineStatusBit8／labMustCheck_35／cbJamNeedRed->Enabled（迴圈變數就是這些 UI 門面）
//    讀  全域 CUSTOMER_CODE、SPIL_FOR_QLE、IniConfig.bSPILFunction、CosFunction 七個旗標、LevelSet.AccessLevel[35]
//
//  為什麼不能把 golden 函式原封丟到背景執行緒：
//    (1) CheckAndReadIniData 走 common.cpp 的**全域 TIniFile 單例** INIFile（OpenIniFile :561：檔名不同就 delete 再 new）。
//        主執行緒每個 ReadIniData／WriteIniData 都用同一個指標 —— 背景執行緒一切換就是 use-after-free。
//    (2) 迴圈變數是 fSecurity 的 UI 門面；主執行緒的 security.jam select／save、Command.cpp:15131 GetBit8、
//        cObserver.cpp:1883 GetJamLevel 都在同一組物件上讀寫。
//    (3) 主執行緒會寫 JAM0000.dat（security.jam save／select／import、getter 補寫預設）—— 背景邊讀邊被改寫會讀到半個檔。
//  所以：主執行緒（持 FormLock）把要讀的東西**複製一份**，背景執行緒只用複本：
//    * JAM0000.dat、AlarmCodeList.txt → CopyFileA 到 %TEMP%（point-in-time，跟 golden 單執行緒看到的一致）
//    * 旗標／CUSTOMER_CODE／SPIL_FOR_QLE／LevelSet[35]／cbJamArea 的 31 個字串／cbUnlockPassWord 起始勾選 → JamSnap
//    * 背景用**自己的** vclcompat::TIniFile 開快照（同一個類別、同一套 Win32 GetPrivateProfileString 規則，含 ValueExists 的
//      16384 bytes 視窗；補寫預設寫進快照，後面的讀取看得到 —— 與 golden 在真檔上的讀寫順序完全相同，只是換了檔）
//    * 不碰 fSecurity、不碰 WebBridge 伺服器物件；進度與結果放在 atomic／只在 state 轉成 done 之後才給主執行緒讀
//  下面的 Snap* 函式是 cSecurity.cpp（V912）同名 getter 的**逐行複本**，只把 CheckAndReadIniData(FileNameJam000,…) 換成
//  SnapRead(ini,…)、全域換成 JamSnap 欄位、拿掉只影響畫面的 labMustCheck_35／cbJamNeedRed->Enabled。
//  ⚠ golden 改了 getter／spbExportClick，這裡要跟著改（兩處）。整合測試用同一份 JAM0000.dat 比對舊路徑與新路徑的 CSV（報告第 4 點）。
// =============================================================================
struct JamSnap {
    std::string jamFile;                  // JAM0000.dat 的快照（真檔不存在時＝不存在的路徑：TIniFile 讀到預設、補寫建立快照）
    std::string codeFile;                 // AlarmCodeList.txt 的快照
    bool        codeFileExists = false;   // golden GetJameCodeOfAxis :1592 if(FileExists(FileName))
    std::string outFile;                  // MyStrList->SaveToFile 的暫存（讀回後刪）
    std::vector<std::string> areas;       // cbJamArea->Items（dfm 31 區）
    int  customerCode = 0, spilForQle = 0, level35 = 0;
    bool bSPILFunction = false;
    bool bUseAlarmUnlockPassWord = false, bIncludeMTBA = false, bUseAlarmLogXml = false, bOLPFunction = false,
         bConAlarmNeedKeyInPassword = false, bConAlarmInTimeLevelUp = false, bEnableHandlerResultServer = false;
    bool unlockChecked0 = false;          // cbUnlockPassWord->Checked 起始值：bUseAlarmUnlockPassWord=false 時 golden :1020 不更新它，CSV 印的就是它
};

enum { kJobIdle = 0, kJobRunning = 1, kJobDone = 2, kJobFailed = 3, kJobCancelled = 4 };
enum { kMergeNone = 0, kMergePending = 1, kMergeRunning = 2, kMergeDone = 3, kMergeFailed = 4 };   //AI(W906-SEC-S128) 20260927

struct JamExportJob {
    webbridge::WbThread        th;
    std::atomic<int>           state{kJobIdle};
    std::atomic<bool>          cancel{false};
    std::atomic<int>           areasDone{0};
    std::atomic<int>           rows{0};
    std::atomic<unsigned long> msElapsed{0};
    unsigned long long         id = 0;       // 主執行緒專用
    unsigned long long         nextId = 1;   // 主執行緒專用
    DWORD                      tStart = 0;   // 主執行緒寫、背景只讀（start 之前寫好）
    JamSnap                    snap;         // 主執行緒在 th.start 之前寫好；running 期間只有背景讀
    std::string                csv;          // 背景寫好之後才 state=done；主執行緒看到 done 才讀
    std::string                error;        // 同上（failed）
    //AI(W906-SEC-S128) 20260927 (St02): 快照裡補的缺鍵（背景寫、state=done 之後主執行緒才讀）與合回狀態（主執行緒專用）
    std::vector<jamini::Seed>  seeds;
    int                        mergeState = 0;   // kMerge*
    std::size_t                mergeNext = 0;
    std::string                mergeReal;        // 真的 JAM0000.dat（export 開始時抄 FileNameJam000）
    jamini::MergeResult        merge;
    ~JamExportJob() { Stop(); }              // 行程結束的保底（正常路徑是 wb_serve 關站呼叫 W906_SecurityJamShutdown）
    void Stop() { cancel = true; if (th.joinable()) th.join(); }
};
JamExportJob g_jamExport;

const char* JobStateName(int s)
{
    switch (s) { case kJobRunning: return "running"; case kJobDone: return "done"; case kJobFailed: return "failed";
                 case kJobCancelled: return "cancelled"; default: return "idle"; }
}

//AI(W906-SEC-S128) 20260927 (St02): 背景執行緒專用——SnapRead 補缺鍵時記一筆（值＝TIniFile 寫進去的字）。只有 JamExportWorker 設它。
std::vector<jamini::Seed>* g_snapSeeds = nullptr;
void RecordSeed(const AnsiString& Group, const AnsiString& Name, const AnsiString& Text)
{
    if (!g_snapSeeds) return;
    jamini::Seed sd;
    sd.group = S(Group); sd.name = S(Name); sd.value = S(Text);
    g_snapSeeds->push_back(sd);
}

// golden common.cpp:432-491 CheckAndReadIniData（int／bool／AnsiString 三個多載）—— 同一段邏輯，TIniFile 換成 worker 私有的那一個
int SnapRead(vclcompat::TIniFile& ini, const AnsiString& Group, const AnsiString& Name, int Value)
{
    if (!ini.ValueExists(Group, Name)) { ini.WriteInteger(Group, Name, Value); RecordSeed(Group, Name, AnsiString(Value)); }   // common.cpp:443 seed（S128 記下）
    else Value = ini.ReadInteger(Group, Name, Value);                         // :445
    return Value;
}
bool SnapRead(vclcompat::TIniFile& ini, const AnsiString& Group, const AnsiString& Name, bool Value)
{
    if (!ini.ValueExists(Group, Name)) { ini.WriteBool(Group, Name, Value); RecordSeed(Group, Name, AnsiString(Value ? 1 : 0)); }   // :460 seed（S128 記下）
    else Value = ini.ReadBool(Group, Name, Value);                            // :462
    return Value;
}
AnsiString SnapRead(vclcompat::TIniFile& ini, const AnsiString& Group, const AnsiString& Name, const AnsiString& Value)
{
    AnsiString Str;
    if (!ini.ValueExists(Group, Name)) { ini.WriteString(Group, Name, Value); Str = Value; RecordSeed(Group, Name, Value); }   // :478-479（S128 記下）
    else {
        Str = ini.ReadString(Group, Name, Value);                                              // :483
        if (Str == "" && Value != "") { Str = Value; ini.WriteString(Group, Name, Value); }    // :484-488
    }
    return Str;
}

// ---- cSecurity.cpp（V912）getter 的逐行複本 ----
int SnapGetJamLevel(vclcompat::TIniFile& ini, const JamSnap& s, const AnsiString& sJamArea, const AnsiString& sJamCode,
                    const AnsiString& JamCode)                                  // golden :1205-1305（JamCode＝成員；匯出迴圈裡等於 sJamCode）
{
    int iLevel=0;
    AnsiString asStr="";
    if(sJamArea!="" && sJamCode!="")
    {
        asStr=SnapRead(ini, sJamArea, sJamCode, AnsiString("0"));
        iLevel=asStr.ToIntDef(0);                                               //jou 20240607 : 修正alarm權限設定異常
        if(s.customerCode==CC_GIGAS && sJamCode=="JAM0201")                     //Richard 20220830
        {
            iLevel=0;
        }

        if(s.spilForQle==1)                                                     //KevinCheng 20260521
        {
        }
        else if(s.customerCode==CC_ASE_KaohSiung)                               //kevin 20170413 (wei) add ASE_KH
        {
            if(sJamCode=="WAR0349" || sJamCode=="WAR16132")
            {
                if(iLevel<s.level35)
                {
                    iLevel=s.level35;
                }
            }
        }
        else if(s.customerCode==CC_AMKOR_China ||
                s.customerCode==CC_QUALCOMM)
        {
            if(sJamCode=="WAR0701" || sJamCode=="JAM0301" ||
               sJamCode=="WAR0702" || sJamCode=="JAM0703" || JamCode=="WAR0705" ||
               sJamCode=="JAM0302" || sJamCode=="JAM0303" ||
               sJamCode=="JAM0304" || sJamCode=="JAM0305" ||
               sJamCode=="JAM0306" || sJamCode=="JAM0306" ||                    // golden 原樣（同一條件寫兩次）
               sJamCode=="WAR0310" || sJamCode=="WAR0343" ||
               sJamCode=="JAM0312" || sJamCode=="JAM0313" ||
               sJamCode=="JAM0314" || sJamCode=="JAM0315")
            {
                if(iLevel<s.level35)
                {
                    iLevel=s.level35;
                }
            }
        }
        else if(s.customerCode==CC_SCC)
        {
            if(sJamCode=="WAR07301" ||
               sJamCode=="WAR07321" ||
               sJamCode=="WAR07329" ||
               sJamCode=="JAM0508"  || sJamCode=="JAM0509" ||
               sJamCode=="JAM0303"  || sJamCode=="JAM0304" ||
               sJamCode=="WAR0310" )
            {
                if(iLevel<s.level35)
                {
                    iLevel=s.level35;
                }
            }
        }
        else if(s.customerCode==CC_ASE_CL)
        {
            if(sJamCode=="WAR16126" || sJamCode=="WAR1685" ||
               sJamCode.AnsiPos("WAR24")>0)
            {
                if(iLevel<1)
                {
                    iLevel=1;
                }
            }
        }
        else
        {
            if(sJamCode=="JAM0203" || sJamCode=="WAR0310" || sJamCode=="WAR0346" ||
               sJamCode=="WAR0343" || sJamCode=="JAM0301" || sJamCode=="JAM0302" ||
               sJamCode=="WAR0701" || sJamCode=="WAR0702" || sJamCode=="WAR0703" || JamCode=="WAR0705" ||
               sJamCode=="JAM0508" || sJamCode=="JAM0509" || sJamCode=="WAR07301"||
               sJamCode=="JAM0312" || sJamCode=="JAM0313" || sJamCode=="WAR0471" || sJamCode=="WAR0472" )
            {
                if(iLevel<s.level35)
                {
                    iLevel=s.level35;
                }
            }
            else if(sJamCode=="WAR16330")                                       //JerryYang 20260626 : add（V912 :1294-1300）
            {                                                                   // ⚠ 移植樹 cSecurity.cpp GetJamLevel 還是 V906 版、沒有這一臂 —— 報告第 6 點交 Jimmy
                if(iLevel<1)
                {
                    iLevel=1;
                }
            }
        }
    }
    return iLevel;
}

int SnapGetBit8(vclcompat::TIniFile& ini, const AnsiString& sJamArea, const AnsiString& sJamCode)       // golden :1307-1313
{
    int iBit8=0;
    iBit8=SnapRead(ini, sJamArea, sJamCode+" Bit8", 0);
    return iBit8;
}

bool SnapGetBool(vclcompat::TIniFile& ini, bool guard, const AnsiString& sJamArea, const AnsiString& key, bool def)
{
    bool b=false;                                                               // 各 getter 的 `bool bXxx=false; if(guard) bXxx=CheckAndReadIniData(...)`
    if(guard)
        b=SnapRead(ini, sJamArea, key, def);
    return b;
}

bool SnapGetJemIncludeMTBA(vclcompat::TIniFile& ini, const JamSnap& s, const AnsiString& sJamArea, const AnsiString& sJamCode)   // golden :1337-1366
{
    bool bIncludeMTBA=false;
    if(sJamArea!="" && sJamCode!="")
    {
        if(s.customerCode==CC_ASE_CL ||
           s.customerCode==CC_TERAPOWER)
        {
            bIncludeMTBA=SnapRead(ini, sJamArea, sJamCode+" IncludeMTBF", true);
        }
        else
        {
            if(sJamCode.Pos("JAM")>0 &&
               (sJamArea=="01 Input Arm" ||
                sJamArea=="02 Output Arm" ||
                sJamArea=="03 Index Unit" ||
                sJamArea=="04 Input Shuttle" ||
                sJamArea=="05 Output Shuttle"))
            {
                bIncludeMTBA=SnapRead(ini, sJamArea, sJamCode+" IncludeMTBA", true);
            }
            else
            {
                bIncludeMTBA=SnapRead(ini, sJamArea, sJamCode+" IncludeMTBA", false);
            }
        }
    }
    return bIncludeMTBA;
}

bool SnapGetJemRed(vclcompat::TIniFile& ini, const JamSnap& s, const AnsiString& sJamArea, const AnsiString& sJamCode)   // golden :1426-1472
{
    bool bNeedRed=false;
    if(sJamArea!="" && sJamCode!="")
    {
        bNeedRed=SnapRead(ini, sJamArea, sJamCode+" Red", false);
        if(s.bSPILFunction==true ||
           s.customerCode==CC_SCS)
        {
            ;
        }
        else if(s.customerCode==CC_AMKOR_China || s.customerCode==CC_QUALCOMM)
        {
            if(sJamCode=="WAR0701" || sJamCode=="JAM0301" ||
               sJamCode=="WAR0702" || sJamCode=="WAR0703" ||
               sJamCode=="JAM0302" || sJamCode=="JAM0303" ||
               sJamCode=="JAM0304" || sJamCode=="JAM0305" ||
               sJamCode=="JAM0306" || sJamCode=="JAM0306" ||
               sJamCode=="WAR0310" || sJamCode=="WAR0343" ||
               sJamCode=="JAM0312" || sJamCode=="JAM0313" ||
               sJamCode=="JAM0314" || sJamCode=="JAM0315")
            {
                bNeedRed=true;
            }
        }
        else if(s.customerCode==CC_SCC)
        {
            if(sJamCode=="WAR07301" ||
               sJamCode=="WAR07321" ||
               sJamCode=="WAR07329" ||
               sJamCode=="JAM0508" ||
               sJamCode=="JAM0509")
            {
                bNeedRed=true;
            }
        }
    }
    return bNeedRed;
}

// golden ChangeJamMessage(false)（V912 :977-1061）在匯出迴圈裡的效果：只留 CSV 會印的四個值＋cbUnlockPassWord，
// 其餘 getter 照 golden 的順序與條件呼叫（它們的補寫會改變快照裡後面鍵的位置，16384 bytes 視窗照樣成立）。
struct SnapRow { int level = 0, bit8 = 0; bool red = false, silent = false; };
SnapRow SnapChangeJamMessage(vclcompat::TIniFile& ini, const JamSnap& s, const AnsiString& JamArea, const AnsiString& JamCode,
                             bool* unlockChecked)
{
    SnapRow r;
    // :984-1013 訊息說明讀進 RichEditJamCode（字型、D:\HT9045\Error\<語言>\<碼>.dat）—— CSV 沒有這一欄，不讀
    r.red   =SnapGetJemRed(ini, s, JamArea, JamCode);                                                          // :1015
    r.level =SnapGetJamLevel(ini, s, JamArea, JamCode, JamCode);                                               // :1016
    r.silent=SnapGetBool(ini, JamArea!="" && JamCode!="", JamArea, JamCode+" Silent", false);                  // :1017（GetJemSilent :1315-1324）
    r.bit8  =SnapGetBit8(ini, JamArea, JamCode);                                                               // :1018
    if(s.bUseAlarmUnlockPassWord==true)                                                                        // :1020-1021（GetJemUnlockPassWord :1326-1335）
        *unlockChecked=SnapGetBool(ini, JamArea!="" && JamCode!="", JamArea, JamCode+" UnlockPassWord", false);
    if(s.bIncludeMTBA==true)                                                                                   // :1023-1024
        SnapGetJemIncludeMTBA(ini, s, JamArea, JamCode);
    if(s.bUseAlarmLogXml)                                                                                      // :1026-1031（GetN27AlarmSel :1727-1739、GetN27AddBoard :1741-1750）
    {
        SnapGetBool(ini, JamArea!="", JamArea, "bN27AlarmSel", true);                                          //   GetN27AlarmSel(JamArea, "")
        if(JamCode=="") SnapGetBool(ini, JamArea!="", JamArea, "bN27AlarmSel", true);
        else            SnapGetBool(ini, JamArea!="", JamArea, JamCode+" bN27AlarmSel", true);
        SnapGetBool(ini, JamArea!="" && JamCode!="", JamArea, JamCode+" bN27AddBoard", false);
    }
    if(s.bOLPFunction)                                                                                         // :1033-1036（GetContAlarmNotUpload :1752-1760）
        SnapGetBool(ini, JamArea!="" && JamCode!="", JamArea, JamCode+" ContAlarmNotUpload", false);
    if(s.bConAlarmNeedKeyInPassword)                                                                           // :1038-1039（GetJemContiAlarm :1368-1377）
        SnapGetBool(ini, JamArea!="" && JamCode.Pos("JAM")>0, JamArea, JamCode+" ContiAlarm", true);
    if(s.bConAlarmInTimeLevelUp)                                                                               // :1041-1042（GetO17ContiAlarm :1379-1388）
        SnapGetBool(ini, JamArea!="", JamArea, JamCode+" O17ContiAlarm", false);
    if(s.bEnableHandlerResultServer)                                                                           // :1044-1045（GetJemTCPAlarm :1401-1409）
        SnapGetBool(ini, JamArea!="", JamArea, JamCode+" TCPAlarm", false);
    SnapGetBool(ini, JamArea=="02 Output Arm", JamArea, JamCode+" AlarmAfterUnloaderFull", false);            // :1046（GetAlarmAfterUnloaderFull :1411-1419）
    if(s.customerCode==CC_PTI)                                                                                 // :1048-1060（GetAddAlarmLog :1390-1399）
    {
        if(JamCode.Pos("WAR")!=0 || JamCode.Pos("JAM")!=0)
            SnapGetBool(ini, JamArea!="", JamArea, JamCode+" GetAddAlarmLog", false);
    }
    return r;
}

// golden GetJameCodeOfAxis（V912 cMyDB.cpp:1586-1611）—— List 由呼叫端讀好一次（快照不會變，與每區重讀等價）
void SnapJamCodesOfAxis(TStringList& List, bool exists, int iAxis, std::vector<AnsiString>* ComboItems)
{
    ComboItems->clear();
    AnsiString Key, Value, Str, Axis;
    int _iAxis;
    if(exists)
    {
        for(int i=0; i<List.Count; i++)
        {
            Str=List.Strings[i];
            Key=Str.SubString(1, Str.AnsiPos("=")-1);
            Value=Str.SubString(Str.AnsiPos("=")+1, Str.Length());
            Axis=Str.SubString(4, 2);
            _iAxis=std::atoi(Axis.c_str());
            if(iAxis==_iAxis)
            {
                ComboItems->push_back(Key+"  :  "+Value);
            }
        }
    }
}

void JamExportCleanup(const JamSnap& s)
{
    ::DeleteFileA(s.jamFile.c_str());
    ::DeleteFileA(s.codeFile.c_str());
    ::DeleteFileA(s.outFile.c_str());
}

// 背景執行緒本體：golden spbExportClick（V912 :1587-1665）。只碰 JamSnap、自己的 TIniFile／TStringList、暫存檔。
void JamExportWorker(void* arg)
{
    JamExportJob* j = static_cast<JamExportJob*>(arg);
    const JamSnap& s = j->snap;
    j->seeds.clear();
    g_snapSeeds = &j->seeds;                                                    //AI(W906-SEC-S128) 20260927: SnapRead 記缺鍵
    struct SinkOff { ~SinkOff() { g_snapSeeds = nullptr; } } sinkOff;
    try {
        vclcompat::TIniFile ini(AnsiString(s.jamFile.c_str()));
        TStringList codeList;
        if (s.codeFileExists) codeList.LoadFromFile(AnsiString(s.codeFile.c_str()));
        TStringList MyStrList;
        AnsiString CommaText;
        AnsiString OldJamCode="";
        bool unlockChecked = s.unlockChecked0;
        std::vector<AnsiString> codes;

        if(s.customerCode==CC_SCK)                                              // :1601-1608
        {
            CommaText.sprintf("JamArea, JamCode, JamLevel, JamNeedRed, SilentMode, UnlockPassWord, Message");
        }
        else
        {
            CommaText.sprintf("JamArea, JamCode, JamLevel, JamNeedRed, SilentMode, UnlockPassWord, MachineStatusBit8, Message");
        }
        MyStrList.Add(CommaText);

        for(std::size_t i=0; i<s.areas.size(); i++)                             // :1611 至少要有輸出一次
        {
            const AnsiString JamArea(s.areas[i].c_str());                       // :1613-1614 cbJamArea->ItemIndex=i; Refresh() → Text
            SnapJamCodesOfAxis(codeList, s.codeFileExists, (int)i+1, &codes);   // :1615
            for(std::size_t k=0; k<codes.size(); k++)                           // :1617
            {
                if (j->cancel) { JamExportCleanup(s); j->state = kJobCancelled; return; }
                const AnsiString& Text = codes[k];                              // :1619-1622 cbJamCode->ItemIndex=j; cbJamLang->ItemIndex=0
                AnsiString JamCode=Text.SubString(1, Text.AnsiPos("  :")-1);    // :1624
                AnsiString Message=Text.SubString(Text.AnsiPos(":")+1, Text.Length());   // :1625
                const SnapRow r = SnapChangeJamMessage(ini, s, JamArea, JamCode, &unlockChecked);   // :1626 ChangeJamMessage(false)

                if(OldJamCode!=JamCode)                                         // :1628
                {
                    OldJamCode=JamCode;
                    if(s.customerCode==CC_SCK)                                  // :1631-1641
                    {
                        CommaText.sprintf("\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\"",
                                           JamArea,
                                           JamCode,
                                           r.level,
                                           (r.red)?1:0,
                                           (r.silent)?1:0,
                                           (unlockChecked)?1:0,
                                           Message);
                    }
                    else                                                        // :1642-1653
                    {
                        CommaText.sprintf("\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\"",
                                           JamArea,
                                           JamCode,
                                           r.level,
                                           (r.red)?1:0,
                                           (r.silent)?1:0,
                                           (unlockChecked)?1:0,
                                           (r.bit8)?1:0,
                                           Message);
                    }
                    MyStrList.Add(CommaText);
                    ++j->rows;
                }
            }
            ++j->areasDone;
        }

        MyStrList.SaveToFile(AnsiString(s.outFile.c_str()));                    // :1659（SaveDialog1->FileName → 暫存，讀回給瀏覽器下載）
        std::string bytes;
        const bool got = ReadFileBytes(s.outFile, &bytes);
        JamExportCleanup(s);
        j->msElapsed = (unsigned long)(::GetTickCount() - j->tStart);
        if (!got) { j->error = "CSV temp file could not be read back"; j->state = kJobFailed; return; }
        j->csv.swap(bytes);
        j->mergeState = j->seeds.empty() ? kMergeNone : kMergePending;         //AI(W906-SEC-S128) 20260927: 完成的匯出才合回（取消／失敗不合）
        j->state = kJobDone;                                                    // 之後主執行緒才讀 csv（與 seeds／mergeState）
    } catch (...) {
        JamExportCleanup(s);
        j->error = "exception in the export worker";
        j->state = kJobFailed;
    }
}

//AI(W906-SEC-S128) 20260927 (St02): Steven Q6＝B —— 主執行緒把快照補的缺鍵合回真檔。每次呼叫最多 kMergeChunk 鍵（toEnd＝一次做完），
//  鎖最多等 2 秒（ELA worker 只在單一鍵的查＋寫拿鎖）；等不到就留在原狀態，下一次 exportStatus 再試。回 true＝已結束（done／failed）。
const std::size_t kMergeChunk = 200;
const char* MergeStateName(int s)
{
    switch (s) { case kMergePending: return "pending"; case kMergeRunning: return "running"; case kMergeDone: return "done";
                 case kMergeFailed: return "failed"; default: return "none"; }
}
bool MergeStep(JamExportJob& j, bool toEnd)
{
    if (j.mergeState != kMergePending && j.mergeState != kMergeRunning) return true;
    jamini::Lock lk(2000);
    if (!lk.held()) { j.merge.error = "busy: JAM0000.dat lock held (event log analyzer); retrying"; return false; }
    j.merge.error.clear();                                                      // a busy note from an earlier poll is stale now
    if (j.mergeState == kMergePending) {
        if (!jamini::Begin(j.mergeReal, jamini::BackupPathFor(j.mergeReal), &j.merge)) {
            j.mergeState = kMergeFailed;
            std::printf("security.jam S128 merge (job %lu) failed: %s\n", (unsigned long)j.id, j.merge.error.c_str());
            return true;
        }
        j.mergeState = kMergeRunning;
        j.mergeNext = 0;
    }
    do {
        const std::size_t n = (j.seeds.size() - j.mergeNext < kMergeChunk) ? (j.seeds.size() - j.mergeNext) : kMergeChunk;
        jamini::Apply(j.mergeReal, j.seeds, j.mergeNext, n, &j.merge);
        j.mergeNext += n;
    } while (toEnd && j.mergeNext < j.seeds.size());
    if (j.mergeNext < j.seeds.size()) return false;
    j.mergeState = jamini::Verify(j.mergeReal, j.seeds, &j.merge) ? kMergeDone : kMergeFailed;
    std::printf("security.jam S128 merge (job %lu): %d key(s) written, %d already there%s%s\n", (unsigned long)j.id,
                j.merge.written, j.merge.skipped, j.mergeState == kMergeFailed ? " -- FAILED, restored: " : "",
                j.mergeState == kMergeFailed ? j.merge.error.c_str() : "");
    return true;
}

void WriteExportState(webbridge::JsonWriter& w)
{
    JamExportJob& j = g_jamExport;
    const int st = j.state;
    w.Key("export").BeginObject();
    w.Key("job").Number((wb_int64)j.id);
    w.Key("state").String(JobStateName(st));
    w.Key("areasDone").Number((wb_int64)j.areasDone.load());
    w.Key("areas").Number((wb_int64)(st == kJobIdle ? 0 : j.snap.areas.size()));
    w.Key("rows").Number((wb_int64)j.rows.load());
    if (st == kJobRunning) w.Key("elapsedMs").Number((wb_int64)(::GetTickCount() - j.tStart));
    else if (st != kJobIdle) w.Key("elapsedMs").Number((wb_int64)j.msElapsed.load());
    if (st == kJobDone) { w.Key("bytes").Number((wb_int64)j.csv.size()); w.Key("fileName").String("JamCode.csv"); }
    if (st == kJobFailed) w.Key("error").String(j.error);
    if (st == kJobDone) {                                                       //AI(W906-SEC-S128) 20260927: 合回真檔的進度
        w.Key("merge").BeginObject();
        w.Key("state").String(MergeStateName(j.mergeState));
        w.Key("total").Number((wb_int64)j.seeds.size());
        w.Key("applied").Number((wb_int64)j.mergeNext);
        w.Key("written").Number((wb_int64)j.merge.written);
        w.Key("skipped").Number((wb_int64)j.merge.skipped);
        w.Key("backup").String(j.merge.backup);
        w.Key("createdFile").Bool(j.merge.createdFile);
        w.Key("error").String(j.merge.error);
        w.EndObject();
    }
    w.EndObject();
}

std::string TempPathPid(const char* suffix)
{
    char name[96];
    std::snprintf(name, sizeof(name), "ht9045_jam_export_%lu%s", (unsigned long)::GetCurrentProcessId(), suffix);
    return TempPath(name);
}

}  // namespace

std::string W906_SecurityJamOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    FormLockGuard lock;
    TfSecurity* f = fSecurity;

    cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
    if (!root) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"value is not JSON\"}";
    struct RootGuard { cJSON* r; ~RootGuard() { cJSON_Delete(r); } } rg{root};
    const cJSON* jo = cJSON_GetObjectItemCaseSensitive(root, "op");
    const std::string op = (jo && cJSON_IsString(jo) && jo->valuestring) ? jo->valuestring : "";
    auto pv = [f]() { Priv p; p.file = S(f->FileNameJam000); p.jamArea = S(f->JamArea); p.jamCode = S(f->JamCode); return p; };

    if (f->FileNameJam000 == "") {                                              // W906_SecurityJamBoot 沒跑（golden :217）
        if (ok) *ok = true;
        return Guard(op, "boot-not-run", "cSecurity.cpp:217", "W906_SecurityJamBoot did not run; FileNameJam000 is empty", pv());
    }

    if (op == "open") {
        jamini::Lock jamLock(2000);                                             //AI(W906-SEC-S128) 20260927: golden getter 會補寫 JAM0000.dat
        if (!jamLock.held()) {
            if (ok) *ok = true;
            return Guard(op, "busy", "JamIniMerge.h", "JAM0000.dat is being written by the event log analyzer; retry", pv());
        }
        // golden FormShow（:261-437）的 Jam 那一半。**不能**直接呼叫 f->FormShow()：W906_SecurityBoot 把 iMaxLevelItem 設成 180
        // （Insufficient 查表用），但 mySecurityPal 在 GATE (SEC1) 仍是空的，FormShow :265-280 的 mySecurityPal[i]->SetEnabled 會越界
        // （20260926 實測 wb_serve segfault）。權限表那一半網頁走 /api/system/levelset，:313-381 的按鈕可見性由 JamTabAllowed() 算，
        // :383-415 是版面排列。以下逐行照 golden：
        f->cbUnlockPassWord->Visible   =CosFunction.bUseAlarmUnlockPassWord;   // :289
        f->cbIncludeMTBA->Visible      =CosFunction.bIncludeMTBA;              // :290
        f->chkCheckContAlarm->Visible  =CosFunction.bConAlarmNeedKeyInPassword;// :291
        f->chkO17->Visible             =CosFunction.bConAlarmInTimeLevelUp;    // :292
        f->cbAddAlarmLog->Visible      =CUSTOMER_CODE==CC_PTI;                 // :294
        f->cbN27AlarmSel->Visible      =CosFunction.bUseAlarmLogXml;           // :296
        f->cbN27AlarmSelByArea->Visible=CosFunction.bUseAlarmLogXml;           // :297
        f->cbN27AddBoard->Visible      =(CosFunction.bUseAlarmLogXml &&
                                         CUSTOMER_CODE==CC_SIGURD_ChungXing);   // :298-299
        f->chkTCPAlarm->Visible        =CosFunction.bEnableHandlerResultServer;// :300
        f->cbContAlarmNotUpload->Visible=CosFunction.bOLPFunction;             // :301
        if(CosFunction.bSecurityHave5Level==true)                              // :302-311
        {
            if(f->rgJamLevel->Columns<=4)
            {
                f->rgJamLevel->Columns=5;
                if(CUSTOMER_CODE==CC_KYEC_LEE)
                    f->rgJamLevel->Items->Insert(0, "Operator");
                else
                    f->rgJamLevel->Items->Insert(0, "Open");
            }
        }
        f->cbJamNeedRed->Visible=(IniConfig.bAlarmMustRedColor==true);         // :417
        f->cbJamArea->ItemIndex=0;                                             // :418
        f->cbJamArea->Refresh();
        GetJameCodeOfAxis(f->cbJamArea->ItemIndex+1, f->cbJamCode);            // :420
        f->cbJamCode->ItemIndex=0;
        f->cbJamCode->Refresh();
        f->cbJamLang->ItemIndex=0;
        f->cbJamLang->Refresh();
        f->JamArea=f->cbJamArea->Text;                                         // :425
        f->JamCode=f->cbJamCode->Text.SubString(1, f->cbJamCode->Text.AnsiPos("  :")-1);
        f->ChangeJamMessage(false); jamw45::Load(pv());                        // :427   //AI(W906-ELA-W45) 20260928: + the second box (Analyzer.cpp:2753)
        f->btnOperator->Visible=CosFunction.bSecurityHave5Level;               // :429
        f->rgMachineStatusBit8->Visible=(CUSTOMER_CODE!=CC_SCK);               // :430
        f->chkAlarmAfterFullTray->Visible=(CosFunction.bNeedAlarmAfterUnloaderFull &&
                                           f->cbJamArea->Text=="02 Output Arm");// :431-432
        f->fShow=true;                                                         // :433
        webbridge::JsonWriter w;
        w.BeginObject(); w.Key("executed").Bool(true); w.Key("op").String(op); WriteState(w, pv()); w.EndObject();
        if (ok) *ok = true;
        return w.Str();
    }
    if (op == "stats") {
        webbridge::JsonWriter w;
        w.BeginObject(); w.Key("executed").Bool(true); w.Key("op").String(op);
        w.Key("tabVisible").Bool(f->tsStatisticsJam->TabVisible);
        w.Key("rows").BeginArray();
        if (f->tsStatisticsJam->TabVisible) {
            for (int r = 0; r < f->sgStatisticsJam->RowCount; ++r) {
                w.BeginArray();
                for (int c = 0; c < f->sgStatisticsJam->ColCount; ++c) w.String(S(f->sgStatisticsJam->Cells[c][r]));
                w.EndArray();
            }
        }
        w.EndArray(); w.EndObject();
        if (ok) *ok = true;
        return w.Str();
    }
    //AI(W906-SEC-S54) 20260926: 背景匯出的查詢三支（不改 fSecurity、不需要 from）
    if (op == "exportStatus" || op == "exportChunk" || op == "exportRelease") {
        JamExportJob& j = g_jamExport;
        const cJSON* jj = cJSON_GetObjectItemCaseSensitive(root, "job");
        const unsigned long long want = (jj && cJSON_IsNumber(jj) && jj->valuedouble >= 0) ? (unsigned long long)jj->valuedouble : 0ULL;
        if (want == 0 || want != j.id)
            return "{\"executed\":false,\"guard\":\"no-such-job\",\"detail\":\"job id does not match the current export (a newer export replaced it, or none was started)\"}";
        const int st = j.state;
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("op").String(op);
        if (op == "exportStatus") {
            if (st == kJobDone) MergeStep(j, false);                            //AI(W906-SEC-S128) 20260927: 主執行緒分段合回
            w.Key("executed").Bool(true);
            WriteExportState(w);
        } else if (op == "exportRelease") {
            if (st == kJobRunning) return "{\"executed\":false,\"guard\":\"export-in-progress\",\"detail\":\"the export is still running\"}";
            if (st == kJobDone && !MergeStep(j, true))                          //AI(W906-SEC-S128) 20260927: 釋放前把合回做完
                return "{\"executed\":false,\"guard\":\"busy\",\"detail\":\"the JAM0000.dat merge is waiting for the lock; retry\"}";
            std::string().swap(j.csv);                                          // 背景已結束（state!=running）→ 主執行緒可以動 csv
            j.state = kJobIdle;                                                 // 之後 exportChunk 回 not-done，不會回一份空的 CSV
            w.Key("executed").Bool(true);
        } else {  // exportChunk：done 之後分段取（每段 96 KB raw；base64 後約 128 KB，低於 maxSendBacklog 256 KiB）
            if (!JamTabAllowed()) {                                             // 內容＝Jam 分頁的設定；golden 看不到分頁的等級不給
                if (ok) *ok = true;
                return Guard(op, "not-authorized", "cSecurity.cpp:302-381",
                             "golden FormShow hides PageControl1 at this AccessLevel; the Jam export cannot be fetched", pv());
            }
            if (st != kJobDone) return "{\"executed\":false,\"guard\":\"not-done\",\"detail\":\"export is not finished (poll exportStatus)\"}";
            const cJSON* jo2 = cJSON_GetObjectItemCaseSensitive(root, "offset");
            const double offD = (jo2 && cJSON_IsNumber(jo2)) ? jo2->valuedouble : 0.0;
            if (offD < 0 || offD > (double)j.csv.size()) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"offset out of range\"}";
            const std::size_t off = (std::size_t)offD, kChunk = 96u * 1024u;
            const std::size_t len = (j.csv.size() - off < kChunk) ? (j.csv.size() - off) : kChunk;
            w.Key("executed").Bool(true);
            w.Key("job").Number((wb_int64)j.id);
            w.Key("fileName").String("JamCode.csv");
            w.Key("offset").Number((wb_int64)off);
            w.Key("length").Number((wb_int64)len);
            w.Key("total").Number((wb_int64)j.csv.size());
            w.Key("last").Bool(off + len >= j.csv.size());
            w.Key("csvBase64").String(webbridge::Base64Encode(reinterpret_cast<const unsigned char*>(j.csv.data()) + off, len));
        }
        w.EndObject();
        if (ok) *ok = true;
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    if (op != "select" && op != "save" && op != "import" && op != "export")
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"op must be open|select|save|import|export|exportStatus|exportChunk|exportRelease|stats\"}";

    if (!JamTabAllowed()) {
        if (ok) *ok = true;
        return Guard(op, "not-authorized", "cSecurity.cpp:302-381",
                     "golden FormShow hides PageControl1 at this AccessLevel; the Jam tab cannot be edited", pv());
    }

    if (op == "export") {
        //AI(W906-SEC-S54) 20260926: 主執行緒只做快照（毫秒級：兩次 CopyFileA＋抄旗標），然後交給背景執行緒。
        //  不再 Select(from)／ChangeJamMessage：golden 的迴圈把選單當迴圈變數，背景版不碰 fSecurity（見檔尾 S54 區塊）。
        JamExportJob& j = g_jamExport;
        if (j.state == kJobRunning) {                                           // 同時只允許一個
            if (ok) *ok = true;
            char d[96]; std::snprintf(d, sizeof(d), "an export is already running (job %lu)", (unsigned long)j.id);
            return Guard(op, "export-in-progress", "Steven S54", d, pv());
        }
        if (j.th.joinable()) j.th.join();                                       // 上一個背景已結束（state!=running），收掉 handle
        if (j.state == kJobDone && !MergeStep(j, true)) {                       //AI(W906-SEC-S128) 20260927: 上一次的合回先做完
            if (ok) *ok = true;
            return Guard(op, "busy", "Steven Q6 B", "the previous export's JAM0000.dat merge is waiting for the lock; retry", pv());
        }
        JamSnap s;
        s.jamFile  = TempPathPid(".dat");
        s.codeFile = TempPathPid(".codes.txt");
        s.outFile  = TempPathPid(".csv");
        JamExportCleanup(s);                                                    // 上一次被砍掉留下的同名暫存
        if (FileExists(f->FileNameJam000) && !::CopyFileA(f->FileNameJam000.c_str(), s.jamFile.c_str(), FALSE))
            return "{\"executed\":false,\"guard\":\"io\",\"detail\":\"cannot snapshot JAM0000.dat to %TEMP%\"}";
        const AnsiString codeList = "D:\\HT9045\\Error\\AlarmCodeList.txt";       // golden GetJameCodeOfAxis 的字面值（V912 cMyDB.cpp:1589）
        s.codeFileExists = FileExists(codeList);
        if (s.codeFileExists && !::CopyFileA(codeList.c_str(), s.codeFile.c_str(), FALSE)) {
            JamExportCleanup(s);
            return "{\"executed\":false,\"guard\":\"io\",\"detail\":\"cannot snapshot AlarmCodeList.txt to %TEMP%\"}";
        }
        for (int i = 0; i < f->cbJamArea->Items->Count; ++i) s.areas.push_back(S(f->cbJamArea->Items->Strings[i]));
        s.customerCode               = CUSTOMER_CODE;
        s.spilForQle                 = SPIL_FOR_QLE;
        s.level35                    = LevelSet.AccessLevel[35];
        s.bSPILFunction              = IniConfig.bSPILFunction;
        s.bUseAlarmUnlockPassWord    = CosFunction.bUseAlarmUnlockPassWord;
        s.bIncludeMTBA               = CosFunction.bIncludeMTBA;
        s.bUseAlarmLogXml            = CosFunction.bUseAlarmLogXml;
        s.bOLPFunction               = CosFunction.bOLPFunction;
        s.bConAlarmNeedKeyInPassword = CosFunction.bConAlarmNeedKeyInPassword;
        s.bConAlarmInTimeLevelUp     = CosFunction.bConAlarmInTimeLevelUp;
        s.bEnableHandlerResultServer = CosFunction.bEnableHandlerResultServer;
        s.unlockChecked0             = f->cbUnlockPassWord->Checked;
        j.snap = s;
        j.csv.clear(); j.error.clear();
        j.seeds.clear(); j.mergeState = kMergeNone; j.mergeNext = 0; j.merge = jamini::MergeResult();   //AI(W906-SEC-S128) 20260927
        j.mergeReal = S(f->FileNameJam000);
        j.cancel = false; j.areasDone = 0; j.rows = 0; j.msElapsed = 0;
        j.id = j.nextId++;
        j.tStart = ::GetTickCount();
        j.state = kJobRunning;
        if (!j.th.start(&JamExportWorker, &j)) {
            j.state = kJobFailed; j.error = "CreateThread failed";
            JamExportCleanup(j.snap);
            return "{\"executed\":false,\"guard\":\"io\",\"detail\":\"CreateThread failed\"}";
        }
        std::printf("security.jam export job %lu started in the background (%u areas)\n", (unsigned long)j.id, (unsigned)j.snap.areas.size());
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("executed").Bool(true);
        w.Key("op").String(op);
        w.Key("started").Bool(true);
        w.Key("job").Number((wb_int64)j.id);
        WriteState(w, pv());
        w.EndObject();
        if (ok) *ok = true;
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    int fa = 0, fc = 0, fl = 0;
    if (!ReadSel(cJSON_GetObjectItemCaseSensitive(root, "from"), &fa, &fc, &fl))
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"from={area,code,lang} required\"}";
    std::string err;
    if (!Select(fa, fc, fl, &err)) return std::string("{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"from: ") + err + "\"}";
    jamini::Lock jamLock(2000);                                                 //AI(W906-SEC-S128) 20260927: select／save／import 都寫 JAM0000.dat
    if (!jamLock.held()) {
        if (ok) *ok = true;
        return Guard(op, "busy", "JamIniMerge.h", "JAM0000.dat is being written by the event log analyzer; retry", pv());
    }
    f->ChangeJamMessage(false); jamw45::Load(pv());                             // from 那筆從檔案讀回（網頁畫面上的就是這個）   //AI(W906-ELA-W45) 20260928: + the second box

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);

    if (op == "select" || op == "save") {
        ApplyValues(cJSON_GetObjectItemCaseSensitive(root, "values"), w);
        if (op == "save") {
            f->SaveJamLevel(); jamw45::Save();                                  // golden FormClose :463   //AI(W906-ELA-W45) 20260928: + the second box (Analyzer.cpp:2772)
            f->ChangeJamMessage(false); jamw45::Load(pv());                     // 讀回（驗證用；golden 關表單後下次 FormShow 才讀）   //AI(W906-ELA-W45) 20260928
        } else {
            int ta = 0, tc = 0, tl = 0;
            if (!ReadSel(cJSON_GetObjectItemCaseSensitive(root, "to"), &ta, &tc, &tl))
                return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to={area,code,lang} required\"}";
            if (ta != fa) {
                if (ta < 0 || ta >= f->cbJamArea->Items->Count) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to.area out of range\"}";
                f->cbJamArea->ItemIndex = ta; f->cbJamArea->Refresh();
                f->cbJamAreaChange(nullptr); jamw45::SaveThenLoad(pv());        // golden :952-963（碼／語言歸 0，SaveJamLevel 存 from）   //AI(W906-ELA-W45) 20260928: box 2 of from, then of to
                w.Key("event").String("cbJamAreaChange");
            } else if (tc != fc) {
                if (tc < 0 || tc >= f->cbJamCode->Items->Count) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to.code out of range\"}";
                f->cbJamCode->ItemIndex = tc; f->cbJamCode->Refresh();
                f->cbJamCodeChange(nullptr); jamw45::SaveThenLoad(pv());        // golden :965-970   //AI(W906-ELA-W45) 20260928: box 2 of from, then of to
                w.Key("event").String("cbJamCodeChange");
            } else if (tl != fl) {
                if (tl < 0 || tl > 3) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to.lang out of range\"}";
                f->cbJamLang->ItemIndex = tl; f->cbJamLang->Refresh();
                f->cbJamLangChange(nullptr); jamw45::SaveThenLoad(pv());        // golden :972-975   //AI(W906-ELA-W45) 20260928: box 2 of from, then of to
                w.Key("event").String("cbJamLangChange");
            } else {
                w.Key("event").String("none");                                  // 同一項：golden 不觸發 OnChange，不存
            }
        }
    } else {  // import（export 已在上面交給背景執行緒）
        const cJSON* jb = cJSON_GetObjectItemCaseSensitive(root, "csvBase64");
        std::string bytes;
        if (!jb || !cJSON_IsString(jb) || !jb->valuestring || !webbridge::Base64Decode(jb->valuestring, &bytes))
            return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"csvBase64 required\"}";
        const std::string tmp = TempPath("ht9045_jam_import.csv");
        FILE* fp = std::fopen(tmp.c_str(), "wb");
        if (!fp) return "{\"executed\":false,\"guard\":\"io\",\"detail\":\"cannot write temp import file\"}";
        std::fwrite(bytes.data(), 1, bytes.size(), fp); std::fclose(fp);
        f->OpenDialog1->FileName = tmp.c_str();
        f->OpenDialog1->W906Armed = true;                                       // ＝操作員在 OpenDialog1 按「開啟」
        f->spbImportClick(nullptr); jamw45::Load(pv());                         // golden :1517-1585   //AI(W906-ELA-W45) 20260928: re-read box 2 (the CSV has no MTBA / MTBF column, W20-8 照 golden)
        std::remove(tmp.c_str());
        w.Key("bytes").Number((wb_int64)bytes.size());
    }
    w.Key("executed").Bool(true);
    WriteState(w, pv());
    w.EndObject();
    if (ok) *ok = true;
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

//AI(W906-SEC-S54) 20260926: wb_serve 關站時收掉背景匯出（取消旗標每一碼檢查一次，join 在數十毫秒內返回）。
//  插入點見 WebSecurityJam.h。沒插也不會 std::terminate：WbThread 解構只 CloseHandle，g_jamExport 解構會 cancel＋join（保底）。
void W906_SecurityJamShutdown()
{
    JamExportJob& j = g_jamExport;
    const bool was = (j.state == kJobRunning);
    j.Stop();
    if (was) std::printf("security.jam export job %lu cancelled at shutdown\n", (unsigned long)j.id);
}
