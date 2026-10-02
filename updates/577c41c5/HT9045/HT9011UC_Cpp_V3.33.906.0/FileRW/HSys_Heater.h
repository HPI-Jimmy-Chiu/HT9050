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
//     HeaterInsIndexOpt   「全機相同」時 Index 位置（D-2a：Head1～4＋Index 32 區，36 個通道）的廠牌 0～4（E029 起 0～6，見檔尾）
//     HeaterInsOtherOpt   「全機相同」時其他位置的廠牌 0～4（E029：仍只有 golden 0～4，EJ1N／DTM 只在 Index，Steven 1002 17:1x）
//     HeaterInsAddr_<通道> 「各溫控器不同」時的站號（1 起算；缺鍵＝golden 算的預設站號）（E029：EJ1N＝台號、DTM＝內部站號 0 起算）
//     HeaterInsCh_<通道>   //AI(W906-FRW-E029) 20261002：EJ1N／DTM 的 CH（1 起算），見檔尾
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
extern int g_iHeaterInsAddr[eHeaterType_Count];          // HeaterInsAddr_<通道> 的檔案值；INVALID_INT_VAL_NEG＝沒有這個鍵（＝預設站號）
                                                         //   //AI(W906-FRW-E029) 20261002 [W906]：以前 0＝沒有鍵；Delta DTM 的內部站號 0（DTME08 主機）是合法值，改用 -9999
bool       W906_HeaterInsIsIndexGroup(int iTypeIdx);     // D-2a：Head1～4、Aa1～Bd2、Ae1～Bh2（36 個）
bool       W906_HeaterInsHasRow(int iTypeIdx);           // 頁面逐通道表的列（有替身）。//AI(W906-FRW-E029) 20261002 [W906]：71 個全部（以前 55 個，見檔尾 E029）
bool       W906_HeaterInsListed(int iTypeIdx);           // D-3a：golden 溫控 COM 埠的輪詢範圍（golden 可見條件＋USE_16_HEATER 是 1／4 時的 Index 區）。
                                                         //   ⛔ 20261002 更正（E029）：頁面已不靠它決定列不列（71 個全列）；存檔檢查改看「這台有裝」＋各匯流排（檔尾 E029）
//   ⛔ 20261002（E029）：W906_HeaterInsIndexLocked()（EJ1N／DTME08 時停用 Index 下拉）拿掉——Steven 1002 17:1x：Index 下拉自己選 EJ1N／DTM
int        W906_HeaterInsDefaultStation(int iTypeIdx, int iHeaterInsOpt);   // golden 算的站號：TC401＝序號÷4＋1（台），其他＝序號＋1
// 溫控底層用：這個通道現在要送的「站號−1」（＝移植樹 cpublic.cpp 通訊函式收的 Addr 參數，函式裡再 +1）。
//   「不同」模式且 HeaterInsAddr_<通道> 在範圍內 → 那個站號−1；否則預設 → KT4H／DTK4848／E5DC 回 iTypeIdx（跟今天一樣）、
//   TC401 回 iTypeIdx/4（呼叫端照 golden bthermo.cpp:2522／:2556 傳 (W906_HeaterStationIdx(Addr), Addr%4)，通道仍是序號%4，D-7a）。
//   //AI(W906-FRW-E029) 20261002：只給溫控 COM 埠的廠牌；EJ1N／DTM／No Heater 回 iTypeIdx（不用），EJ1N／DTM 用檔尾 W906_HeaterInsUnitCh。
int        W906_HeaterStationIdx(int iTypeIdx);
void       W906_HeaterMixReadFile();                     // HeaterInsOpt_Read 的本體（不寫檔，Q15）
void       W906_HeaterMixLoaderSystemSet();              // golden LoaderSystemSet :262-278 的取代（開頁：讀檔＋回填替身）
void       W906_HeaterMixTypeClick(int iOpt);            // golden rgHeaterTypeClick :1529-1538 的取代（相同模式、Index＝其他＝iOpt；不寫檔，Q14＝B）
bool       W906_HeaterMixSaveCheck();                    // 存檔前檢查（D-7a／D-8a）；false＝擋下（ELMessage 說明），什麼都不寫
                                                         //   //AI(W906-FRW-E029) 20261002：先做 Index 區 EJ1N／DTM 與 USE_16_HEATER 的連動（會改 rgHeater 替身）
