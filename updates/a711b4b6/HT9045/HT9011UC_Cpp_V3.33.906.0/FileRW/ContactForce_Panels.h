// ===========================================================================
//  FileRW/ContactForce_Panels.h -- golden TfContactForce 的四個動態面板類別（ContactForce.h:18-127、ContactForce.cpp:21-419、
//  :1405-1425、:1442-1568，V912 D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy）的 C 路轉錄。
//
//  //AI(W906-FRW-S57) 20260926: 新檔（Steven 團隊，S57 Setup.ContactForce 讀寫）。只給 FileRW/ContactForce.cpp 一個 TU include
//    （經 tools/editlist/ContactForce.py 的 includes 帶進 FileRW/ContactForce.gen.inc）。
//
//  為什麼手寫：tools/gen_editlist.py 只轉「表單類別 TfContactForce::」的方法；這四個類別是執行期才 new 的面板
//    （golden 建構子 :464-636 依 ContactInfo.ini [SLK Type] 的 CSV 一型一個／一型 8 或 16 個），不是 DFM 元件。
//    做法照 references/generators.md 十五「動態面板」＋上一位工程師的計畫（scratchpad contactforce/plan.md §3）：
//    * 資料與 golden 同形：golden 的 vector<THTSLKClass*> 等四個 vector 在 gen.inc 的 members（本 TU static），
//      golden 轉出來的 ReadFile／WriteFile／FormShow 原文 `SLKClass[i]->trckbrDiameter->Position` 照抄成立。
//    * 元件用 golden 自己的 Name（Str1，例 "trckbrDiameter_30"、"edtContactOffset_30_5"）當具名替身
//      EL<T>("TfContactForce", Name)＝頁面元件 id；父子照 golden 的 Parent（→ ELSetParents，權限／可見判斷用）。
//    * 版面（Top／Left／Height／Width／Align／Font）與小鍵盤 OnClick（fQwertyKey）不轉：HTML 自己排、頁面 kb 表。
//
//  ⚠ 與 golden 的差異（每一條都寫在該行）：
//    D-P1 OnChange：golden 是每個面板自己的成員函式（trckbrDiameter_Change 等）；filerw::ELTrackBar::OnChange 是無參數的
//         函式指標 → 接到一支「全部面板重算」的跳板 CF_PanelTrackBarsChanged（FileRW/ContactForce.cpp）。每個 golden 處理器
//         只做 `edtLoadRate->Text=AnsiString(trckbrDiameter->Position/100.0)`（冪等、只讀自己的 trackbar），全部重算一次效果相同。
//    D-P2 TTrackBar 的 Min／Max：golden 先設 Position=100（VCL 預設 Min=0/Max=10 → 夾成 10），再設 Min=80、Max=150。
//         VCL TTrackBar.SetMin 只在 Value<=FMax 時生效、SetMax 只在 Value>=FMin 時生效 —— 所以 golden 建構完是
//         Position=10、Min=0、Max=150（golden FormShow :861-864 才對 SLKClass 再設一次 Min=80／Max=150，Ind／OneByOne 沒有）。
//         filerw::ELTrackBar 的 Lim::operator= 沒有這個守衛（一律採用），直接照抄會變成 Position=80、Min=80。
//         這裡用 CF_VclSetMin／CF_VclSetMax 照 VCL 守衛設。⚠ 這條 VCL 語意是推論（本機沒有 BCB6 VCL 原始碼；依據是 golden
//         FormShow 那四行「重設 Min／Max」只有在建構子的 Min=80 沒生效時才有意義），待 BCB6 實機核對 —— 只影響
//         「沒有 ContactInfo.ini、建構子直接 WriteFile」那一次寫進檔案的 LoadRate（0.1 或 0.8），ReadFile 之後記憶體一律被
//         CheckRange 夾回 0.8～1.5。
//    D-P3 成員初值：golden 這四個類別是 TComponent 衍生，BCB `new` 會把物件清成 0（dLoadRate／dHotOffset／dContactOffset…
//         建構子沒設的都是 0.0）；這裡用 0 初始化，照 BCB。（ContactForce.h 的 SlkForceData 預設 dLoadRate=1.0 是另一回事，
//         那是 906 載入器的容器，見 ContactForce.h「KNOWN DIVERGENCE」。）
//    D-P4 同名：golden 同一個 Name 在同一個 Owner 下出現兩次（例 [SLK Type] Type=30,30）VCL 會丟 EComponentError（建構子失敗）；
//         具名替身同名就是同一個物件（兩個面板共用元件）。golden 本來就不允許，照留、不另外擋。
// ===========================================================================
#pragma once

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "FileRW/_EditList.h"
#include "cmydef.h"          // EP_Install（golden :181）、INSTALL_DOUBLE_EP／DOUBLE_EP_MULTI（golden :218／:1454）

