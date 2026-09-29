// ===========================================================================
//  FileRW/HSys_Heater.h -- golden V912 逐通道溫控器廠牌表（EN_HEATER_SHEET=1）的宣告，HandlerSys 頁讀寫檔用。
//
//  //AI(W906-FRW-P8) 20260926: 新檔（Steven 團隊）。Steven 20260926 S52「先以大量把讀寫檔進行移植為首要工作」，
//    盤點 cmydef_io_audit.md P8 的 TC401HeaterControl 一項。golden 一律照 V912：
//    D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950）。
//
//  這一份把 golden 散在三處的宣告收在一起（逐字，只改排版與註解）：
//    * MachineType.h:657-714  EN_HEATER_SHEET、eHeaterType_Count、eHeaterInsOpt_Count、THeaterInsInfo、g_tHeaterInsInfo、
//                             IsValEqual_HeaterInsOpt 等 8 支小工具的宣告（本體 golden MachineTypeUtility.cpp）
//    * cmydef.h:169            INVALID_INT_VAL_NEG（-9999）
//    * HandlerSys.cpp:18-49    版面常數、g_vecHeaterTypeIdxForShow／兩張對照表、TypeIdxToShowIdx／ShowIdxToTypeIdx
//    * HandlerSys.h:14         HeaterInsOpt_Read
//  本體在 FileRW/HSys.cpp（手寫入口，已在 _editlist_sources.cmake，不必動 CMake）。
//
//  ⚠ 範圍：只做「讀寫檔＋一個全域表」。golden V912 的溫控流程也改成逐通道判斷廠牌
//    （bthermo.cpp DoThermo:1167 第一次呼叫時 HeaterInsOpt_Read、:2520-2595／:3219 IsValEqual_HeaterInsOpt(Addr, …)；
//    rs232.cpp:164／:742-747；cConfiguration.cpp:5618-5741；EJ1N/OmronEJ1N.cpp:622）——那是底層（Jimmy，第 37 條），
//    移植樹那些檔仍是 906 的單一廠牌 TC401HeaterControl。所以：
//    * EN_HEATER_SHEET 只在 include 本檔的 TU（HSys 的產生檔＋FileRW/HSys.cpp）等於 1；其他 TU 看不到，行為不變。
//      Jimmy 翻 bthermo 等檔時，照 golden 把整段宣告搬回 MachineType.h、本體搬成根目錄的 MachineTypeUtility.cpp
//      （放進 ht9045_globals），本檔改成 include MachineType.h 即可（#ifndef 守衛讓兩邊可以並存一陣子）。
//    * g_tHeaterInsInfo 目前只有 HandlerSys 頁讀寫；溫控流程不讀它（讀的是 TC401HeaterControl＝[TempCtrl] HEATER_CTRL_TYPE）。
//      頁面若存成「各通道廠牌不同」，golden 912 的溫控會照逐通道跑，移植樹的溫控仍用 HEATER_CTRL_TYPE（＝rgHeaterType）——
//      差異列給 Jimmy。
// ===========================================================================
#pragma once

#include <map>
#include <vector>

#include "MachineType.h"          // tcTotalCount／tcHotPlate1… 列舉（與 golden V912 MachineType.h:638-655 逐字相同，20260926 比過）
#include "cmydef.h"               // extern const int TC401／KT4H／E5DC／NoHeater／DTK4848
#include "vclcompat/Controls.h"   // TLabel／TComboBox（THeaterInsInfo 存畫面元件指標；C 路是具名替身）

// golden cmydef.h:168-171
#ifndef INVALID_INT_VAL_NEG
#define INVALID_INT_VAL_POS                  9999                               //AI(ht9045-heater-control) 20260618 (RogerYang) : 移植創發2type溫控模組設定
#define INVALID_INT_VAL_NEG                  -9999
#endif

// golden MachineType.h:657-714 ----------------------------------------------------------------
#ifndef EN_HEATER_SHEET                                                         //AI(ht9045-heater-control) 20260618 (RogerYang) : 移植創發2type溫控模組設定
#define EN_HEATER_SHEET                 1                                       //0=use global TC401HeaterControl ; 1=use per-channel g_tHeaterInsInfo
#endif
#ifndef eHeaterType_Count
#define eHeaterType_Count               tcTotalCount
#define eHeaterInsOpt_Count             5                                       //TC401/KT4H/E5DC/NoHeater/DTK4848
#endif

