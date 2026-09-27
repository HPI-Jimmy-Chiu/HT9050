// ===========================================================================
//  FileRW/CfgTrayPlate.cpp -- golden TfConfiguration 的 Tray／Plate 資料表（Configuration 頁 tsTrayData／tsHPData 分頁）：
//    讀 sbtReloadTrayClick（V912 cConfiguration.cpp:7037）／sbtReloadHPClick（:7148），
//    寫 sbUpdateTrayClick（:7012）／sbUpdateHPClick（:7194）。
//
//  //AI(W906-FRW-S98) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S98）。golden 一律照主 repo V912
//    D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950 → UTF-8，行號不變）。
//
//  ---- 為什麼是新檔、不是 cConfiguration.cpp 那兩支 ----------------------------------------------------------------
//    移植樹 cConfiguration.cpp:350／:468 已經有 sbtReloadTrayClick／sbtReloadHPClick 的逐字翻譯，但它們是門面
//    forms/fConfiguration.h 的 TfConfiguration 成員，而那個類別沒有執行期實例（fConfiguration.h 的 INTEGRATION STATUS：
//    建構子是寫檔路徑、延後）。要用它就得 new 一個 TfConfiguration：會把 cConfiguration.cpp 整支（FormShow、
//    InitConfigEdtList_*… 上千行）拉進 wb_serve 的連結，還會與 C 路 FileRW/IniConfig.cpp（同一個 golden 表單的具名替身）
//    變成兩份 TfConfiguration。所以這裡只放 golden 這兩個分頁用到的元件＋四個處理器，一份，全樹共用。
//    ⚠ cConfiguration.cpp:350／:468 那兩份仍在、仍沒有呼叫者；不要把它們接起來（會多出第二張表）。
//
//  ---- golden 什麼時候讀、什麼時候寫（開機順序照 golden）------------------------------------------------------------
//    ① CreateForm(TfConfiguration)（HT9045.cpp:207）→ 建構子尾段 cConfiguration.cpp:226-234
//         if(bHasTrayCSV) sbtReloadTray->Click();  if(bHasPlateCSV) sbtReloadHP->Click();
//       → W906_CfgTrayPlate_CreateForm()（tools/wb_serve.cpp，FileRW_IniConfig_Boot() 之後；片段見交件報告）。
//       ⚠ golden 在這一刻 bHasTrayCSV／bHasPlateCSV 還是初值 false（cmydef.cpp:5119-5120）：兩個旗標要到
//         Application->Run 之後的 TfMain::FormShow（main.cpp:9987-9988）才設。所以 **golden 開機時建構子這兩行不會讀檔**，
//         表格是「用到的地方先 Click 一次 Load Data」才讀。這裡照 golden 順序接，開機時一樣不讀。
//    ② TfMain::FormShow main.cpp:9987-9988 bHasTrayCSV=FileExists(TrayTablePath); bHasPlateCSV=FileExists(PlateTablePath);
//       → W906_CfgTrayPlate_MainFormShowFlags()（tools/wb_serve.cpp，InitialHandler() 之前；golden :9992 InitialHandler）。
//    ③ 讀（sbtReloadTray->Click()，每次都重讀檔）：
//         TfConfiguration::FormShow :4676-4684（C 路 FileRW/IniConfig.gen.inc IC_FormShow 那兩行 EL<TSpeedButton>(…)->Click()：
//           本檔把這兩顆鈕登記成同名具名替身，Click() 才會真的跑到這裡）；
//         TfTrayForm::FormShow cTrayForm.cpp:162-175（C 路 FileRW/UserDefForm_File.gen.inc，本波解閘）；
//         TfCleaning::FormShow uCleaning.cpp:1488-1499（C 路 FileRW/TestIF_File_Cleaning.gen.inc，本波解閘）；
//         TfHotPlate::FormShow cHotPlate.cpp:47-58（A 形狀 FileRW/HotPlateForm_File.cpp，sbtReloadHP；tools/formbridge/TfHotPlate.py
//           在 :49 前宣告同名區域變數 fConfiguration=W906_CfgTrayPlate()，S98 收尾 20260926 解閘）；
//         還沒接的：TfTrayForm::ShowTypePage cTrayForm.cpp:583（頁面「Select from Database」選一筆）、TfCleaning::cbbSelectTrayChange
//         uCleaning.cpp:2326（ASE 高雄／OSE／K3 才顯示，S25）、TfHotPlate::cbSelectHPFromDBChange cHotPlate.cpp:424（頁面選一筆；
//         A 形狀沒有事件入口）、TfHotPlate::ShowTypePage cHotPlate.cpp:747（golden 只有 MES 下載 ProductionInfo.cpp:5471 呼叫，移植樹 #if 0）。
//    ④ 寫：只有 Configuration 頁 tsTrayData／tsHPData 分頁的 Save 鈕（sbUpdateTray／sbUpdateHP）。網頁沒有這兩個分頁
//       （FileRW/IniConfig.gen.inc 的 kIC_EnableAll 本來就排除 tsTrayData 等）⇒ 目前**沒有呼叫者**，開機與開頁都不寫。
//
//  ---- 會碰到的真實檔 ------------------------------------------------------------------------------------------------
//    D:\HT9045\System\TrayForm.csv、PlateForm.csv：③ 只讀（TStringList::LoadFromFile）；④ 整檔覆寫（SaveToFile），目前沒有呼叫者。
//    路徑是 golden 的全域 TrayTablePath／PlateTablePath（common.cpp:234-235，寫死字面值，--dry 蓋不到）。
//
//  ---- 照 golden 留著、看起來像錯的地方（不修，使用者決定）--------------------------------------------------------------
//    (a) sbUpdateHPClick 存完 PlateForm.csv 之後呼叫的是 sbtReloadTray->Click()、清的是 sbUpdateTray->Down（golden :7215-7216）——
//        應該是 sbtReloadHP／sbUpdateHP，golden 抄 Tray 那支沒改。後果：HP 表存檔後不重讀，畫面留著剛打的值。
//    (b) reload 的解析只收「後面有逗號」的欄位（golden :7065-7075 do/while 只在找到 ',' 時存 S2）；update 用 CommaText 寫，
//        最後一欄後面沒有逗號 ⇒ 存一次再讀，第 16 欄（表頭 BlockPitchY）會掉。這台 TrayForm.csv 每行結尾都有 ','，
//        所以現在讀得到 16 欄；經過 golden 的 Save 之後就只剩 15 欄。
//    (c) reload 先把所有引號拿掉（StringReplace "\"" → ""）才切逗號：欄位本身含逗號（CommaText 會加引號）時會切錯。
// ===========================================================================
#include "FileRW/CfgTrayPlate.h"

