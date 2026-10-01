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
//    ④ 寫：只有 Configuration 頁 tsTrayData／tsHPData 分頁的 Save 鈕（sbUpdateTray／sbUpdateHP）。AI(W906-S98) 20261001：網頁兩個分頁接上了——
//       WS cfgtrayplate.op op=save（本檔檔尾）才寫；開機與開頁都不寫（kIC_EnableAll 仍排除 tsTrayData 等：兩張表不走 editlist.save）。
//
//  ---- 會碰到的真實檔 ------------------------------------------------------------------------------------------------
//    D:\HT9045\System\TrayForm.csv、PlateForm.csv：③ 只讀（TStringList::LoadFromFile）；④ 整檔覆寫（SaveToFile），只有 cfgtrayplate.op save（AI(W906-S98) 20261001）。
//    路徑是 golden 的全域 TrayTablePath／PlateTablePath（common.cpp:234-235，寫死字面值，--dry 蓋不到）；AI(W906-S98) 20261001：本檔一律經測試縫 W906_TrayTablePath()／W906_PlateTablePath()（沒設＝這兩個全域）。
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
bool g_created = false;   void S98FileCheck(const AnsiString& path, bool forSave);   // AI(W906-S98) 20261001: 本檔檔尾（網頁按鈕時照 VCL 的 EFOpenError／EFCreateError）

}  // namespace

TfConfigurationTrayPlate::TfConfigurationTrayPlate()
    : strngrdTray(new vclcompat::TStringGrid()),     // VCL 設計期預設 5x5（dfm 沒有 ColCount／RowCount）
      strngrdHP(new vclcompat::TStringGrid()),
      sbUpdateTray(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbUpdateTrayClick)),
      sbtReloadTray(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbtReloadTrayClick)),
      sbUpdateHP(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbUpdateHPClick)),
      sbtReloadHP(new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::sbtReloadHPClick))
{   W906CtorTail();   // AI(W906-S98) 20261001: 新成員（Add／Delete／Modify 六顆鈕、edtTemp、iSel*、VCL 目前格）的初值，本檔檔尾
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

    S98FileCheck(W906_TrayTablePath(), true);  sList->SaveToFile(W906_TrayTablePath());   // golden :7027 sList->SaveToFile(TrayTablePath);（AI(W906-S98) 20261001：測試縫；網頁按鈕時照 VCL EFCreateError）
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

    S98FileCheck(W906_TrayTablePath(), false);  List->LoadFromFile(W906_TrayTablePath());   // golden :7054 List->LoadFromFile(TrayTablePath);（AI(W906-S98) 20261001：測試縫；網頁按鈕時照 VCL EFOpenError）
    VclSetRowCount(false, List->Count);                                         // golden :7055 strngrdTray->RowCount=List->Count;（AI(W906-S98) 20261001：VCL SetRowCount 的副作用，本檔檔尾）

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
        VclSetFixedRows(false, 1);                                              // golden :7076 strngrdTray->FixedRows=1;（AI(W906-S98) 20261001：VCL SetFixedRows）
    VclSetFixedCols(false, 0);                                                  // golden :7077 strngrdTray->FixedCols=0;（AI(W906-S98) 20261001：VCL SetFixedCols）
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

    S98FileCheck(W906_PlateTablePath(), false);  List->LoadFromFile(W906_PlateTablePath());   // golden :7165 List->LoadFromFile(PlateTablePath);（AI(W906-S98) 20261001：測試縫；網頁按鈕時照 VCL EFOpenError）
    VclSetRowCount(true, List->Count);                                          // golden :7166 strngrdHP->RowCount=List->Count;（AI(W906-S98) 20261001）

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
        VclSetFixedRows(true, 1);                                               // golden :7187 strngrdHP->FixedRows=1;（AI(W906-S98) 20261001）
    VclSetFixedCols(true, 0);                                                   // golden :7188 strngrdHP->FixedCols=0;（AI(W906-S98) 20261001）
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

    S98FileCheck(W906_PlateTablePath(), true);  sList->SaveToFile(W906_PlateTablePath());   // golden :7209 sList->SaveToFile(PlateTablePath);（AI(W906-S98) 20261001：測試縫；網頁按鈕時照 VCL EFCreateError）
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
    bHasTrayCSV=FileExists(W906_TrayTablePath());                               //Steven 20210629 : Tray Form改成CSV      //AI(W906-S98) 20261001: golden FileExists(TrayTablePath)——測試縫
    bHasPlateCSV=FileExists(W906_PlateTablePath());                             //Steven 20210629 : Plate Form改成CSV     //AI(W906-S98) 20261001: 同上（PlateTablePath）
    std::printf("CfgTrayPlate: golden TfMain::FormShow :9987-9988 bHasTrayCSV=%d (%s) bHasPlateCSV=%d (%s)\n",
                (int)bHasTrayCSV, W906_TrayTablePath().c_str(), (int)bHasPlateCSV, W906_PlateTablePath().c_str());
}