// 跳板（D-P1）：定義在 FileRW/ContactForce.cpp（要看得到 gen.inc 的四個 vector）
static void CF_PanelTrackBarsChanged();

namespace cfpanel {
static const char* const kForm = "TfContactForce";

// golden `X->Parent=Y`（D-P0：只記父子，權限／可見判斷用；golden 的 Owner 與排版不轉）
inline void Parent(const AnsiString& child, const char* parent)
{
    const std::string c = child.c_str();
    const char* const pr[1][2] = {{c.c_str(), parent}};
    filerw::ELSetParents(kForm, pr, 1);
}
inline void Parent(const AnsiString& child, const AnsiString& parent)
{
    const std::string p = parent.c_str();
    Parent(child, p.c_str());
}
// D-P2：VCL TTrackBar.SetMin／SetMax 的守衛（見檔頭）
inline void CF_VclSetMin(filerw::ELTrackBar* t, int v) { if (v <= (int)t->Max) t->Min = v; }
inline void CF_VclSetMax(filerw::ELTrackBar* t, int v) { if (v >= (int)t->Min) t->Max = v; }
}  // namespace cfpanel

// golden 的 dMinForce 階梯（:191-202 THTSLKClass 含 402；:315-324／:406-415／:1550-1559 其餘三個不含 402）
inline double CF_MinForceLadder(double dDiameter, bool bHas402)
{
    if (dDiameter <= 30.0)                  return 0.5;
    else if (dDiameter < 40.0)              return 1;
    else if (dDiameter < 50.0)              return 2;
    else if (dDiameter < 60.0)              return 4;
    else if (bHas402 && dDiameter == 402)   return 4;
    else                                    return 8;
}

// ---------------------------------------------------------------------------
// golden ContactForce.h:18-51 / ContactForce.cpp:21-206  THTSLKClass
// ---------------------------------------------------------------------------
class THTSLKClass
{
public:
    THTSLKClass(AnsiString Dia, int Tag, bool bDefault);

    TGroupBox *gbLoadRate = nullptr;
    TLabel *lblDiameter = nullptr;
    TLabel *lblHotOffset = nullptr;
    TLabel *lblContactOffset = nullptr;
    TLabel *lblDiameter_NS = nullptr;
    TLabel *lblContactOffset_NS = nullptr;        //kevin 20170807 (Steven) NS offset
    TEdit *edtHotOffset = nullptr;
    filerw::ELTrackBar *trckbrDiameter = nullptr;
    filerw::ELTrackBar *trckbrDiameter_NS = nullptr;
    TEdit *edtLoadRate = nullptr;
    TEdit *edtLoadRate_NS = nullptr;
    TEdit *edtContactOffset = nullptr;
    TEdit *edtContactOffset_NS = nullptr;         //kevin 20170807 (Steven) NS offset
    AnsiString sDiameter;
    void trckbrDiameter_Change();                 // golden :1405（__fastcall，TObject *Sender 沒讀，拿掉）
    void trckbrDiameter_NSChange();               // golden :1422
    double dLoadRate = 0, dLoadRate_NS = 0, dDiameter = 0, dHotOffset = 0, dContactOffset = 0, dContactOffset_NS = 0;   // D-P3
    double dMinForce = 0, dMaxForce = 0;
    bool   bShow = false;
    int    iTag = 0;
    // 本 TU 自己的：這個面板的替身名稱（角色 → golden Name），給 editlist.get 的 extra 帶給頁面建 DOM
    std::vector<std::pair<std::string, std::string> > ids;
};

