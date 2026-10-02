// ===========================================================================
//  FileRW/DeviceForm_File.cpp -- 結構 DeviceForm_File 的讀寫檔（<recipe>\Contact.Data、Position Offset.Data；
//  CosFunction.bContactHeightSaveToContactIni 時高度寫 D:\HT9045\system\Contact.ini）。
//
//  Steven 20260925.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（DeviceForm_File）。
//
//  golden TfContact（cContact.cpp）由 tools/gen_editlist.py 轉成 DeviceForm_File.gen.inc（元件改成具名替身）：
//    建構子（:107）＝選項清單、scrbSLK->Max；FormShow（:1094）＝開頁；spbSaveClick（:14179）＝存檔鈕 → SaveSetupFile（:14313）。
//    力量公式 CalculateTotalAirForce（:18966）／GetMaxIndexForceLimit（:19184）／GetMinForce（:19248）／CountDieForceKg（:18662）
//    與 DFM 綁的 OnChange／OnClick 處理器全部照 golden 轉；fContactForce->SLKClass 接 ContactForceTables()。
//  elContact（HTEditList）golden 沒有任何 Add()，存檔流程讀的替身全部是 mustSend（palTorqueArm1/2 除外，見 BuildReads）。
//  開機讀檔仍是移植樹 fContactForm->ReadFile()（tools/wb_serve.cpp），本檔不改它。
//
//  ── 為什麼存檔前要重算（Steven 20260925 第二輪）──────────────────────────────────────────
//  golden 畫面上的衍生欄位是 VCL 事件在操作員打字「當下」算的（DFM 綁定，golden cContact.dfm）：
//      edPinCount.OnChange        = edPinCountChange        → ShowArmAndDeviceForce
//      edForcePerPinN.OnChange    = edForcePerPinNChange    → (!bForecePerPinKGf) G=N*1000/9.8 → ShowArmAndDeviceForce
//      edForcePerPinG.OnChange    = edDieForcePerPinGChange → dieN=dieG*9.8/1000、N=G*9.8/1000 → ShowArmAndDeviceForce
//      edDieForcePerPinG.OnChange = edDieForcePerPinGChange →   （INSTALL_DOUBLE_EP 時再 CountDieForceKg(true)）
//      edAirForce.OnChange        = edAirForceChange        → DeviceForm.dPress／DeviceForm_File.dPress（＋EP 輸出）
//      scrbSLK.OnChange           = scrbSLKChange           → DutCount → ShowArmAndDeviceForce → edAirForceChange
//      rgKitDiameter.OnClick／rgOutKitDiameter.OnClick／rgDieForceKitDiameter.OnClick／chkUseAddWeight.OnClick／
//      cbEnableUK.OnClick／cbContactMode.OnChange／coD41.OnChange；小鍵盤關閉後：edtPinOfDie→CountDieForceKg(true)、
//      edDoubleForce→CountDieForceKg(false)、edForcePerDeviceKG→edForcePerDeviceKGClick（G=devKG*1000/pin）。
//  網頁只送畫面最後狀態，事件沒有發生 → DF_DeriveBeforeSave 依下面的順序從「主輸入」重跑同一批 golden 處理器，
//  衍生欄位（edAirForce／edAirForceN／edForcePerDeviceKG／N／非主的 N 或 G／edDieForcePerPinN／edDoubleForce 或 edtPinOfDie）
//  永遠不會是頁面帶來的舊值。「主輸入」＝這次頁面值與開頁（或上次存檔後）快照不同的欄位＝操作員動過的欄位。
// ===========================================================================
#include "FileRW/DeviceForm_File.gen.inc"