void       W906_HeaterMixSave();                         // golden SaveSystemSet :779-794 的取代（寫檔）

// ===========================================================================
// //AI(W906-FRW-E029) 20261002 [W906] 偏離 golden（Steven 裁決過）：逐通道廠牌加 Omron EJ1N 與 Delta DTM、71 個通道全部可選、
//   Index 區的 EJ1N／DTM 跟 [System] USE_16_HEATER（Index Heater Counts）自動連動（todo E-029、Q72）。Steven 1002 原話：
//   16:4x「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」；指著 golden V912 MachineType.h:638-655（71 個 eTempControll；
//   ＝906 MachineType.h:632 enum eTempControll，71 個名字相同，Jimmy RULINGS_20261002 #20）
//   「這些全部都要可以選,所以數量也是少了」；16:5x「使用 EJ1N 或是 DTM 應該是要設定兩個變數」。Steven 答 ST01-E 的兩題：
//   (1) 同一個鍵、新代碼：[TempCtrl] HeaterInsOpt_<通道>＝5（EJ1N）／6（DTM）。golden V912 只有 0～4（eHeaterInsOpt_Count 5，
//       MachineType.h:660；顯示字 MachineTypeUtility.cpp:19 g_HeaterInsOptStr）。
//       ⚠ 已接受的風險（Steven 1002）：同一台之後若跑 BCB V912，5／6 的通道 golden 認不得——golden 的廠牌分派沒有 else
//       （bthermo.cpp:2520-2598，ht9045-heater-control §3.1）：裝了的通道溫控迴圈會停在那一通道，後面的通道都不再輪詢。
//       Index 區在 USE_16_HEATER＝EJ1N／DTME08 時 golden 先跳過（bthermo.cpp:1331-1367），不受影響。
//   (2) 自動連動 Index Heater Counts（rgHeater＝USE_16_HEATER：0 4 Heaters、1 16 KT4H（COM）、2 16 EJ1N、3 32 EJ1N、4 32 KT4H、
//       5 16 DTME08、6 32 DTME08；golden HandlerSys.dfm:3055-3069、MachineType.h:717-723）——兩個變數同一次存檔一起寫：
//       * Index 區＝Aa1～Bd2（序號 11～26），32 組時加 Ae1～Bh2（33～48）。Index 區選 EJ1N／DTM → USE_16_HEATER 換成同組數的
//         EJ1N／DTME08（16→2／5、32→3／6；0＝4 Heaters→16）；改回溫控 COM 埠的廠牌 → 1（16）／4（32）。
//       * Index Heater Counts 選 EJ1N／DTME08 → Heater 分頁的 Index 區顯示 EJ1N／DTM；選回 1／4 → Index 區原本是 EJ1N／DTM 的
//         通道改成 KT4H（golden eht16Heater／eht32HeaterKT4H＝KT4H 版）。
//       * golden 的 USE_16_HEATER 是一個選項、整個 Index 區一起（bthermo 一起跳過那些通道），所以 Index 區任一通道選 EJ1N／DTM
//         ＝整個 Index 區都是；Head1～4 照 golden 留在溫控 COM 埠（全機相同模式：Index 選 EJ1N／DTM 時 Head1～4 跟著「其他位置」）。
//       * 讀檔時以 USE_16_HEATER 為準（golden 真正看的是它）；存檔時誰改了跟誰（開頁時的 USE_16_HEATER 記在 W906_HeaterMixLoaderSystemSet），
//         兩邊都改而且對不上 → 整頁不存（W906_HeaterMixSaveCheck）。頁面（ht9045_hsys_heater_c.js）即時連動，正常不會走到伺服器的補救。
//   golden 的 5 個（g_HeaterInsOptStr／eHeaterInsOpt_Count／IsValEqual_HeaterInsOpt…）原樣不動：記憶體裡的 5／6 對那幾支一律
//   「不等於 0～4 的任何一個」，跟 golden 讀到認不得的值一樣（不會被當成某個 COM 廠牌）。HEATER_CTRL_TYPE 永遠不寫 5／6（D-6a 只算
//   0、1、2、4；沒有就沿用檔案值，3 也不寫——golden main.cpp:10939-10961 EJ1N／DTME08 要 HEATER_CTRL_TYPE≠NoHeater 才啟動）。
//   ⚠ 底層：移植樹溫控（bthermo／rs232／OmronEJ1N／fDTME08，Jimmy 的檔）翻完之前仍只看 HEATER_CTRL_TYPE；逐通道的 EJ1N／DTM
//     要等底層會讀 5／6 與下面的台號／CH 才生效（頁面與存檔 ack 照 D-9a 提示）。
//   站號（台號／站＋CH）：
//     EJ1N：台號＝TC4 的 SW1（1～F，SW2 pin 1、2 OFF；D:\HT9045\.claude\skills\ht9045-temperature\references\controllers\omron-ej1n.md
//       §1.3、§3.1、§5.1；golden 只用 1～8），CH 1～4（TC4）。走自己的 [TempCtrl] COM_PORT_OMRON（golden database.cpp:523，預設 COM7）。
//     DTM：內部站號 0～3（0＝DTME08 主機、1～3＝DTMN08 旋鈕；golden GetMaxStationNumber()=4，controllers\delta-dtm.md §1.3），CH 1～8。
//       走 Ethernet（DTME08_Control.ini [SocketSetting] asAddress／asPort；golden 讀 exe 資料夾，移植樹讀 system\，todo D-035）。
//     檔案：HeaterInsAddr_<通道>＝台號／內部站號、HeaterInsCh_<通道>＝CH（新鍵，只有 EJ1N／DTM 用）。空白＝預設：Index 區照 golden
//       的接線（iTempCode 第 p 個：EJ1N 第 p/4+1 台 CH p%4+1，golden bthermo.cpp:3903-3918／:3943-3958；DTM 站 p/8 CH p%8+1，
//       cmydef.cpp:111-117、fDTME08.cpp:266-270）；Index 區以外 golden 沒有對照 ⇒ 沒有預設，「各溫控器不同」裝了的通道要填。
//     D-8a：溫控 COM 埠、EJ1N（COM_PORT_OMRON）、DTM（Ethernet）各算各的匯流排：EJ1N 同台同 CH、DTM 同站同 CH 擋下；
//       R113 的「不同廠牌同站號」只在溫控 COM 埠上算（EJ1N／DTM 不在那條埠上）。
//   Steven 1002 17:1x 細則（經 ST01-E2 問過、每題選建議；decisions-decided Q73～Q76）：「我的認知是: 當選擇全機相同, 那就是 index 跟其他部位
//   的分成兩種溫控器進行選擇 EJ1N 跟 DTM 在index站都是可以選的」「當選擇全機不同單獨設定, 那就是每個位置要可以單獨設定, 包含使用 EJ1N跟DTM」
//   「index區裡面, Head 1234 跟 Ax Bx這32組屬於互斥的, 也就是同時間只會顯示其中的一種」「socket跟 Dut 1~4也是互斥的」——
//     * 全機相同：EJ1N／DTM 只在「Index 位置溫控器」（7 個）；「其他位置溫控器」只有 golden 5 個（替身 5 項）。各溫控器不同：每個通道 7 個。
//     * Head1～4⇄Ax／Bx 照 rgHeater（4 Heaters＝Head1～4、16 組＝Aa1～Bd2、32 組＝Aa1～Bh2）——跟 golden V912 MachineTypeUtility.cpp:241-293
//       GetCtrlItemVisProp 相反（golden 16／32 組才顯示 Head1～4），刻意偏離。Socket⇄DUT1～4 照 rgUse4DUT（1 EA＝Socket、2 EA＝DUT1～2、
//       4 EA＝DUT1～4；golden :268 註解掉的 Socket 條件打開）。這兩組跟著頁面上的值即時變（golden 只開機算一次）；本體 FileRW/HSys.cpp HmActive。
//   golden 對照（Jimmy RULINGS_20261002 #20：golden＝906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260625_Steven，V912 只拿來查 906 漏了什麼）：
//     * 逐通道廠牌（HeaterInsOpt_、g_tHeaterInsInfo、GetCtrlItemVisProp、g_HeaterInsOptStr、EN_HEATER_SHEET、eHeaterInsOpt_Count）是 **V912 才有的功能**
//       （906_0625 沒有 MachineTypeUtility.cpp、沒有 EN_HEATER_SHEET）；本檔照 Steven Q34／Q71～Q76 當 St01 的設計留著，等 Jimmy 定
//       （RULINGS_20261002 #20、NIGHT_REPORT s0 #63）。上面引 V912 的這幾處照留。
//     * 其他引用 906 與 V912 相同（20261002 對過 906_0625）：eTempControll 906 MachineType.h:632（V912 :638）、eSocketTempControll 906 :616-618
//       （V912 :742-744）、eHeaterType 906 :651-657（V912 :717-723）；rgHeater／rgUse4DUT 的 DFM 選項 906 HandlerSys.dfm:3029／:2387（字一樣）；
//       讀寫 906 HandlerSys.cpp:162／:651（USE_16_HEATER）、:339／:819（SocketBasedAdd4Temp）；Index 區跳過溫控 COM 埠 906 bthermo.cpp:1155-1190
//       （V912 :1331-1367）；EJ1N／DTME08 啟動條件 906 main.cpp:10503-10526（V912 :10939-10961）；COM_PORT_OMRON 906 database.cpp:521（V912 :523）；
//       iTempCode 906 cmydef.cpp:111-117（同）；DTME08_Control.ini 讀 exe 資料夾 906 EJ1N/uDTME08Control.h:54（同）。沒有找到不同的地方。
// ===========================================================================
#define W906_HEATER_INS_EJ1N            5       // Omron EJ1N（[TempCtrl] HeaterInsOpt_<通道> 的新代碼）
#define W906_HEATER_INS_DTM             6       // Delta DTM（DTME08／DTMN08）
#define W906_HEATER_INS_OPT_COUNT       7       // golden 5 個＋EJ1N＋DTM（golden eHeaterInsOpt_Count 不動）
extern AnsiString g_W906HeaterInsOptStr[W906_HEATER_INS_OPT_COUNT];   // 頁面下拉的 7 個字（前 5 個＝golden g_HeaterInsOptStr）
#define HEATER_INS_EJ1N_UNIT_MIN        1       // EJ1N 台號＝SW1
#define HEATER_INS_EJ1N_UNIT_MAX       15
#define HEATER_INS_EJ1N_CH_MAX          4       // TC4：CH1～4
#define HEATER_INS_DTM_STATION_MIN      0       // DTM 內部站號：0＝DTME08 主機
#define HEATER_INS_DTM_STATION_MAX      3       // golden 只掃 0～3（uDTME08Control.h:107 GetMaxStationNumber()=4）
#define HEATER_INS_DTM_CH_MAX           8
extern int g_iHeaterInsCh[eHeaterType_Count];            // HeaterInsCh_<通道> 的檔案值（EJ1N／DTM 的 CH，1 起算）；INVALID_INT_VAL_NEG＝沒有這個鍵
#define W906_HEATER_BUS_NONE            0       // No Heater／不合法
#define W906_HEATER_BUS_COM             1       // [TempCtrl] COM_PORT（TC401、KT4H、E5DC、DTK4848）
#define W906_HEATER_BUS_EJ1N            2       // [TempCtrl] COM_PORT_OMRON
#define W906_HEATER_BUS_DTM             3       // Ethernet（DTME08_Control.ini）
int  W906_HeaterInsBusOf(int iHeaterInsOpt);             // 廠牌 → 匯流排
int  W906_HeaterInsBus(int iTypeIdx);                    // 通道現在（記憶體）的廠牌走哪條匯流排
// 給溫控底層（Jimmy）：EJ1N／DTM 通道的位址。EJ1N：*piUnit＝台號（1 起算，＝送出的站號）、*piCh＝CH（1 起算）；DTM：*piUnit＝
//   內部站號（0 起算，Modbus 起始位址＝站×0x1000）、*piCh＝CH（1 起算）。「各溫控器不同」且檔案有台號＋CH → 那個；否則 Index 區的
//   golden 預設；Index 區以外沒有預設 → false。不是 EJ1N／DTM 的通道 → false（COM 埠廠牌照舊用 W906_HeaterStationIdx）。
bool W906_HeaterInsUnitCh(int iTypeIdx, int* piUnit, int* piCh);
int  W906_HeaterInsU16Area(int iU16);                    // USE_16_HEATER 的 Index 區控制器：2／3→5 EJ1N、5／6→6 DTM、其他→-1（溫控 COM 埠）
int  W906_HeaterInsU16For(int iIndexAreaOpt, int iU16Now);   // 連動：Index 區選 iIndexAreaOpt（5／6／其他＝COM 廠牌）時 USE_16_HEATER 該是多少