#include <cstdio>
#include <string>

#include "FileRW/_EditList.h"      // ELKeep／ELFind（C 路具名替身，FileRW/IniConfig.gen.inc 的 IC_FormShow 用同一顆鈕）
#include "cmydef.h"                // bHasTrayCSV／bHasPlateCSV（cmydef.h:5045-5046）
#include "common.h"                // TrayTablePath／PlateTablePath（common.h:77-78）
#include "vclcompat/SysUtils.h"    // FileExists

namespace {

// VCL TSpeedButton::Click() → TControl::Click() → OnClick（DFM 設計期綁定）。vclcompat::TControl::Click() 是空的
// （Controls.h:260「offline no-op」），golden 這四顆鈕卻是靠 ->Click() 去跑處理器（例 cTrayForm.cpp:164），所以包一層。
// 只改 Click()，其餘（Down、GroupIndex、Caption）照 vclcompat::TSpeedButton。
class TfCfgTrayPlateButton : public TSpeedButton
{
public:
    typedef void (TfConfigurationTrayPlate::*Handler)();
    TfCfgTrayPlateButton(TfConfigurationTrayPlate* f, Handler h) : f_(f), h_(h) {}
    void Click() override { if (f_ && h_) (f_->*h_)(); }
private:
    TfConfigurationTrayPlate* f_;
    Handler h_;
};

// golden `SL->CommaText`（BCB6 Classes.pas TStrings.GetDelimitedText，Delimiter=','、QuoteChar='"'）：
//   只有一項而且是空字串 → `""`；其餘逐項：含 #0..' '、'"'、',' 的才用 AnsiQuotedStr 加引號（內部 '"' 變 '""'），**空字串不加引號**；
//   項與項之間 ','，最後一個 ',' 刪掉。
// ⚠ vclcompat::TStringList::GetCommaText（vclcompat/TStringList.cpp:163-186 buildDelimited）把空字串一律寫成 `""` —— 與 BCB6 不同。
//   TrayForm.csv 每一列 16 欄裡常有空欄（Group／Memo／Block*），用 vclcompat 的會寫出 `...,6.35,"","",...`，golden 寫 `...,6.35,,,...`。
//   這裡照 BCB6 寫；vclcompat 那支是共用元件，要不要一起改由 Steven 決定（交件報告「待決定」）。
AnsiString W906_Bcb6CommaText(TStringList* SL)
{
    const int Count = SL->Count;
    if (Count == 1 && AnsiString(SL->Strings[0]) == "")
        return AnsiString("\"\"");
    std::string Result;
    for (int I = 0; I < Count; I++) {
        const std::string S = AnsiString(SL->Strings[I]).c_str();
        bool quote = false;
        for (std::string::size_type k = 0; k < S.size(); ++k) {
            const unsigned char ch = static_cast<unsigned char>(S[k]);
            if (ch <= ' ' || ch == '"' || ch == ',') { quote = true; break; }
        }
        if (quote) {
            Result += '"';
            for (std::string::size_type k = 0; k < S.size(); ++k) {
                if (S[k] == '"') Result += '"';
                Result += S[k];
            }
            Result += '"';
        } else {
            Result += S;
        }
        Result += ',';
    }
    if (!Result.empty()) Result.erase(Result.size() - 1);
    return AnsiString(Result);
}

TfConfigurationTrayPlate* g_cfg = nullptr;
bool g_created = false;

}  // namespace