#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include "FileRW/WindowEdgeTails.h"   // AI(W906-EVB10A) 20260929 [W906]: CT-3b FileRW_Contact_WindowEdge／FileRW_Contact_EvB10AFormClose 的宣告（檔尾）；佔用原本的空行
#include "FileRW/_EditPage.h"
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbContactClickOpen（FileRW/MainClick.cpp；Q41 第 3 項 CT-4，R84）；佔用原本的空行
void FileRW_Contact_TimerEPTick();  void FileRW_Contact_OTDTimerTick();  /* AI(W906-B8-CTL2) 20260930 [W906]: golden TimerEP 一拍（本體檔尾）；同一行插入 */  /* AI(W906-B8-CT3D) 20261001 [W906] (St01): + golden OTDTimer 一拍（本體檔尾）；同一行插入 */  std::string FileRW_Contact_KbExtraJson();  /* AI(W906-SETUPA-KB) 20261002: editlist.get extra（golden 小鍵盤的執行期上下限＋edDropWaitTimeMouseDown 輸入後修正要的值；FileRW/DeviceForm_KbExtra.cpp）；同一行插入 */  namespace {
bool g_booted = false;
bool Booted() { return g_booted; } void EvB3SessionReset(); void EvB3SaveBegin(); void EvB3SaveEnd();   //AI(W906-EVB3) 20260928 [W906]: CT-1 機台記憶體 session，定義在檔尾；同一行附加

const char* const kForm = "TfContact";

// ---- 開頁快照：判斷操作員動過哪些欄位 ----
// golden 的事件只在操作員改動時觸發；沒動的欄位 golden 不會重算（例如檔案裡 N/G 不一致，沒人碰就原樣存回）。
const char* const kWatch[] = {
    "cbContactMode", "edDropOffset1", "edDropOffset2",
    "rgOutKitDiameter", "rgKitDiameter", "rgDieForceKitDiameter", "chkUseAddWeight", "cbEnableUK", "coD41", "scrbSLK",
    "edForcePerDeviceKG", "edForcePerPinN", "edForcePerPinG", "edDieForcePerPinG", "edPinCount",
    "edtPinOfDie", "edDoubleForce",
    // golden DFM：這幾格 OnChange = edContactHeight1Change
    "edContactHeight1", "edContactHeight2", "edLoadCellHeight1", "edLoadCellHeight2", "edUpOffset1", "edUpOffset2",
};
std::map<std::string, std::string> g_snap;

std::string ValOf(const char* n)
{
    TControl* c = filerw::ELFind(kForm, n);
    if (!c) return std::string();
    if (filerw::ELTrackBar* t = dynamic_cast<filerw::ELTrackBar*>(c)) return std::to_string((int)t->Position);
    if (TCheckBox* t = dynamic_cast<TCheckBox*>(c)) return t->Checked ? "1" : "0";
    if (TComboBox* t = dynamic_cast<TComboBox*>(c)) return std::to_string(t->ItemIndex);
    if (TRadioGroup* t = dynamic_cast<TRadioGroup*>(c)) return std::to_string(t->ItemIndex);
    if (TCustomEdit* t = dynamic_cast<TCustomEdit*>(c)) return t->Text.c_str();
    return std::string();
}
void TakeSnapshot()
{
    g_snap.clear();
    for (const char* n : kWatch) g_snap[n] = ValOf(n);
}
bool Changed(const char* n) { return ValOf(n) != g_snap[n]; }
AnsiString Txt(const char* n) { return EL<TEdit>(kForm, n)->Text; }
void SetTxt(const char* n, const AnsiString& v) { EL<TEdit>(kForm, n)->Text = v; }

// ---- VCL 事件連鎖：edForcePerPinN.OnChange=edForcePerPinNChange、edForcePerPinG／edDieForcePerPinG.OnChange=edDieForcePerPinGChange ----
// 處理器改了另一格的 Text，VCL 只在「值真的變了」才觸發那一格的 OnChange（TControl::SetText 先比較）。
// 這裡照同一規則跑到不動點：N→G→N… 在 FormatFloat("0.0000") 下兩三輪就穩定（例 0.3 → 30.6122 → 0.3000 → 30.6122）。
void PinForceCascade(void (*first)())
{
    void (*next)() = first;
    for (int guard = 0; next && guard < 16; ++guard) {
        const AnsiString n0 = Txt("edForcePerPinN"), g0 = Txt("edForcePerPinG");
        next();
        const bool gC = Txt("edForcePerPinG") != g0, nC = Txt("edForcePerPinN") != n0;
        next = gC ? &DF_edDieForcePerPinGChange : (nC ? &DF_edForcePerPinNChange : nullptr);
    }
}

// ---- 存檔前推導（順序見檔頭與下面每一步的理由）----
void DF_DeriveBeforeSave()
{
    // 先記下操作員的值（後面的處理器可能蓋掉，需要時還原）
    std::map<std::string, AnsiString> op;
    std::map<std::string, bool> ch;
    for (const char* n : kWatch) ch[n] = Changed(n);
    for (const char* n : {"edDropOffset1", "edDropOffset2", "edtPinOfDie", "edDoubleForce", "edForcePerPinN", "edForcePerPinG",
                          "edDieForcePerPinG", "edPinCount", "edForcePerDeviceKG"})
        op[n] = Txt(n);

    // ── 頁面回音過濾（整合測試 20260925：--edit edForcePerPinG=50 讓 Pin of Die 10→6）──
    // ch／op 是在任何推導之前、頁面值剛套上替身時取的（PageSave 套值後立刻呼叫本流程），沒有被本檔的連鎖改過。
    // 真正的原因是頁面自己的 JS（web/page/ht9045_contact_slk.js）也照 golden 跑了 edDieForcePerPinGChange／CountDieForceKg，
    // 所以頁面送來的 edDoubleForce 已經是 0.30：「與快照不同」不等於「操作員打的」。
    // 規則：衍生欄位的頁面值若等於 golden 用「其他頁面輸入」算出來的值，視為衍生（頁面回音），不當主輸入。
    auto F4 = [](double v) { return FormatFloat("0.0000", v); };
    //   edForcePerDeviceKG（只有 AMKOR_Korea＋Qorvo 可編輯）＝CalcDeviceForce(pin, N, G, true)＝pin*G*0.001（:23058）
    if (ch["edForcePerDeviceKG"] &&
        op["edForcePerDeviceKG"] == F4(atof(op["edPinCount"].c_str()) * atof(op["edForcePerPinG"].c_str()) * 0.001))
        ch["edForcePerDeviceKG"] = false;
    //   N／G 兩格都不同：其中一格等於另一格的 golden 換算 → 那一格是衍生（:1965 G=N*1000/9.8、:1981 N=G*9.8/1000）
    if (ch["edForcePerPinN"] && ch["edForcePerPinG"]) {
        if (op["edForcePerPinG"] == F4(atof(op["edForcePerPinN"].c_str()) * 1000.0 / 9.8))
            ch["edForcePerPinG"] = false;
        else if (op["edForcePerPinN"] == F4(atof(op["edForcePerPinG"].c_str()) * 9.8 / 1000.0))
            ch["edForcePerPinN"] = false;
    }
    //   Pin of Die／Double Force 在第 4 步判斷（要用第 3 步之後的 G）

    // 1. Contact Mode（畫面上方 Contact Parameter 群組）：golden cbContactModeChange（:14084）選模式時改寫 Drop Offset 並設 Enabled。
    //    操作員在選完模式之後才可能打 Drop Offset → 處理器之後、欄位仍 Enabled 時還原操作員打的值；被停用的格子以處理器值為準。
    if (ch["cbContactMode"]) {
        DF_cbContactModeChange();
        for (const char* n : {"edDropOffset1", "edDropOffset2"})
            if (ch[n] && EL<TEdit>(kForm, n)->Enabled) SetTxt(n, op[n]);
    }
    // 2. 選項類（Contact Force 群組）：Kit Diameter／Out Kit／Die Force Kit／Add Weight／Universal Kit／D41／頭數。
    //    它們只改 DeviceForm_File 欄位、可見性或 IniConfig 旗標，不改打字欄位，所以先跑；力量最後一次重算在第 5 步。
    if (ch["rgOutKitDiameter"]) {
        const int k0 = EL<TRadioGroup>(kForm, "rgKitDiameter")->ItemIndex;
        DF_rgOutKitDiameterClick();
        // VCL：程式改 TRadioGroup::ItemIndex 也會觸發 OnClick
        if (EL<TRadioGroup>(kForm, "rgKitDiameter")->ItemIndex != k0) DF_rgKitDiameterClick();
    } else if (ch["rgKitDiameter"]) {
        DF_rgKitDiameterClick();
    }
    if (ch["rgDieForceKitDiameter"]) DF_rgDieForceKitDiameterClick();
    if (ch["chkUseAddWeight"]) DF_chkUseAddWeightClick();
    if (ch["cbEnableUK"]) DF_cbEnableUKClick();
    if (ch["coD41"]) DF_coD41Change();
    if (ch["scrbSLK"]) DF_scrbSLKChange();

    // 3. Pin force（主輸入 → 衍生）：
    //    edForcePerDeviceKG 只在 AMKOR_Korea＋Qorvo 時 Enabled（FormShow :1752），其餘 PageSave 已丟掉頁面值 → 不會在 ch 裡。
    if (ch["edForcePerDeviceKG"]) {
        const AnsiString g0 = Txt("edForcePerPinG");
        DF_edForcePerDeviceKGClick();                      // golden :18699：G = devKG*1000/pinCount
        if (Txt("edForcePerPinG") != g0) PinForceCascade(&DF_edDieForcePerPinGChange);
    }
    //    bForecePerPinKGf==true：edForcePerPinN 停用（DoIniDataToForm :948）→ G 是主輸入，N 由 edDieForcePerPinGChange 算。
    //    bForecePerPinKGf==false：兩格都能打；只動了一格就以那一格為主；兩格都動了，golden 的結果取決於最後打哪一格（網頁看不到），
    //    依 Steven 裁定以 N 為主（edForcePerPinNChange 只在這個模式有作用）。
    const bool opN = ch["edForcePerPinN"] && !ch["edForcePerDeviceKG"];
    const bool opG = ch["edForcePerPinG"] && !ch["edForcePerDeviceKG"];
    if (CosFunction.bForecePerPinKGf) {
        if (opG) PinForceCascade(&DF_edDieForcePerPinGChange);
    } else if (opN) {
        PinForceCascade(&DF_edForcePerPinNChange);
    } else if (opG) {
        PinForceCascade(&DF_edDieForcePerPinGChange);
    }
    if (ch["edDieForcePerPinG"]) PinForceCascade(&DF_edDieForcePerPinGChange);   // 同一個處理器（dieN、N、CountDieForceKg(true)）
    if (ch["edPinCount"]) DF_edPinCountChange();

    // 4. Die force 的 Pin of Die ↔ Double Force（小鍵盤關閉後的 CountDieForceKg）：第 3 步的 edDieForcePerPinGChange 可能已用
    //    Pin of Die 重算過 Double Force；操作員動過哪一格就以那一格為主（兩格都動 → Pin of Die 為主，與 edDieForcePerPinGChange 同向）。
    //    頁面回音過濾（同上）：Double Force 等於 CountDieForceKg(true)＝sprintf("%0.2f", pinOfDie*dieG/1000)（:18669-18673）
    //    → 衍生；Pin of Die 等於 CountDieForceKg(false)＝int(DoubleForce*1000/G)（:18677-18683）且 Double Force 也不同 → 衍生。
    bool opPin = ch["edtPinOfDie"], opDF = ch["edDoubleForce"];
    {
        AnsiString expDF;
        expDF.sprintf("%0.2f", atof(op["edtPinOfDie"].c_str()) * (atof(Txt("edDieForcePerPinG").c_str()) / 1000.0));
        if (opDF && op["edDoubleForce"] == expDF) opDF = false;
        const double g = atof(Txt("edForcePerPinG").c_str());
        const int expPin = (int)(atof(op["edDoubleForce"].c_str()) * 1000 / (g != 0 ? g : 1.0));
        if (opPin && opDF && atoi(op["edtPinOfDie"].c_str()) == expPin) opPin = false;
    }
    if (opPin) {
        SetTxt("edtPinOfDie", op["edtPinOfDie"]);
        DF_CountDieForceKg(true);                          // golden edtPinOfDieMouseDown :18644
    } else if (opDF) {
        SetTxt("edDoubleForce", op["edDoubleForce"]);
        DF_CountDieForceKg(false);                         // golden edDoubleForceMouseDown :18659
    }

    // 5. 最後一次、無條件：ShowArmAndDeviceForce（:1925，CalculateTotalAirForce → iTotalGf → edAirForce／edAirForceN、
    //    edForcePerDeviceKG／N）＋ edAirForce.OnChange（edAirForceChange → DeviceForm.dPress／DeviceForm_File.dPress）。
    //    它們是 (edPinCount, N, G, scrbSLK, rgKitDiameter, ContactForce 表, TestIF/LastSet/IniConfig) 的純函數、可重複呼叫，
    //    所以不論前面觸發了幾次，結果都等於 golden 畫面在存檔當下的值。
    DF_ShowArmAndDeviceForce();
    DF_edAirForceChange();

    // 6. 高度類欄位的 OnChange（edContactHeight1Change）：golden 只設 fOffSet->bEnterSpecialOffset（移植樹沒有 → ELTodo）
    for (const char* n : {"edContactHeight1", "edContactHeight2", "edLoadCellHeight1", "edLoadCellHeight2", "edUpOffset1", "edUpOffset2"})
        if (ch[n]) { DF_edContactHeight1Change(); break; }
}

// ContactForce 表是 golden TfContactForce 建構子載的（ContactForce.cpp:430 只在 bUseDynamicKitDiameter 時）。
// 該載卻沒載 → Kit Diameter 選項與 CalculateTotalAirForce 的 dKitDiameter／dMinForce 都不會是 golden 值 → 只有這種情況拒存。
bool ContactForceInputsMissing()
{
    if (!CosFunction.bUseDynamicKitDiameter) return false;                  // golden 的表本來就是空的
    if (!ContactForceTables().SLKClass.bLoaded) return true;
    if (INSTALL_DOUBLE_EP > 0 && !ContactForceTables().DieForceSLKClass.bLoaded) return true;
    return false;
}

void FormShowAndSnap()
{
    DF_FormShow();  FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();  /* AI(W906-B8-CT3D) 20261001 [W906] (St01): golden FormShow :1646 開 OTDTimer（USE_OTD==1，100 ms）⇒ 頁面拿到的是第一拍之後的畫面（檔尾）；同一行插入 */  /* AI(W906-B8-CTL2) 20260930 [W906]: golden FormShow 開 TimerEP（EP_Install 3／5，1000 ms）⇒ 頁面拿到的是第一拍之後的畫面（檔尾）；同一行插入 */  // golden FormShow 已把衍生欄位算好（ShowArmAndDeviceForce）
    if (ContactForceInputsMissing())
        filerw::ELTodo("ContactForceTables() not loaded while CosFunction.bUseDynamicKitDiameter -- Kit Diameter items / Contact Force are not golden values (LoadContactForceTables)");
    TakeSnapshot(); if (const char* w = W906_Main_sbContactClickOpen()) filerw::ELTodo(w); EvB3SessionReset();  /* AI(W906-EVB3) 20260928 [W906]: CT-1 開頁當下的記憶體＝這一頁的 session（檔尾）；同一行插入 */   // AI(W906-FRW-S158) 20260927 [W906]: R84＝A —— golden V912 main.cpp:28314 fContact->Show()（非 modal）之後的 :28315-28316（DoStructUnitConvert、SetWorkParameter）在開窗當下跑；運轉中不跑（R86）；接在同一行
}

// golden 關頁 FormClose（:1842）的 ReadFile＋DoIniDataToForm（:1847-1848）—— 只取「替身還原成檔案值」那一半；
// FormClose 其餘（SystemStart=false、fAllMotorHome=false、NewRecordProcess、MyEtherCAT->Pause…）是執行期效果，網頁不跑
void Reload()
{
    DF_ReadFile();
    DF_DoIniDataToForm();
    DF_edAirForceChange();                                                  // VCL：DoIniDataToForm :939 改 edAirForce->Text 會觸發 edAirForce.OnChange
    TakeSnapshot(); EvB3SessionReset();   //AI(W906-EVB3) 20260928 [W906]: CT-1 重讀檔之後重記 session（檔尾）；同一行附加
}

void DF_SaveFlow()
{
    filerw::ELMark("DF_SaveFlow");
    if (ContactForceInputsMissing()) {
        filerw::ELMessage("Contact save refused: ContactForce table (system\\ContactInfo.ini) is not loaded, "
                          "Kit Diameter / Torque cannot be computed as golden does",
                          "Contact 存檔拒絕：ContactForce 表（system\\ContactInfo.ini）沒有載入，"
                          "Kit Diameter／Torque 無法照 golden 計算");
        return;
    }
    EvB3SaveBegin(); DF_DeriveBeforeSave();   //AI(W906-EVB3) 20260928 [W906]: CT-1 存檔前把 form.event 累積的 session 記憶體換上（檔尾）；同一行插入
    DF_spbSaveClick(); EvB3SaveEnd();  if (filerw::ELMarked("closed")) ::FileRW_Contact_EvB10AFormClose("A02 save: golden spbSaveClick :14190 Close()");   //AI(W906-EVB3) 20260928 [W906]: CT-1 沒存成 ⇒ 機台記憶體放回存檔前（檔尾）；同一行附加  //AI(W906-EVB10A) 20260929 [W906]: CT-3b golden Close() → OnClose＝FormClose（:1842），本體檔尾
    TakeSnapshot(); EvB3SessionReset();   /* AI(W906-EVB3) 20260928 [W906]: CT-1（檔尾）；同一行插入 */  // golden 存後 ReadFile＋DoIniDataToForm（:14262-14263）後的畫面
}

// 存檔流程讀的替身：產生器掃出的（kDF_SaveReads）減去伺服器端狀態 ——
//   palTorqueArm1/2（TPanel）：golden SaveSetupFile :14499-14500 存它們的 Caption，但 Caption 只由 DoIniDataToForm :995-996
//   （bChangeKitNoHardStop 時＝檔案的 dZ1Torue／dZ2Torue）與 Contact Test 狀態機 :13200/:13322（讀扭力，網頁不跑）設定，
//   操作員不能打字 → 值留在伺服器端，不列必送（頁面也沒辦法送 Caption，列了會 400）。同 FileRW/TestIF_File_SetUp.cpp 的容器規則。
std::vector<const char*> g_reads;
void BuildReads()
{
    for (const char* n : kDF_SaveReads) {
        TControl* c = filerw::ELFind(kForm, n);
        if (dynamic_cast<TPanel*>(c) || dynamic_cast<TGroupBox*>(c) || dynamic_cast<TTabSheet*>(c)) continue;
        g_reads.push_back(n);
    }
}

HTEditList** const kLists[] = {&elContact};
const char* const kListNames[] = {"elContact"};

filerw::PageDesc g_page = {
    "DeviceForm_File", "TfContact", "Setup.Contact.html",
    kLists, kListNames, 1,
    nullptr, 0,                                                             // BuildReads() 之後填
    &FormShowAndSnap, &DF_SaveFlow, "SaveSetupFile", &Reload, &Booted, nullptr, &FileRW_Contact_KbExtraJson,   //AI(W906-SETUPA-KB) 20261002: extraJson（見檔頭宣告那一行）；同一行
};
filerw::PageRegistrar g_reg(&g_page);
}  // namespace