// ===========================================================================
//  AI(W906-S98) 20261001 [W906] St01 —— todo E-003 ①：Configuration 頁 Tray／Hot Plate 分頁接上網頁（WS cfgtrayplate.op）。
// ---------------------------------------------------------------------------
//  依據：RULINGS_20260926 S98（這兩張表的讀寫）＋ S169（Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有
//    移植的, 我們直接實作」）——蓋過 Q41 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 五、那一條
//    「Configuration 的 Tray／HP 分頁：S98，屬 Q41 的『新頁面先不做』」（S158）。golden 一律 V912
//    D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp（cp950，行號照原檔；dfm 同目錄 cConfiguration.dfm）。
//  golden 兩個分頁（dfm:21346 tsTrayData／:22005 tsHPData）：上面 pnlTray／pnlHP 一排五顆鈕（Modify Data、Add、Delete、Load Data、Save），
//    下面 strngrdTray／strngrdHP（dfm 沒設 Options ⇒ 沒有 goEditing：格子不能直接打字，只能點格＋Modify Data／連點兩下開小鍵盤）。
//    處理器 :6945-7217，**沒有任何 MessageBox／確認框**（Save、Delete 都是按下就做）；唯一的「對話框」是 Modify Data 的小鍵盤
//    （fQwertyKey->ShowQwertyKey，modal）⇒ 這裡拆成兩步（step 0 開鍵盤、step 1 OK／Cancel），同 act.main.closeProgram 的分步寫法。
//  權限（golden FormShow :5381-5382，C 路 FileRW/IniConfig.gen.inc 同一段）：pnlTray＝AccessLevel>=LevelSet.AccessLevel[30] 且 >=[78]、
//    pnlHP＝[30] 且 [79]，不夠就整排鈕停用。格子不在 pnl 裡（dfm:21366 是 tsTrayData 的子元件）⇒ golden 權限不夠還是點得到格、
//    連點兩下還是開得到小鍵盤（strngrdTrayDblClick :6954 btnModifyTray->Click()：VCL TControl.Click 不看 Enabled）、改得了畫面上的格子，
//    只是存不了檔——照翻（看起來像漏洞，但存不了檔；別的讀者 cTrayForm.cpp:164 等都先 Load Data 重讀檔，畫面上改的值不會外流）。
//    切到這一頁時 golden PageControl1Change :6113 再依 Security_new.def [Config] 鎖整頁（ChangeCompomentEnabled，只鎖不開）——
//    頁面切頁籤會送 form.event PageControl1（web/page/ht9045_config_st01_ev.js），伺服器跑同一支 ⇒ 替身的 Enabled 就是 golden 的狀態。
//
//  WS cfgtrayplate.op，value＝JSON 字串：
//    {"op":"get"}                     兩張表的快照（唯讀、不跑 golden、運轉中也給）。頁面 web/page/ht9045_config_trayplate.js 在引擎
//                                     editlist.get IniConfig（＝golden FormShow，:4676-4684 已經重讀兩張表）回來之後叫，所以跟著開窗邊緣走。
//    {"op":"select","table":"tray"|"hp","col":c,"row":r}     點格＝VCL MouseDown → MoveCurrent → OnSelectCell（:6945／:7083）
//    {"op":"modify","table":…,"step":0,"via":"button"|"dblclick"}   Modify Data 鈕（:6957／:7095）或連點兩下（:6952／:7090）開小鍵盤：
//                                     回 keypad{open,flags,dp,checkRange,min,max,current,header}；golden 條件不成立（沒選格）＝open:false
//    {"op":"modify","table":…,"step":1,"text":"…"}｜{…,"step":1,"cancel":true}   小鍵盤 OK／Cancel：同一支處理器再跑一次，小鍵盤那一行
//                                     用操作員打的字（Cancel＝原字），照 golden 尾段（myQwertyKeyBoard.cpp:285-301）atof→CheckRange→AnsiString
//                                     寫回框，處理器再寫進格子（:6979）——所以 Cancel 在數字欄也會把字整理成 golden 的樣子（例 "6.350"→"6.35"）
//    {"op":"add"|"delete"|"reload"|"save","table":…}      btnAdd*／btnDelete*／sbtReload*／sbUpdate*（Tray :6984／:6996／:7037／:7012；
//                                     HP :7120／:7132／:7148／:7194）。HP 的 Save 存完重讀的是 Tray 表（R33，本檔 sbUpdateHPClick）。
//  回覆（成功，併進 ack）：{op,table,golden,ran,running,pageOpen,tray:{…},hp:{…},keypad?,error?,wrote?,todo[]}；每張表
//    {rowCount,colCount,fixedRows,fixedCols,defaultColWidth,colWidths[],cursor{col,row},sel{col,row},cells[[…]],
//     operable{grid,buttons,reload},onTab,file,fileExists}。cursor＝VCL 目前格（畫面反白那一格）、sel＝golden iSel*Row／Col
//    （Modify／Delete 用的那一格；兩者可以不一樣，見「VCL 格子」）。error＝golden 會跳 VCL 例外框（找不到檔、寫不了檔），處理器停在那一行。
//  守衛（get 以外；不信任前端，每一次都重查，前端只是第二道）：
//    running       SystemStart||SoftStart（RULINGS_20260927 第 7 條、route-c §3.0d／§3.0g：運轉中不從網頁改設定；golden 開 Configuration 的
//                  主畫面鈕 sbConfigClick main.cpp:29012 也是 if(SystemStart) return;，表單是 modal）
//    reload page   Configuration 沒開（editlist.get IniConfig＝golden FormShow 沒跑、或關窗邊緣清掉）或開頁之後等級變了——看 form.event
//                  別名頁 Config.Configuration 的開頁紀錄（FileRW/IniConfig.cpp kEvPage；filerw::PageShownLevel），同 RunPageEvent 第 1 步
//    not-on-tab    伺服器的 PageControl1 不在這一頁（golden 使用者只點得到目前分頁上的元件；切頁籤的 form.event 也是 golden 上鎖的時機）
//    not-operable  按鈕：pnlTray／pnlHP（Load Data 另看 sbtReload* 替身）連同祖先的 Enabled／Visible／TabVisible（filerw::ELOperable）；
//                  點格／連點兩下：格子的容器 tsTrayData／tsHPData 連同祖先
//    bad-payload／bad-cell／bad-value（小鍵盤打不出來的字）／stale（開鍵盤之後格子變了）／keypad（沒開鍵盤就送 step 1）
//  鎖：整段持 FormJson 鎖（IniConfig FormShow、TrayForm／Cleaning／HotPlate 開頁也會重讀這兩張表）。
//  檔：只有 save 寫（整檔覆寫 System\TrayForm.csv 或 PlateForm.csv；經 W906_TrayTablePath／W906_PlateTablePath 測試縫）。
//    save 不重設 bHasTrayCSV／bHasPlateCSV（golden 只在 TfMain::FormShow main.cpp:9987-9988 設）。
// ===========================================================================
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "Public/cJSON.h"            // value 解析
#include "WebBridge/JsonWriter.h"    // 回覆（cp950 格子內容照 JsonWriter 的規則轉成 UTF-8）
#include "FileRW/_EditPage.h"        // filerw::PageShownLevel（別名頁 Config.Configuration 的開頁紀錄）

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp（只在 wb_serve；CRITICAL_SECTION，可重入）

// ---- 測試縫（宣告與說明在 CfgTrayPlate.h）---------------------------------------------------------------------------------------------
AnsiString W906_TrayTablePath()
{
    const char* e = std::getenv("W906_TRAYFORMCSV_PATH");
    return (e != 0 && *e != 0) ? AnsiString(e) : TrayTablePath;
}

AnsiString W906_PlateTablePath()
{
    const char* e = std::getenv("W906_PLATEFORMCSV_PATH");
    return (e != 0 && *e != 0) ? AnsiString(e) : PlateTablePath;
}