extern bool IsValEqual_HeaterInsOpt(int iHeaterTypeIdx, int iHeaterInsOpt_Cmp);
extern bool IsAllValEqual_HeaterInsOpt(int iHeaterInsOpt_Cmp, bool bJustForShowItem, int *piHeaterInsOpt_AllSame = NULL);
extern int  GetFirstHeaterInsOpt(bool bJustForShowItem);
extern bool IsAllSame_HeaterInsOpt(bool bJustForShowItem, int *piFirstHeaterInsOpt = NULL);
extern bool GetStaTable_HeaterInsOpt(int *pStaTable, bool bJustForShowItem);
extern bool IsNoHeaterMachine();                                                //RogerYang 20260630 : Add NoHeater 判斷
#if EN_HEATER_SHEET
#define DEFAULT_HEATER_INS_OPT          KT4H    //legacy int TC401HeaterControl=1;
extern bool IsExistVal_HeaterInsOpt(int iHeaterInsOpt_Cmp, bool bJustForShowItem);
extern bool GetCtrlItemVisProp(int iHeaterTypeIdx);
extern bool g_bGetStaTable_HeaterInsOpt_Already;
extern int  g_StaTable_HeaterInsOpt[eHeaterType_Count];
extern AnsiString g_HeaterInsOptStr[eHeaterInsOpt_Count];

typedef struct T_HeaterInsInfo
{
    bool       m_bOccupy;
    AnsiString m_asShowName;
    AnsiString m_asSaveName;
    int        m_iHeaterInsOpt;
    bool       m_bShow;
    TLabel    *m_pCtrlItem_Lb;
    TComboBox *m_pCtrlItem_Cb;
    T_HeaterInsInfo()
    {
        m_bOccupy=false; m_asShowName=""; m_asSaveName="";
        m_iHeaterInsOpt=DEFAULT_HEATER_INS_OPT;   //fix: ref left this uninitialized
        m_bShow=false; m_pCtrlItem_Lb=NULL; m_pCtrlItem_Cb=NULL;
    }
    bool       GetOccupy(){ return m_bOccupy; }
    AnsiString GetShowName(){ return m_asShowName; }
    AnsiString GetSaveName(){ return m_asSaveName; }
    int        GetHeaterInsOpt(){ return m_iHeaterInsOpt; }
    bool       GetShow(){ return m_bShow; }
    TLabel    *GetCtrlItem_Lb(){ return m_pCtrlItem_Lb; }
    void       SetCtrlItem_Lb(TLabel *pLb){ m_pCtrlItem_Lb=pLb; }
    TComboBox *GetCtrlItem_Cb(){ return m_pCtrlItem_Cb; }
    void       SetCtrlItem_Cb(TComboBox *pCb){ m_pCtrlItem_Cb=pCb; }
    T_HeaterInsInfo(bool bOccupy, AnsiString asShowName, AnsiString asSaveName, int iHeaterInsOpt)
    {   m_bOccupy=bOccupy; m_asShowName=asShowName; m_asSaveName=asSaveName; m_iHeaterInsOpt=iHeaterInsOpt;
        m_bShow=false; m_pCtrlItem_Lb=NULL; m_pCtrlItem_Cb=NULL; }
    void SetCtrlItemProp(bool bShow, TLabel *pCtrlItem_Lb, TComboBox *pCtrlItem_Cb)
    {   m_bShow=bShow; m_pCtrlItem_Lb=pCtrlItem_Lb; m_pCtrlItem_Cb=pCtrlItem_Cb; SetCtrlItemVis(m_bShow); }
    void SetCtrlItemVis(bool bShow)
    {
        if(NULL!=m_pCtrlItem_Lb) m_pCtrlItem_Lb->Visible=bShow;
        if(NULL!=m_pCtrlItem_Cb) m_pCtrlItem_Cb->Visible=bShow;
    }
}THeaterInsInfo;
extern THeaterInsInfo g_tHeaterInsInfo[eHeaterType_Count];