// golden TfContact 建構（HT9045.cpp CreateForm）：scrbSLK 先建成 ELTrackBar（TScrollBar 不在產生器 MAPPED，
// 其他地方拿到的 EL<TControl> 會 static_cast 回同一個物件）→ DFM 設計期狀態 → 建構子 → 存檔流程讀的替身
void FileRW_Contact_Boot()
{
    if (g_booted) return;
    if (!elContact) elContact = new HTEditList;                                 // golden main.cpp:1534（TfMain 建構子）
    filerw::ELTrackBar* slk = EL<filerw::ELTrackBar>("TfContact", "scrbSLK");
    slk->DfmInit(2, 5, 2);                                                      // golden cContact.dfm:16129-16132 Max=5 Min=2 Position=2
    slk->OnChange = &DF_scrbSLKChange;                                          // golden cContact.dfm:16134 OnChange=scrbSLKChange
    DF_DfmItems();
    DF_DfmState();
    DF_TfContact();
    DF_CreateSaveProxies();
    DF_CreateContainerProxies(); { void FileRW_Contact_Ct3aBoot(); FileRW_Contact_Ct3aBoot(); } { void FileRW_Contact_Ct3dBoot(); FileRW_Contact_Ct3dBoot(); } { void FileRW_Contact_EvBoot(); FileRW_Contact_EvBoot(); }   //AI(W906-EVB3) 20260928 [W906]: form.event 控制項替身檢查（檔尾）；同一行附加  //AI(W906-B8-CT3A) 20261001 [W906] (St01): CT-3a 的 14 個事件元件開機就建替身（檔尾；golden 方法本體要到第一次 FormShow 才用到）；同一行插入  //AI(W906-B8-CT3D) 20261001 [W906] (St01): CT-3d palOTD_4 / palOTD_6 / ledOTD / OTDTimer proxies at boot (file tail); same line
    BuildReads();
    g_page.saveReads = g_reads.data();
    g_page.nSaveReads = (int)g_reads.size();
    std::printf("FileRW DeviceForm_File: TfContact proxies ready (%d save reads, %d must-send) -- golden cContact.cpp\n",
                (int)(sizeof(kDF_SaveReads) / sizeof(kDF_SaveReads[0])), g_page.nSaveReads);
    g_booted = true;
}

// ===========================================================================
//  AI(W906-EVB3) 20260928 [W906] 批次 B3 CT-1／CT-L1（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md；
//    Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」）—— WS form.event。
//    事件表＝產生的 kDF_Events（tools/editlist/DeviceForm_File.py 的 events，8 列）抄一份、處理器換成跳板 EvB3Run：
//      CT-1 選項類（原本只在存檔時由 DF_DeriveBeforeSave 第 1、2 步重跑；現在點了就跑，畫面當場連動）：
//        cbContactMode change   golden cbContactModeChange（V912 cContact.cpp:14084）Drop Offset 兩格的值與可改、pnlSidePush 顯示
//        rgKitDiameter click    golden rgKitDiameterClick（:15509）DeviceForm_File.dKitDiameter → ShowArmAndDeviceForce → edAirForceChange
//        rgOutKitDiameter click golden rgOutKitDiameterClick（:17393）改 rgKitDiameter（VCL 程式改 ItemIndex 也會 OnClick → 再跑 rgKitDiameterClick，
//                               同 DF_DeriveBeforeSave 第 2 步）→ ShowArmAndDeviceForce
//        rgDieForceKitDiameter  golden rgDieForceKitDiameterClick（:19796）DeviceForm_File.dDieForceKitDiameter
//        chkUseAddWeight click  golden chkUseAddWeightClick（:17387）DeviceForm_File.bUseAddWeight → edAirForceChange
//        cbEnableUK click       golden cbEnableUKClick（:17430，CosFunction.bUniversalKit）十個元件的顯示／可改＋IniConfig.bRemeberAutoHeight／
//                               bChangeKitNoHardStop
//        coD41 change           golden coD41Change（:15274）edD41／labD41 顯示
//      CT-L1 pnlSensorAdj click golden pnlSensorAdjClick（:14171-14177）：fSecurity->Insufficient(109)（等級不足 golden 跳 WAR1676，照翻，
//                               ShowErrorMessage 在 wb_serve 走 ForwardShowErrorMessage）；過了 → ack.todo "open:cclink …"，頁面開 background 視窗
//                               'cclink'（HW.MyCCLinkSensor.html）。fCCLink->ShowUseSensor／Show 移植樹沒有（產生器 replace，列給 Jimmy）。
//    跳板規則：
//      (1) VCL：TRadioGroup／TCheckBox 的 OnClick 只在值真的變了才發生 —— 頁面送的值（RunPageEvent 第 5 步已套上）跟事件前的伺服器值
//          （g_snap）一樣就不跑處理器。
//      (2) 每一次之後更新 DF_DeriveBeforeSave 的比較基準 g_snap：處理器前後有變的欄位＋送事件的那一格換成現在的值 ⇒ 存檔時不會把
//          已經當場跑過的選項事件再跑一次（例：先點 Out Kit、再手動改 Kit，存檔時不會被 Out Kit 蓋回去）。state 只改了、處理器沒動到的
//          打字欄位不進基準 ⇒ 存檔時照舊是「操作員動過」、照舊重算。
//      (3) ⚠ 偏離 golden（R 題）：這幾支處理器會改**機台正在用的記憶體** —— DeviceForm.dPress（生產時 ADAM_WriteVoltage 的 EP 壓力，
//          atester.cpp:9662 等）、DeviceForm_File.dKitDiameter（adam6024.cpp:178 算 EP）、dPress、dDieForceKitDiameter、bUseAddWeight、
//          iKitDiameterMode、IniConfig.iEP_Min_KG／bRemeberAutoHeight／bChangeKitNoHardStop、iTotalGf。golden 點了就改（還馬上送 EP 電壓），
//          關窗 FormClose（:1842 ReadFile＋DoIniDataToForm）再改回檔案值；網頁沒有關窗事件（X-1／CT-3b 還沒做），照 golden 改的話
//          「點了 Kit 直徑、沒存就關頁」之後機台會用沒存的壓力生產。所以：處理器跑之前換上這一頁的 session 記憶體、跑完把 session 記下、
//          機台記憶體放回原值（畫面照 golden 變、機台記憶體等存檔才改；同 IniConfig udD46 與 Q14＝B 的方向）。session 只帶這一頁改過的格子（MemTake）。存檔時 DF_SaveFlow 先換上
//          session（EvB3SaveBegin）再跑 DF_DeriveBeforeSave／golden 存檔鈕；沒存成（A02／取消）就放回存檔前的機台值（EvB3SaveEnd）。
//          session 在開頁（FormShowAndSnap）、重讀（Reload）、存完之後重記。EP 電壓輸出本來就沒做（edAirForceChange 的 ADAM_WriteVoltage 是
//          ELTodo，產生器 replace）。
//          ⚠ 之後 CT-3（Contact 頁的機台操作：啟動、One Cycle、Auto Z Teach…）接上時，golden 是用「畫面上剛改、還沒存」的記憶體去跑 ——
//          那時要先 EvB3SessionLoad 換上 session（交件列給 Jimmy／CT-3）。
// ===========================================================================
#include <stdexcept>
#include <cstring>

