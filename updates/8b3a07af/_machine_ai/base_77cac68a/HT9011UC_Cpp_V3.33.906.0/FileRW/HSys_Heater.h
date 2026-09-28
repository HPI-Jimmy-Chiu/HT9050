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
extern void HeaterInsOpt_Read();                                                //AI(ht9045-heater-control) 20260618 (RogerYang) : 移植創發2type溫控模組設定

// //AI(W906-FRW-P8) 20260926: 移植樹專用（golden 沒有）——
//   W906_HeaterInsCreateProxies()：golden THandlerSystem 建構子 :70-112 的 C 路版（new TLabel／TComboBox → 具名替身
//     lb<m_asSaveName>／cb<m_asSaveName>，Parent＝grpHeater 改成父子表）。產生檔的建構子取代段呼叫它。
//   W906_HeaterInsProxyName()：替身名稱（＝HTML 元件 id）。
void       W906_HeaterInsCreateProxies();
AnsiString W906_HeaterInsProxyName(int iTypeIdx, bool bCombo);