TfConfigurationTrayPlate::TfConfigurationTrayPlate()
    : strngrdTray(new vclcompat::TStringGrid()),     // VCL 設計期預設 5x5（dfm 沒有 ColCount／RowCount）
      strngrdHP(new vclcompat::TStringGrid()),
      sbUpdateTray(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbUpdateTrayClick)),
      sbtReloadTray(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbtReloadTrayClick)),
      sbUpdateHP(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbUpdateHPClick)),
      sbtReloadHP(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbtReloadHPClick))
{
}

TfConfigurationTrayPlate* W906_CfgTrayPlate()
{
    if (!g_cfg) g_cfg = new TfConfigurationTrayPlate();
    return g_cfg;
}

// ---------------------------------------------------------------------------
// golden cConfiguration.cpp:7012-7035（V912），逐字；`TObject *Sender` 拿掉（本體沒讀）。
// 寫 D:\HT9045\System\TrayForm.csv（整檔覆寫）。
// ---------------------------------------------------------------------------
void TfConfigurationTrayPlate::sbUpdateTrayClick()
{
    TStringList *sList=new TStringList();
    TStringList *SL=new TStringList();

    for(int i=0; i<strngrdTray->RowCount; i++)
    {
        SL->Clear();
        for(int j=0; j<strngrdTray->ColCount; j++)
        {
            SL->Add(strngrdTray->Cells[j][i].Trim());
        }
        sList->Add(W906_Bcb6CommaText(SL));   // golden :7024 sList->Add(SL->CommaText);（BCB6 CommaText，見 W906_Bcb6CommaText）
    }

    sList->SaveToFile(TrayTablePath);
    sList->Clear();
    SL->Clear();
    delete sList;
    delete SL;

    sbtReloadTray->Click();
    sbUpdateTray->Down=false;
}