namespace {  bool Ct3aOwns(const char* control); void Ct3aRun(TControl* sender);  bool Ct3bOwns(const char* control); void Ct3bRun(TControl* sender);  bool Ct3dOwns(const char* control); void Ct3dRun(TControl* sender);   //AI(W906-B8-CT3A) 20261001 [W906] (St01): B8 CT-3a 的 14 列不走下面的 session 跳板（本體檔尾）；同一行附加  //AI(W906-B8-CT3B) 20261001 [W906] (St01): B8 CT-3b' btnStart / btnPause -> Ct3bRun (file tail); same line, before the trailing //  //AI(W906-B8-CT3D) 20261001 [W906] (St01): B8 CT-3d palOTD_4 / palOTD_6 -> Ct3dRun (file tail); same line, before the trailing //
// (3) 這幾支處理器會寫的機台記憶體（型別跟著 cprod.h／Config.h／cmydef.h 走）
struct DfMem {
    decltype(DeviceForm_File.dKitDiameter) kitDia;
    decltype(DeviceForm_File.iKitDiameterMode) kitMode;
    decltype(DeviceForm_File.dDieForceKitDiameter) dieKitDia;
    decltype(DeviceForm_File.bUseAddWeight) addWeight;
    decltype(DeviceForm_File.dPress) pressFile;
    decltype(DeviceForm.dPress) pressRun;
    decltype(IniConfig.iEP_Min_KG) epMinKg;
    decltype(IniConfig.bRemeberAutoHeight) rememberAH;
    decltype(IniConfig.bChangeKitNoHardStop) noHardStop;
    decltype(iTotalGf) totalGf;
};
DfMem MemNow()
{
    DfMem m;
    m.kitDia = DeviceForm_File.dKitDiameter;           m.kitMode = DeviceForm_File.iKitDiameterMode;
    m.dieKitDia = DeviceForm_File.dDieForceKitDiameter; m.addWeight = DeviceForm_File.bUseAddWeight;
    m.pressFile = DeviceForm_File.dPress;              m.pressRun = DeviceForm.dPress;
    m.epMinKg = IniConfig.iEP_Min_KG;                  m.rememberAH = IniConfig.bRemeberAutoHeight;
    m.noHardStop = IniConfig.bChangeKitNoHardStop;     m.totalGf = iTotalGf;
    return m;
}
void MemSet(const DfMem& m)
{
    DeviceForm_File.dKitDiameter = m.kitDia;           DeviceForm_File.iKitDiameterMode = m.kitMode;
    DeviceForm_File.dDieForceKitDiameter = m.dieKitDia; DeviceForm_File.bUseAddWeight = m.addWeight;
    DeviceForm_File.dPress = m.pressFile;              DeviceForm.dPress = m.pressRun;
    IniConfig.iEP_Min_KG = m.epMinKg;                  IniConfig.bRemeberAutoHeight = m.rememberAH;
    IniConfig.bChangeKitNoHardStop = m.noHardStop;     iTotalGf = m.totalGf;
}
// dst 的每一格：from 跟 ref 不一樣（＝被這一頁的事件改過）才換成 from。session 只帶「這一頁改過的格子」——
// 開頁之後別的頁存檔改了同一個全域（例 Configuration 頁的 IniConfig 旗標、ContactForce 頁的 DeviceForm.dPress），沒被這一頁改過的格子不會被蓋回開頁值。
void MemTake(DfMem& dst, const DfMem& from, const DfMem& ref)
{
    if (from.kitDia != ref.kitDia) dst.kitDia = from.kitDia;
    if (from.kitMode != ref.kitMode) dst.kitMode = from.kitMode;
    if (from.dieKitDia != ref.dieKitDia) dst.dieKitDia = from.dieKitDia;
    if (from.addWeight != ref.addWeight) dst.addWeight = from.addWeight;
    if (from.pressFile != ref.pressFile) dst.pressFile = from.pressFile;
    if (from.pressRun != ref.pressRun) dst.pressRun = from.pressRun;
    if (from.epMinKg != ref.epMinKg) dst.epMinKg = from.epMinKg;
    if (from.rememberAH != ref.rememberAH) dst.rememberAH = from.rememberAH;
    if (from.noHardStop != ref.noHardStop) dst.noHardStop = from.noHardStop;
    if (from.totalGf != ref.totalGf) dst.totalGf = from.totalGf;
}
DfMem g_base, g_sess, g_saveMachine;   // g_base＝session 起點的機台值；g_sess＝這一頁的值（沒改過的格子＝g_base）
bool g_sessValid = false;

void EvB3SessionReset() { g_base = g_sess = MemNow(); g_sessValid = true; }
void EvB3SessionLoad()                                                          // 這一頁改過的格子換上去
{
    if (!g_sessValid) return;
    DfMem cur = MemNow();
    MemTake(cur, g_sess, g_base);
    MemSet(cur);
}
void EvB3SaveBegin() { g_saveMachine = MemNow(); EvB3SessionLoad(); }
void EvB3SaveEnd() { if (!filerw::ELMarked("SaveSetupFile")) MemSet(g_saveMachine); }

// (2)
void EvB3Merge(const std::map<std::string, std::string>& pre, const char* sender)
{
    for (const char* n : kWatch) {
        const std::string now = ValOf(n);
        std::map<std::string, std::string>::const_iterator p = pre.find(n);
        if (std::strcmp(n, sender) == 0 || p == pre.end() || p->second != now) g_snap[n] = now;
    }
}

void EvB3Body(const filerw::PageEvent& e, TControl* sender)
{
    // (1) 值沒變 ⇒ VCL 不會 OnClick
    if (dynamic_cast<TRadioGroup*>(sender) || dynamic_cast<TCheckBox*>(sender))
        if (g_snap.count(e.control) && g_snap[e.control] == ValOf(e.control)) return;
    if (std::strcmp(e.control, "rgOutKitDiameter") == 0) {
        const int k0 = EL<TRadioGroup>(kForm, "rgKitDiameter")->ItemIndex;
        e.handler(sender);                                                      // golden :17393
        if (EL<TRadioGroup>(kForm, "rgKitDiameter")->ItemIndex != k0) DF_rgKitDiameterClick();   // VCL：程式改 ItemIndex → OnClick（:15509）
        return;
    }
    e.handler(sender);
}

// RunPageEvent 以 Sender＝ELFind(TfContact, control) 呼叫 → 找回 kDF_Events 那一列
void EvB3Run(TControl* sender)
{
    const filerw::PageEvent* e = nullptr;
    for (std::size_t i = 0; i < sizeof(kDF_Events) / sizeof(kDF_Events[0]) && !e; ++i)
        if (sender && filerw::ELFind(kForm, kDF_Events[i].control) == sender) e = &kDF_Events[i];
    if (!e) throw std::runtime_error("EVB3: form.event sender is not a TfContact event control proxy");
    std::map<std::string, std::string> pre;
    for (const char* n : kWatch) pre[n] = ValOf(n);
    if (!g_sessValid) EvB3SessionReset();                                     // 保險：開頁一定已經記過（RunPageEvent 要先 editlist.get）
    const DfMem machine = MemNow();
    EvB3SessionLoad();                                                          // (3) 這一頁到目前為止改過的記憶體
    const DfMem before = MemNow();
    try {
        EvB3Body(*e, sender);  FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();   //AI(W906-B8-CTL2) 20260930 [W906]: golden TimerEP 每秒都在跑 ⇒ 事件回覆的 changed 帶下一拍之後的畫面（同 FileRW/IniConfig.cpp 的 Timer1 一拍，檔尾）；同一行附加  //AI(W906-B8-CT3D) 20261001 [W906] (St01): + golden OTDTimer beat (CT-3d, file tail); same line
    } catch (...) {
        MemTake(g_sess, MemNow(), before); MemSet(machine); EvB3Merge(pre, e->control);   // 例外之前改過的照 golden 不還原（留在 session）
        throw;
    }
    MemTake(g_sess, MemNow(), before);                                          // (3) 處理器改的格子記進 session
    MemSet(machine);                                                            //     機台記憶體放回原值
    EvB3Merge(pre, e->control);                                                 // (2)
}

const filerw::PageEvent* EvB3Table()
{
    static filerw::PageEvent t[sizeof(kDF_Events) / sizeof(kDF_Events[0])];
    for (std::size_t i = 0; i < sizeof(kDF_Events) / sizeof(kDF_Events[0]); ++i) {
        t[i] = kDF_Events[i];
        t[i].handler = Ct3aOwns(kDF_Events[i].control) ? &Ct3aRun : &EvB3Run;  if (Ct3bOwns(kDF_Events[i].control)) t[i].handler = &Ct3bRun;  if (Ct3dOwns(kDF_Events[i].control)) t[i].handler = &Ct3dRun;   //AI(W906-B8-CT3A) 20261001 [W906] (St01): 切模式／T.Start／T.Step／One Cycle 直接寫機台用的旗標（golden 點了就改），不換 CT-1 的 session；同一行改寫  //AI(W906-B8-CT3B) 20261001 [W906] (St01): B8 CT-3b' btnStart / btnPause -> Ct3bRun (file tail); same line, before the trailing //  //AI(W906-B8-CT3D) 20261001 [W906] (St01): B8 CT-3d palOTD_4 / palOTD_6 -> Ct3dRun (file tail); same line, before the trailing //
    }
    return t;
}
filerw::PageEventsRegistrar g_evreg("DeviceForm_File", EvB3Table(), (int)(sizeof(kDF_Events) / sizeof(kDF_Events[0])));
}  // namespace

// 開機（FileRW_Contact_Boot 同一行呼叫）：8 個事件控制項產生器都已建替身（golden 方法本體裡都用到）；少了就印出來，不靜默
void FileRW_Contact_EvBoot()
{
    for (std::size_t i = 0; i < sizeof(kDF_Events) / sizeof(kDF_Events[0]); ++i)
        if (!filerw::ELFind("TfContact", kDF_Events[i].control))
            std::printf("FileRW DeviceForm_File: WARNING form.event control %s has no proxy (add it to FileRW_Contact_EvBoot)\n",
                        kDF_Events[i].control);
}

// ===========================================================================
//  AI(W906-EVB10A) 20260929 [W906]：事件批次 B10 part a CT-3b（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；
//    Steven 20260928「如果沒有移植的, 我們直接實作」、20260929「照 BCB 的邏輯」）—— 關 Contact 視窗＝golden TfContact::FormClose（V912 cContact.cpp:1842-1923；
//    產生檔 FileRW/DeviceForm_File.gen.inc 檔尾 DF_FormClose，設定 tools/editlist/DeviceForm_File.py 的 _FC_REPLACE）：
//      計時器停、fShow=false、ReadFile＋DoIniDataToForm（丟掉沒存的改動）、**SystemStart=false**（Form Close時,機台要停住!!）、
//      iGPIBIndexStatus=Z1_Z2_Normal、手動燈滅、bDoRTCLearning=false、CarlibrationTask!=1（Auto Height／Contact Test 沒做完）⇒ fAllMotorHome=false、
//      三格力量欄位打開、Step Contact／Device Map 時清 Tray Y、RTC Auto Verify 沒過的訊息、NewRecordProcess("MES2170","Exit Contact")、
//      iContactMode=CONTACT_NORMAL＋rbModeNormal＋SetContactMode、One Touch Auto Height 取消、EtherCAT shuttle sensor 暫停（CCLink 視窗沒開時）、
//      bIndexCheckVacum=false、RTC Auto Tuning 取消。
//    golden Contact 只能按 Exit（sbtExitClick :14586 → Close()；BorderIcons 在建構子清掉 :112）；網頁的 Exit／✕ 都是外框關視窗
//    ⇒ 頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge，表 FileRW/WindowEdgeTails.h）呼叫 FileRW_Contact_WindowEdge；
//    另外 golden 存檔鈕的 A02 權限不足（spbSaveClick :14190 Close()）也是 FormClose（上面 DF_SaveFlow 那一行）。
//  ⚠ 會停機：運轉中照跑（golden FormClose 本來就是「關 Contact 就停」）—— Contact 開著按 START 跑的就是 Contact 模式（golden main.cpp 的
//    fContact->fShow 分支，移植樹批 2 已改問頁面表），關掉視窗要停下來。F5 重新整理 HMI 時頁面表也會看到 Contact 關掉 ⇒ 一樣停（跟操作員關掉同一件事）。
//  ⚠ 跟 golden 不同（寫明）：
//    1. :1866-1879（iContactMode!=NORMAL：Latch 清空＋FTestSuck／BTestSuck 還原開頁備份）沒做：開頁的備份（:1388-1404）也擋著（TMyKitSuck 標頭衝突），
//       兩半要一起接（ack.todo／主控台有一行）。今天網頁還不能跑 Contact 模式的機台操作（CT-3），iContactMode 只會被 SetContactMode 設。
//    2. :1904-1905 FLCarryKit／BLCarryKit.SetAll(NULL_IC) 沒做（同一個標頭衝突；bSetHasIC 只有 Contact Test 會設，CT-3 還沒移植）。
//    3. :1912 fCCLink->bShow 改問頁面表（W906_FormShowing("fCCLink")）；:1913 MyEtherCAT 在移植樹沒有人建（NULL）⇒ 記 todo、不呼叫。
//    4. golden sbtExitClick（:14586）的「Auto Height／Contact Test 沒做完不准離開」（CarlibrationTask!=1 ⇒ 訊息、不關）沒做：頁面沒有送 Exit 事件
//       （CT-3 的機台操作還沒移植，CarlibrationTask 在網頁上不會離開 1）；視窗已經關了，這裡照 FormClose 的 CarlibrationTask!=1 ⇒ 要重新回原點。
// ===========================================================================
const char* FileRW_Contact_EvB10AFormClose(const char* why)
{
    static std::string s_what;
    const bool running = SystemStart;
    const bool reHome = CarlibrationTask != 1;
    DF_FormClose();                                                             // golden cContact.cpp:1842
    DF_edAirForceChange();                                                      // VCL：FormClose 的 DoIniDataToForm（:939）改 edAirForce->Text 會觸發 edAirForce.OnChange（同上面 Reload）
    TakeSnapshot(); EvB3SessionReset();                                         // C 路：FormClose 的 ReadFile＋DoIniDataToForm 之後的畫面＝新的比較基準；CT-1 session 作廢
    s_what = std::string("ran golden TfContact::FormClose (cContact.cpp:1842) [") + (why ? why : "") + "]: ReadFile + DoIniDataToForm, " +
             (running ? "SystemStart was 1 -> SystemStart=false (golden: closing Contact stops the machine)" : "SystemStart=false (was already 0)") +
             (reHome ? ", CarlibrationTask!=1 -> fAllMotorHome=false (next START homes all)" : "") +
             ", iContactMode=CONTACT_NORMAL, NewRecordProcess(\"MES2170\",\"Exit Contact\")";
    std::printf("[EVB10A] fContact FormClose -> %s\n", s_what.c_str());
    return s_what.c_str();
}

