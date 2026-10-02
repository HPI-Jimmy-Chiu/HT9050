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
void FileRW_Contact_TimerEPTick();  /* AI(W906-B8-CTL2) 20260930 [W906]: golden TimerEP 一拍（本體檔尾）；同一行插入 */  std::string FileRW_Contact_KbExtraJson();  /* AI(W906-SETUPA-KB) 20261002: editlist.get extra（golden 小鍵盤的執行期上下限＋edDropWaitTimeMouseDown 輸入後修正要的值；FileRW/DeviceForm_KbExtra.cpp）；同一行插入 */  namespace {
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
    DF_FormShow();  FileRW_Contact_TimerEPTick();  /* AI(W906-B8-CTL2) 20260930 [W906]: golden FormShow 開 TimerEP（EP_Install 3／5，1000 ms）⇒ 頁面拿到的是第一拍之後的畫面（檔尾）；同一行插入 */  // golden FormShow 已把衍生欄位算好（ShowArmAndDeviceForce）
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
    DF_CreateContainerProxies(); { void FileRW_Contact_EvBoot(); FileRW_Contact_EvBoot(); }   //AI(W906-EVB3) 20260928 [W906]: form.event 控制項替身檢查（檔尾）；同一行附加
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

namespace {
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
        EvB3Body(*e, sender);  FileRW_Contact_TimerEPTick();   //AI(W906-B8-CTL2) 20260930 [W906]: golden TimerEP 每秒都在跑 ⇒ 事件回覆的 changed 帶下一拍之後的畫面（同 FileRW/IniConfig.cpp 的 Timer1 一拍，檔尾）；同一行附加
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
        t[i].handler = &EvB3Run;
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