// ---- 建構子尾（新成員的初值）-------------------------------------------------------------------------------------------------------------
void TfConfigurationTrayPlate::W906CtorTail()
{
    btnAddTray    = new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::btnAddTrayClick);
    btnDeleteTray = new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::btnDeleteTrayClick);
    btnModifyTray = new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::btnModifyTrayClick);
    btnAddHP      = new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::btnAddHPClick);
    btnDeleteHP   = new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::btnDeleteHPClick);
    btnModifyHP   = new TfCfgTrayPlateButton(this, &TfConfigurationTrayPlate::btnModifyHPClick);
    btnAddTray->GroupIndex = 1;  btnModifyTray->GroupIndex = 1;  sbtReloadTray->GroupIndex = 1;   // dfm:21392／:21622／:21878（視覺，沒有讀者）
    btnAddHP->GroupIndex = 1;    btnModifyHP->GroupIndex = 1;    sbtReloadHP->GroupIndex = 1;     // dfm:22040／:22270／:22526 同
    edtTemp = new TEdit();
    edtTemp->Text = "edtTemp";                                   // dfm:22001
    iSelTrayRow=0;                                               // golden 建構子 :138-141
    iSelTrayCol=0;
    iSelHPRow=0;
    iSelHPCol=0;
    Tag = 0;                                                     // TComponent 預設（dfm 沒寫 Tag）
    // VCL TStringGrid 設計期預設 FixedRows=1、FixedCols=1（dfm:21366／:22637 沒寫 ⇒ 預設；vclcompat 的預設是 0）；建立時 Initialize：
    // 目前格＝(FixedCols, FixedRows)。第一次 Load Data（:7076-7077）之後變成 FixedRows=1（兩列以上）、FixedCols=0。
    strngrdTray->FixedRows = 1;  strngrdTray->FixedCols = 1;  strngrdTray->Row = 1;
    strngrdHP->FixedRows   = 1;  strngrdHP->FixedCols   = 1;  strngrdHP->Row   = 1;
    curTray.col = 1;  curTray.row = 1;
    curHP.col   = 1;  curHP.row   = 1;
}

// ---- VCL 格子 --------------------------------------------------------------------------------------------------------------------------
//  Delphi 6 Grids.pas（BCB6 的 VCL）：
//    TCustomGrid.MoveCurrent(ACol,ARow,…)：if SelectCell(ACol,ARow) then FCurrent:=(ACol,ARow)…；TCustomDrawGrid.SelectCell 呼叫 OnSelectCell。
//    SetRow(Value)：if Row<>Value then FocusCell(Col,Value,True)（→ MoveCurrent）。
//    SetRowCount(Value)：if FRowCount<>Value：Value<1 夾 1；Value<=FixedRows 先 FixedRows:=Value-1；ChangeSize → DoChange：目前格的列
//      >= RowCount ⇒ MoveCurrent(Col, RowCount-1)。
//    SetFixedRows／SetFixedCols：值有變才設、Initialize（FCurrent:=(FixedCols,FixedRows)，直接設、不經 SelectCell ⇒ 沒有 OnSelectCell）。
//    MouseDown：點在固定列／欄上不搬目前格（op select 回 bad-cell）。
//  ⚠ 依據是 Delphi 6/7 Grids.pas 的寫法（記憶，沒有在 BCB6 實機量過；同 R35）。鍵盤方向鍵搬格網頁沒做（只有滑鼠點格）。
//  golden 看得到的後果：Add 之後新的那一列被選（:6993 Row=RowCount-1 → OnSelectCell）；刪到最後一列時選取往上一列；Load Data 讓列數
//    變少時也會觸發；FixedRows／FixedCols 變了（第一次 Load Data：FixedCols 1→0）只搬反白、iSel* 不變。
//  ⚠ golden 怪處照留（看起來像錯，R 題候選，不修）：Load Data 之後 iSel* 可以停在比 RowCount 大的列（檔案變短但反白格沒被擠掉、或
//    Initialize 只搬反白）——這時按 Delete，:7001 的迴圈一次都不跑、:7008 RowCount-1 ⇒ 刪掉的是**最後一列**，不是畫面反白那一列；
//    按 Modify 寫進畫面外的格子（VCL TStringGrid 的稀疏儲存，看不到、存檔不寫）。改成擋要 Steven 決定。
void TfConfigurationTrayPlate::VclMoveCurrent(bool hp, int ACol, int ARow)
{
    bool CanSelect=true;
    if (hp) strngrdHPSelectCell(ACol, ARow, CanSelect);
    else    strngrdTraySelectCell(ACol, ARow, CanSelect);
    if (!CanSelect) return;
    VclCursor& c = hp ? curHP : curTray;
    c.col = ACol;
    c.row = ARow;
    (hp ? strngrdHP : strngrdTray)->Row = ARow;
}

void TfConfigurationTrayPlate::VclSetFixedRows(bool hp, int v)
{
    vclcompat::TStringGrid* g = hp ? strngrdHP : strngrdTray;
    if (g->FixedRows == v) return;
    g->FixedRows = v;
    VclCursor& c = hp ? curHP : curTray;                         // Initialize
    c.col = g->FixedCols;
    c.row = g->FixedRows;
    g->Row = c.row;
}

void TfConfigurationTrayPlate::VclSetFixedCols(bool hp, int v)
{
    vclcompat::TStringGrid* g = hp ? strngrdHP : strngrdTray;
    if (g->FixedCols == v) return;
    g->FixedCols = v;
    VclCursor& c = hp ? curHP : curTray;                         // Initialize
    c.col = g->FixedCols;
    c.row = g->FixedRows;
    g->Row = c.row;
}

void TfConfigurationTrayPlate::VclSetRowCount(bool hp, int v)
{
    vclcompat::TStringGrid* g = hp ? strngrdHP : strngrdTray;
    if ((int)g->RowCount == v) return;
    if (v < 1) v = 1;
    if (v <= g->FixedRows) VclSetFixedRows(hp, v - 1);
    g->RowCount = v;
    VclCursor& c = hp ? curHP : curTray;                         // ChangeSize → DoChange
    if (c.row >= (int)g->RowCount) VclMoveCurrent(hp, c.col, (int)g->RowCount - 1);
}

void TfConfigurationTrayPlate::VclSetRow(bool hp, int v)
{
    VclCursor& c = hp ? curHP : curTray;
    if (c.row != v) VclMoveCurrent(hp, c.col, v);
}