inline THTSLKClass::THTSLKClass(AnsiString Dia, int Tag, bool bDefault)
{
    using filerw::EL;
    AnsiString Str1, Str2;
    sDiameter   =Dia;                                                           // golden :24
    iTag        =Tag;
    bShow       =bDefault;

    Str1.sprintf("gbLoadRate_%s", Dia.c_str());                                 // golden :28
    Str2.sprintf("Load rate of %s mm", Dia.c_str());
    gbLoadRate                  =EL<TGroupBox>(cfpanel::kForm, Str1.c_str());   // golden :30 new TGroupBox(fContactForce->scrlbxDynamicKit)
    cfpanel::Parent(Str1, "scrlbxDynamicKit");                                  // golden :31 Parent=fContactForce->scrlbxDynamicKit
    gbLoadRate->Caption         =Str2;                                          // golden :33（:34-39 字型／版面：HTML）
    ids.push_back(std::make_pair(std::string("gbLoadRate"), std::string(Str1.c_str())));
    const AnsiString gb = Str1;

    Str1.sprintf("lblDiameter_%s", Dia.c_str());                                // golden :41
    Str2.sprintf("%s mm :", Dia.c_str());
    lblDiameter=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblDiameter->Caption        =Str2;                                          // golden :48（:46-47 版面）
    ids.push_back(std::make_pair(std::string("lblDiameter"), std::string(Str1.c_str())));

    Str1.sprintf("lblDiameter_NS_%s", Dia.c_str());                             // golden :50
    Str2.sprintf("%s mm for NS :", Dia.c_str());
    lblDiameter_NS=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblDiameter_NS->Caption     =Str2;                                          // golden :57
    ids.push_back(std::make_pair(std::string("lblDiameter_NS"), std::string(Str1.c_str())));

    Str1.sprintf("lblHotOffset_%s", Dia.c_str());                               // golden :59
    Str2.sprintf("%s mm offset by heater mode:", Dia.c_str());
    lblHotOffset=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblHotOffset->Caption       =Str2;                                          // golden :66
    ids.push_back(std::make_pair(std::string("lblHotOffset"), std::string(Str1.c_str())));

    Str1.sprintf("lblContactOffset_%s", Dia.c_str());                           // golden :68
    Str2.sprintf("%s mm contact offset:", Dia.c_str());
    lblContactOffset=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblContactOffset->Caption   =Str2;                                          // golden :75
    ids.push_back(std::make_pair(std::string("lblContactOffset"), std::string(Str1.c_str())));

    Str1.sprintf("edtHotOffset_%s", Dia.c_str());                               // golden :77
    edtHotOffset=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtHotOffset->Text          ="";                                            // golden :85（:86 OnClick＝小鍵盤：頁面 kb 表 N_DOUBLE 2 位 -0.5～0.5，golden :1402）
    ids.push_back(std::make_pair(std::string("edtHotOffset"), std::string(Str1.c_str())));

    Str1.sprintf("edtContactOffset_%s", Dia.c_str());                           // golden :88
    edtContactOffset=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtContactOffset->Text          ="";                                        // golden :96（:97 OnClick＝小鍵盤 N_DOUBLE 2 位 -10～10，golden :1434）
    ids.push_back(std::make_pair(std::string("edtContactOffset"), std::string(Str1.c_str())));

    Str1.sprintf("trckbrDiameter_%s", Dia.c_str());                             // golden :99
    trckbrDiameter=EL<filerw::ELTrackBar>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    trckbrDiameter->Position    =100;                                           // golden :103（VCL 預設 Max=10 → 夾成 10）
    cfpanel::CF_VclSetMin(trckbrDiameter, 80);                                  // golden :108 Min=80（D-P2：80>Max 10，VCL 不採用）
    cfpanel::CF_VclSetMax(trckbrDiameter, 150);                                 // golden :109 Max=150
    trckbrDiameter->OnChange    =&CF_PanelTrackBarsChanged;                     // golden :110 OnChange=trckbrDiameter_Change（D-P1）
    ids.push_back(std::make_pair(std::string("trckbrDiameter"), std::string(Str1.c_str())));

    Str1.sprintf("trckbrDiameter_NS_%s", Dia.c_str());                          // golden :112
    trckbrDiameter_NS=EL<filerw::ELTrackBar>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    trckbrDiameter_NS->Position =100;                                           // golden :116
    cfpanel::CF_VclSetMin(trckbrDiameter_NS, 80);                               // golden :121（D-P2）
    cfpanel::CF_VclSetMax(trckbrDiameter_NS, 150);                              // golden :122
    trckbrDiameter_NS->OnChange =&CF_PanelTrackBarsChanged;                     // golden :123 OnChange=trckbrDiameter_NSChange（D-P1）
    ids.push_back(std::make_pair(std::string("trckbrDiameter_NS"), std::string(Str1.c_str())));

    Str1.sprintf("edtLoadRate%s", Dia.c_str());                                 // golden :125
    edtLoadRate=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtLoadRate->Text           =AnsiString(trckbrDiameter->Position/100.0);    // golden :133
    edtLoadRate->Enabled        =false;                                         // golden :134（:135 OnClick 小鍵盤；Enabled=false 按不到）
    ids.push_back(std::make_pair(std::string("edtLoadRate"), std::string(Str1.c_str())));

    Str1.sprintf("edtLoadRate_NS%s", Dia.c_str());                              // golden :137
    edtLoadRate_NS=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtLoadRate_NS->Text        =AnsiString(trckbrDiameter_NS->Position/100.0); // golden :145
    edtLoadRate_NS->Enabled     =false;                                         // golden :146
    ids.push_back(std::make_pair(std::string("edtLoadRate_NS"), std::string(Str1.c_str())));

    Str1.sprintf("lblContactOffset_NS%s", Dia.c_str());                         // golden :149 kevin 20170807 (Steven) add
    Str2.sprintf("%s mm contact offset_NS:", Dia.c_str());
    lblContactOffset_NS=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblContactOffset_NS->Caption   =Str2;                                       // golden :156
    ids.push_back(std::make_pair(std::string("lblContactOffset_NS"), std::string(Str1.c_str())));

    Str1.sprintf("edtContactOffset_NS_%s", Dia.c_str());                        // golden :158
    edtContactOffset_NS=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtContactOffset_NS->Text          ="";                                     // golden :166（:167 OnClick 小鍵盤 -10～10，golden :1439）
    ids.push_back(std::make_pair(std::string("edtContactOffset_NS"), std::string(Str1.c_str())));

    if(Dia=="80_Hi")                                                            // golden :171
    {
        dDiameter=80;
    }
    else if(Dia=="40x2")                                                        //Ifor 20230830 add:40 倍立缸
    {
        dDiameter=402;
    }
    else
    {
        if(EP_Install==5)                                                       // golden :181
        {
            if(Dia.Pos("Arm2_")==0)                                             // golden :183 照抄：只有「沒有 Arm2_」才去掉前 5 字 → Arm2_ 那筆 atof 得 0（golden 的，照留）
            {
                Dia=Dia.SubString(6, Dia.Length());
            }
        }
        dDiameter=atof(Dia.c_str());
    }

    dMinForce=CF_MinForceLadder(dDiameter, true);                               // golden :191-202
    dMaxForce=3.14*(dDiameter/100.0)*(dDiameter/100.0)*500;                     // golden :204
    gbLoadRate->Visible=bShow;                                                  // golden :205
}

