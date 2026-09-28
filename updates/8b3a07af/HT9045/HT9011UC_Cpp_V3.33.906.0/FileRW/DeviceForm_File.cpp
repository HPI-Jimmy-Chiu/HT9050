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

#include "FileRW/_EditPage.h"
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbContactClickOpen（FileRW/MainClick.cpp；Q41 第 3 項 CT-4，R84）；佔用原本的空行
namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

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
    DF_FormShow();                                                          // golden FormShow 已把衍生欄位算好（ShowArmAndDeviceForce）
    if (ContactForceInputsMissing())
        filerw::ELTodo("ContactForceTables() not loaded while CosFunction.bUseDynamicKitDiameter -- Kit Diameter items / Contact Force are not golden values (LoadContactForceTables)");
    TakeSnapshot(); if (const char* w = W906_Main_sbContactClickOpen()) filerw::ELTodo(w);   // AI(W906-FRW-S158) 20260927 [W906]: R84＝A —— golden V912 main.cpp:28314 fContact->Show()（非 modal）之後的 :28315-28316（DoStructUnitConvert、SetWorkParameter）在開窗當下跑；運轉中不跑（R86）；接在同一行
}

// golden 關頁 FormClose（:1842）的 ReadFile＋DoIniDataToForm（:1847-1848）—— 只取「替身還原成檔案值」那一半；
// FormClose 其餘（SystemStart=false、fAllMotorHome=false、NewRecordProcess、MyEtherCAT->Pause…）是執行期效果，網頁不跑
void Reload()
{
    DF_ReadFile();
    DF_DoIniDataToForm();
    DF_edAirForceChange();                                                  // VCL：DoIniDataToForm :939 改 edAirForce->Text 會觸發 edAirForce.OnChange
    TakeSnapshot();
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
    DF_DeriveBeforeSave();
    DF_spbSaveClick();
    TakeSnapshot();                                                         // golden 存後 ReadFile＋DoIniDataToForm（:14262-14263）後的畫面
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
    &FormShowAndSnap, &DF_SaveFlow, "SaveSetupFile", &Reload, &Booted,
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
    DF_CreateContainerProxies();
    BuildReads();
    g_page.saveReads = g_reads.data();
    g_page.nSaveReads = (int)g_reads.size();
    std::printf("FileRW DeviceForm_File: TfContact proxies ready (%d save reads, %d must-send) -- golden cContact.cpp\n",
                (int)(sizeof(kDF_SaveReads) / sizeof(kDF_SaveReads[0])), g_page.nSaveReads);
    g_booted = true;
}
