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
//    export  {"from":{...}}：武裝 SaveDialog1 到暫存檔，呼叫 golden spbExportClick，把輸出檔原位元組 base64 回給瀏覽器下載（替代 SaveDialog1）。
//              ⚠ golden 副作用照留：迴圈跑完選單停在最後一區最後一碼（畫面跟著顯示那一筆）；讀的時候缺鍵會補寫預設（CheckAndReadIniData）。
//    stats   tsStatisticsJam：sgStatisticsJam 全表（唯讀；只在記憶體，golden AddJamCount 由 note.cpp:1819 累加，移植樹 fNote 尚未翻）。
//
//  權限：golden FormShow :302-381 依登入等級決定 PageControl1->Visible（整組分頁看不到＝不能改 Jam）。
//        select／save／import／export 在看不到時回 guard "not-authorized"（open／stats 照回狀態，網頁顯示原因）。
//  訊息說明（RichEditJamCode）：網頁只顯示（textarea 唯讀）；存檔時 SaveJamLevel :1201 照 golden 把「載入時的行」寫回訊息檔。
//  寫入的檔：D:\HT9045\Error\English\JAM0000.dat、D:\HT9045\Error\<語言>\<碼>.dat；暫存 %TEMP%\ht9045_jam_{import,export}.csv（用完刪）。
// =============================================================================
#include <cstdio>
#include <string>
#include <windows.h>

#include "WebSecurityJam.h"
#include "WebBridge/JsonWriter.h"
#include "WebBridge/Base64.h"
#include "Public/cJSON.h"
#include "forms/fSecurity.h"
#include "cmydef.h"        // AccessLevel, CUSTOMER_CODE, CC_*
#include "Config.h"        // IniConfig
#include "CosFunction.h"   // CosFunction

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
            if (!done) { if (!ignored.empty()) ignored += ","; ignored += k; }
        }
    }
    w.EndArray();
    w.Key("ignored").String(ignored);
}

// FileNameJam000／JamArea／JamCode 是 TfSecurity 的 private，只有 friend W906_SecurityJamOp 讀得到 —— 由它填進來
struct Priv { std::string file, jamArea, jamCode; };

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
    w.EndObject();
    std::string msg;
    for (int i = 0; i < f->RichEditJamCode->Lines->Count; ++i) { if (i) msg += "\n"; msg += S(f->RichEditJamCode->Lines->Strings[i]); }
    w.Key("message").String(msg);                                               // JsonQuote 會把 Big5／cp1252 轉 UTF-8（JsonWriter.cpp SanitizeToUtf8）
    w.Key("messageIsRtf").Bool(msg.compare(0, 5, "{\\rtf") == 0);
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
        f->ChangeJamMessage(false);                                            // :427
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
    if (op != "select" && op != "save" && op != "import" && op != "export")
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"op must be open|select|save|import|export|stats\"}";

    if (!JamTabAllowed()) {
        if (ok) *ok = true;
        return Guard(op, "not-authorized", "cSecurity.cpp:302-381",
                     "golden FormShow hides PageControl1 at this AccessLevel; the Jam tab cannot be edited", pv());
    }

    int fa = 0, fc = 0, fl = 0;
    if (!ReadSel(cJSON_GetObjectItemCaseSensitive(root, "from"), &fa, &fc, &fl))
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"from={area,code,lang} required\"}";
    std::string err;
    if (!Select(fa, fc, fl, &err)) return std::string("{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"from: ") + err + "\"}";
    f->ChangeJamMessage(false);                                                 // from 那筆從檔案讀回（網頁畫面上的就是這個）

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);

    if (op == "select" || op == "save") {
        ApplyValues(cJSON_GetObjectItemCaseSensitive(root, "values"), w);
        if (op == "save") {
            f->SaveJamLevel();                                                  // golden FormClose :463
            f->ChangeJamMessage(false);                                         // 讀回（驗證用；golden 關表單後下次 FormShow 才讀）
        } else {
            int ta = 0, tc = 0, tl = 0;
            if (!ReadSel(cJSON_GetObjectItemCaseSensitive(root, "to"), &ta, &tc, &tl))
                return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to={area,code,lang} required\"}";
            if (ta != fa) {
                if (ta < 0 || ta >= f->cbJamArea->Items->Count) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to.area out of range\"}";
                f->cbJamArea->ItemIndex = ta; f->cbJamArea->Refresh();
                f->cbJamAreaChange(nullptr);                                    // golden :952-963（碼／語言歸 0，SaveJamLevel 存 from）
                w.Key("event").String("cbJamAreaChange");
            } else if (tc != fc) {
                if (tc < 0 || tc >= f->cbJamCode->Items->Count) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to.code out of range\"}";
                f->cbJamCode->ItemIndex = tc; f->cbJamCode->Refresh();
                f->cbJamCodeChange(nullptr);                                    // golden :965-970
                w.Key("event").String("cbJamCodeChange");
            } else if (tl != fl) {
                if (tl < 0 || tl > 3) return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"to.lang out of range\"}";
                f->cbJamLang->ItemIndex = tl; f->cbJamLang->Refresh();
                f->cbJamLangChange(nullptr);                                    // golden :972-975
                w.Key("event").String("cbJamLangChange");
            } else {
                w.Key("event").String("none");                                  // 同一項：golden 不觸發 OnChange，不存
            }
        }
    } else if (op == "import") {
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
        f->spbImportClick(nullptr);                                             // golden :1517-1585
        std::remove(tmp.c_str());
        w.Key("bytes").Number((wb_int64)bytes.size());
    } else {  // export
        const std::string tmp = TempPath("ht9045_jam_export.csv");
        std::remove(tmp.c_str());
        f->SaveDialog1->FileName = tmp.c_str();
        f->SaveDialog1->W906Armed = true;                                       // ＝操作員在 SaveDialog1 按「存檔」
        f->spbExportClick(nullptr);                                             // golden :1587-1665
        std::string bytes;
        const bool got = ReadFileBytes(tmp, &bytes);
        std::remove(tmp.c_str());
        if (!got) return "{\"executed\":false,\"guard\":\"io\",\"detail\":\"golden spbExportClick wrote no file\"}";
        w.Key("fileName").String("JamCode.csv");
        w.Key("bytes").Number((wb_int64)bytes.size());
        w.Key("csvBase64").String(webbridge::Base64Encode(bytes));
    }
    w.Key("executed").Bool(true);
    WriteState(w, pv());
    w.EndObject();
    if (ok) *ok = true;
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