inline void THTSLKClass::trckbrDiameter_Change()                                // golden :1405-1408
{
    edtLoadRate->Text=AnsiString(trckbrDiameter->Position/100.0);
}
inline void THTSLKClass::trckbrDiameter_NSChange()                              // golden :1422-1425
{
    edtLoadRate_NS->Text=AnsiString(trckbrDiameter_NS->Position/100.0);
}

// ---------------------------------------------------------------------------
// golden ContactForce.h:79-102 / ContactForce.cpp:209-328  THTDieForceOneByOneSLKClass（Eastsun 20260525 INSTALL_DOUBLE_EP_3）
// ---------------------------------------------------------------------------
class THTDieForceOneByOneSLKClass
{
public:
    THTDieForceOneByOneSLKClass(AnsiString Dia, int Tag, bool bDefault);

    TGroupBox *gbDieForceOneByOneLoadRate = nullptr;
    TLabel *lblDieForceOneByOneDiameter = nullptr;
    TLabel *lblDieForceOneByOneContactOffset = nullptr;
    filerw::ELTrackBar *trckbrDieForceOneByOneDiameter = nullptr;
    TEdit *edtDieForceOneByOneLoadRate = nullptr;
    TEdit *edtDieForceOneByOneContactOffset = nullptr;
    AnsiString sDiameter;
    void trckbrDieForceOneByOneDiameter_Change();   // golden :1417
    double dLoadRate = 0, dDiameter = 0, dHotOffset = 0, dContactOffset = 0, dMinForce = 0, dMaxForce = 0;   // D-P3
    bool   bShow = false;
    int    iTag = 0;
    std::vector<std::pair<std::string, std::string> > ids;
};