// ---------------------------------------------------------------------------
// golden cConfiguration.cpp:7037-7081（V912），逐字。讀 D:\HT9045\System\TrayForm.csv。
// （cConfiguration.cpp:350 門面那份是同一段 golden，沒有實例、沒有呼叫者。）
// golden LoadFromFile 找不到檔會丟 EFOpenError；vclcompat 回空表（RowCount=0）。呼叫者都先檢查 bHasTrayCSV（開機時 FileExists）。
// ---------------------------------------------------------------------------
void TfConfigurationTrayPlate::sbtReloadTrayClick()
{
    TStringList *List=new TStringList();
    AnsiString S1, S2;
    int iPos=0;

    strngrdTray->ColCount=16;
    strngrdTray->Font->Size=10;
    strngrdTray->DefaultColWidth=80;
    strngrdTray->ColWidths[0]=200;

    for(int i=0; i<strngrdTray->RowCount; i++)
    {
        for(int j=0; j<16; j++)
            strngrdTray->Cells[j][i]="";
    }

    List->LoadFromFile(TrayTablePath);
    strngrdTray->RowCount=List->Count;

    for(int i=0; i<List->Count; i++)
    {
        int j=0;
        S1=List->Strings[i];
        S1=StringReplace(S1, "\"", "", TReplaceFlags()<<rfReplaceAll);
        do
        {
            iPos=S1.AnsiPos(",");
            if(iPos>0)
            {
                S2=S1.SubString(0, iPos-1);
                S1=S1.SubString(iPos+1, S1.Length());
                strngrdTray->Cells[j][i]=S2;
                j++;
            }
        }while(iPos>0);
    }

    if(strngrdTray->RowCount>1)
        strngrdTray->FixedRows=1;
    strngrdTray->FixedCols=0;
    List->Clear();
    delete List;
    sbtReloadTray->Down=false;
}

// ---------------------------------------------------------------------------
// golden cConfiguration.cpp:7148-7192（V912），逐字。讀 D:\HT9045\System\PlateForm.csv。
// ---------------------------------------------------------------------------
void TfConfigurationTrayPlate::sbtReloadHPClick()
{
    TStringList *List=new TStringList();
    AnsiString S1, S2;
    int iPos=0;

    strngrdHP->ColCount=16;
    strngrdHP->Font->Size=10;
    strngrdHP->DefaultColWidth=80;
    strngrdHP->ColWidths[ 0]=200;

    for(int i=0; i<strngrdHP->RowCount; i++)
    {
        for(int j=0; j<16; j++)
            strngrdHP->Cells[j][i]="";
    }

    List->LoadFromFile(PlateTablePath);
    strngrdHP->RowCount=List->Count;

    for(int i=0; i<List->Count; i++)
    {
        int j=0;
        S1=List->Strings[i];
        S1=StringReplace(S1, "\"", "", TReplaceFlags()<<rfReplaceAll);
        do
        {
            iPos=S1.AnsiPos(",");
            if(iPos>0)
            {
                S2=S1.SubString(0, iPos-1);
                S1=S1.SubString(iPos+1, S1.Length());
                strngrdHP->Cells[j][i]=S2;
                j++;
            }
        }while(iPos>0);
    }

    if(strngrdHP->RowCount>1)
        strngrdHP->FixedRows=1;
    strngrdHP->FixedCols=0;
    List->Clear();
    delete List;
    sbtReloadHP->Down=false;
}

// ---------------------------------------------------------------------------
// golden cConfiguration.cpp:7194-7216（V912），逐字。寫 D:\HT9045\System\PlateForm.csv（整檔覆寫）。
// ---------------------------------------------------------------------------
void TfConfigurationTrayPlate::sbUpdateHPClick()
{
    TStringList *sList=new TStringList();
    TStringList *SL=new TStringList();

    for(int i=0; i<strngrdHP->RowCount; i++)
    {
        SL->Clear();
        for(int j=0; j<strngrdHP->ColCount; j++)
        {
            SL->Add(strngrdHP->Cells[j][i].Trim());
        }
        sList->Add(W906_Bcb6CommaText(SL));   // golden :7206 sList->Add(SL->CommaText);
    }

    sList->SaveToFile(PlateTablePath);
    sList->Clear();
    SL->Clear();
    delete sList;
    delete SL;

    // AI(W906-FRW-S98) 20260926: golden :7215-7216 照留 —— 存的是 HP 表，重讀與清 Down 的卻是 Tray 那兩顆（golden 抄 sbUpdateTrayClick 沒改；
    //   檔頭 (a)）。改成 sbtReloadHP／sbUpdateHP 要使用者決定。
    sbtReloadTray->Click();
    sbUpdateTray->Down=false;
}