const char* FileRW_Contact_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfContact proxies are not booted";
    if (!filerw::PageShownNow("DeviceForm_File"))
        return "not run: golden FormShow did not run in this window-open, or golden Close() already ran (A02 save) -- no second FormClose";
    return FileRW_Contact_EvB10AFormClose("window closed (Exit / x)");
}

// ===========================================================================
//  AI(W906-B8-CTL2) 20260930 [W906]：B8 CT-L2（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-L2」；Steven 20260928
//    「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）—— Contact 頁的 RTC Auto Tuning 勾選看不看得到。
//  golden：TfContact::TimerEPTimer（V912 cContact.cpp:17177-17232；產生檔 FileRW/DeviceForm_File.gen.inc 檔尾 DF_TimerEPTimer，
//    設定 tools/editlist/DeviceForm_File.py 的 _EP_REPLACE／_EP_BLOCKS）。TimerEP（cContact.dfm:18634，Enabled=False、Interval 沒寫＝VCL 1000 ms）
//    由 FormShow 開：Enabled=(EP_Install==3 || EP_Install==5)（產生檔 DF_FormShow，golden :1647）、FormClose 關（:1845）。
//    每一拍：REAL_TIME_CCD 時 cbRTCAutoTuning->Visible＝!COM2->bCCDDummyRum && IniConfig.bD74RTCAutoTuning && fSecurity->Insufficient(176, false)
//    && COM2->bRTCVerSupportAutoTurnning；看得見與否變了（bOldRTCAutoTuning）⇒ SIGURD 湖口：Checked＝Visible、Enabled＝!Visible、bRTCAutoTuning＝Visible；
//    其他：Checked=false。沒有 REAL_TIME_CCD ⇒ Visible=false、Checked=false。
//  ⚠ golden 事實（照翻、不修）：COM2->bRTCVerSupportAutoTurnning 在 V912 只有 rs232.cpp:119 設 false、沒有人設 true
//    （20260930 11:39 grep -rl bRTCVerSupportAutoTurnning golden V912 全樹：cContact.cpp、rs232.cpp、rs232.h）⇒ 計時器有跑的機台
//    （EP_Install 3／5；Steven01 開發機 D:\HT9045\system\Gerneral.ini 是 EP_Install=3）一拍之後這一格一定看不見；計時器沒跑的機台，
//    FormShow（:1425）設的 Visible=!bCCDDummyRum 留著。rs232.cpp:119 的註解「啟動時傳送 AutoTeach 功能，若沒有回應代表不支援就不要顯示」
//    看起來原意是 vision 有回應時設 true —— golden 還沒做完。移植樹的 COM2（TCOM2Shim）今天 bCCDDummyRum 一律 true（TCOM2Shim 建構子，移植樹 rs232.cpp:162），
//    所以開頁那一段本來就隱藏；這裡把計時器那一段的條件本身接上，RTC 通訊（P-4）接上之後照樣是 golden 的結果。
//  網頁拿畫面的時機：頁面只在 editlist.get（golden FormShow）與 form.event 的回覆拿替身狀態，沒有每秒推送 ⇒ 開頁跑完 golden FormShow 之後
//    補第一拍（FormShowAndSnap）、每個 form.event 的處理器之後再補一拍（EvB3Run）—— 同 FileRW/IniConfig.cpp IC_EvAfterFormShow／IC_EvClick
//    的 Timer1 一拍。頁面看到的永遠不會比一拍更舊；每拍都冪等（bOldRTCAutoTuning 只在「看得見與否變了」才動勾選），多補的拍子不改結果。
//    存檔之後引擎一定重讀（editlist.get）⇒ 也會補到。定時拍子（B8 P-1，wb_serve 主迴圈）可以直接呼叫 FileRW_Contact_TimerEPTick，這一列不需要它。
//  不在這一列（原文在產生檔 #if 0）：EP 讀值 ADAM_ReadPA（:17180-17189，ADAM-6024 回讀沒有本體，Command.cpp:2934 同一件）、
//    KYEC_LEE ATC 等待倒數字（:17219-17230，每秒更新的秒數，快照會停住）—— 都在 B8 CT-3 列的 TimerEP 那一項。
//  測試：ctest B8_CtL2_TimerEP（tests/test_b8_ctl2_timerep.cpp）。
// ===========================================================================
bool W906_COM2_bRTCVerSupportAutoTurnning = false;   // golden rs232.h:156 TCOM2 成員；rs232.cpp:119 開機設 false（Sam 20240425），V912 沒有人設 true

// golden TTimer OnTimer＝TimerEPTimer：Enabled=false 的計時器不會觸發
void FileRW_Contact_TimerEPTick()
{
    if (!g_booted) return;
    if (!EL<TControl>(kForm, "TimerEP")->Enabled) return;                      // golden FormShow :1647 開、FormClose :1845 關
    DF_TimerEPTimer();                                                          // golden cContact.cpp:17177
}

// ===========================================================================
//  AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」第 8 點第一段；
//    Jimmy RULINGS_20261001 第 0 條「照 golden 翻、會動的也接上」；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」）——
//    Contact 頁切模式＋T.Start／T.Step／One Cycle。只設旗標：機台要等 START 之後，流程才照著做（golden 也是）。
//  golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp（產生檔 FileRW/DeviceForm_File.gen.inc，設定 tools/editlist/DeviceForm_File.py 的 _CT3A_*）：
//    11 顆模式單選（cContact.dfm:18125-18318，父層 rgHandlerMode）OnClick＝rbModeNormalClick :15555-15564：ASE 高雄點 Auto Height ⇒ fAllMotorHome=false；
//      SetContactMode :15566-15696 設全域 iContactMode（cmydef.cpp:3346）＋顯示（cbOneTouchAutoContactHight、chkDailyCorrelation…；KYEC_CHEN＋A16 ChangeContactMode）。
//    btnTStart :2280-2283 bSetupStart=true；btnTStep :2285-2288 bSetupStep=true ⇒ 寫到 fContact（TfContactShim，atester_shims.h:222-223、:251，jimmychiu）——
//      流程讀的就是它：ckernel.cpp:324 WaitManualStartKey／:255 WaitManualStepKey（golden ckernel.cpp:116／:74），活的呼叫點 aTester_Front.cpp:3486／:3802-3804、
//      aTester_Rear.cpp:3363／:3673-3675（[D37] Manual Process）、BarCode/BarCode_Shuttle1_SFCAutoTune.cpp:1127／:1230、BarCode_Shuttle2_SFCAutoTune.cpp:939／:1045。
//      ⚠ 兩個 fContact：移植樹的 TfContact 表單物件叫 fContactForm（forms/fContact.h:1634、forms/fContact.cpp:90），golden 的全域名 fContact 留給 TfContactShim
//      （forms/fContact.h:66-100 的說明）；forms/fContact.cpp:701-711 的 btnTStartClick／btnTStepClick 寫的是 fContactForm 自己的成員，沒有人讀 ⇒ 不用它。
//    spbOneCycle :17152-17156 → OneCycleProcess :11770-11775（bContinueContact 反相；iContactMode<CONTACT_TEST(3) 一律 false）＋ledOneCycle 燈（替身的 Tag）。
//      bContinueContact 在移植樹今天沒有人讀（golden 讀它的 Contact 狀態機都閘著，見產生器設定）。
//  跳板 Ct3aRun（CT-1 的 EvB3Run 不用）：golden 點了就改機台用的全域，沒有 CT-1 那種「存檔才改」的 session（B8_RISK「CT-3」第 4 點：CT-3 的旗標要直接寫）。
//    處理器之後照 CT-1 補 TimerEP 一拍（CT-L2）、更新存檔前推導的比較基準（EvB3Merge：KYEC_CHEN ChangeContactMode 程式改的 cbContactMode 不算操作員改的）。
//  VCL TRadioButton.SetChecked(True)（vclcompat 只是欄位）：FileRW_Contact_RbClickTrue。「哪一顆勾著」＝g_ct3aRb（VCL 的 FChecked），不信替身上的值——
//    頁面存檔（editlist.save 的 widgets）會照網頁畫面套 11 顆的 checked，form.event 第 5 步也會套頁面帶的 checked；使用者點單選＝SetChecked(True)，跟頁面帶什麼值無關。
//    值有變才 TurnSiblingsOff（其他 10 顆 false）＋Click→OnClick（rbModeNormalClick）；點已經勾著的那顆 VCL 不 Click（golden 不跑處理器）。
//  運轉中：模式單選沒有運轉中例外（form.event 照舊 running）——golden 運轉中只有「機台沒有料」時點得到（timerContact :21648-21658 一有料就把 rgHandlerMode 停用，
//    移植樹沒有這支計時器）⇒ 比 golden 嚴（交件 human-review B）。T.Start／T.Step／One Cycle 有（FileRW/_FormEvent.cpp 檔尾 runexc 第 15～17 列）。
//  主控台：每一次印一行 "[B8-CT3A] …"（點了什麼、旗標前後值），上機看這一行。
//  測試：ctest B8_Ct3a_ContactFlags（tests/test_b8_ct3a_contactflags.cpp）。
// ===========================================================================
namespace {
const char* const kCt3aRb[11] = {"rbModeNormal", "rbAutoHeight", "rbManualHeight", "rbContactTest", "rbAutoContactTest", "rbStepContactTest",
                                 "rbDeviceMapping", "rbLoadCellAutoHigh", "rbKTempIndexMove", "rbDeviceLoopTest", "rbVisualDetectionTest"};   // golden cContact.dfm:18125-18318（rgHandlerMode 底下全部）
const char* const kCt3aBtn[3] = {"btnTStart", "btnTStep", "spbOneCycle"};   // golden cContact.dfm:16136／:16146／:15887
int g_ct3aRb = 0;   // 勾著的那一顆（kCt3aRb 的索引）；golden DFM rbModeNormal Checked=True（cContact.dfm:18131）

int Ct3aRbIndex(const TControl* c)
{
    for (int k = 0; c && k < 11; ++k)
        if (filerw::ELFind(kForm, kCt3aRb[k]) == c) return k;
    return -1;
}
void Ct3aImpose()   // VCL：勾著的那顆 true、同一個父層的其他顆 false
{
    for (int k = 0; k < 11; ++k) EL<TRadioButton>(kForm, kCt3aRb[k])->Checked = (k == g_ct3aRb);
}
// 靜態初始化時（g_evreg ← EvB3Table）就會被叫：只讀常數表
bool Ct3aOwns(const char* control)
{
    for (const char* n : kCt3aRb) if (control && std::strcmp(control, n) == 0) return true;
    for (const char* n : kCt3aBtn) if (control && std::strcmp(control, n) == 0) return true;
    return false;
}
}  // namespace

