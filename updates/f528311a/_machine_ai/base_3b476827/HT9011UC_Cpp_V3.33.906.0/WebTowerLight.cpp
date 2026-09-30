// =============================================================================
//  WebTowerLight.cpp -- Status.TowerLight.html 的 WS 指令 towerlight.op（golden V912 TfTowerLight，cTowerLight.cpp）
//
//  AI(W906-TOWERLIGHT) 20260925 (Steven 團隊)。NOT in golden as a file：golden 本體在 forms/fTowerLight.cpp
//  （FormShow／FormClose／RGB00Click 逐行翻 V912 cTowerLight.cpp；行號一律 V912）；這裡只是 WS 的 JSON 包裝，
//  放在 wb_serve 的來源清單（同 WebLotInfo.cpp）：它要用 WebBridge 的 JsonWriter／cJSON，ht9045_forms 不連 ht9045_webbridge。
//
//  分派：tools/wb_serve.cpp 的 towerlight.op 一臂（寫在 counterclear.* 那一行的前面，不移動行號）。
//  網頁（web/page/ht9045_towerlight_wire.js）送 towerlight.op，value = JSON 字串。
//
//  開窗路徑（golden 的閘，每個 op 都重查一次，Insufficient(n,false) 不跳 WAR1676，理由回給頁面）
//    main.cpp:29009-29016 sbConfigClick：SystemStart → return；fSecurity->Insufficient(1)==false → return
//    main.cpp:28579-28586 sbTowerLightClick：fSecurity->Insufficient(28)==false → return；
//                         NewRecordProcess("MES2183","Enter Tower Light")（移植樹是空 shim）；ShowModal → FormShow
//
//  op
//    "get"    = golden FormShow（cTowerLight.cpp:83-127）：MusicSelect[0..7] 鉗制 0～4（寫記憶體，golden 同）→ 8 個下拉框、
//               Panel11 顯示與否（:103-120）、palART 標題、cbOffLine 反灰（:122-123 SPIL）→ 回整份畫面狀態
//    "click"  {"led":"RGB12"} = golden RGB00Click（:55-81）：A01_2 權限（:57-63）→ i=Tag/3、j=Tag%3 → 超出 iOfflineRun
//               （7；SPIL 為 5，:70-75）就 return → MessageLight[i][j] 0→1→2→0 → WriteLastDataFile()（:79）
//    "music"  {"combo":"cbJam","index":2} = 操作員在開著的表單上換一個下拉框再關窗：
//               FormShow（:83-127，先把 8 個框對回目前的 LastSet）→ 設該框 ItemIndex → FormClose（:129-140，
//               8 個框 → LastSet.MusicSelect[i]）→ WriteLastDataFile()
//               ⚠ 決斷 D3：golden FormClose 本身不寫檔，MusicSelect 要等下一次 WriteLastDataFile 才落地
//                 （例 main.cpp:11861 TfMain::FormClose、:13642 UpdateMainOperateMode、RGB00Click :79）。
//                 移植樹那些呼叫點不一定走得到，照 Steven「Setup 的項目實際上都必須要進行讀寫」這裡立刻寫一次。
//
//  fShow（決斷 D2）
//    golden FormShow 設 fShow=true、FormClose 設 false；ShowRunLed（ckernel.cpp，golden ckernel.cpp:790）在 fShow 為 true 時
//    **完全不碰蜂鳴器**（讓 rgMusicTest 試聽）。網頁沒有可靠的關窗事件（分頁直接關掉、斷線都收不到），
//    fShow 留在 true 會讓整台機台的警報音樂停在當下狀態。所以每個 op 呼叫完 golden FormShow／FormClose 之後
//    把 fShow、Timer1->Enabled 還原成呼叫前的值。fShow 的正式來源是視窗狀態總表（WebWindowRegistry.h，P6-b），
//    本檔不接。試聽（rgMusicTestClick :142-147，SW[SwMusic1..4] 實體輸出）不接，見 forms/fTowerLight.cpp GATE (T-3)。
//
//  移植樹加的拒絕（golden 的操作員點不到，不是 golden 的判斷）
//    P1 led／combo 只收 dfm 的 24／8 個元件名
//    P2 index 只收 0～4（dfm Items 5 項；VCL csDropDown 打字可得 -1，網頁 <select> 做不到）
//    P3 看不見（Panel11->Visible==false 時的 RGB70～72、cbART）或反灰（cbOffLine，SPIL）的元件 → not-visible／not-enabled
//
//  寫入
//    click／music：D:\HT9045\system\lastdata.dat、lastdata_backup.dat（WriteLastDataFile 寫死路徑，sizeof(LAST_GENERAL_SET) 整塊，
//      OPEN_EXISTING 不截斷），以及它檔尾的 config.ini 計數鍵（golden cprod.cpp WriteLastDataFile 同一函式）。
// =============================================================================
#include <cstddef>
#include <cstdio>
#include <string>