// ---- 小鍵盤 --------------------------------------------------------------------------------------------------------------------------
namespace {
enum S98KpMode { kS98KpNone = 0, kS98KpProbe, kS98KpAnswer };
struct S98Keypad {
    S98KpMode   mode = kS98KpNone;
    bool        cancel = false;
    std::string text;                 // 操作員打的字（UTF-8；只收 golden 打得出來的字，見 S98Typable）
    bool        opened = false;       // 處理器真的開了鍵盤（golden 條件成立）
    int         iFunction = 0, iDP = 0;
    bool        bCheckRange = false;
    double      min = 0, max = 0;
    AnsiString  current;              // 開鍵盤那一刻框裡的字（＝格子原字）
};
S98Keypad g_kp;

// step 0 開了鍵盤、step 1 還沒來（golden：modal 鍵盤開著，別的都按不到）
struct S98Pending {
    bool       on = false;
    bool       hp = false;
    bool       dbl = false;           // 由連點兩下開的
    int        row = -1, col = -1;    // 開鍵盤時的 iSel*
    AnsiString current;
};
S98Pending g_pend;

// golden 小鍵盤打得出來的字（V912 myQwertyKeyBoard.cpp／.dfm）——網頁送的字不信任，照這張表驗，打不出來的整個拒（bad-value，格子不動）：
//   N_INTEGER：數字鍵＋'-'（KeyPress :497-510 → OnlyNumberInPut common.cpp:1338-1343；'-' 鍵與 spbMinusClick :421-431 都是切換開頭的負號；
//     ±10/100/1000 鍵 spbAdd1Click :444-445 寫 AnsiString(int)）⇒ 數字＋最多一個開頭的 '-'；空字串也打得出來（spbClearClick :352-355）。
//   N_DOUBLE：再加 '.'，最多一個（spbDPClick :451-458、KeyPress :483）；±鍵寫 "%1.6f"（:447），同一個樣子。
//   N_NO_SYMBOL（文字欄）：螢幕鍵盤前 47 鍵裡 eKeySymbol 七顆停用（spbChangeCaseClick :316：\ | ; : ' " , < . > / ? ` ~），eKeyNumAndSymbol
//     八顆固定顯示數字（:311-313），其餘（字母、9( 0) -_ =+ [{ ]}，建構子 :71-117）大小寫都打得出來；空白鍵（:118，不在 47 鍵裡）照顯示
//     （:217 !N_NO_SPACE）；數字鍵盤（:119-128）只有數字。實體鍵盤 KeyPress :520-529 只收 0-9 . a-z A-Z - _（還有退格）。兩條路的聯集
//     ＝英數字、空白、- _ = + [ ] { } ( ) .。CUSTOMER_CODE==CC_JCET 時 KeyPress 不過濾（:518「長電舊廠密碼有特殊字元」）⇒ 實體鍵盤什麼
//     都打得出來：這裡收可見的 ASCII（0x20-0x7E）；非 ASCII（輸入法中文）不收——兩張表是 cp950 檔、網頁送 UTF-8，轉碼不做（偏離，記在交件）。
//     實體鍵盤貼上（Ctrl+V 不經 KeyPress）golden 什麼都貼得進去：網頁沒有這條路，不收。
//   長度：golden 沒有上限（myQwertyKeyBoard.dfm:84 edQwertyContent 沒設 MaxLength）；255 bytes 是移植樹的防呆（一格 CSV 欄位）。
void S98Typable(int iFunction, const std::string& s)
{
    if (s.size() > 255)
        throw std::invalid_argument("bad-value: more than 255 bytes (port guard for one CSV field; golden has no limit)");
    if ((iFunction & N_INTEGER) || (iFunction & N_DOUBLE)) {
        const bool dbl = (iFunction & N_DOUBLE) != 0;
        int dots = 0;
        for (std::string::size_type k = 0; k < s.size(); ++k) {
            const char ch = s[k];
            if (ch >= '0' && ch <= '9') continue;
            if (ch == '-' && k == 0) continue;
            if (dbl && ch == '.' && ++dots == 1) continue;
            throw std::invalid_argument(dbl ? "bad-value: the golden N_DOUBLE keypad only types digits, one '.' and one leading '-'"
                                            : "bad-value: the golden N_INTEGER keypad only types digits and one leading '-'");
        }
        return;
    }
    const bool jcet = CUSTOMER_CODE == CC_JCET;
    for (std::string::size_type k = 0; k < s.size(); ++k) {
        const unsigned char ch = static_cast<unsigned char>(s[k]);
        if (jcet) {
            if (ch >= 0x20 && ch <= 0x7E) continue;
            throw std::invalid_argument("bad-value: only printable ASCII (CC_JCET: golden's physical keyboard is not filtered, :518; "
                                        "non-ASCII is not transcoded to the cp950 CSV)");
        }
        if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
            continue;
        if (ch != 0 && std::strchr(" -_=+[]{}().", ch) != 0)
            continue;
        throw std::invalid_argument("bad-value: the golden N_NO_SYMBOL keypad (on-screen keys + KeyPress filter, myQwertyKeyBoard.cpp:311-316 / "
                                    ":520-529) only types letters, digits, space and - _ = + [ ] { } ( ) .");
    }
}

void S98KpReset(S98KpMode mode)
{
    g_kp = S98Keypad();
    g_kp.mode = mode;
}
}  // namespace

// golden fQwertyKey->ShowQwertyKey(Ptr, …)（V912 myQwertyKeyBoard.cpp:169-302）對 Ptr（TEdit）的淨效果：
//   :238-240 sBackup／edQwertyContent 都是框裡的字 → :283 ShowModal（操作員打字，Summit／Enter 關窗、Cancel :364 還原成 sBackup）→
//   :285-292 數字模式 atof → bCheckRange 時 CheckRange(d, min, max)（MachineType.h:1653；參數名對調但兩種順序結果一樣，
//   forms/fQwertyKey.h (G-a)）→ AnsiString(double)（BCB FloatToStr 樣式，vclcompat/AnsiString.h:28）→ :298-299 寫回框。
//   鍵盤畫面（大小寫、±鍵、目前值／上下限）是網頁的 qwerty.js；fQwertyKey2 的雙鍵盤（:171-178）只有「鍵盤開著又來一個」才用，網頁一次只開一個。
//   kS98KpProbe（step 0）：只記規格、框不動（操作員還沒按 OK／Cancel；處理器後面 :6979 把原字寫回原格＝沒變）。
//   kS98KpNone（沒有網頁操作員，例：將來別的呼叫者）：同 forms/fQwertyKey.h BEHAVIOUR NOTE「打開鍵盤、原字送出」。
void TfConfigurationTrayPlate::W906ShowQwertyKey(TEdit *Ptr, int iFunction, int iDP, bool bCheckRange, double min, double max)
{
    g_kp.opened      = true;
    g_kp.iFunction   = iFunction;
    g_kp.iDP         = iDP;
    g_kp.bCheckRange = bCheckRange;
    g_kp.min         = min;
    g_kp.max         = max;
    g_kp.current     = Ptr->Text;
    if (g_kp.mode == kS98KpProbe) return;

    AnsiString sBackup=Ptr->Text;                               // golden :238
    AnsiString content=sBackup;                                 // golden :239 edQwertyContent->Text=EditPtr->Text;
    if (g_kp.mode == kS98KpAnswer && !g_kp.cancel)
    {
        S98Typable(iFunction, g_kp.text);                       // 打不出來的字 ⇒ 丟 invalid_argument（格子不動）
        content=AnsiString(g_kp.text.c_str());                  // 操作員打的字（spbSummitClick :347／Enter :463 關窗）
    }                                                           // Cancel：spbCancelClick :364 edQwertyContent->Text=sBackup;
    if(iFunction&N_INTEGER || iFunction&N_DOUBLE)               // golden :285-292
    {
        double d=atof(content.c_str());
        if(bCheckRange)
        {
            content=AnsiString(CheckRange(d, min, max));
        }
    }
    Ptr->Text=content;                                          // golden :298-299 EditPtr->Text=edQwertyContent->Text;
}