inline THTDieForceOneByOneSLKClass::THTDieForceOneByOneSLKClass(AnsiString Dia, int Tag, bool bDefault)
{
    using filerw::EL;
    AnsiString Str1, Str2;
    sDiameter   =Dia;                                                           // golden :212
    iTag        =Tag;
    bShow       =bDefault;
    int iCount=(Tag%8)+1;                                                       // golden :215

    Str1.sprintf("gbDieForceOneByOneLoadRate_%s_%d", Dia.c_str(), Tag);         // golden :217
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        if(iCount<5)
            Str2.sprintf("DieForce One By One Load rate of %s mm Arm1_%d", Dia.c_str(), iCount);
        else
            Str2.sprintf("DieForce One By One Load rate of %s mm Arm2_%d", Dia.c_str(), iCount-4);
    }
    else
    {
       Str2.sprintf("DieForce One By One Load rate of %s mm", Dia.c_str());
    }
    gbDieForceOneByOneLoadRate                  =EL<TGroupBox>(cfpanel::kForm, Str1.c_str());   // golden :229
    cfpanel::Parent(Str1, "scrlbxDieForceOneByOneDynamicKit");                  // golden :230
    gbDieForceOneByOneLoadRate->Caption         =Str2;                          // golden :232（:233-238 字型／版面）
    ids.push_back(std::make_pair(std::string("gbDieForceOneByOneLoadRate"), std::string(Str1.c_str())));
    const AnsiString gb = Str1;

    Str1.sprintf("lblDieForceOneByOneDiameter_%s_%d", Dia.c_str(), Tag);        // golden :240
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        if(iCount<5)
            Str2.sprintf("%s mm 1_%d:", Dia.c_str(), iCount);
        else
            Str2.sprintf("%s mm 2_%d:", Dia.c_str(), iCount-4);
    }
    else
    {
        Str2.sprintf("%s mm :", Dia.c_str());
    }
    lblDieForceOneByOneDiameter=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblDieForceOneByOneDiameter->Caption        =Str2;                          // golden :257
    ids.push_back(std::make_pair(std::string("lblDieForceOneByOneDiameter"), std::string(Str1.c_str())));

    Str1.sprintf("lblDieForceOneByOneContactOffset_%s_%d", Dia.c_str(), Tag);   // golden :259
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        if(iCount<5)
            Str2.sprintf("%s mm contact offset 1_%d:", Dia.c_str(), iCount);
        else
            Str2.sprintf("%s mm contact offset 2_%d:", Dia.c_str(), iCount-4);
    }
    else
    {
        Str2.sprintf("%s mm contact offset:", Dia.c_str());
    }
    lblDieForceOneByOneContactOffset=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblDieForceOneByOneContactOffset->Caption   =Str2;                          // golden :276
    ids.push_back(std::make_pair(std::string("lblDieForceOneByOneContactOffset"), std::string(Str1.c_str())));

    Str1.sprintf("edtDieForceOneByOneContactOffset_%s_%d", Dia.c_str(), Tag);   // golden :278
    edtDieForceOneByOneContactOffset=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtDieForceOneByOneContactOffset->Text          ="";                        // golden :286（:287 OnClick 小鍵盤 -10～10）
    ids.push_back(std::make_pair(std::string("edtDieForceOneByOneContactOffset"), std::string(Str1.c_str())));

    Str1.sprintf("trckbrDieForceOneByOneDiameter_%s_%d", Dia.c_str(), Tag);     // golden :289
    trckbrDieForceOneByOneDiameter=EL<filerw::ELTrackBar>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    trckbrDieForceOneByOneDiameter->Position    =100;                           // golden :293
    cfpanel::CF_VclSetMin(trckbrDieForceOneByOneDiameter, 80);                  // golden :298（D-P2）
    cfpanel::CF_VclSetMax(trckbrDieForceOneByOneDiameter, 150);                 // golden :299
    trckbrDieForceOneByOneDiameter->OnChange    =&CF_PanelTrackBarsChanged;     // golden :300（D-P1）
    ids.push_back(std::make_pair(std::string("trckbrDieForceOneByOneDiameter"), std::string(Str1.c_str())));

    Str1.sprintf("edtDieForceOneByOneLoadRate%s_%d", Dia.c_str(), Tag);         // golden :302
    edtDieForceOneByOneLoadRate=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtDieForceOneByOneLoadRate->Text           =AnsiString(trckbrDieForceOneByOneDiameter->Position/100.0);   // golden :310
    edtDieForceOneByOneLoadRate->Enabled        =false;                         // golden :311
    ids.push_back(std::make_pair(std::string("edtDieForceOneByOneLoadRate"), std::string(Str1.c_str())));

    dDiameter=atof(Dia.c_str());                                                // golden :313

    dMinForce=CF_MinForceLadder(dDiameter, false);                              // golden :315-324
    dMaxForce=3.14*(dDiameter/100.0)*(dDiameter/100.0)*500;                     // golden :326
    gbDieForceOneByOneLoadRate->Visible=bShow;                                  // golden :327
}