#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fTowerLight.h"
#include "forms/fSecurity.h"   // fSecurity->Insufficient (cSecurity.cpp, ht9045_sm)
#include "LastSet.h"           // LastSet / LAST_GENERAL_SET
#include "cprod.h"             // WriteLastDataFile
#include "Config.h"            // IniConfig.bA02DisableSaveParsWhenSwitchToOp / bSPILFunction
#include "cmydef.h"            // SystemStart / AccessLevel / RunState
#include "canary_support.h"    // W906_ShowMyMessage_Count / W906_ShowMyMessage_LastS1

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:149-150
// cMyDB.h:129（本體 acatchtray_shims.cpp:152，空 shim）。同 WebBuilder.cpp：不 include cMyDB.h（與 canary_support.h 預設引數衝突）
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);

extern AnsiString W906_ShowMyMessage_LastS1;   // canary_support.cpp:67
extern int        W906_ShowMyMessage_Count;    // canary_support.cpp:68

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

// golden FormShow/FormClose :85-86 的順序（MusicSelect[i] 對應的下拉框）
const char* const kCombo[8] = { "cbRunning", "cbJam", "cbPause", "cbMessage", "cbHeating", "cbHome", "cbOffLine", "cbART" };
// dfm 左側列名（Panel4..9、Panel1、palART 的 Caption）
const char* const kRowName[8] = { "Running", "Error/Jam", "Pause", "Message", "Heating", "Homing", "OffLine Running", "Auto Retest" };

TComboBox* Combo(int i)
{
    TfTowerLight* f = fTowerLight;
    TComboBox* const p[8] = { f->cbRunning, f->cbJam, f->cbPause, f->cbMessage, f->cbHeating, f->cbHome, f->cbOffLine, f->cbART };
    return p[i];
}

TfTowerLightLed* Led(int i, int j)
{
    TfTowerLight* f = fTowerLight;
    TfTowerLightLed* const p[8][3] = {
        { f->RGB00, f->RGB01, f->RGB02 }, { f->RGB10, f->RGB11, f->RGB12 }, { f->RGB20, f->RGB21, f->RGB22 },
        { f->RGB30, f->RGB31, f->RGB32 }, { f->RGB40, f->RGB41, f->RGB42 }, { f->RGB50, f->RGB51, f->RGB52 },
        { f->RGB60, f->RGB61, f->RGB62 }, { f->RGB70, f->RGB71, f->RGB72 },
    };
    return p[i][j];
}

// 呼叫 golden FormShow／FormClose 時保住 fShow／Timer1（決斷 D2，見檔頭）
struct KeepShowFlag {
    bool show, timer;
    KeepShowFlag() : show(fTowerLight->fShow), timer(fTowerLight->Timer1->Enabled) {}   //AI(W906-FSHOW-B2) 20260929: 存／還原成員，不問頁面表（問了還原時會把網頁答案寫進成員）
    ~KeepShowFlag() { fTowerLight->fShow = show; fTowerLight->Timer1->Enabled = timer; }
};