// ---- golden 處理器（V912 cConfiguration.cpp，逐字；Sender 拿掉；fQwertyKey->ShowQwertyKey 換成上面的 W906ShowQwertyKey；
//      VCL 屬性副作用照上面「VCL 格子」）--------------------------------------------------------------------------------------------------
// golden :6945-6950
void TfConfigurationTrayPlate::strngrdTraySelectCell(int ACol, int ARow, bool &CanSelect)
{
    (void)CanSelect;
    iSelTrayRow=ARow;
    iSelTrayCol=ACol;
}

// golden :6952-6955
void TfConfigurationTrayPlate::strngrdTrayDblClick()
{
    btnModifyTray->Click();
}

// golden :6957-6982
void TfConfigurationTrayPlate::btnModifyTrayClick()
{
    if(iSelTrayRow>0 && iSelTrayCol>=0)
    {
        edtTemp->Text=strngrdTray->Cells[iSelTrayCol][iSelTrayRow];
        if(strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Package Type"))!=0 ||
           strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Group"))!=0     ||
           strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Memo"))!=0)
        {
            W906ShowQwertyKey(edtTemp, N_NO_SYMBOL);                           // golden :6966 fQwertyKey->ShowQwertyKey(edtTemp, N_NO_SYMBOL);
        }
        else if(strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Columns (X)"))!=0 ||
                strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("Rows (Y)"))!=0     ||
                strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("BlockNumberX"))!=0 ||
                strngrdTray->Cells[iSelTrayCol][0].AnsiPos(AnsiString("BlockNumberY"))!=0)
        {
            W906ShowQwertyKey(edtTemp, N_INTEGER, 0, true, 0, 1000);            // golden :6973
        }
        else
        {
            W906ShowQwertyKey(edtTemp, N_DOUBLE, 2, true, 0.00, 1000.00);       // golden :6977
        }
        strngrdTray->Cells[iSelTrayCol][iSelTrayRow]=edtTemp->Text;
    }
    btnModifyTray->Down=false;
}

// golden :6984-6994
void TfConfigurationTrayPlate::btnAddTrayClick()
{
    int iRow=strngrdTray->RowCount;
    VclSetRowCount(false, iRow+1);                                              // golden :6987 strngrdTray->RowCount=iRow+1;
    for(int j=0; j<strngrdTray->ColCount; j++)
    {
        strngrdTray->Cells[j][iRow]="";
    }
    btnAddTray->Down=false;
    VclSetRow(false, strngrdTray->RowCount-1);                                  // golden :6993 strngrdTray->Row=strngrdTray->RowCount-1;（→ OnSelectCell）
}

// golden :6996-7010
void TfConfigurationTrayPlate::btnDeleteTrayClick()
{
    if(iSelTrayRow<=0)
        return;
    Tag=atoi(strngrdTray->Cells[0][iSelTrayRow].c_str());
    for(int i=iSelTrayRow; i<strngrdTray->RowCount-1; i++)
    {
        for(int j=0; j<strngrdTray->ColCount; j++)
        {
            strngrdTray->Cells[j][i]=strngrdTray->Cells[j][i+1];
        }
    }
    VclSetRowCount(false, strngrdTray->RowCount-1);                             // golden :7008 strngrdTray->RowCount=strngrdTray->RowCount-1;
    btnDeleteTray->Down=false;
}

// golden :7083-7088
void TfConfigurationTrayPlate::strngrdHPSelectCell(int ACol, int ARow, bool &CanSelect)
{
    (void)CanSelect;
    iSelHPRow=ARow;
    iSelHPCol=ACol;
}

// golden :7090-7093
void TfConfigurationTrayPlate::strngrdHPDblClick()
{
    btnModifyHP->Click();
}