inline void THTDieForceOneByOneSLKClass::trckbrDieForceOneByOneDiameter_Change()   // golden :1417-1420
{
    edtDieForceOneByOneLoadRate->Text=AnsiString(trckbrDieForceOneByOneDiameter->Position/100.0);
}

// ---------------------------------------------------------------------------
// golden ContactForce.h:53-76 / ContactForce.cpp:331-419  THTDieForceSLKClass（Ifor 20191003 Die Force 自定義 Kit 直徑）
// ---------------------------------------------------------------------------
class THTDieForceSLKClass
{
public:
    THTDieForceSLKClass(AnsiString Dia, int Tag, bool bDefault);

    TGroupBox *gbDieForceLoadRate = nullptr;
    TLabel *lblDieForceDiameter = nullptr;
    TLabel *lblDieForceContactOffset = nullptr;
    filerw::ELTrackBar *trckbrDieForceDiameter = nullptr;
    TEdit *edtDieForceLoadRate = nullptr;
    TEdit *edtDieForceContactOffset = nullptr;
    AnsiString sDiameter;
    void trckbrDieForceDiameter_Change();         // golden :1410
    double dLoadRate = 0, dDiameter = 0, dHotOffset = 0, dContactOffset = 0, dMinForce = 0, dMaxForce = 0;   // D-P3
    bool   bShow = false;
    int    iTag = 0;
    std::vector<std::pair<std::string, std::string> > ids;
};