struct Guard { const char* guard; const char* goldenLine; const char* detail; };
const Guard* RouteGuard()
{
    static const Guard kRun  = { "SystemStart", "golden V912 main.cpp:29012-29013 sbConfigClick",
        "機台運轉中不能開 Tower Light（sbTowerLight 在 palConfig 裡，sbConfigClick 運轉中直接 return）" };
    static const Guard kLv1  = { "not-authorized", "golden V912 main.cpp:29015 sbConfigClick fSecurity->Insufficient(1)",
        "權限不足（[1] Config，system\\levelset.dat）" };
    static const Guard kLv28 = { "not-authorized", "golden V912 main.cpp:28582 sbTowerLightClick fSecurity->Insufficient(28)",
        "權限不足（[28] Config - Tower Light，cSecurity.cpp:102）" };
    if (SystemStart) return &kRun;
    if (fSecurity == 0 || fSecurity->Insufficient(1, false) == false) return &kLv1;
    if (fSecurity->Insufficient(28, false) == false) return &kLv28;
    return 0;
}

std::string Refuse(const std::string& op, const char* guard, const std::string& golden, const std::string& detail)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("op").String(op);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(golden);
    w.Key("detail").String(detail);
    w.EndObject();
    return w.Str();
}

int OfflineRunLimit()   // golden RGB00Click :70-72
{
    int iOfflineRun = 7;
    if (IniConfig.bSPILFunction == true) iOfflineRun = 5;
    return iOfflineRun;
}

void WriteState(webbridge::JsonWriter& w)
{
    TfTowerLight* f = fTowerLight;
    const int lim = OfflineRunLimit();
    w.Key("messageLight").BeginArray();
    for (int i = 0; i < 8; ++i) {
        w.BeginArray();
        for (int j = 0; j < 3; ++j) w.Number((wb_int64)LastSet.MessageLight[i][j]);
        w.EndArray();
    }
    w.EndArray();
    w.Key("musicSelect").BeginArray();
    for (int i = 0; i < 8; ++i) w.Number((wb_int64)LastSet.MusicSelect[i]);
    w.EndArray();
    w.Key("rows").BeginArray();
    for (int i = 0; i < 8; ++i) w.String(kRowName[i]);
    w.EndArray();
    w.Key("widgets").BeginObject();
    for (int i = 0; i < 8; ++i) {
        TComboBox* c = Combo(i);
        const bool vis = c->Visible && (i != 7 || f->Panel11->Visible);
        w.Key(kCombo[i]).BeginObject();
        w.Key("itemIndex").Number((wb_int64)c->ItemIndex);
        w.Key("enabled").Bool(c->Enabled);
        w.Key("visible").Bool(vis);
        w.EndObject();
    }
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 3; ++j) {
            TfTowerLightLed* l = Led(i, j);
            char name[8];
            std::snprintf(name, sizeof(name), "RGB%d%d", i, j);
            w.Key(name).BeginObject();
            w.Key("tag").Number((wb_int64)l->Tag);
            w.Key("value").Number((wb_int64)LastSet.MessageLight[i][j]);   // 0 滅／1 亮／2 閃（golden UpdateTowerLed :48-50）
            w.Key("visible").Bool(l->Visible && (i != 7 || f->Panel11->Visible));
            w.Key("clickChanges").Bool(i <= lim);                            // 超出 iOfflineRun：golden :74-75 直接 return
            w.EndObject();
        }
    }
    w.Key("Panel11").BeginObject().Key("visible").Bool(f->Panel11->Visible).EndObject();
    w.Key("palART").BeginObject().Key("caption").String(std::string(f->palART->Caption.c_str())).EndObject();
    w.Key("rgMusicTest").BeginObject().Key("itemIndex").Number((wb_int64)f->rgMusicTest->ItemIndex)
        .Key("enabled").Bool(false)
        .Key("reason").String("試聽＝SW[SwMusic1..4] 實體輸出（golden rgMusicTestClick :142-147），網頁不接（forms/fTowerLight.cpp GATE (T-3)）")
        .EndObject();
    w.EndObject();
    w.Key("iOfflineRun").Number((wb_int64)lim);
    w.Key("a02OperatorLock").Bool(IniConfig.bA02DisableSaveParsWhenSwitchToOp == true && AccessLevel == 0);
    w.Key("accessLevel").Number((wb_int64)AccessLevel);
    w.Key("runState").Number((wb_int64)RunState);                            // ShowRunLed 目前用哪一列（ckernel.cpp）
    w.Key("lastdata").BeginObject();
    w.Key("file").String("D:\\HT9045\\system\\lastdata.dat");
    w.Key("sizeof").Number((wb_int64)sizeof(LAST_GENERAL_SET));
    w.Key("offsetMessageLight").Number((wb_int64)offsetof(LAST_GENERAL_SET, MessageLight));
    w.Key("sizeMessageLight").Number((wb_int64)sizeof(LastSet.MessageLight));
    w.Key("offsetMusicSelect").Number((wb_int64)offsetof(LAST_GENERAL_SET, MusicSelect));
    w.Key("sizeMusicSelect").Number((wb_int64)sizeof(LastSet.MusicSelect));
    w.EndObject();
}