// golden :7095-7118（HP 表只有 Columns (X)／Rows (Y) 是整數，沒有 BlockNumberX／Y）
void TfConfigurationTrayPlate::btnModifyHPClick()
{
    if(iSelHPRow>0 && iSelHPCol>=0)
    {
        edtTemp->Text=strngrdHP->Cells[iSelHPCol][iSelHPRow];
        if(strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Package Type"))!=0 ||
           strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Group"))!=0     ||
           strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Memo"))!=0)
        {
            W906ShowQwertyKey(edtTemp, N_NO_SYMBOL);                           // golden :7104
        }
        else if(strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Columns (X)"))!=0 ||
                strngrdHP->Cells[iSelHPCol][0].AnsiPos(AnsiString("Rows (Y)"))!=0)
        {
            W906ShowQwertyKey(edtTemp, N_INTEGER, 0, true, 0, 1000);            // golden :7109
        }
        else
        {
            W906ShowQwertyKey(edtTemp, N_DOUBLE, 2, true, 0.00, 1000.00);       // golden :7113
        }
        strngrdHP->Cells[iSelHPCol][iSelHPRow]=edtTemp->Text;
    }
    btnModifyHP->Down=false;
}

// golden :7120-7130
void TfConfigurationTrayPlate::btnAddHPClick()
{
    int iRow=strngrdHP->RowCount;
    VclSetRowCount(true, iRow+1);                                               // golden :7123 strngrdHP->RowCount=iRow+1;
    for(int j=0; j<strngrdHP->ColCount; j++)
    {
        strngrdHP->Cells[j][iRow]="";
    }
    btnAddHP->Down=false;
    VclSetRow(true, strngrdHP->RowCount-1);                                     // golden :7129 strngrdHP->Row=strngrdHP->RowCount-1;
}

// golden :7132-7146
void TfConfigurationTrayPlate::btnDeleteHPClick()
{
    if(iSelHPRow<=0)
        return;
    Tag=atoi(strngrdHP->Cells[0][iSelHPRow].c_str());
    for(int i=iSelHPRow; i<strngrdHP->RowCount-1; i++)
    {
        for(int j=0; j<strngrdHP->ColCount; j++)
        {
            strngrdHP->Cells[j][i]=strngrdHP->Cells[j][i+1];
        }
    }
    VclSetRowCount(true, strngrdHP->RowCount-1);                                // golden :7144 strngrdHP->RowCount=strngrdHP->RowCount-1;
    btnDeleteHP->Down=false;
}

// ---- WS cfgtrayplate.op ------------------------------------------------------------------------------------------------------------------
namespace {
const char kS98Form[]  = "TfConfiguration";
const char kS98EvTag[] = "Config.Configuration";   // IniConfig 的 form.event 別名頁（FileRW/IniConfig.cpp kEvTag）：開頁 FormShow 之後 PageJson(kEvPage) 記下等級

struct S98Tab { const char* key; const char* tab; const char* panel; const char* reload; };
const S98Tab kS98Tabs[2] = {
    {"tray", "tsTrayData", "pnlTray", "sbtReloadTray"},   // golden dfm:21346／:21377／:21872
    {"hp",   "tsHPData",   "pnlHP",   "sbtReloadHP"},     // golden dfm:22005／:22025／:22520
};

std::string S98N(long v) { char b[32]; std::snprintf(b, sizeof(b), "%ld", v); return b; }   // MinGW 6.3：不用 std::to_string

struct S98Lock {
    S98Lock()  { ht9045::formjson::FormLock(); }
    ~S98Lock() { ht9045::formjson::FormUnlock(); }
};

bool g_s98Vcl = false;   // 網頁按鈕（op）正在跑 golden 處理器：S98FileCheck 照 VCL 丟例外（其他呼叫者照舊，見 S98FileCheck）
struct S98VclOn {
    S98VclOn()  { g_s98Vcl = true; }
    ~S98VclOn() { g_s98Vcl = false; }
};

struct S98VclError : std::runtime_error {
    std::string cls;
    S98VclError(const std::string& c, const std::string& t) : std::runtime_error(t), cls(c) {}
};
}  // namespace

// golden TStrings::LoadFromFile／SaveToFile 失敗是 VCL 例外（EFOpenError／EFCreateError，TFileStream.Create），處理器停在那一行、VCL 跳例外框；
// vclcompat 的 LoadFromFile 找不到檔回空表、SaveToFile 開不了檔直接 return（不丟）⇒ 網頁按鈕照 golden：只在 op 裡（g_s98Vcl）先查、丟例外。
// 例：Load Data 找不到檔 ⇒ :7043-7052 已經把格子清空、:7054 丟例外 ⇒ 畫面是空格子＋錯誤（golden 同）；HP 的 Save 寫完檔，接著重讀 Tray
// 表（R33）找不到 TrayForm.csv ⇒ 同樣停在那裡。開頁（IniConfig／TrayForm／Cleaning／HotPlate FormShow）的讀取照舊（呼叫端先看
// bHasTrayCSV），不在這次範圍。訊息字照 Delphi 6 RTLConsts（SFOpenError 'Cannot open file %s'、SFCreateError 'Cannot create file %s'；記憶，沒實機量）。
namespace {
void S98FileCheck(const AnsiString& path, bool forSave)
{
    if (!g_s98Vcl) return;
    if (forSave) {
        std::ofstream probe(path.c_str(), std::ios::binary | std::ios::app);   // golden SaveToFile 本來就會建檔；開不了＝golden 的 EFCreateError
        if (!probe) throw S98VclError("EFCreateError", std::string("Cannot create file ") + path.c_str());
    } else if (!FileExists(path)) {
        throw S98VclError("EFOpenError", std::string("Cannot open file ") + path.c_str());
    }
}

vclcompat::TStringGrid* S98Grid(TfConfigurationTrayPlate* f, int t) { return t ? f->strngrdHP : f->strngrdTray; }

bool S98Running() { return SystemStart || SoftStart; }

bool S98PageOpen()
{
    const int lv = filerw::PageShownLevel(kS98EvTag);
    return lv >= 0 && lv == AccessLevel;
}

bool S98OnTab(int t)
{
    TPageControl* pc = dynamic_cast<TPageControl*>(filerw::ELFind(kS98Form, "PageControl1"));
    const int k = filerw::ELPageIndexOf(kS98Form, "PageControl1", kS98Tabs[t].tab);   // FileRW/IniConfig.cpp:976 ELSetPageOrder（Tray＝3、HP＝4）
    return pc != nullptr && k >= 0 && pc->ActivePageIndex == k;
}

bool S98Operable(const char* name)   // 替身不在＝IniConfig 沒開機建好 ⇒ 不放行（ELOperable 對不存在的名字會回 true）
{
    return filerw::ELFind(kS98Form, name) != nullptr && filerw::ELOperable(kS98Form, name);
}

void S98GridJson(webbridge::JsonWriter& w, TfConfigurationTrayPlate* f, int t)
{
    vclcompat::TStringGrid* g = S98Grid(f, t);
    const TfConfigurationTrayPlate::VclCursor& c = t ? f->curHP : f->curTray;
    const int rows = g->RowCount, cols = g->ColCount;
    const AnsiString path = t ? W906_PlateTablePath() : W906_TrayTablePath();
    w.BeginObject();
    w.Key("rowCount").Number((wb_int64)rows);
    w.Key("colCount").Number((wb_int64)cols);
    w.Key("fixedRows").Number((wb_int64)g->FixedRows);
    w.Key("fixedCols").Number((wb_int64)g->FixedCols);
    w.Key("defaultColWidth").Number((wb_int64)g->DefaultColWidth);
    w.Key("colWidths").BeginArray();
    for (int j = 0; j < cols; ++j) w.Number((wb_int64)g->ColWidths[j]);
    w.EndArray();
    w.Key("cursor").BeginObject().Key("col").Number((wb_int64)c.col).Key("row").Number((wb_int64)c.row).EndObject();
    w.Key("sel").BeginObject()
        .Key("col").Number((wb_int64)(t ? f->iSelHPCol : f->iSelTrayCol))
        .Key("row").Number((wb_int64)(t ? f->iSelHPRow : f->iSelTrayRow)).EndObject();
    w.Key("cells").BeginArray();
    for (int i = 0; i < rows; ++i) {
        w.BeginArray();
        for (int j = 0; j < cols; ++j) w.String(std::string(g->Cells[j][i].c_str()));
        w.EndArray();
    }
    w.EndArray();
    w.Key("operable").BeginObject()
        .Key("grid").Bool(S98Operable(kS98Tabs[t].tab))
        .Key("buttons").Bool(S98Operable(kS98Tabs[t].panel))
        .Key("reload").Bool(S98Operable(kS98Tabs[t].reload)).EndObject();
    w.Key("onTab").Bool(S98OnTab(t));
    w.Key("file").String(std::string(path.c_str()));
    w.Key("fileExists").Bool(FileExists(path));
    w.EndObject();
}

std::string S98Reply(TfConfigurationTrayPlate* f, const std::string& op, int t, const char* golden, bool ran,
                     const std::string& keypadJson, const std::string& errCls, const std::string& errText,
                     const std::string& wrote, const std::vector<std::string>& todo, bool* ok)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    if (t >= 0) w.Key("table").String(kS98Tabs[t].key);
    w.Key("golden").String(golden ? golden : "");
    w.Key("ran").Bool(ran);
    w.Key("running").Bool(S98Running());
    w.Key("pageOpen").Bool(S98PageOpen());
    w.Key("tray"); S98GridJson(w, f, 0);
    w.Key("hp");   S98GridJson(w, f, 1);
    if (!keypadJson.empty()) w.Key("keypad").RawValue(keypadJson);
    if (!errCls.empty()) {
        w.Key("error").BeginObject().Key("class").String(errCls).Key("text").String(errText).EndObject();
        w.Key("messages").BeginArray().BeginObject()
            .Key("en").String("golden raises " + errCls + ": " + errText + " (VCL exception box; the handler stopped there)")
            .Key("zh").String("golden 會跳 VCL 例外框（" + errCls + "）：" + errText + "；處理器停在那一行").EndObject().EndArray();
    }
    if (!wrote.empty()) w.Key("wrote").String(wrote);
    w.Key("todo").BeginArray();
    for (std::size_t i = 0; i < todo.size(); ++i) w.String(todo[i]);
    w.EndArray();
    w.EndObject();
    // WebBridgeServer.cpp:258 單則訊息 64 KiB 上限（超過是斷線、不是錯誤）⇒ 留 4 KiB 給 ack 外框
    if (w.Str().size() > 60000u) {
        if (ok) *ok = false;
        return "too-large: the two tables serialise to " + S98N((long)w.Str().size()) +
               " bytes, over the 64 KiB WebSocket message limit (WebBridgeServer.cpp) -- nothing is lost, but the page cannot show them";
    }
    if (ok) *ok = true;
    return w.Str();
}

std::string S98KeypadJson(bool open, const std::string& header)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("open").Bool(open);
    if (open) {
        w.Key("flags").Number((wb_int64)g_kp.iFunction);
        w.Key("kind").String((g_kp.iFunction & N_INTEGER) ? "int" : (g_kp.iFunction & N_DOUBLE) ? "double" : "text");
        w.Key("dp").Number((wb_int64)g_kp.iDP);
        w.Key("checkRange").Bool(g_kp.bCheckRange);
        w.Key("min").Number(g_kp.min);
        w.Key("max").Number(g_kp.max);
        w.Key("current").String(std::string(g_kp.current.c_str()));
        w.Key("header").String(header);
        w.Key("jcet").Bool(CUSTOMER_CODE == CC_JCET);
    }
    w.EndObject();
    return w.Str();
}
}  // namespace