inline THTDieForceSLKClass::THTDieForceSLKClass(AnsiString Dia, int Tag, bool bDefault)
{
    using filerw::EL;
    AnsiString Str1, Str2;
    sDiameter   =Dia;                                                           // golden :334
    iTag        =Tag;
    bShow       =bDefault;

    Str1.sprintf("gbDieForceLoadRate_%s", Dia.c_str());                         // golden :338
    Str2.sprintf("DieForce Load rate of %s mm", Dia.c_str());
    gbDieForceLoadRate                  =EL<TGroupBox>(cfpanel::kForm, Str1.c_str());   // golden :340
    cfpanel::Parent(Str1, "scrlbxDieForceDynamicKit");                          // golden :341
    gbDieForceLoadRate->Caption         =Str2;                                  // golden :343（:344-349 字型／版面）
    ids.push_back(std::make_pair(std::string("gbDieForceLoadRate"), std::string(Str1.c_str())));
    const AnsiString gb = Str1;

    Str1.sprintf("lblDieForceDiameter_%s", Dia.c_str());                        // golden :351
    Str2.sprintf("%s mm :", Dia.c_str());
    lblDieForceDiameter=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblDieForceDiameter->Caption        =Str2;                                  // golden :358
    ids.push_back(std::make_pair(std::string("lblDieForceDiameter"), std::string(Str1.c_str())));

    Str1.sprintf("lblDieForceContactOffset_%s", Dia.c_str());                   // golden :360
    Str2.sprintf("%s mm contact offset:", Dia.c_str());
    lblDieForceContactOffset=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblDieForceContactOffset->Caption   =Str2;                                  // golden :367
    ids.push_back(std::make_pair(std::string("lblDieForceContactOffset"), std::string(Str1.c_str())));

    Str1.sprintf("edtDieForceContactOffset_%s", Dia.c_str());                   // golden :369
    edtDieForceContactOffset=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtDieForceContactOffset->Text          ="";                                // golden :377（:378 OnClick 小鍵盤 -10～10）
    ids.push_back(std::make_pair(std::string("edtDieForceContactOffset"), std::string(Str1.c_str())));

    Str1.sprintf("trckbrDieForceDiameter_%s", Dia.c_str());                     // golden :380
    trckbrDieForceDiameter=EL<filerw::ELTrackBar>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    trckbrDieForceDiameter->Position    =100;                                   // golden :384
    cfpanel::CF_VclSetMin(trckbrDieForceDiameter, 80);                          // golden :389（D-P2）
    cfpanel::CF_VclSetMax(trckbrDieForceDiameter, 150);                         // golden :390
    trckbrDieForceDiameter->OnChange    =&CF_PanelTrackBarsChanged;             // golden :391（D-P1）
    ids.push_back(std::make_pair(std::string("trckbrDieForceDiameter"), std::string(Str1.c_str())));

    Str1.sprintf("edtDieForceLoadRate%s", Dia.c_str());                         // golden :393
    edtDieForceLoadRate=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtDieForceLoadRate->Text           =AnsiString(trckbrDieForceDiameter->Position/100.0);   // golden :401
    edtDieForceLoadRate->Enabled        =false;                                 // golden :402
    ids.push_back(std::make_pair(std::string("edtDieForceLoadRate"), std::string(Str1.c_str())));

    dDiameter=atof(Dia.c_str());                                                // golden :404

    dMinForce=CF_MinForceLadder(dDiameter, false);                              // golden :406-415
    dMaxForce=3.14*(dDiameter/100.0)*(dDiameter/100.0)*500;                     // golden :417
    gbDieForceLoadRate->Visible=bShow;                                          // golden :418
}

inline void THTDieForceSLKClass::trckbrDieForceDiameter_Change()                // golden :1410-1413
{
    edtDieForceLoadRate->Text=AnsiString(trckbrDieForceDiameter->Position/100.0);
}

// ---------------------------------------------------------------------------
// golden ContactForce.h:105-127 / ContactForce.cpp:1442-1563  THTSLKIndClass（逐站微調）
// ---------------------------------------------------------------------------
class THTSLKIndClass
{
public:
    THTSLKIndClass(AnsiString Dia, int Tag, bool bDefault);

    TGroupBox *gbLoadRateInd = nullptr;
    TLabel *lblDiameterInd = nullptr;
    TLabel *lblContactOffsetInd = nullptr;
    filerw::ELTrackBar *trckbrDiameterInd = nullptr;
    TEdit *edtLoadRateInd = nullptr;
    TEdit *edtContactOffsetInd = nullptr;
    AnsiString sDiameter;
    void trckbrDiameterInd_Change();              // golden :1565
    double dLoadRate = 0, dDiameter = 0, dContactOffset = 0, dMinForce = 0, dMaxForce = 0;   // D-P3
    bool   bShow = false;
    int    iTag = 0;
    std::vector<std::pair<std::string, std::string> > ids;
};