std::string Finish(webbridge::JsonWriter& w, bool* ok)
{
    if (ok) *ok = w.Ok();
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

}  // namespace

std::string W906_TowerLightOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;

    std::string op, led, combo;
    int index = -999;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0)
            return Refuse("", "bad-payload", "", "value 不是合法 JSON（要 {\"op\":\"get\"}／{\"op\":\"click\",\"led\":\"RGB12\"}／{\"op\":\"music\",\"combo\":\"cbJam\",\"index\":2}）");
        const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, "op");
        if (j && cJSON_IsString(j) && j->valuestring) op = j->valuestring;
        j = cJSON_GetObjectItemCaseSensitive(root, "led");
        if (j && cJSON_IsString(j) && j->valuestring) led = j->valuestring;
        j = cJSON_GetObjectItemCaseSensitive(root, "combo");
        if (j && cJSON_IsString(j) && j->valuestring) combo = j->valuestring;
        j = cJSON_GetObjectItemCaseSensitive(root, "index");
        if (j && cJSON_IsNumber(j) && j->valuedouble == (double)(int)j->valuedouble) index = (int)j->valuedouble;
        cJSON_Delete(root);
    }
    if (op != "get" && op != "click" && op != "music")
        return Refuse(op, "bad-payload", "", "op 只有 get／click／music");
    if (fTowerLight == 0)
        return Refuse(op, "bad-payload", "", "fTowerLight 是 NULL");

    FormLockGuard lock;
    if (const Guard* g = RouteGuard())
        return Refuse(op, g->guard, g->goldenLine, g->detail);

    webbridge::JsonWriter w;

    if (op == "get") {
        NewRecordProcess("MES2183", "Enter Tower Light", " ");                // golden main.cpp:28585（cMyDB.h 預設 Debug=" "）
        {
            KeepShowFlag keep;
            fTowerLight->FormShow(0);                                          // golden :83-127
        }
        w.BeginObject();
        w.Key("executed").Bool(true);
        w.Key("op").String(op);
        WriteState(w);
        w.Key("goldenLine").String("V912 cTowerLight.cpp:83-127 FormShow（MusicSelect 鉗制 0～4 只寫記憶體）");
        w.EndObject();
        return Finish(w, ok);
    }

    if (op == "click") {
        int ci = -1, cj = -1;
        for (int i = 0; i < 8 && ci < 0; ++i)
            for (int j = 0; j < 3; ++j) {
                char name[8];
                std::snprintf(name, sizeof(name), "RGB%d%d", i, j);
                if (led == name) { ci = i; cj = j; break; }
            }
        if (ci < 0)
            return Refuse(op, "bad-payload", "", "led 只收 RGB00～RGB72（dfm 的 24 個 TALed）");
        {
            KeepShowFlag keep;
            fTowerLight->FormShow(0);                                          // 表單開著才點得到：先照 golden 開窗（Panel11 顯示與否）
        }
        TfTowerLightLed* l = Led(ci, cj);
        if (!l->Visible || (ci == 7 && !fTowerLight->Panel11->Visible))
            return Refuse(op, "not-visible", "V912 cTowerLight.cpp:103-120 FormShow Panel11->Visible",
                          "這台機台 Auto Retest／LD-ULD 那一列（Panel11）不顯示，golden 的操作員點不到 " + led);
        const int before = LastSet.MessageLight[ci][cj];
        const int msgBefore = W906_ShowMyMessage_Count;
        fTowerLight->RGB00Click(l);                                            // golden :55-81（含 WriteLastDataFile :79）
        const int after = LastSet.MessageLight[ci][cj];
        const bool a02 = (W906_ShowMyMessage_Count != msgBefore);
        const bool wrote = !a02 && ci <= OfflineRunLimit();                    // 走到 :79 才寫檔
        std::printf("towerlight.op click %s [%d][%d] %d -> %d%s\n", led.c_str(), ci, cj, before, after,
                    a02 ? " (A01_2 refused)" : (wrote ? " (lastdata.dat written)" : " (beyond iOfflineRun, golden return)"));
        w.BeginObject();
        w.Key("executed").Bool(true);
        w.Key("op").String(op);
        w.Key("led").String(led);
        w.Key("cell").BeginArray().Number((wb_int64)ci).Number((wb_int64)cj).EndArray();
        w.Key("before").Number((wb_int64)before);
        w.Key("after").Number((wb_int64)after);
        w.Key("written").Bool(wrote);
        if (a02) {
            w.Key("guard").String("A01_2");
            w.Key("message").String(std::string(W906_ShowMyMessage_LastS1.c_str()));
        }
        WriteState(w);
        w.Key("goldenLine").String("V912 cTowerLight.cpp:55-81 RGB00Click");
        w.EndObject();
        return Finish(w, ok);
    }

    // op == "music"
    int ci = -1;
    for (int i = 0; i < 8; ++i) if (combo == kCombo[i]) { ci = i; break; }
    if (ci < 0)
        return Refuse(op, "bad-payload", "", "combo 只收 cbRunning／cbJam／cbPause／cbMessage／cbHeating／cbHome／cbOffLine／cbART");
    if (index < 0 || index > 4)
        return Refuse(op, "bad-payload", "V912 cTowerLight.dfm:448-453 Items（Silent／Music 1～4）", "index 只收 0～4");
    const int before = LastSet.MusicSelect[ci];
    {
        KeepShowFlag keep;
        fTowerLight->FormShow(0);                                              // golden :83-127：8 個框 ← LastSet（含鉗制）
        TComboBox* c = Combo(ci);
        if (!c->Visible || (ci == 7 && !fTowerLight->Panel11->Visible))
            return Refuse(op, "not-visible", "V912 cTowerLight.cpp:103-120 FormShow Panel11->Visible",
                          "這台機台 Auto Retest／LD-ULD 那一列（Panel11）不顯示，golden 的操作員選不到 " + combo);
        if (!c->Enabled)
            return Refuse(op, "not-enabled", "V912 cTowerLight.cpp:122-123 FormShow（SPIL：cbOffLine->Enabled=false）",
                          combo + " 反灰，golden 的操作員選不到");
        c->ItemIndex = index;
        fTowerLight->FormClose();                                              // golden :129-140：8 個框 → LastSet.MusicSelect[]
    }
    const bool wrote = WriteLastDataFile();                                    // 決斷 D3（檔頭）
    std::printf("towerlight.op music %s MusicSelect[%d] %d -> %d (lastdata.dat %s)\n", combo.c_str(), ci, before,
                LastSet.MusicSelect[ci], wrote ? "written" : "WRITE FAILED");
    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("op").String(op);
    w.Key("combo").String(combo);
    w.Key("before").Number((wb_int64)before);
    w.Key("after").Number((wb_int64)LastSet.MusicSelect[ci]);
    w.Key("written").Bool(wrote);
    {
        KeepShowFlag keep;
        fTowerLight->FormShow(0);                                              // 回傳畫面＝重新開窗看到的（含鉗制）
    }
    WriteState(w);
    w.Key("goldenLine").String("V912 cTowerLight.cpp:83-127 FormShow → 換框 → :129-140 FormClose → WriteLastDataFile（golden 延後到下一次寫檔，見檔頭 D3）");
    w.EndObject();
    return Finish(w, ok);
}