std::string W906_CfgTrayPlateOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    std::string op, table, via, text;
    int col = -1, row = -1, step = -1;
    bool cancel = false, hasText = false;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0 || !cJSON_IsObject(root)) {
            if (root) cJSON_Delete(root);
            return "bad-payload: value must be a JSON object {\"op\":\"get\"|\"select\"|\"modify\"|\"add\"|\"delete\"|\"reload\"|\"save\","
                   "\"table\":\"tray\"|\"hp\",...}";
        }
        const cJSON* j = 0;
        if ((j = cJSON_GetObjectItemCaseSensitive(root, "op")) && cJSON_IsString(j) && j->valuestring) op = j->valuestring;
        if ((j = cJSON_GetObjectItemCaseSensitive(root, "table")) && cJSON_IsString(j) && j->valuestring) table = j->valuestring;
        if ((j = cJSON_GetObjectItemCaseSensitive(root, "via")) && cJSON_IsString(j) && j->valuestring) via = j->valuestring;
        if ((j = cJSON_GetObjectItemCaseSensitive(root, "text")) && cJSON_IsString(j) && j->valuestring) { text = j->valuestring; hasText = true; }
        if ((j = cJSON_GetObjectItemCaseSensitive(root, "cancel")) && cJSON_IsBool(j)) cancel = cJSON_IsTrue(j) != 0;
        const char* const kInts[3] = {"col", "row", "step"};
        int* const kDst[3] = {&col, &row, &step};
        for (int k = 0; k < 3; ++k) {
            j = cJSON_GetObjectItemCaseSensitive(root, kInts[k]);
            if (!j) continue;
            if (!cJSON_IsNumber(j) || j->valuedouble != (double)(int)j->valuedouble || j->valuedouble < 0 || j->valuedouble > 100000) {
                cJSON_Delete(root);
                return std::string("bad-payload: \"") + kInts[k] + "\" must be a non-negative integer";
            }
            *kDst[k] = (int)j->valuedouble;
        }
        cJSON_Delete(root);
    }
    static const char* const kOps[] = {"get", "select", "modify", "add", "delete", "reload", "save"};
    bool known = false;
    for (std::size_t k = 0; k < sizeof(kOps) / sizeof(kOps[0]); ++k) known = known || op == kOps[k];
    if (!known) return "bad-payload: op must be get / select / modify / add / delete / reload / save";

    S98Lock lock;
    if (!g_created)
        return "not-ready: the Tray / Hot Plate tables are not built (W906_CfgTrayPlate_CreateForm = golden CreateForm(TfConfiguration) has not run)";
    TfConfigurationTrayPlate* f = W906_CfgTrayPlate();
    std::vector<std::string> todo;

    if (op == "get") {
        if (g_pend.on) {
            g_pend = S98Pending();
            todo.push_back("a Modify Data keypad was left open (step 0 without step 1) -- dropped, the cell is unchanged (golden: the keypad is modal)");
        }
        return S98Reply(f, op, -1, "", false, std::string(), std::string(), std::string(), std::string(), todo, ok);
    }

    const int t = table == "tray" ? 0 : table == "hp" ? 1 : -1;
    if (t < 0) return "bad-payload: table must be \"tray\" (golden tsTrayData) or \"hp\" (tsHPData)";
    const bool hp = t == 1;
    if (op == "modify" && step != 0 && step != 1) return "bad-payload: modify needs \"step\":0 (open the keypad) or 1 (OK / cancel)";
    if (op == "modify" && step == 0 && !via.empty() && via != "button" && via != "dblclick")
        return "bad-payload: via must be \"button\" (Modify Data) or \"dblclick\" (double click on the grid)";
    if (op == "modify" && step == 1) {
        if (!g_pend.on || g_pend.hp != hp)
            return "keypad: no Modify Data keypad is open for this table (send step 0 first)";
        via = g_pend.dbl ? "dblclick" : "button";
        if (!hasText && !cancel) return "bad-payload: modify step 1 needs \"text\" (OK) or \"cancel\":true";
    }

    // ---- 守衛 ----
    if (S98Running())
        return "running: SystemStart / SoftStart -- settings are not changed from the web while the machine runs (RULINGS_20260927 #7); "
               "golden opens Configuration only from the stopped main screen (sbConfigClick main.cpp:29012 if(SystemStart) return;) and the form is modal";
    if (!S98PageOpen())
        return "bad-payload: reload page: open the Configuration page (editlist.get IniConfig = golden FormShow) with the current access "
               "level before using the Tray / Hot Plate tabs";
    if (!S98OnTab(t))
        return std::string("not-on-tab: the server's PageControl1 is not on ") + kS98Tabs[t].tab +
               " (golden: only the controls of the page on screen can be clicked) -- click the tab again";
    const bool gridOp = op == "select" || (op == "modify" && via == "dblclick");
    const char* need = gridOp ? kS98Tabs[t].tab : (op == "reload" ? kS98Tabs[t].reload : kS98Tabs[t].panel);
    if (!S98Operable(need))
        return std::string("not-operable: ") + need + " (or a container) is disabled or hidden after golden FormShow / PageControl1Change "
               "(access level: LevelSet.AccessLevel[30] and [" + (hp ? "79" : "78") + "], cConfiguration.cpp:" + (hp ? "5382" : "5381") +
               "; Security_new.def [Config] tab lock, :6113)";
    if (!(op == "modify" && step == 1) && g_pend.on) {
        g_pend = S98Pending();
        todo.push_back("a Modify Data keypad was left open (step 0 without step 1) -- dropped, the cell is unchanged (golden: the keypad is modal)");
    }

    // ---- 跑 golden ----
    vclcompat::TStringGrid* g = S98Grid(f, t);
    const char* golden = "";
    std::string keypad, errCls, errText, wrote;
    try {
        S98VclOn vcl;
        if (op == "select") {
            golden = hp ? "cConfiguration.cpp:7083 strngrdHPSelectCell" : "cConfiguration.cpp:6945 strngrdTraySelectCell";
            if (col < 0 || row < 0) return "bad-payload: select needs \"col\" and \"row\"";
            if (col < g->FixedCols || row < g->FixedRows || col >= (int)g->ColCount || row >= (int)g->RowCount)
                return "bad-cell: (" + S98N(col) + "," + S98N(row) + ") is a fixed cell or outside the " +
                       S98N((int)g->ColCount) + "x" + S98N((int)g->RowCount) +
                       " grid (VCL MouseDown does not move the current cell there)";
            f->VclMoveCurrent(hp, col, row);
        } else if (op == "modify") {
            const bool dbl = via == "dblclick";
            golden = dbl ? (hp ? "cConfiguration.cpp:7090 strngrdHPDblClick -> :7095 btnModifyHPClick"
                               : "cConfiguration.cpp:6952 strngrdTrayDblClick -> :6957 btnModifyTrayClick")
                         : (hp ? "cConfiguration.cpp:7095 btnModifyHPClick" : "cConfiguration.cpp:6957 btnModifyTrayClick");
            const int sr = hp ? f->iSelHPRow : f->iSelTrayRow, sc = hp ? f->iSelHPCol : f->iSelTrayCol;
            if (step == 0) {
                S98KpReset(kS98KpProbe);
                if (dbl) { if (hp) f->strngrdHPDblClick(); else f->strngrdTrayDblClick(); }
                else     { (hp ? f->btnModifyHP : f->btnModifyTray)->Click(); }
                const bool opened = g_kp.opened;
                if (opened) {
                    g_pend.on = true;  g_pend.hp = hp;  g_pend.dbl = dbl;
                    g_pend.row = sr;   g_pend.col = sc; g_pend.current = g_kp.current;
                }
                keypad = S98KeypadJson(opened, opened ? std::string(g->Cells[sc][0].c_str()) : std::string());
                S98KpReset(kS98KpNone);
            } else {
                const bool same = sr == g_pend.row && sc == g_pend.col && AnsiString(g->Cells[sc][sr]) == g_pend.current;
                g_pend = S98Pending();
                if (!same)
                    return "stale: the selected cell or its text changed after the keypad opened (the table was reloaded?) -- nothing written";
                S98KpReset(kS98KpAnswer);
                g_kp.cancel = cancel;
                g_kp.text = text;
                try {
                    if (dbl) { if (hp) f->strngrdHPDblClick(); else f->strngrdTrayDblClick(); }
                    else     { (hp ? f->btnModifyHP : f->btnModifyTray)->Click(); }
                } catch (...) { S98KpReset(kS98KpNone); throw; }
                S98KpReset(kS98KpNone);
            }
        } else if (op == "add") {
            golden = hp ? "cConfiguration.cpp:7120 btnAddHPClick" : "cConfiguration.cpp:6984 btnAddTrayClick";
            (hp ? f->btnAddHP : f->btnAddTray)->Click();
        } else if (op == "delete") {
            golden = hp ? "cConfiguration.cpp:7132 btnDeleteHPClick" : "cConfiguration.cpp:6996 btnDeleteTrayClick";
            (hp ? f->btnDeleteHP : f->btnDeleteTray)->Click();
        } else if (op == "reload") {
            golden = hp ? "cConfiguration.cpp:7148 sbtReloadHPClick" : "cConfiguration.cpp:7037 sbtReloadTrayClick";
            (hp ? f->sbtReloadHP : f->sbtReloadTray)->Click();
        } else {   // save
            golden = hp ? "cConfiguration.cpp:7194 sbUpdateHPClick (then :7215 sbtReloadTray->Click(), R33)" : "cConfiguration.cpp:7012 sbUpdateTrayClick";
            const AnsiString path = hp ? W906_PlateTablePath() : W906_TrayTablePath();
            try {
                (hp ? f->sbUpdateHP : f->sbUpdateTray)->Click();
            } catch (const S98VclError&) {
                if (FileExists(path)) wrote = path.c_str();      // HP：檔已經寫了、之後重讀 Tray 表才出事
                throw;
            }
            wrote = path.c_str();
            std::printf("cfgtrayplate.op save %s -> %s written (golden %s)\n", kS98Tabs[t].key, path.c_str(), golden);
        }
    } catch (const S98VclError& e) {
        errCls = e.cls;
        errText = e.what();
        std::printf("cfgtrayplate.op %s %s -> golden %s: %s\n", op.c_str(), kS98Tabs[t].key, e.cls.c_str(), e.what());
    } catch (const std::invalid_argument& e) {
        return e.what();                                          // bad-value：格子不動
    } catch (const std::exception& e) {
        return std::string("handler-failed: ") + e.what();
    }
    return S98Reply(f, op, t, golden, true, keypad, errCls, errText, wrote, todo, ok);
}