inline THTSLKIndClass::THTSLKIndClass(AnsiString Dia, int Tag, bool bDefault)
{
    using filerw::EL;
    AnsiString Str1, Str2;
    sDiameter   =Dia;                                                           // golden :1445
    iTag        =Tag;
    bShow       =bDefault;
    int iCount=(Tag%8)+1;                                                       // golden :1449

    Str1.sprintf("gbLoadRate_%s_%d", Dia.c_str(), Tag);                         // golden :1452
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        if(iCount<5)
            Str2.sprintf("Load rate of %s mm Arm1_%d", Dia.c_str(), iCount);
        else
            Str2.sprintf("Load rate of %s mm Arm2_%d", Dia.c_str(), iCount-4);
    }
    else
    Str2.sprintf("Load rate of %s mm", Dia.c_str());
    gbLoadRateInd                  =EL<TGroupBox>(cfpanel::kForm, Str1.c_str());   // golden :1464
    cfpanel::Parent(Str1, "scrlbxDynamicKitInd");                               // golden :1465
    gbLoadRateInd->Caption         =Str2;                                       // golden :1467（:1468-1473 字型／版面）
    ids.push_back(std::make_pair(std::string("gbLoadRateInd"), std::string(Str1.c_str())));
    const AnsiString gb = Str1;

    Str1.sprintf("lblDiameter_%s_%d", Dia.c_str(), Tag);                        // golden :1475
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        if(iCount<5)
            Str2.sprintf("%s mm 1_%d:", Dia.c_str(), iCount);
        else
            Str2.sprintf("%s mm 2_%d:", Dia.c_str(), iCount-4);
    }
    else
    Str2.sprintf("%s mm_%d:", Dia.c_str(), Tag);
    lblDiameterInd=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblDiameterInd->Caption        =Str2;                                       // golden :1492
    ids.push_back(std::make_pair(std::string("lblDiameterInd"), std::string(Str1.c_str())));

    Str1.sprintf("lblContactOffset_%s_%d", Dia.c_str(), Tag);                   // golden :1494
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        if(iCount<5)
            Str2.sprintf("%s mm contact offset 1_%d:", Dia.c_str(), iCount);
        else
            Str2.sprintf("%s mm contact offset 2_%d:", Dia.c_str(), iCount-4);
    }
    else
    Str2.sprintf("%s mm contact offset%d:", Dia.c_str(), Tag);
    lblContactOffsetInd=EL<TLabel>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    lblContactOffsetInd->Caption   =Str2;                                       // golden :1511
    ids.push_back(std::make_pair(std::string("lblContactOffsetInd"), std::string(Str1.c_str())));

    Str1.sprintf("edtContactOffset_%s_%d", Dia.c_str(), Tag);                   // golden :1513
    edtContactOffsetInd=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtContactOffsetInd->Text          ="";                                     // golden :1521（:1522 OnClick 小鍵盤 -10～10）
    ids.push_back(std::make_pair(std::string("edtContactOffsetInd"), std::string(Str1.c_str())));

    Str1.sprintf("trckbrDiameter_%s_%d", Dia.c_str(), Tag);                     // golden :1524
    trckbrDiameterInd=EL<filerw::ELTrackBar>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    trckbrDiameterInd->Position    =100;                                        // golden :1528
    cfpanel::CF_VclSetMin(trckbrDiameterInd, 80);                               // golden :1533（D-P2）
    cfpanel::CF_VclSetMax(trckbrDiameterInd, 150);                              // golden :1534
    trckbrDiameterInd->OnChange    =&CF_PanelTrackBarsChanged;                  // golden :1535（D-P1）
    ids.push_back(std::make_pair(std::string("trckbrDiameterInd"), std::string(Str1.c_str())));

    Str1.sprintf("edtLoadRate%s_%d", Dia.c_str(), Tag);                         // golden :1537
    edtLoadRateInd=EL<TEdit>(cfpanel::kForm, Str1.c_str());
    cfpanel::Parent(Str1, gb);
    edtLoadRateInd->Text           =AnsiString(trckbrDiameterInd->Position/100.0);   // golden :1545
    edtLoadRateInd->Enabled        =false;                                      // golden :1546
    ids.push_back(std::make_pair(std::string("edtLoadRateInd"), std::string(Str1.c_str())));

    dDiameter=atof(Dia.c_str());                                                // golden :1548

    dMinForce=CF_MinForceLadder(dDiameter, false);                              // golden :1550-1559
    dMaxForce=3.14*(dDiameter/100.0)*(dDiameter/100.0)*500;                     // golden :1561
    gbLoadRateInd->Visible=bShow;                                               // golden :1562
}

inline void THTSLKIndClass::trckbrDiameterInd_Change()                          // golden :1565-1568
{
    edtLoadRateInd->Text=AnsiString(trckbrDiameterInd->Position/100.0);
}