// golden HandlerSys.cpp:19-31（THandlerSystem 建構子動態建立逐通道 Label／ComboBox 的版面；C 路只給頁面排版用）
#define WIDTH_OF_LB_HEATER              100
#define HEIGHT_OF_LB_HEATER              28
#define WIDTH_OF_CB_HEATER              170
#define HEIGHT_OF_CB_HEATER              28
#define TOP_OF_LB                        12
#define TOP_OF_CB                        12
#define LEFT_OF_1ST                      12
#define GAP_OF_HOR_IN_SET                10
#define GAP_OF_HOR_OUT_SET               30
#define GAP_OF_VER                       10
#define COUNT_OF_ITEM_IN_COL             10

// golden HandlerSys.cpp:32-34, :38-49（本體 FileRW/HSys.cpp）
extern std::vector<int>   g_vecHeaterTypeIdxForShow;
extern std::map<int, int> g_mapHeaterTypeIdxToHeaterShowIdx;
extern std::map<int, int> g_mapHeaterShowIdxToHeaterTypeIdx;
int TypeIdxToShowIdx(int iTypeIdx);
int ShowIdxToTypeIdx(int iShowIdx);
#endif  // EN_HEATER_SHEET

// golden HandlerSys.h:14（本體 golden HandlerSys.cpp:50-60 → FileRW/HSys.cpp）
//   //AI(W906-FRW-S166) 20260927 [W906] 偏離 golden：本體改成 Q34 方案 D＋Q15 的不寫檔讀法（W906_HeaterMixReadFile，見檔尾），
//   golden 原文（缺鍵補寫 -9999）留在 FileRW/HSys.cpp 的 #if 0。
extern void HeaterInsOpt_Read();                                               //AI(ht9045-heater-control) 20260618 (RogerYang) : 移植創發2type溫控模組設定

// //AI(W906-FRW-P8) 20260926: 移植樹專用（golden 沒有）——
//   W906_HeaterInsCreateProxies()：golden THandlerSystem 建構子 :70-112 的 C 路版（new TLabel／TComboBox → 具名替身
//     lb<m_asSaveName>／cb<m_asSaveName>，Parent＝grpHeater 改成父子表）。產生檔的建構子取代段呼叫它。
//   W906_HeaterInsProxyName()：替身名稱（＝HTML 元件 id）。
void       W906_HeaterInsCreateProxies();
AnsiString W906_HeaterInsProxyName(int iTypeIdx, bool bCombo);