// VCL TRadioButton.SetChecked(True)：FormShow :1178、FormClose :1898（產生檔）與網頁點選（Ct3aRun）
void FileRW_Contact_RbClickTrue(TRadioButton* self)
{
    const int k = Ct3aRbIndex(self);
    if (k < 0) { if (self) self->Checked = true; return; }   // 不是 rgHandlerMode 的單選（今天沒有這種呼叫）：只設值
    const bool changed = k != g_ct3aRb;
    g_ct3aRb = k;
    Ct3aImpose();                                                               // FChecked:=True＋TurnSiblingsOff（兄弟取消不 Click）
    if (changed) DF_rbModeNormalClick(self);                                    // Click → OnClick＝rbModeNormalClick（golden cContact.cpp:15555）
}

// 存檔後的重讀（golden 存檔不碰模式單選）：替身照 g_ct3aRb 放回（頁面存檔可能套過別的值），不 Click
void FileRW_Contact_RbReimpose() { Ct3aImpose(); }

// 開機（FileRW_Contact_Boot 同一行、在 FileRW_Contact_EvBoot 之前）：14 個事件元件與燈號照 golden header 的型別建替身
//   （golden cContact.h:164-…／:79／:212-213；ELKeep 的預設 Visible／Enabled＝true，DFM 與 VCL 預設不同的那幾格 DF_DfmState 已經套過）
void FileRW_Contact_Ct3aBoot()
{
    for (const char* n : kCt3aRb) EL<TRadioButton>(kForm, n);
    EL<TButton>(kForm, "btnTStart");
    EL<TButton>(kForm, "btnTStep");
    EL<TSpeedButton>(kForm, "spbOneCycle");
    EL<TControl>(kForm, "ledOneCycle");
}

namespace {
void Ct3aRun(TControl* sender)
{
    const filerw::PageEvent* e = nullptr;
    for (std::size_t i = 0; i < sizeof(kDF_Events) / sizeof(kDF_Events[0]) && !e; ++i)
        if (sender && filerw::ELFind(kForm, kDF_Events[i].control) == sender) e = &kDF_Events[i];
    if (!e) throw std::runtime_error("B8-CT3A: form.event sender is not a TfContact event control proxy");
    std::map<std::string, std::string> pre;
    for (const char* n : kWatch) pre[n] = ValOf(n);
    const int mode0 = iContactMode;
    const bool home0 = fAllMotorHome, start0 = bSetupStart, step0 = bSetupStep, cont0 = bContinueContact;
    const int rb = Ct3aRbIndex(sender);
    const bool wasChecked = rb >= 0 && rb == g_ct3aRb;
    TRadioButton* r = rb >= 0 ? dynamic_cast<TRadioButton*>(sender) : nullptr;
    if (r) FileRW_Contact_RbClickTrue(r);                                       // VCL 點單選＝SetChecked(True)
    else e->handler(sender);                                                    // golden :2280／:2285／:17152
    FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();               // 同 CT-1 EvB3Run（CT-L2：golden TimerEP 每秒都在跑）  //AI(W906-B8-CT3D) 20261001 [W906] (St01): + golden OTDTimer beat (CT-3d, file tail); same line
    EvB3Merge(pre, e->control);
    if (r)
        std::printf("[B8-CT3A] form.event %s click -> golden %s%s: iContactMode %d -> %d, fAllMotorHome %d -> %d\n", e->control, e->golden,
                    wasChecked ? " not run (already checked: VCL does not Click)" : " + SetContactMode", mode0, (int)iContactMode, (int)home0,
                    (int)fAllMotorHome);
    else if (std::strcmp(e->control, "btnTStart") == 0)
        std::printf("[B8-CT3A] form.event btnTStart click -> golden %s: fContact->bSetupStart %d -> %d (TfContactShim; ckernel.cpp:324 "
                    "WaitManualStartKey takes it when the flow waits for T.Start)\n", e->golden, (int)start0, (int)bSetupStart);
    else if (std::strcmp(e->control, "btnTStep") == 0)
        std::printf("[B8-CT3A] form.event btnTStep click -> golden %s: fContact->bSetupStep %d -> %d (TfContactShim; ckernel.cpp:255 "
                    "WaitManualStepKey takes it when the flow waits for T.Step)\n", e->golden, (int)step0, (int)bSetupStep);
    else
        std::printf("[B8-CT3A] form.event spbOneCycle click -> golden %s: bContinueContact %d -> %d (iContactMode %d; golden OneCycleProcess forces 0 "
                    "below CONTACT_TEST=3; no reader in the port yet -- the golden contact state machines are gated)\n", e->golden, (int)cont0,
                    (int)bContinueContact, (int)iContactMode);
    std::fflush(stdout);
}
}  // namespace

// ===========================================================================
//  AI(W906-B8-CT3B) 20261001 [W906] (St01)：B8 CT-3b'（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」第 8 點第二段、P-2／P-3；
//    Jimmy RULINGS_20261001 第 0 條「照 golden 翻、會動的也接上」；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」）——
//    Contact 頁的 START／PAUSE。會讓機台動（START）／停（PAUSE）。
//  golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp（產生檔 FileRW/DeviceForm_File.gen.inc DF_btnStartClick／DF_btnPauseClick，
//    設定 tools/editlist/DeviceForm_File.py 的 _CT3B_*）：
//    btnStartClick :14072-14077  fMain->BtnStartClick(fMain); fMain->edTorue0->Text="10"; fMain->edTorue1->Text="10";
//    btnPauseClick :14079-14082  fMain->BtnPauseClick(fMain);
//  (1) START 是主畫面 START 鈕的 OnClick，不是面板 START 鍵：golden TfMain::BtnStartClick（V912 main.cpp:6529-6593，本段 Ct3bBtnStartClick 逐句）——
//      CosFunction.bEnableSoftWareControlButton（TSMC 台南／SIGURD 北興／UTAC TW，CosFunction.cpp:288／:1390／:3512）：Teradyne-US 等級低於 HonPrec 就 return，
//        否則 Start("bEnableSoftWareControlButton")（:6531-6536）；
//      否則只有 SOFT_SIMULTE 建置：CheckAutoOnlySetOneBin（:6540，V912 :33616-33645）→ AAL 自動調光旗標（:6545-6576）→ [I52] AQL（:6579-6582）→
//        [P28] CheckAuto1OnlyBin1（:6585，V912 :33595-33614）→ Start("BtnStartClick_SOFT_SIMULTE")（:6590）；
//      ⚠ golden 事實（照翻、不修）：出貨建置（沒有 SOFT_SIMULTE）、客戶沒開 bEnableSoftWareControlButton ⇒ BtnStartClick 什麼都不做（真機的 START 是面板鍵
//        TfMain::ScanKey）——Contact 頁按 START 只會把主畫面兩格扭力框 edTorue0／edTorue1 設成 "10"。同 wb_serve W906_BtnHomeClick（main.home）的處理。
//      ⓘ 跟網頁主畫面 START 不同：wb_serve start.run 直接叫 TfMainWeb::StartFromWeb（當面板鍵用），不經 BtnStartClick（交件列給 Jimmy）。
//    Start(Func) 的落點：W906_RemoteRunStart(Func)（forms/fMain.h 檔尾；wb_serve 裝的是 TfMainWeb::StartFromWeb＝TfMain::Start 的翻譯，START 全套；
//      手動教導中先拒絕，同 start.run），本段 Ct3bStart 是本檔唯一的啟動呼叫點（ctest START_SitesCensus 多一個活的，tests/CMakeLists.txt --check）。
//      運轉中按：golden 照樣叫 Start，Start 跑完前面的檢查後在 :6137 if(SystemStart) return false ⇒ 移植樹 StartFromWeb 同一句擋（WebStart.cpp:3165，那裡註的是舊版行號 :5871）。
//  (2) PAUSE：golden TfMain::BtnPauseClick（V912 main.cpp:7387-7393）Pause("BtnPauseClick")＋#ifndef SOFT_SIMULTE OEE fProductionInfo->ClickPause()。
//      移植樹 TfMain::BtnPauseClick（forms/fMain.cpp:497）只翻了第一句 ⇒ Pause（:246）→ W906_RemoteRunPause → TfMainWeb::PauseFromWeb
//      （WebStart.cpp，golden :6325-6379 的翻譯：SoftStop=true、StopAllMotor、SECS DoPause、ESD_SYSTEM_STOP）。OEE ClickPause 沒翻（forms/fMain.h:484-488 記過，
//      網頁主畫面 pause.run 同樣沒有）——交 Jimmy。golden Pause 不查 SystemStart（停機時按也是 SoftStop=true＋StopAllMotor）。
//  (3) 為什麼兩支都在回覆之後跑（form.event after-ack，FileRW/_FormEvent.cpp 檔尾；同 B8 OS-1b FileRW/Offset_File.cpp 檔尾）：
//      處理器在 W906_FormEvent 裡、持 FormLock；START（TfMain::Start）有 YES／NO 框等瀏覽器回答；PAUSE 的 SetRunStartMode(rsmAutoSiteMap)
//      （golden :6369，[I50] Pause 觸發 Auto Site Map）在 SCK ART 機台會變成 rsmInitial_ART（RunStartMode.cpp:244）、KYEC_LEE 沒開 [A10] 就
//      ShowMyMessage("No Run ART Mode")（RunStartMode.cpp:526）——也是等瀏覽器。鎖裡等框＝socket 執行緒卡在 GET /api/editlist 的鎖上收不到回答＝整台卡死。
//      ⇒ 處理器只登記，wb_serve form.event 臂 CompleteCommand 之後（鎖外、主迴圈執行緒）照登記順序跑；跟 golden 一樣在同一次點擊裡，中間沒有 MainProc 拍子。
//      START 那一項照 golden 順序：BtnStartClick 先、edTorue0／edTorue1="10" 後（golden :14074-14076；BtnStartClick 裡的 return 只離開 BtnStartClick，
//      扭力框照設）。edTorue0／edTorue1＝forms/fMain.h:1262-1263 的真成員（讀者：atester.cpp 12110 等扭力值、cinitial.cpp ShowMainScreenPresure；rs232.cpp 寫）。
//  (4) 運轉中：golden fContact 非模態（sbContactClick main.cpp:28302-28317，Show() :28314），兩顆鈕的處理器都不查 SystemStart／SoftStart，
//      主畫面 BtnStart 運轉中也不停用（ProcessKeyFlush main.cpp:4228-4393 只停用 CleanOut／TrayEnd／Home／Reset／AlarmReset）
//      ⇒ FileRW/_FormEvent.cpp 檔尾 runexc 第 18～19 列（兩格都放行；頁面表要說 fContact 開著）。看得見才點得到照 RunPageEvent：
//      [D16] Step Contact Test（FormShow :1628-1629）；SOFT_SIMULTE 一律看得見（:1662-1663）。沒有等級閘（pnlBottom，FormShow 不停用）。
//  主控台：每一次印 "[B8-CT3B] …"（點了什麼、登記了什麼；回覆之後再印 BtnStartClick 走哪一支、StartFromWeb 有沒有被叫、扭力框前後值）。
//  測試：ctest B8_Ct3b_ContactStartPause（tests/test_b8_ct3b_contactstartpause.cpp）。
// ===========================================================================
#include "FileRW/_FormEvent.h"   // formevent::afterack::Defer（form.event 的 after-ack 佇列）
#include "MainCalcCore.h"        // ComputeCheckAutoOnlySetOneBin／ComputeCheckAuto1OnlyBin1（golden TfMain::CheckAutoOnlySetOneBin／CheckAuto1OnlyBin1 的判定）