// ---------------------------------------------------------------------------
// golden CreateForm(TfConfiguration)（HT9045.cpp:207）的 Tray／Plate 這一塊。冪等。
// ---------------------------------------------------------------------------
void W906_CfgTrayPlate_CreateForm()
{
    if (g_created) return;
    TfConfigurationTrayPlate* f = W906_CfgTrayPlate();
    // C 路 IniConfig（golden TfConfiguration 的具名替身）的 IC_FormShow 在 FormShow :4676-4684 呼叫
    // EL<TSpeedButton>("TfConfiguration", "sbtReloadTray"／"sbtReloadHP")->Click()：先登記成同一顆，那兩行才會讀表。
    // 那兩個名字在 FileRW/IniConfig.gen.inc 只有 IC_FormShow 會建（開頁時；DfmState／存檔替身都沒有），開機時一定還沒建。
    const char* const kNames[2] = {"sbtReloadTray", "sbtReloadHP"};
    TSpeedButton* const kBtns[2] = {f->sbtReloadTray, f->sbtReloadHP};
    for (int i = 0; i < 2; ++i) {
        if (filerw::ELFind("TfConfiguration", kNames[i]))
            std::printf("W906_CfgTrayPlate_CreateForm: TfConfiguration.%s proxy already exists -- IniConfig FormShow's ->Click() "
                        "will NOT reach the Tray/Plate table (call this before the first editlist.get IniConfig)\n", kNames[i]);
        else
            filerw::ELKeep("TfConfiguration", kNames[i], kBtns[i]);
    }
    // golden cConfiguration.cpp:226-234（建構子尾段）。開機時 bHasTrayCSV／bHasPlateCSV 還是 false（檔頭 ①）⇒ 不讀檔。
    if(bHasTrayCSV)                                                             //Steven 20210629 : Tray Form改成CSV
    {
        f->sbtReloadTray->Click();
    }

    if(bHasPlateCSV)                                                            //Steven 20210629 : HP Form改成CSV
    {
        f->sbtReloadHP->Click();
    }
    g_created = true;
    std::printf("CfgTrayPlate: TfConfiguration tsTrayData/tsHPData table ready -- golden cConfiguration.cpp ctor :226-234 "
                "(bHasTrayCSV=%d bHasPlateCSV=%d at CreateForm; golden sets them later in TfMain::FormShow :9987-9988) "
                "Tray rows=%d HP rows=%d\n",
                (int)bHasTrayCSV, (int)bHasPlateCSV, (int)f->strngrdTray->RowCount, (int)f->strngrdHP->RowCount);
}

// ---------------------------------------------------------------------------
// golden TfMain::FormShow main.cpp:9987-9988（V912），逐字。只查檔在不在。
// ---------------------------------------------------------------------------
void W906_CfgTrayPlate_MainFormShowFlags()
{
    bHasTrayCSV=FileExists(TrayTablePath);                                      //Steven 20210629 : Tray Form改成CSV
    bHasPlateCSV=FileExists(PlateTablePath);                                    //Steven 20210629 : Plate Form改成CSV
    std::printf("CfgTrayPlate: golden TfMain::FormShow :9987-9988 bHasTrayCSV=%d (%s) bHasPlateCSV=%d (%s)\n",
                (int)bHasTrayCSV, TrayTablePath.c_str(), (int)bHasPlateCSV, PlateTablePath.c_str());
}