// ===========================================================================
// //AI(W906-FRW-S166) 20260927 [W906] Q34 方案 D（全機相同／各溫控器不同＋站號）＋ Q15（缺鍵＝3 No Heater、開頁不寫檔）。
//   裁決：RULINGS_20260926 S166（Steven 20260927「開工，其他照建議」）；D-2＝A 依 RULINGS_20260927 第 7 條第 35 題；
//   ⑥ 第 4 點（No Heater 通道讓迴圈跳過）不做（RULINGS_20260927 第 7 條第 34 題：照 golden）。全文：
//   D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md「### Q34.」。
//   golden V912 沒有這些鍵與全域（偏離 golden，裁決過）；本體 FileRW/HSys.cpp 第 (4) 段。
//
//   檔案 system\Gerneral.ini [TempCtrl]（golden 原有的 HEATER_CTRL_TYPE／71 個 HeaterInsOpt_<通道> 照留）：
//     HeaterInsMode       0＝全機相同、1＝各溫控器不同
//     HeaterInsIndexOpt   「全機相同」時 Index 位置（D-2a：Head1～4＋Index 32 區，36 個通道）的廠牌 0～4
//     HeaterInsOtherOpt   「全機相同」時其他位置的廠牌 0～4
//     HeaterInsAddr_<通道> 「各溫控器不同」時的站號（1 起算；缺鍵＝golden 算的預設站號）
//   讀（HeaterInsOpt_Read → W906_HeaterMixReadFile，不寫檔）：
//     模式＝HeaterInsMode；缺鍵照 D-1b 推斷（有下拉的 23 個通道裡 0～4 的值彼此不同＝不同，-9999 先換成 HEATER_CTRL_TYPE）。
//     相同：通道廠牌＝Index 組用 IndexOpt、其他用 OtherOpt（71 個 HeaterInsOpt_ 只是 V912 的相容副本）。
//     不同：通道廠牌＝自己的 HeaterInsOpt_<通道>；缺鍵＝3 No Heater（Q15）；-9999＝HEATER_CTRL_TYPE（D-5a）。
//   存（SaveSystemSet 答 YES 之後，W906_HeaterMixSave）：模式三鍵＋71 鍵一律寫「實際廠牌」（D-4a，不寫 -9999）＋
//     「不同」模式列出的通道寫站號（D-7a）＋ HEATER_CTRL_TYPE（D-6a）。存檔前 W906_HeaterMixSaveCheck 擋站號範圍（D-7a）與
//     同一個溫控 COM 埠同廠牌同站號（D-8a）。
//     //AI(W906-FRW-S166) 20260928 [W906] R113（Steven 20260928「要擋」）：同一個溫控 COM 埠上不同廠牌同站號也擋，所有廠牌都算
//     （TC401 的站號＝第幾台）；「全機相同」模式用預設站號一起檢查。本體 FileRW/HSys.cpp HmDuplicates。
//   給溫控底層（Jimmy，⑥ 1～3、5～8）：第一次跑溫控時照 golden bthermo.cpp:1167-1175 呼叫 HeaterInsOpt_Read()（已是不寫檔的讀法）、
//     廠牌照舊 IsValEqual_HeaterInsOpt(Addr, X)、站號用 W906_HeaterStationIdx(Addr)（見下）。
// ===========================================================================
#define HEATER_INS_MODE_SAME            0
#define HEATER_INS_MODE_DIFF            1
#define HEATER_INS_ADDR_MIN             1       // D-7a：站號 1～247（Modbus 位址範圍）
#define HEATER_INS_ADDR_MAX           247
#define HEATER_INS_ADDR_MAX_E5DC       99       // D-7a：E5DC 用十進位兩位數組命令（golden V912 cpublic.cpp:467、:484）
extern int g_iHeaterInsMode;                             // HeaterInsMode（記憶體；讀檔／存檔／按 Heater Type 時更新）
extern int g_iHeaterInsIndexOpt;                         // HeaterInsIndexOpt
extern int g_iHeaterInsOtherOpt;                         // HeaterInsOtherOpt
extern int g_iHeaterInsAddr[eHeaterType_Count];          // HeaterInsAddr_<通道> 的檔案值；0＝沒有這個鍵（＝預設站號）
bool       W906_HeaterInsIsIndexGroup(int iTypeIdx);     // D-2a：Head1～4、Aa1～Bd2、Ae1～Bh2（36 個）
bool       W906_HeaterInsHasRow(int iTypeIdx);           // 頁面逐通道表可能列的通道（有下拉的 23 個＋Index 32 區＝55 個；有替身）
bool       W906_HeaterInsListed(int iTypeIdx);           // D-3a：「不同」模式列出的通道（golden 可見條件＋依 USE_16_HEATER 走溫控 COM 埠的 Index 區）
bool       W906_HeaterInsIndexLocked();                  // USE_16_HEATER 是 EJ1N／DTME08：Index 區不走溫控 COM 埠（golden V912 bthermo.cpp:1331-1367），Index 下拉停用
int        W906_HeaterInsDefaultStation(int iTypeIdx, int iHeaterInsOpt);   // golden 算的站號：TC401＝序號÷4＋1（台），其他＝序號＋1
// 溫控底層用：這個通道現在要送的「站號−1」（＝移植樹 cpublic.cpp 通訊函式收的 Addr 參數，函式裡再 +1）。
//   「不同」模式且 HeaterInsAddr_<通道> 在範圍內 → 那個站號−1；否則預設 → KT4H／DTK4848／E5DC 回 iTypeIdx（跟今天一樣）、
//   TC401 回 iTypeIdx/4（呼叫端照 golden bthermo.cpp:2522／:2556 傳 (W906_HeaterStationIdx(Addr), Addr%4)，通道仍是序號%4，D-7a）。
int        W906_HeaterStationIdx(int iTypeIdx);
void       W906_HeaterMixReadFile();                     // HeaterInsOpt_Read 的本體（不寫檔，Q15）
void       W906_HeaterMixLoaderSystemSet();              // golden LoaderSystemSet :262-278 的取代（開頁：讀檔＋回填替身）
void       W906_HeaterMixTypeClick(int iOpt);            // golden rgHeaterTypeClick :1529-1538 的取代（相同模式、Index＝其他＝iOpt；不寫檔，Q14＝B）
bool       W906_HeaterMixSaveCheck();                    // 存檔前檢查（D-7a／D-8a）；false＝擋下（ELMessage 說明），什麼都不寫
void       W906_HeaterMixSave();                         // golden SaveSystemSet :779-794 的取代（寫檔）