void ShowMyMessage(AnsiString S1, AnsiString S2, AnsiString S3, bool Ok, bool bServoOff);   // canary_support.h:80（golden mymessbox.h:58；wb_serve 轉到網頁）
int FileRW_InArmSuckShtRow();   // FileRW/_KitSuck.cpp 檔尾（golden main.cpp:6556 InArmSuck.iShtRow；本 TU 不能 include aHotPlateSubstrate.h）

namespace {
const char* const kCt3bBtn[2] = {"btnStart", "btnPause"};   // golden cContact.dfm:236／:246（pnlBottom）

// 靜態初始化時（g_evreg ← EvB3Table）就會被叫：只讀常數表
bool Ct3bOwns(const char* control)
{
    for (const char* n : kCt3bBtn) if (control && std::strcmp(control, n) == 0) return true;
    return false;
}

// golden TfMain::Start(Func) → W906_RemoteRunStart（本檔唯一的啟動呼叫點）
bool Ct3bStart(const char* func)
{
    const bool called = W906_RemoteRunStart(AnsiString(func));
    std::printf("[B8-CT3B] golden TfMain::Start(\"%s\") -> %s (SoftStart=%d SystemStart=%d)\n", func,
                called ? "TfMainWeb::StartFromWeb was called" : "NOT started (W906_RemoteRun not installed, or manual teach refused START)",
                (int)SoftStart, (int)SystemStart);
    std::fflush(stdout);
    return called;
}

#ifdef SOFT_SIMULTE
// golden TfMain::CheckAutoOnlySetOneBin（V912 main.cpp:33616-33645）：判定是 MainCalcCore 的可攜版（MainCalcCore.cpp，逐句、不含對話框），
//   對話框 golden :33637 ShowMyMessage(str1, str2) 在這裡補回（同 tools/wb_serve.cpp W906_CheckAutoOnlySetOneBin）。
bool Ct3bCheckAutoOnlySetOneBin()
{
    AnsiString str1, str2;
    if(ComputeCheckAutoOnlySetOneBin(CosFunction.bUsePassBinOnlyCanSetOneBin, Prod.iIsPassT6, Prod.iT6CatData, iTestBinCount, s6TrayName, str1, str2))
    {
        ShowMyMessage(str1, str2, "", false, false);                            // golden :33637
        return true;                                                            // golden :33638
    }
    return false;                                                               // golden :33644
}
// golden TfMain::CheckAuto1OnlyBin1（V912 main.cpp:33595-33614）：同上；對話框 golden :33604／:33609（單參數）。
bool Ct3bCheckAuto1OnlyBin1()
{
    AnsiString msg;
    if(ComputeCheckAuto1OnlyBin1(CUSTOMER_CODE, LastSet.iTester, Prod.iT6PosCate, iTestBinCount, msg))
    {
        ShowMyMessage(msg, "", "", false, false);                               // golden :33604／:33609
        return true;                                                            // golden :33605／:33610
    }
    return false;                                                               // golden :33598／:33613
}
#endif

// golden TfMain::BtnStartClick（V912 main.cpp:6529-6593）逐句；自由函式（移植樹 TfMain 沒有這個成員，forms/fMain.* 不是 St01 的檔）。
// PORT-ONLY：回傳值（golden 是 void）只為了主控台說明走了哪一支 —— 0 = 什麼都沒做（golden 非模擬的 else 支）、1 = Teradyne-US 等級不足、
//   2 = Start 已呼叫（StartFromWeb）、3 = Start 沒呼叫成功（安裝座沒裝／手動教導中）、4 = CheckAutoOnlySetOneBin 擋、5 = [P28] CheckAuto1OnlyBin1 擋。
int Ct3bBtnStartClick()
{
    if(CosFunction.bEnableSoftWareControlButton)                                //ChungHung 20150609 add only for TSMC
    {
        if(CUSTOMER_CODE==CC_TERADYNE_US && AccessLevel<iDefHonPrecLevel)       // golden :6533
            return 1;                                                           // golden :6534
        return Ct3bStart("bEnableSoftWareControlButton") ? 2 : 3;               // golden :6535
    }
    else
    {
#ifdef SOFT_SIMULTE
        if(Ct3bCheckAutoOnlySetOneBin())                                        // golden :6540 CheckAutoOnlySetOneBin()
        {
            return 4;
        }

        //==> Eastsun 20260527 整合#028.AAL.P-rev17 SOFT_SIMULTE AAL flag setup :KYEC
        if(CosFunction.bUseBarcodeAutoAdjustLight==true &&
           TestIF_File.bUseBarcodeAutoAdjustLight==true &&
           fMain->cbRunStartMode->Text.Pos("Initial")!=0)                       //Ifor 20210408 add:Barcode 自動調整光源
        {
            bStartAutoAdjustLight=true;
            for(int i=0; i<4; i++)
            {
                bBarcodeNeedAutoAdjust[i]=true;
            }

            if(FileRW_InArmSuckShtRow()==1)                                     // golden :6556 InArmSuck.iShtRow==1（FileRW/_KitSuck.cpp 轉接）
            {
                bBarcodeNeedAutoAdjust[1]=false;
                bBarcodeNeedAutoAdjust[2]=false;
            }

            if(TestIF_File.iShuttleMode==1)
            {
                if(TestIF_File.iShuttle_Sel==0)         //Front Arm Only
                {
                    bBarcodeNeedAutoAdjust[2]=false;
                    bBarcodeNeedAutoAdjust[3]=false;
                }
                else if(TestIF_File.iShuttle_Sel==1)    //Rear Arm Only
                {
                    bBarcodeNeedAutoAdjust[0]=false;
                    bBarcodeNeedAutoAdjust[1]=false;
                }
            }
        }
        //<== Eastsun 20260527 整合#028.AAL.P-rev17

        //==> Eastsun 20260520 整合
        if(IniConfig.bI52_bAQLSortMode==true)                                   //Ifor 20210322 add:New AQL
        {
#if 0 // GATE (W906-B8-CT3B) golden main.cpp:6581 fLotInfo->SetAQLMode() -- forms/fLotInfo.h has no SetAQLMode (same gap as SECSGEM/uHGemHT9045.cpp G39); the AQL sort mode is not set
            fLotInfo->SetAQLMode();
#endif
            std::printf("[B8-CT3B] TODO golden main.cpp:6581 fLotInfo->SetAQLMode() not done ([I52] AQL sort mode; forms/fLotInfo.h has no SetAQLMode)\n");
        }
        //<== Eastsun 20260520

        if(IniConfig.bP28Auto1OnlyBin1==true && Ct3bCheckAuto1OnlyBin1())      //Ifor 20171017 P28 功能整理
        {
            return 5;
        }

        return Ct3bStart("BtnStartClick_SOFT_SIMULTE") ? 2 : 3;                 // golden :6590
#endif
    }
    return 0;
}

// after-ack：golden cContact.cpp:14074-14076（回覆之後、鎖外、主迴圈執行緒）
void Ct3bRunStartAfterAck(const std::string&)
{
    if (!fMain) { std::printf("[B8-CT3B] fMain is null -- golden btnStartClick not run\n"); return; }   // PORT-ONLY 保險（golden 的 fMain 一定在）
    const int r = Ct3bBtnStartClick();                                          // :14074 fMain->BtnStartClick(fMain)
    const AnsiString t0 = fMain->edTorue0->Text, t1 = fMain->edTorue1->Text;
    fMain->edTorue0->Text="10";                                                 // :14075
    fMain->edTorue1->Text="10";                                                 // :14076
    static const char* const kWhat[] = {
        "did nothing (golden: no SOFT_SIMULTE and CosFunction.bEnableSoftWareControlButton off -- a real machine starts from the panel START key)",
        "returned: Teradyne-US below the HonPrec level (golden main.cpp:6533-6534)",
        "called TfMain::Start (StartFromWeb)",
        "TfMain::Start was NOT called (W906_RemoteRun not installed, or manual teach refused START)",
        "returned: CheckAutoOnlySetOneBin refused (golden main.cpp:6540, message shown)",
        "returned: [P28] CheckAuto1OnlyBin1 refused (golden main.cpp:6585, message shown)",
    };
    std::printf("[B8-CT3B] golden TfContact::btnStartClick (cContact.cpp:14072) after the reply: TfMain::BtnStartClick %s (code %d); "
                "fMain->edTorue0 \"%s\" -> \"%s\", edTorue1 \"%s\" -> \"%s\" (golden :14075-14076)\n",
                kWhat[r >= 0 && r <= 5 ? r : 0], r, t0.c_str(), fMain->edTorue0->Text.c_str(), t1.c_str(), fMain->edTorue1->Text.c_str());
    std::fflush(stdout);
}
// after-ack：golden cContact.cpp:14081
void Ct3bRunPauseAfterAck(const std::string&)
{
    if (!fMain) { std::printf("[B8-CT3B] fMain is null -- golden btnPauseClick not run\n"); return; }   // PORT-ONLY 保險
    fMain->BtnPauseClick(fMain);                                                // :14081
    std::printf("[B8-CT3B] golden TfContact::btnPauseClick (cContact.cpp:14079) after the reply: TfMain::BtnPauseClick -> Pause(\"BtnPauseClick\") "
                "(SoftStop=%d SystemStart=%d)\n", (int)SoftStop, (int)SystemStart);
    std::fflush(stdout);
}

void Ct3bRun(TControl* sender)
{
    const filerw::PageEvent* e = nullptr;
    for (std::size_t i = 0; i < sizeof(kDF_Events) / sizeof(kDF_Events[0]) && !e; ++i)
        if (sender && filerw::ELFind(kForm, kDF_Events[i].control) == sender) e = &kDF_Events[i];
    if (!e) throw std::runtime_error("B8-CT3B: form.event sender is not a TfContact event control proxy");
    std::map<std::string, std::string> pre;
    for (const char* n : kWatch) pre[n] = ValOf(n);
    const int q0 = formevent::afterack::Pending();
    e->handler(sender);                                                         // golden :14072／:14079（產生檔：登記 after-ack 動作）
    FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();               // 同 CT-1 EvB3Run／CT-3a Ct3aRun（CT-L2：golden TimerEP 每秒都在跑）  //AI(W906-B8-CT3D) 20261001 [W906] (St01): + golden OTDTimer beat (CT-3d, file tail); same line
    EvB3Merge(pre, e->control);
    std::printf("[B8-CT3B] form.event %s click -> golden %s: %d after-ack action queued (runs after this reply, outside FormLock; SystemStart=%d SoftStart=%d)\n",
                e->control, e->golden, formevent::afterack::Pending() - q0, (int)SystemStart, (int)SoftStart);
    std::fflush(stdout);
}
}  // namespace

// 產生檔 DF_btnStartClick 呼叫（golden :14074-14076 那三行；處理器在 W906_FormEvent 裡、持 FormLock）
void W906_Contact_B8BtnStartClick()
{
    formevent::afterack::Defer(&Ct3bRunStartAfterAck, "",
                               "golden cContact.cpp:14074-14076 TfContact::btnStartClick: TfMain::BtnStartClick (main.cpp:6529-6593; starts only with "
                               "CosFunction.bEnableSoftWareControlButton or in SOFT_SIMULTE) then edTorue0 / edTorue1 = \"10\", right after this reply");
}
// 產生檔 DF_btnPauseClick 呼叫（golden :14081）
void W906_Contact_B8BtnPauseClick()
{
    formevent::afterack::Defer(&Ct3bRunPauseAfterAck, "",
                               "golden cContact.cpp:14081 TfContact::btnPauseClick: TfMain::BtnPauseClick -> Pause(\"BtnPauseClick\") (main.cpp:7387-7393), right after this reply");
}

// ===========================================================================
//  AI(W906-B8-CT3D) 20261001 [W906] (St01)：B8 CT-3d（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」第 8 點第四段；
//    Jimmy RULINGS_20261001 第 0 條「照 golden 翻、會動的也接上」；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」）——
//    Contact 頁 OTD（One Touch Docking，測試頭 Dock 鎖）兩顆面板＋OTDTimer。會動氣缸（Dock Y／X 軸鎖）、會改兩個輸出點。
//  golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cContact.cpp（產生檔 FileRW/DeviceForm_File.gen.inc DF_palOTD_4Click／
//    DF_palOTD_6Click／DF_OTDTimerTimer，設定 tools/editlist/DeviceForm_File.py 的 _CT3D_*）：
//    palOTD_4Click :15395-15418 "OTD Under 240KG"：bDown==false ⇒ YOn On、YOff Off、XOn Off、XOff On（Y 鎖、X 不鎖）；再按一次 ⇒ 回 Off 組
//      （YOn Off、YOff On、XOn Off、XOff On）。
//    palOTD_6Click :15420-15443 "OTD Over 360KG"：bDown==false ⇒ YOn On、YOff Off、XOn On、XOff Off（Y、X 都鎖）；再按一次 ⇒ 回 Off 組。
//      兩支各有自己的 static bDown，另一顆面板只被改回凸起（BevelOuter=bvRaised），它的 bDown 不動 ⇒ golden 原樣的結果：按 240、再按 360
//      （360 的 bDown 還是 false ⇒ 進「都鎖」）、再按 240（240 的 bDown 留著 true ⇒ 走「回 Off 組」）。照抄、不修。
//      golden 沒有任何檢查（不查 SystemStart、門、Index 位置、Dock 感測器）。
//    OTDTimerTimer :15445-15507（DFM Interval=100 ms；FormShow :1646 Enabled=(USE_OTD==1)，FormClose :1844 關）：依四顆氣缸的 Status 與 On 感測器
//      決定 ledOTD（紅＝沒插好／其他、綠＝鎖好、滅＝Dock 全開）與兩個輸出點 SW[SwUnDock]（全開）、SW[SwDockError]（錯誤）（cmydef.cpp:2060-2061）。
//      bBusy、InitialOK==false、IO 頁開著（W906_FormShowing("fiosetview")）不做。golden 疑點 :15478／:15483-15485 見產生器設定（照翻）。
//  跳板 Ct3dRun（不走 CT-1 的 session 跳板 EvB3Run）：golden 點了就動氣缸，沒有「存檔才生效」（B8_RISK「CT-3」第 4 點：CT-3 要直接寫機台用的）。
//    處理器之後照 CT-1／CT-3a／CT-3b 補 TimerEP 一拍、再補 OTDTimer 一拍（golden 100 ms 後就會跑），更新存檔前推導的比較基準（EvB3Merge）。
//    主控台每一次印一行 "[B8-CT3D] …"（四顆氣缸 Status 前後；OTDTimer 那一拍有變再印燈與兩個輸出點），上機看這一行。
//  計時器拍子（B8 P-1）：golden 每 100 ms 一拍；網頁只在開頁與 form.event 回覆拿畫面 ⇒ 開頁（FormShowAndSnap）與每個 form.event 之後
//    （EvB3Run／Ct3aRun／Ct3bRun／Ct3dRun）各補一拍。⚠ 跟 golden 不同（寫明）：兩個輸出點只在頁面有動作時更新——Dock 感測器自己變了
//    （例：測試頭被手動插拔）要等下一次開頁或點擊才反映。定時拍子（wb_serve 主迴圈，P-1，誰擁有那一行就誰接）可以直接呼叫
//    FileRW_Contact_OTDTimerTick（Enabled 才跑；每一拍都照 golden 重算，多補的拍子不改結果）。
//  看得見才點得到：palOTD_4／palOTD_6 Visible=(USE_OTD==1)（FormShow :1329-1330）⇒ RunPageEvent 第 2 步；開發機 USE_OTD=2 ⇒ 看不到、點不到、計時器不開。
//  運轉中：Steven Q65＝B（1002 08:0x）⇒ 照 golden 放行：FileRW/_FormEvent.cpp runexc 第 20～21 列（TfContact palOTD_4／palOTD_6，SystemStart、
//    SoftStart 都放行；頁面表要說 fContact 開著）。golden 運轉中按得到（處理器不查、fContact 非模態 main.cpp:28302-28317），流程不等這兩顆；
//    按了就在運轉中動 Dock 氣缸（規則例外，human-review C；上機由 EastSun 在 USE_OTD==1 的機台看）。Ct3dRun 本身照舊不查狀態（golden 也不查）。
//    AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01)：CT-3d 交件時沒有 runexc 列、運轉中照舊回 running（比 golden 嚴，Steven 決定 A 維持拒收／B 加 2 列；
//    動 Dock 氣缸是安全判斷，所以沒有自己決定放行）；Steven 回 B。
//  測試：ctest B8_Ct3d_OtdDock（tests/test_b8_ct3d_otddock.cpp）、B8_Ct3d_ContactPage（tools/webprobe/ct3d_contact_ev_selftest.cjs）。
// ===========================================================================
namespace {
const char* const kCt3dPnl[2] = {"palOTD_4", "palOTD_6"};   // golden cContact.dfm:16329／:16355（TPanel，OnClick=palOTD_4Click／palOTD_6Click）

// 靜態初始化時（g_evreg ← EvB3Table）就會被叫：只讀常數表
bool Ct3dOwns(const char* control)
{
    for (const char* n : kCt3dPnl) if (control && std::strcmp(control, n) == 0) return true;
    return false;
}

// 四顆 Dock 氣缸的 Status、燈、兩個輸出點（主控台前後對照用；測試的記錄點也是這幾個欄位）
struct Ct3dIo {
    bool cyl[4];            // C_DockYAxisOn／C_DockYAxisOff／C_DockXAxisOn／C_DockXAxisOff 的 Status（On()／Off() 設，mycylin.cpp:344-362）
    bool unDock, dockErr;   // SW[SwUnDock]／SW[SwDockError] 的 OutValue（On()／Off() 第一句就設，myswitch.cpp）
    int led;                // ledOTD 替身 Tag（0 滅／1 綠／2 紅）
};
Ct3dIo Ct3dNow()
{
    Ct3dIo s;
    const int ids[4] = {C_DockYAxisOn, C_DockYAxisOff, C_DockXAxisOn, C_DockXAxisOff};
    for (int i = 0; i < 4; ++i) s.cyl[i] = Cylinder[ids[i]].Status;
    s.unDock = SW[SwUnDock].OutValue;
    s.dockErr = SW[SwDockError].OutValue;
    s.led = (int)EL<TControl>(kForm, "ledOTD")->Tag;
    return s;
}
const char* Ct3dLed(int t) { return t == DF_ledOTDRed ? "red" : t == DF_ledOTDLime ? "lime" : "off"; }
std::string Ct3dIoText(const Ct3dIo& a, const Ct3dIo& b, bool cylinders)
{
    static const char* const kName[4] = {"C_DockYAxisOn", "C_DockYAxisOff", "C_DockXAxisOn", "C_DockXAxisOff"};
    char buf[256];
    std::string s;
    if (cylinders) {
        for (int i = 0; i < 4; ++i) {
            std::snprintf(buf, sizeof(buf), "%s%s %d -> %d", i ? ", " : "Cylinder Status ", kName[i], (int)a.cyl[i], (int)b.cyl[i]);
            s += buf;
        }
        return s;
    }
    std::snprintf(buf, sizeof(buf), "ledOTD %s -> %s, SW[SwUnDock] %d -> %d, SW[SwDockError] %d -> %d", Ct3dLed(a.led), Ct3dLed(b.led),
                  (int)a.unDock, (int)b.unDock, (int)a.dockErr, (int)b.dockErr);
    return buf;
}
}  // namespace

// 開機（FileRW_Contact_Boot 同一行、在 FileRW_Contact_EvBoot 之前）：兩顆面板、燈、計時器照 golden header 的型別建替身
//   （golden cContact.h：palOTD_4／palOTD_6 TPanel、ledOTD TMyLed、OTDTimer TTimer；後兩個 vclcompat 沒有 → TControl，同產生檔）
void FileRW_Contact_Ct3dBoot()
{
    for (const char* n : kCt3dPnl) EL<TPanel>(kForm, n);
    EL<TControl>(kForm, "ledOTD");
    EL<TControl>(kForm, "OTDTimer");
}

// golden TTimer OnTimer＝OTDTimerTimer：Enabled=false 的計時器不會觸發
void FileRW_Contact_OTDTimerTick()
{
    if (!g_booted) return;
    if (!EL<TControl>(kForm, "OTDTimer")->Enabled) return;                     // golden FormShow :1646 開（USE_OTD==1）、FormClose :1844 關
    const Ct3dIo a = Ct3dNow();
    DF_OTDTimerTimer();                                                         // golden cContact.cpp:15445
    const Ct3dIo b = Ct3dNow();
    if (a.led != b.led || a.unDock != b.unDock || a.dockErr != b.dockErr) {    // 有變才印（每個 form.event 都會補一拍）
        std::printf("[B8-CT3D] OTDTimer beat (golden cContact.cpp:15445-15507): %s\n", Ct3dIoText(a, b, false).c_str());
        std::fflush(stdout);
    }
}

namespace {
void Ct3dRun(TControl* sender)
{
    const filerw::PageEvent* e = nullptr;
    for (std::size_t i = 0; i < sizeof(kDF_Events) / sizeof(kDF_Events[0]) && !e; ++i)
        if (sender && filerw::ELFind(kForm, kDF_Events[i].control) == sender) e = &kDF_Events[i];
    if (!e) throw std::runtime_error("B8-CT3D: form.event sender is not a TfContact event control proxy");
    std::map<std::string, std::string> pre;
    for (const char* n : kWatch) pre[n] = ValOf(n);
    const Ct3dIo io0 = Ct3dNow();
    e->handler(sender);                                                         // golden :15395／:15420（Cylinder[].On()／Off()，沒有任何檢查）
    const Ct3dIo io1 = Ct3dNow();
    std::printf("[B8-CT3D] form.event %s click -> golden %s: %s (OTDTimer %s; SystemStart=%d SoftStart=%d)\n", e->control, e->golden,
                Ct3dIoText(io0, io1, true).c_str(), EL<TControl>(kForm, "OTDTimer")->Enabled ? "on: one beat follows" : "off: USE_OTD!=1 or Contact closed",
                (int)SystemStart, (int)SoftStart);
    std::fflush(stdout);
    FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();               // 同 CT-1 EvB3Run／CT-3a／CT-3b（golden TimerEP 1 s、OTDTimer 100 ms 都在跑）
    EvB3Merge(pre, e->control);
}
}  // namespace
